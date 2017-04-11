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
  typedef uint64_t HeapId;

#if MEMTRACE_ENABLE

  /**
  * @brief Initializes MemoryTrace to save to file
  * @param character string to absolute filepath
  */
  void InitFile(const char *trace_file);

  /**
  * @brief Initializes MemoryTrace to connect to server
  * @param character string to server ip address
  * @param port number that server is listening to
  * @code
  * InitSocket("192.168.0.1", 8181);
  * @endcode
  */
  void InitSocket(const char *server_ip_address, int server_port);

  /**
  * @brief Closes tracking, ending connection if any was established
  */
  void Shutdown();

  /**
  * @brief Forces the content in buffer to be transmitted
  */
  void Flush();

  /**
  * @brief Event: Initializes the stream
  *
  * Sends the following information, in order:
  * - Stream magic
  * - Platform name
  * - Start time
  */
  void BeginStream();

  /**
  * @brief Create a heap
  *
  * Send the following information, in order:
  * - heapid
  * - type
  * - name
  * @code
  * id = MemTrace::HeapCreate("Block Allocator", "Debug");
  * @endcode
  * @param Type of heap, typically class name
  * @param Name of heap, typically function
  * @return Designated number of heap
  */
  HeapId HeapCreate(const char* type, const char* name);

  /**
  * @brief Destroy previously created heap
  * @param Character string to be printed
  * @param ... extra parameters for in string output
  * @return 
  */
  void HeapDestroy(HeapId heap_id);

/**
  * @brief Add core to a heap
  *
  * Send the following information, in order:
  * - heapid
  * - pointer to start of core
  * - size
  * @param Id to heap that adds the core
  * @param Pointer to start of base
  * @param Size of core
*/
  void HeapAddCore(HeapId heap_id, const void* base, size_t size_bytes);

/**
  * @brief Increases the size of a existing core
  *
  * Send the following information, in order:
  * - heapid
  * - pointer to start of core
  * - new size
  * @param Id to heap that grows the core
  * @param Pointer to start of base
  * @param The new size of the core
*/
  void HeapGrowCore(HeapId heap_id, const void* base, size_t new_size_bytes);

/**
  * @brief Remove a existing core
  *
  * Send the following information, in order:
  * - heapid
  * - pointer to start of core
  * - size of core
  * @param Id to heap that removes the core
  * @param Pointer to start of base
  * @param The size of the core
*/
  void    HeapRemoveCore(HeapId heap_id, const void* base, size_t size_bytes);

/**
  * @brief Shrink a existing core
  *
  * Send the following information, in order:
  * - heapid
  * - pointer to start of core
  * - size of core
  * @param Id to heap that shrinks the core
  * @param Pointer to start of base
  * @param The new size of the core
*/
  void	  HeapShrinkCore(HeapId heap_id, const void* base, size_t new_size_bytes);

/**
  * @brief Allocate a new piece of memory
  *
  * Send the following information, in order:
  * - heapid
  * - pointer to start of allocation
  * - size of allocation
  * - If allocated by this allocator
  * @param Id to heap that allocates memory
  * @param Pointer to start of allocation
  * @param The size of allocation
  * @param Is it allocated by another heap
*/
  void HeapAllocate(HeapId heap_id, const void* ptr, size_t size_bytes, bool allocated_by_heap = true);
/**
  * @brief Free memory
  *
  * Send the following information, in order:
  * - heapid
  * - pointer to start of allocation
  * - If allocated by this allocator
  * @param Id to heap that frees memory
  * @param Pointer to start of allocation
  * @param Is it allocated by another heap
*/
  void HeapFree(HeapId heap_id, const void* ptr, bool allocated_by_heap = true);

/**
  * @brief Free all memory
  *
  * Used when a memory allocator is destroyed to reduce the amount of events fired
  *
  * Send the following information, in order:
  * - heapid
  * @param Id to heap that allocates memory
*/
  void HeapFreeAll(HeapId heap_id);

/**
  * @brief Start user defined event
  *
  * Useful for indicating non memory related events such as a hash table rehash.
  * Needs to be followed by a StopRecordingEvent with same name
  *
  * Send the following information, in order:
  * - EventName
  * @param Name of event
*/
  void StartRecordingEvent(const char* eventName);

/**
  * @brief Ends user defined event
  *
  * Useful for indicating non memory related events such as a hash table rehash.
  * Needs to be preceeded by a StartRecordingEvent with same name
  *
  * Send the following information, in order:
  * - EventName
  * @param Name of event
*/
  void StopRecordingEvent(const char* eventName);

/**
  * @brief Function that handles incomming messages from the network
  *
  * @param Message to be handled
  * @param Size of message
*/
  void HandleMessage(char* msg, size_t size);

/**
  * @brief Pause function
  *
  * Runs an infinite loop, not good for multi threaded applications
*/
  void Pause();

/**
  * @brief Sets a memoryprovided for a heap
  *
  * Send the following information, in order:
  * - current heapid
  * - memory providing heapid
  * @param Id for current heap
  * @param Id for backing heap
*/
  void HeapSetBackingAllocator(HeapId for_heap, HeapId set_to_heap);

#endif

}
