/******************************************************************************
WHAIS - An advanced database system
Copyright(C) 2014-2018  Iulian Popa

Address: Str Olimp nr. 6
         Pantelimon Ilfov,
         Romania
Phone:   +40721939650
e-mail:  popaiulian@gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <cstdlib>

#include "whais.h"

#ifndef ENABLE_MEMORY_TRACE
extern "C"
{

void*
custom_mem_alloc(size_t size)
{
  return malloc(size);
}

void*
custom_mem_realloc(void*  oldPtr,
                    size_t newSize)
{
  return realloc(oldPtr, newSize);
}

void
custom_mem_free(void* ptr)
{
  free(ptr);
}

}; /* extern "C" */
#endif /* ENABLE_MEMORY_TRACE */

#undef new

#ifdef CXX_CUSTOM_MEMORY_ALLOCATOR

void*
operator new(std::size_t size)
{
#ifndef ENABLE_MEMORY_TRACE
  void *ptr = custom_mem_alloc(size);
#else
  void *ptr = custom_trace_mem_alloc(size, nullptr, 0);
#endif

  if (ptr == nullptr)
    throw std::bad_alloc();
  return ptr;
}

void*
operator new(std::size_t size, const std::nothrow_t&) throw()
{
#ifndef ENABLE_MEMORY_TRACE
  void *ptr = custom_mem_alloc(size);
#else
  void *ptr = custom_trace_mem_alloc(size, nullptr, 0);
#endif

  return ptr;
}

void*
operator new(std::size_t size, const char* pFile, uint_t line)
{
#ifndef ENABLE_MEMORY_TRACE
  (void)pFile;
  (void)line;
  void *ptr = custom_mem_alloc(size);
#else
  void *ptr = custom_trace_mem_alloc(size, pFile, line);
#endif

  if (ptr == nullptr)
    throw std::bad_alloc();
  return ptr;
}


void*
operator new [] (std::size_t size)
{
#ifndef ENABLE_MEMORY_TRACE
  void *ptr = custom_mem_alloc(size);
#else
  void *ptr = custom_trace_mem_alloc(size, nullptr, 0);
#endif

  if (ptr == nullptr)
    throw std::bad_alloc();
  return ptr;
}

void*
operator new[] (std::size_t size, const std::nothrow_t&) throw()
{
#ifndef ENABLE_MEMORY_TRACE
  void *ptr = custom_mem_alloc(size);
#else
  void *ptr = custom_trace_mem_alloc(size, nullptr, 0);
#endif

  return ptr;
}

void*
operator new [] (size_t size, const char* pFile, uint_t line)
{
#ifndef ENABLE_MEMORY_TRACE
  (void)pFile;
  (void)line;
  void *ptr = custom_mem_alloc(size);
#else
  void *ptr = custom_trace_mem_alloc(size, pFile, line);
#endif

  if (ptr == nullptr)
    throw std::bad_alloc();
  return ptr;
}


CUSTOM_SHL void
operator delete(void* ptr)
{
  if (ptr != nullptr)
#ifndef ENABLE_MEMORY_TRACE
    custom_mem_free(ptr);
#else
  custom_trace_mem_free(ptr, nullptr, 0);
#endif
}

void
operator delete(void* ptr, const char*, uint_t)
{
  if (ptr != nullptr)
#ifndef ENABLE_MEMORY_TRACE
    custom_mem_free(ptr);
#else
  custom_trace_mem_free(ptr, nullptr, 0);
#endif
}

void
operator delete [] (void* ptr)
{
  if (ptr != nullptr)
#ifndef ENABLE_MEMORY_TRACE
    custom_mem_free(ptr);
#else
  custom_trace_mem_free(ptr, nullptr, 0);
#endif
}

void
operator delete[] (void* ptr, const char*, uint_t )
{
  if (ptr != nullptr)
#ifndef ENABLE_MEMORY_TRACE
    custom_mem_free(ptr);
#else
    custom_trace_mem_free(ptr, nullptr, 0);
#endif
}

#endif


