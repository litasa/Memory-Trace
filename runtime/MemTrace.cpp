/*
Copyright (c) 2015, Insomniac Games All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MemTrace.h"
#include "MemTraceSys.h"
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(MEMTRACE_WINDOWS)
#include <psapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if MEMTRACE_ENABLE

#if defined(MEMTRACE_WINDOWS)
#pragma comment(lib, "ws2_32")
#endif

static size_t total_sent = 0;
static size_t count = 0;

// We can't use Printf/printf() in general because they're not initialized yet.
// Vsnprintf() is OK because it doesn't allocate.
static void MemTracePrint(const char* fmt, ...)
{
  using namespace MemTrace;

  char msg[1024];
  va_list args;
  va_start(args, fmt);
  Vsnprintf(msg, sizeof msg, fmt, args);
  va_end(args);
  OutputDebugStringA(msg);
}

namespace MemTrace
{
  //-----------------------------------------------------------------------------
  // Various limits
  enum
  {
    kBufferSize     = 17520
  };

  // Start of stream protocol value - to handle version changes without crashing decoder.
  static const uint32_t kStreamMagic = 0xbfaf0003;

  //-----------------------------------------------------------------------------
  // Event codes that can be sent to the output stream.
  enum EventCode : uint8_t
  {
    kBeginStream     = 1,
    kEndStream,

    kHeapCreate = 10,
    kHeapDestroy,

    kHeapAddCore,
    kHeapRemoveCore,

    kHeapAllocate,
    kHeapFree,

	kEventStart = 20,
	kEventEnd,

	//Stingray specifics from here on
	kSetBackingAllocator = 30,
  };

  //-----------------------------------------------------------------------------
  // Type of callback to transmit an encoded block of event data to output stream
  typedef void (TransmitBlockFn)(const void* block, size_t size_bytes);
  typedef void (ListeningFn)(const void* block, size_t size_bytes);

  //-----------------------------------------------------------------------------
  // Local functions.

  // Common init routine.
  static void InitCommon(TransmitBlockFn* transFn, ListeningFn* listFn);
  // Panic shutdown
  static void ErrorShutdown();

  //-----------------------------------------------------------------------------
  // Encodes integers and strings using variable-length encoding and windowing
  // to compress the outgoing data.
  class Encoder
  {
  public:
	  void RecieveMessage() {
		  m_ListenFn(m_Buffers[m_CurBuffer ^ 1], kBufferSize);
	  }
  private:
    size_t                        m_WriteOffset;                    // Write offset within current buffer
    TransmitBlockFn*              m_TransmitFn;                     // Function to transmit (partially) filled blocks
	ListeningFn*			      m_ListenFn;
    uint64_t                      m_StartTime;                      // System timer for initial event. We use a delta to generate smaller numbers.

    int                           m_CurBuffer;                      // Index of current encoding buffer
    uint8_t                       m_Buffers[2][kBufferSize];        // Raw encoding buffers

  private:
    //-----------------------------------------------------------------------------
    // Flush current buffer and flip buffers.
    void TransmitCurrentBuffer()
    {
      (*m_TransmitFn)(m_Buffers[m_CurBuffer], m_WriteOffset);

      // Flip buffers.
      m_WriteOffset = 0;
      m_CurBuffer  ^= 1;
	  RecieveMessage();
    }

    //-----------------------------------------------------------------------------
    // Reserve space for <size> bytes. Can cause a buffer flipflush.
    uint8_t* Reserve(size_t size)
    {
      ASSERT_FATAL(size < kBufferSize, "Block size too small for reservation");
      uint8_t *base = m_Buffers[m_CurBuffer];

      if (size + m_WriteOffset > kBufferSize)
      {
        TransmitCurrentBuffer();

        base          = m_Buffers[m_CurBuffer];
      }

      return base + m_WriteOffset;
    }

    //-----------------------------------------------------------------------------
    // Commit <size> bytes. At least <size> bytes must have been previously reserved via Reserve()
    void Commit(size_t size)
    {
      m_WriteOffset += size;
    }

    //-----------------------------------------------------------------------------
    // Emit a relative time stamp to the stream.
    uint64_t EmitTimeStamp()
    {
      uint64_t t = TimerGetSystemCounter();
      uint64_t delta = t - m_StartTime;
      EmitUnsigned(delta);
	  return delta;
    }

  public:
    //-----------------------------------------------------------------------------
    // Initialize the encoder with a function that writes encoded blocks to some output device.
    void Init(TransmitBlockFn *transmit_fn, ListeningFn* listen_fn)
    {
      m_WriteOffset  = 0;
      m_TransmitFn   = transmit_fn;
	  m_ListenFn	 = listen_fn;
      m_StartTime    = TimerGetSystemCounter();
    }

    //-----------------------------------------------------------------------------
    // Swap out the transmit function (for file->socket switcharoo)
    void SetTransmitFn(TransmitBlockFn* fn, ListeningFn* listen_fn)
    {
      m_TransmitFn = fn;
	  m_ListenFn = listen_fn;
    }

    //-----------------------------------------------------------------------------
    // Flush any pending data to the output device.
    void Flush()
    {
      TransmitCurrentBuffer();
      // Immediately sync async write.
      (*m_TransmitFn)(NULL, 0);
    }

  public:
    //-----------------------------------------------------------------------------
    // Encode a 64-bit integer to the output stream using an encoding that favors small numbers.
    //
    // Encoding used:
    // - There is 7 bits of payload per byte, leading to a worst case of 10 bytes to store a 64-bit number
    // - Decoding proceeds as long as the MSB is zero.
    void EmitUnsigned(uint64_t val)
    {
      uint8_t* out  = Reserve(10);
      uint8_t byte;
      size_t   i    = 0;

      do
      {
		uint64_t var = (val & 0x7f);
        byte     = (uint8_t) (var);
        out[i++] = byte;
        val    >>= 7;
      } while (val);
	  uint8_t temp = byte | 0x80;
	  out[i - 1] = temp;

      Commit(i);
    }

    //-----------------------------------------------------------------------------
    // Emit a pointer to the output stream.
    void EmitPointer(const void* ptr)
    {
      EmitUnsigned(uintptr_t(ptr));
    }

    //-----------------------------------------------------------------------------
    // Emit a string to the output stream.
    void EmitString(const char* str)
    {
      if (!str)
        str = "(null)";

      const size_t   len   = strlen(str);
      EmitUnsigned(len);
      memcpy(Reserve(len), str, len);
      Commit(len);
    }

    //-----------------------------------------------------------------------------
    // Emit common data that goes with every event.  
    void BeginEvent(EventCode code)
	{
	  EmitUnsigned(count);
      EmitUnsigned(code);
      uint64_t delta = EmitTimeStamp();
	  EmitUnsigned(GetCurrentThreadId());
    }
	void EndEvent(EventCode code)
	{
		count++;
	}

  };

  //-----------------------------------------------------------------------------
  // Subsystem global state
  struct
  {
    bool            m_Active;                     // Non-zero if the system is active
    Encoder         m_Encoder;                    // Encoder
    uint32_t        m_NextHeapId;                 // Next free heap ID
    CriticalSection m_Lock;                       // Synchronizes access to encoder/stream

    FileHandle      m_BootFile;
    SOCKET          m_Socket;                     // Output socket during normal operation
    char            m_BootFileName[128];

	bool			m_Paused = false;

  } State;
}

//-----------------------------------------------------------------------------
// Dummy to force initialization object to exist in top-level executable
  void MemTrace::DummyInitFunction(char)
  {
  }

//-----------------------------------------------------------------------------
// Common init routine for first time setup

static void MemTrace::InitCommon(TransmitBlockFn* write_block_fn, ListeningFn* listening_fn)
{
  if (State.m_Active)
  {
#if defined(MEMTRACE_WINDOWS)
    DebugBreak();
#else
    abort();
#endif
  }

  State.m_Active     = true;
  State.m_Socket     = INVALID_SOCKET;

  State.m_Lock.Init();
  State.m_Encoder.Init(write_block_fn, listening_fn);

  BeginStream();
}

//-----------------------------------------------------------------------------
void MemTrace::InitFile(const char* trace_temp_file)
{
  FileHandle hf = FileOpenForReadWrite(trace_temp_file);

  if (hf == kInvalidFileHandle)
  {
    MemTracePrint("MemTrace: Failed to open %s for writing, disabling system\n", trace_temp_file);
    return;
  }

  // Stash the boot filename so we can delete it later.
  Strcpy(State.m_BootFileName, ARRAY_SIZE(State.m_BootFileName), trace_temp_file);

  State.m_BootFile = hf;

  // Callback that dumps event buffer data using async writes to our socket.
  auto write_block_fn = [](const void* block, size_t size) -> void
  {
    FileWrite(State.m_BootFile, block, size);
  };

  InitCommon(write_block_fn, nullptr);
}

//-----------------------------------------------------------------------------
void MemTrace::InitSocket(const char *server_ip_address, int server_port)
{
  bool error = false;

  // Remember if we were already active; if we were we need to protect against memory allocations
  // on other threads trying to trace while we're switching protocols.
  const bool was_active = State.m_Active;

  if (was_active)
  {
    State.m_Lock.Enter();
  }

#if defined(MEMTRACE_WINDOWS)
  WSADATA wsa_data;
  WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

  sockaddr_in address;
  memset(&address, 0, sizeof address);

#if defined(MEMTRACE_WINDOWS)
  InetPtonA(AF_INET, server_ip_address, &address.sin_addr);
#else
  inet_pton(AF_INET, server_ip_address, &address.sin_addr);
#endif


  address.sin_family = AF_INET;
  address.sin_port = htons((u_short) server_port);

  // Connect to the server.
  SOCKET sock = socket(address.sin_family, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == sock)
  {
    MemTracePrint("MemTrace: Failed to create socket\n");
    ErrorShutdown();
    return;
  }

  if (0 != connect(sock, (struct sockaddr*) &address, sizeof address))
  {
    MemTracePrint("MemTrace: Failed to connect to %s (port %d)- is the server running?\n", server_ip_address, ntohs(address.sin_port));
    ErrorShutdown();
    return;
  }

  // Set send buffer size appropriately to avoid blocking needlessly.
  int sndbufsize = 15* kBufferSize;
  int result = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*) &sndbufsize, sizeof sndbufsize);
  if (0 != result)
  {
    MemTracePrint("MemTrace: Warning: Couldn't set send buffer size to %d bytes\n", sndbufsize);
  }
  unsigned long mode = 1;  // 1 to enable non-blocking socket
  ioctlsocket(sock, FIONBIO, &mode);

  //creating the network send block function inside InitSocket
			  auto write_block_fn = [](const void* block, size_t size) -> void
			  {
				// If we don't have a socket, we drop everything on the floor.
				if (INVALID_SOCKET == State.m_Socket)
				  return;
				size_t sent = 0;
				while(true)
				{
					sent = send(State.m_Socket, (const char*)block, (int)size, 0);
					if(sent != -1) {
						break;
					}
				}
				if (size != sent)
				{
				  MemTracePrint("MemTrace: send() failed - shutting down\n");
				  MemTrace::ErrorShutdown();
				}
				total_sent += sent;
			  };

			  auto listen_fn = [](const void* block, size_t size) -> void
			  {
				  if (INVALID_SOCKET == State.m_Socket)
					  return;
				  while (true)
				  {
					  size_t recieved = recv(State.m_Socket, (char *)block, (int)size, 0);
					  int error = WSAGetLastError();
					  if (error == WSAEWOULDBLOCK) //if no data is there to be read to read
					  {
						  break;
					  }
					  if (recieved != -1) {
						  MemTrace::HandleMessage((char *)block, recieved);
						  break;
					  }
				  }
			  };
	//end of write_block_fn
  if (!was_active)
  {
    InitCommon(write_block_fn, listen_fn);
    State.m_Socket = sock;
  }

  if (was_active)
  {
    State.m_Lock.Leave();
  }

  if (error)
  {
    ErrorShutdown();
  }
}

void MemTrace::HandleMessage(char* block, size_t size)
{
	char str [255];
	memcpy(str,block,size);
	if (strcmp(str, "pause") == 0)
	{
		State.m_Paused = true;
		MemTrace::Pause();
	}
	if (strcmp(str, "resume") == 0)
	{
		State.m_Paused = false;
	}
	MemTracePrint("Message recieved, contained %s\n",str);
}

void MemTrace::Pause()
{
	while (State.m_Paused)
	{
		State.m_Encoder.RecieveMessage();
	}
}

void MemTrace::ErrorShutdown()
{
  bool was_active = State.m_Active;

  if (was_active)
    State.m_Lock.Enter();

  if (State.m_BootFile != kInvalidFileHandle)
  {
    FileClose(State.m_BootFile);
    State.m_BootFile = kInvalidFileHandle;

#if defined(MEMTRACE_WINDOWS)
    if (State.m_BootFileName[0])
    {
      DeleteFileA(State.m_BootFileName);
    }
#endif
  }

  if (State.m_Socket != INVALID_SOCKET)
  {
    closesocket(State.m_Socket);
    State.m_Socket = INVALID_SOCKET;
  }

  State.m_Active = false;

  if (was_active)
    State.m_Lock.Leave();
}

void MemTrace::Flush()
{
  if (!State.m_Active)
    return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.Flush();
}

void MemTrace::Shutdown()
{
  if (!State.m_Active)
    return;

  State.m_Lock.Enter();

  State.m_Encoder.BeginEvent(kEndStream);
  State.m_Encoder.EndEvent(kEndStream);

  MemTracePrint("MemTrace: Shutting down.. ");

  // There's a tiny chance of a race on shutdown here, but it's small enough
  // that it shouldn't be a real problem. The race is:
  // 1. Thread checks m_Active, finds 1, is timesliced before taking the lock
  // 2. Main thread calls Shutdown(), destroying everything
  // 3. Thread resumes
  State.m_Active = false;

  // Flush and shut down writer.
  State.m_Encoder.Flush();
  MemTracePrint("Sent: %i Bytes\n", total_sent);
  closesocket(State.m_Socket);

  State.m_Lock.Leave();
  State.m_Lock.Destroy();
}

void MemTrace::BeginStream() {
	if (!State.m_Active)
		return;

	CSAutoLock lock(State.m_Lock);

	State.m_Encoder.BeginEvent(kBeginStream);
	State.m_Encoder.EmitUnsigned(kStreamMagic);
	State.m_Encoder.EmitString(kPlatformName);
	State.m_Encoder.EmitUnsigned(TimerGetSystemFrequencyInt());
	State.m_Encoder.EndEvent(kBeginStream);
}

MemTrace::HeapId MemTrace::HeapCreate(const char* type, const char* name)
{
  if (!State.m_Active)
    return ~0u;

  CSAutoLock lock(State.m_Lock);

  HeapId id = State.m_NextHeapId++;

  State.m_Encoder.BeginEvent(kHeapCreate);
  State.m_Encoder.EmitUnsigned(id);
  State.m_Encoder.EmitString(type);
  State.m_Encoder.EmitString(name);
  State.m_Encoder.EndEvent(kHeapCreate);
  return id;
}

void MemTrace::HeapDestroy(HeapId heap_id)
{
  if (!State.m_Active)
    return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kHeapDestroy);
  State.m_Encoder.EmitUnsigned(heap_id);
  State.m_Encoder.EndEvent(kHeapDestroy);
}

void MemTrace::HeapAddCore(HeapId heap_id, const void* base, size_t size_bytes)
{
  if (!State.m_Active)
    return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kHeapAddCore);
  State.m_Encoder.EmitUnsigned(heap_id);
  State.m_Encoder.EmitPointer(base);
  State.m_Encoder.EmitUnsigned(size_bytes);
  State.m_Encoder.EndEvent(kHeapAddCore);
}

void MemTrace::HeapRemoveCore(HeapId heap_id, const void* base, size_t size_bytes)
{
  if (!State.m_Active)
    return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kHeapRemoveCore);
  State.m_Encoder.EmitUnsigned(heap_id);
  State.m_Encoder.EmitPointer(base);
  State.m_Encoder.EmitUnsigned(size_bytes);
  State.m_Encoder.EndEvent(kHeapRemoveCore);
}

void MemTrace::HeapAllocate(HeapId id, const void* ptr, size_t size_bytes)
{
  if (!State.m_Active)
    return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kHeapAllocate);
  State.m_Encoder.EmitUnsigned(id);
  State.m_Encoder.EmitPointer(ptr);
  State.m_Encoder.EmitUnsigned(size_bytes);
  State.m_Encoder.EndEvent(kHeapAllocate);
}

void MemTrace::HeapFree(HeapId id, const void* ptr)
{
  if (!State.m_Active)
    return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kHeapFree);
  State.m_Encoder.EmitUnsigned(id);
  State.m_Encoder.EmitPointer(ptr);
  State.m_Encoder.EndEvent(kHeapFree);
}

/* Starts a new new event recording. */
void MemTrace::StartRecordingEvent(const char* eventName) {
	if (!State.m_Active)
		return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kEventStart);
  State.m_Encoder.EmitString(eventName);
  State.m_Encoder.EndEvent(kEventStart);
}

void MemTrace::StopRecordingEvent(const char* eventName) {
	if (!State.m_Active)
		return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kEventEnd);
  State.m_Encoder.EmitString(eventName);
  State.m_Encoder.EndEvent(kEventEnd);
}

void MemTrace::HeapSetBackingAllocator(HeapId for_heap, HeapId set_to_heap) {
	if(!State.m_Active)
		return;

  CSAutoLock lock(State.m_Lock);

  State.m_Encoder.BeginEvent(kSetBackingAllocator);
  State.m_Encoder.EmitUnsigned(for_heap);
  State.m_Encoder.EmitUnsigned(set_to_heap);
  State.m_Encoder.EndEvent(kSetBackingAllocator);
}

#endif // MEMTRACE_ENABLE
