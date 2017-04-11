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
		kBufferSize = 17520
	};

	// Start of stream protocol value - to handle version changes without crashing decoder.
	static const uint32_t kStreamMagic = 0xbfaf0003;

	//-----------------------------------------------------------------------------
	// Event codes that can be sent to the output stream.
	enum EventCode : uint8_t
	{
		kBeginStream = 1,
		kEndStream,

		kHeapCreate = 10,
		kHeapDestroy,

		kHeapAddCore,
		kHeapGrowCore,
		kHeapRemoveCore,
		kHeapShrinkCore,

		kHeapAllocate,
		kHeapFree,
		kHeapFreeAll,

		kEventStart = 20,
		kEventEnd,

		//Stingray specifics from here on
		kSetBackingAllocator = 30
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
	class encoder
	{
	private:
		size_t                        _write_offset;                    // Write offset within current buffer
		TransmitBlockFn*              _transmit_function;                     // Function to transmit (partially) filled blocks
		ListeningFn*			      _listen_function;
		uint64_t                      _start_time;                      // System timer for initial event. We use a delta to generate smaller numbers.

		int                           _current_buffer;                      // Index of current encoding buffer
		uint8_t                       _buffers[2][kBufferSize];        // Raw encoding buffers

		//-----------------------------------------------------------------------------
		// Flush current buffer and flip buffers.
		void TransmitCurrentBuffer()
		{
			(*_transmit_function)(_buffers[_current_buffer], _write_offset);

			// Flip buffers.
			_write_offset = 0;
			_current_buffer ^= 1;
			RecieveMessage();
		}

		//-----------------------------------------------------------------------------
		// Reserve space for <size> bytes. Can cause a buffer flipflush.
		uint8_t* Reserve(size_t size)
		{
			ASSERT_FATAL(size < kBufferSize, "Block size too small for reservation");
			uint8_t *base = _buffers[_current_buffer];

			if (size + _write_offset > kBufferSize)
			{
				TransmitCurrentBuffer();

				base = _buffers[_current_buffer];
			}

			return base + _write_offset;
		}

		//-----------------------------------------------------------------------------
		// Commit <size> bytes. At least <size> bytes must have been previously reserved via Reserve()
		void Commit(size_t size)
		{
			_write_offset += size;
		}

		//-----------------------------------------------------------------------------
		// Emit a relative time stamp to the stream.
		uint64_t EmitTimeStamp()
		{
			uint64_t t = TimerGetSystemCounter();
			uint64_t delta = t - _start_time;
			EmitUnsigned(delta);
			return delta;
		}

	public:
		void RecieveMessage() {
			_listen_function(_buffers[_current_buffer ^ 1], kBufferSize);
		}
		//-----------------------------------------------------------------------------
		// Initialize the encoder with a function that writes encoded blocks to some output device.
		void Init(TransmitBlockFn *transmit_fn, ListeningFn* listen_fn)
		{
			_write_offset = 0;
			_transmit_function = transmit_fn;
			_listen_function = listen_fn;
			_start_time = TimerGetSystemCounter();
		}

		//-----------------------------------------------------------------------------
		// Swap out the transmit function (for file->socket switcharoo)
		void Settransmit_function(TransmitBlockFn* fn, ListeningFn* listen_fn)
		{
			_transmit_function = fn;
			_listen_function = listen_fn;
		}

		//-----------------------------------------------------------------------------
		// Flush any pending data to the output device.
		void Flush()
		{
			TransmitCurrentBuffer();
			// Immediately sync async write.
			(*_transmit_function)(NULL, 0);
		}

		//-----------------------------------------------------------------------------
		// Encode a 64-bit integer to the output stream using an encoding that favors small numbers.
		//
		// Encoding used:
		// - There is 7 bits of payload per byte, leading to a worst case of 10 bytes to store a 64-bit number
		// - Decoding proceeds as long as the MSB is zero.
		void EmitUnsigned(uint64_t val)
		{
			uint8_t* out = Reserve(10);
			uint8_t  byte;
			size_t   i = 0;

			do
			{
				out[i++] = (uint8_t)(val & 0x7f);
				val >>= 7;
			} while (val);
			out[i - 1] |= 0x80;
			Commit(i);
		}

		void EmitBool(bool val)
		{
			uint8_t* out = Reserve(1);
			out[0] = (uint8_t(val) & 0x7f) | 0x80;
			Commit(1);
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

			const size_t   len = strlen(str);
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
			EmitTimeStamp();
			EmitUnsigned(GetCurrentThreadId());
		}
		void EndEvent(EventCode code)
		{
			EmitUnsigned(code);
			count++;
		}

	};

	//-----------------------------------------------------------------------------
	// Subsystem global state
	struct
	{
		bool            _active;                     // Non-zero if the system is active
		encoder         _encoder;                    // encoder
		uint32_t        _next_heap_id;                 // Next free heap ID
		CriticalSection _lock;                       // Synchronizes access to encoder/stream

		FileHandle      _boot_file;
		SOCKET          _socket;                     // Output socket during normal operation
		char            _boot_fileName[128];

		bool			_paused = false;

	} State;
}

