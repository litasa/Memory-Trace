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

#pragma once

#include <stdint.h>
#include <stddef.h>

// You can control this with a build macro if desired to compile MemTrace out.
#define MEMTRACE_ENABLE 1

namespace MemTrace
{
  typedef uint32_t HeapId;

#if MEMTRACE_ENABLE

  // Get connection parameters specified on the command line, if any (returns true)
  // Useful to forward memtrace configuration along to spawned child processes.
  bool    GetSocketData(char (&ip_addr_out)[128], int* port_out);

  void    InitFile(const char *trace_temp_file);

  void    InitSocket(const char *server_ip_address, int server_port);

  void    Shutdown();

  void    Flush();

  void	  BeginStream();

  HeapId  HeapCreate(const char* type, const char* name);
  void    HeapDestroy(HeapId heap_id);

  void    HeapAddCore(HeapId heap_id, const void* base, size_t size_bytes);
  void    HeapRemoveCore(HeapId heap_id, const void* base, size_t size_bytes);

  void    HeapAllocate(HeapId heap_id, const void* ptr, size_t size_bytes);
  void    HeapFree(HeapId heap_id, const void* ptr);

  void	  StartRecordingEvent(const char* eventName);
  void    StopRecordingEvent(const char* eventName);

  void    HandleMessage(char* msg, size_t size);
  void	  Pause();


  void DummyInitFunction(char dummy);

  /* Stingray Engine Specifics */
  void	  HeapSetBackingAllocator(HeapId for_heap, HeapId set_to_heap);

#endif

}