//-----------------------------------------------------------------------------
// Common init routine for first time setup

static void MemTrace::InitCommon(TransmitBlockFn* write_block_fn, ListeningFn* listening_fn)
{
	if (State._active)
	{
#if defined(MEMTRACE_WINDOWS)
		DebugBreak();
#else
		abort();
#endif
	}

	State._active = true;
	State._socket = INVALID_SOCKET;

	State._lock.Init();
	State._encoder.Init(write_block_fn, listening_fn);

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
	Strcpy(State._boot_fileName, ARRAY_SIZE(State._boot_fileName), trace_temp_file);

	State._boot_file = hf;

	// Callback that dumps event buffer data using async writes to our socket.
	auto write_block_fn = [](const void* block, size_t size) -> void
	{
		FileWrite(State._boot_file, block, size);
	};

	InitCommon(write_block_fn, nullptr);
}

//-----------------------------------------------------------------------------
void MemTrace::InitSocket(const char *server_ip_address, int server_port)
{
	bool error = false;

	// Remember if we were already active; if we were we need to protect against memory allocations
	// on other threads trying to trace while we're switching protocols.
	const bool was_active = State._active;

	if (was_active)
	{
		State._lock.Enter();
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
	address.sin_port = htons((u_short)server_port);

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
	int sndbufsize = 16 * kBufferSize;
	int result = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&sndbufsize, sizeof sndbufsize);
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
		if (INVALID_SOCKET == State._socket)
			return;
		size_t sent = 0;
		while (true)
		{
			sent = send(State._socket, (const char*)block, (int)size, 0);
			if (sent != -1) {
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
		if (INVALID_SOCKET == State._socket)
			return;
		while (true)
		{
			size_t recieved = recv(State._socket, (char *)block, (int)size, 0);
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
		State._socket = sock;
	}

	if (was_active)
	{
		State._lock.Leave();
	}

	if (error)
	{
		ErrorShutdown();
	}
}

void MemTrace::HandleMessage(char* block, size_t size)
{
	char str[255];
	memcpy(str, block, size);
	if (strcmp(str, "pause") == 0)
	{
		State._paused = true;
		MemTrace::Pause();
	}
	if (strcmp(str, "resume") == 0)
	{
		State._paused = false;
	}
	//MemTracePrint("Message recieved, contained %s\n",str);
}

void MemTrace::Pause()
{
	//Insert own pause function here. Current implementation is only single threaded
	while (State._paused)
	{
		State._encoder.RecieveMessage();
	}
}

void MemTrace::ErrorShutdown()
{
	bool was_active = State._active;

	if (was_active)
		State._lock.Enter();

	if (State._boot_file != kInvalidFileHandle)
	{
		FileClose(State._boot_file);
		State._boot_file = kInvalidFileHandle;

#if defined(MEMTRACE_WINDOWS)
		if (State._boot_fileName[0])
		{
			DeleteFileA(State._boot_fileName);
		}
#endif
	}

	if (State._socket != INVALID_SOCKET)
	{
		closesocket(State._socket);
		State._socket = INVALID_SOCKET;
	}

	State._active = false;

	if (was_active)
		State._lock.Leave();
}

void MemTrace::Flush()
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.Flush();
}

void MemTrace::Shutdown()
{
	if (!State._active)
		return;

	State._lock.Enter();

	State._encoder.BeginEvent(kEndStream);
	State._encoder.EndEvent(kEndStream);

	MemTracePrint("MemTrace: Shutting down.. ");

	// There's a tiny chance of a race on shutdown here, but it's small enough
	// that it shouldn't be a real problem. The race is:
	// 1. Thread checks _active, finds 1, is timesliced before taking the lock
	// 2. Main thread calls Shutdown(), destroying everything
	// 3. Thread resumes
	State._active = false;

	// Flush and shut down writer.
	State._encoder.Flush();
	MemTracePrint("Sent: %i Bytes\n", total_sent);
	closesocket(State._socket);

	State._lock.Leave();
	State._lock.Destroy();
}

void MemTrace::BeginStream() {
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kBeginStream);
	State._encoder.EmitUnsigned(kStreamMagic);
	State._encoder.EmitString(kPlatformName);
	State._encoder.EmitUnsigned(TimerGetSystemFrequencyInt());
	State._encoder.EndEvent(kBeginStream);
}

MemTrace::HeapId MemTrace::HeapCreate(const char* type, const char* name)
{
	if (!State._active)
		return ~0u;

	CSAutoLock lock(State._lock);

	HeapId id = State._next_heap_id++;

	State._encoder.BeginEvent(kHeapCreate);
	State._encoder.EmitUnsigned(id);
	State._encoder.EmitString(type);
	State._encoder.EmitString(name);
	State._encoder.EndEvent(kHeapCreate);
	return id;
}

void MemTrace::HeapDestroy(HeapId heap_id)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapDestroy);
	State._encoder.EmitUnsigned(heap_id);
	State._encoder.EndEvent(kHeapDestroy);
}

void MemTrace::HeapAddCore(HeapId heap_id, const void* base, size_t size_bytes)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapAddCore);
	State._encoder.EmitUnsigned(heap_id);
	State._encoder.EmitPointer(base);
	State._encoder.EmitUnsigned(size_bytes);
	State._encoder.EndEvent(kHeapAddCore);
}

void MemTrace::HeapGrowCore(HeapId heap_id, const void * base, size_t new_size_bytes)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapGrowCore);
	State._encoder.EmitUnsigned(heap_id);
	State._encoder.EmitPointer(base);
	State._encoder.EmitUnsigned(new_size_bytes);
	State._encoder.EndEvent(kHeapGrowCore);
}

void MemTrace::HeapRemoveCore(HeapId heap_id, const void* base, size_t size_bytes)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapRemoveCore);
	State._encoder.EmitUnsigned(heap_id);
	State._encoder.EmitPointer(base);
	State._encoder.EmitUnsigned(size_bytes);
	State._encoder.EndEvent(kHeapRemoveCore);
}

void MemTrace::HeapShrinkCore(HeapId heap_id, const void * base, size_t new_size_bytes)
{
	State._encoder.BeginEvent(kHeapShrinkCore);
	State._encoder.EmitUnsigned(heap_id);
	State._encoder.EmitPointer(base);
	State._encoder.EmitUnsigned(new_size_bytes);
	State._encoder.EndEvent(kHeapShrinkCore);
}

void MemTrace::HeapAllocate(HeapId id, const void* ptr, size_t size_bytes, bool allocated_by_heap)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapAllocate);
	State._encoder.EmitUnsigned(id);
	State._encoder.EmitPointer(ptr);
	State._encoder.EmitUnsigned(size_bytes);
	State._encoder.EmitBool(allocated_by_heap);
	State._encoder.EndEvent(kHeapAllocate);
}

void MemTrace::HeapFree(HeapId id, const void* ptr, bool allocated_by_heap)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapFree);
	State._encoder.EmitUnsigned(id);
	State._encoder.EmitPointer(ptr);
	State._encoder.EmitBool(allocated_by_heap);	
	State._encoder.EndEvent(kHeapFree);
}

void MemTrace::HeapFreeAll(HeapId heap_id)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kHeapFreeAll);
	State._encoder.EmitUnsigned(heap_id);
	State._encoder.EndEvent(kHeapFreeAll);
}

/* Starts a new new event recording. */
void MemTrace::StartRecordingEvent(const char* eventName)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kEventStart);
	State._encoder.EmitString(eventName);
	State._encoder.EndEvent(kEventStart);
}

void MemTrace::StopRecordingEvent(const char* eventName)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kEventEnd);
	State._encoder.EmitString(eventName);
	State._encoder.EndEvent(kEventEnd);
}

void MemTrace::HeapSetBackingAllocator(HeapId for_heap, HeapId set_to_heap)
{
	if (!State._active)
		return;

	CSAutoLock lock(State._lock);

	State._encoder.BeginEvent(kSetBackingAllocator);
	State._encoder.EmitUnsigned(for_heap);
	State._encoder.EmitUnsigned(set_to_heap);
	State._encoder.EndEvent(kSetBackingAllocator);
}

//void MemTrace::HeapTrackAllocation(HeapId id, const void* ptr, const size_t size_bytes)
//{
//	if (!State._active)
//		return;
//
//	CSAutoLock lock(State._lock);
//
//	State._encoder.BeginEvent(kTrackHeapAllocation);
//	State._encoder.EmitUnsigned(id);
//	State._encoder.EmitPointer(ptr);
//	State._encoder.EmitUnsigned(size_bytes);
//	State._encoder.EndEvent(kTrackHeapAllocation);
//}
//
//void MemTrace::HeapTrackFree(HeapId id, const void* ptr)
//{
//	if (!State._active)
//		return;
//
//	CSAutoLock lock(State._lock);
//
//	State._encoder.BeginEvent(kTrackHeapFree);
//	State._encoder.EmitUnsigned(id);
//	State._encoder.EmitPointer(ptr);
//	State._encoder.EndEvent(kTrackHeapFree);
//}

#endif // MEMTRACE_ENABLE
