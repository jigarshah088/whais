
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "custom/include/test/test_fmw.h"

/* GLOBAL variables for testing */
static size_t test_mem_used;
static size_t test_mem_peak;
static size_t test_max_mem  = 40 * 1024 * 1024;

#define CONTROL_BYTE    0xAC    /* all correct */

static W_ALLOCATED_MEMORY* spListHead      = NULL;
static uint64_t            sMemAllocations = 0;

static WH_LOCK             sMemSync;
static bool_t              sMemSyncInit;

/* set the maximum memory usage, 0 for unlimited */

void
test_set_mem_max(size_t size)
{
  test_max_mem = size;
}

size_t
test_get_mem_max()
{
  return test_max_mem;
}


size_t
test_get_mem_used(void)
{
  return test_mem_used;
}

void
test_add_used_mem(size_t add_size)
{
  test_mem_used += add_size;
  if (test_mem_used > test_mem_peak)
    test_mem_peak = test_mem_used;
}

void
test_free_used_mem(size_t free_size)
{
  test_mem_used -= free_size;
}

size_t
test_get_mem_peak(void)
{
  return test_mem_peak;
}

void
test_print_unfree_mem(void)
{
  W_ALLOCATED_MEMORY *pIt = spListHead;

  printf("%10s %10s %5s %s\n", "Count", "Size", "Line", "File");

  assert(pIt->size >= (sizeof( W_ALLOCATED_MEMORY) + 1));
  while (pIt != NULL)
  {
    printf("%10llu %10d %5d %s\n",
            (unsigned long long)pIt->count,
            (unsigned int)(pIt->size - (sizeof( W_ALLOCATED_MEMORY) + 1)),
            (unsigned int)(pIt->line),
            ((pIt->file == NULL) ? "NULL" : pIt->file));
    pIt = pIt->next;
  }
}

void*
custom_trace_mem_alloc(size_t size, const char *file, uint_t line)
{
  W_ALLOCATED_MEMORY* result = NULL;

  if ( !sMemSyncInit)
  {
    wh_lock_init( &sMemSync);
    sMemSyncInit = TRUE;
  }

  wh_lock_acquire( &sMemSync);

  result = (W_ALLOCATED_MEMORY *)custom_mem_alloc(size);
  if (result != NULL)
    --result;
  else
  {
    wh_lock_release( &sMemSync);
    return result;
  }

  result->line = line;
  result->file = file;
  result->count = ++sMemAllocations;

  if (spListHead == NULL)
  {
    result->prev = NULL;
    result->next = NULL;
  }
  else
  {
    result->next = spListHead;
    result->prev = NULL;

    assert(spListHead->prev == NULL);
    spListHead->prev = result;
  }

  spListHead = result;

  wh_lock_release( &sMemSync);

  return result + 1;
}

void*
custom_trace_mem_realloc(void *old_ptr, size_t new_size, const char *file, uint_t line)
{
  W_ALLOCATED_MEMORY* result  = NULL;
  W_ALLOCATED_MEMORY* pOldMem = NULL;
  uint64_t            size;

  result  = custom_trace_mem_alloc(new_size, file, line);
  pOldMem = (W_ALLOCATED_MEMORY*)old_ptr;

  if ((result == NULL) || (old_ptr == NULL))
    return result;

  size = pOldMem[-1].size;

  assert(size >= sizeof(W_ALLOCATED_MEMORY) + 1);
  size -= (sizeof( W_ALLOCATED_MEMORY) + 1);

  if (size > new_size)
    size = new_size;

  memcpy(result, pOldMem, size);

  custom_trace_mem_free(old_ptr, file, line);

  return result;
}

void
custom_trace_mem_free(void *ptr, const char *file, uint_t line)
{
  W_ALLOCATED_MEMORY *pMem = (W_ALLOCATED_MEMORY *)ptr;

  assert(sMemSyncInit);
  wh_lock_acquire( &sMemSync);

  pMem--;
  if (pMem == spListHead)
  {
    spListHead = spListHead->next;
    if (spListHead != NULL)
    {
      assert(spListHead->prev == pMem);
      spListHead->prev = NULL;
    }
  }
  else
  {
    if (pMem->next != NULL)
      pMem->next->prev = pMem->prev;

    assert(pMem->prev != NULL);
    pMem->prev->next = pMem->next;
  }

  custom_mem_free(ptr);
  wh_lock_release( &sMemSync);
}

void*
custom_mem_alloc(size_t size)
{
  W_ALLOCATED_MEMORY *result = NULL;

  size += sizeof(W_ALLOCATED_MEMORY);
  size++; /* control byte */
  if (test_get_mem_max() && ((test_get_mem_used() + size) > test_get_mem_max()))
  {
    /* do not allow allocation */
    return NULL;
  }

  result = (W_ALLOCATED_MEMORY *)malloc(size);
  if (result)
  {
    /* store the size */
    result->size = size;

    assert(size >= (sizeof(W_ALLOCATED_MEMORY) + 1));
    test_add_used_mem(size - (sizeof(W_ALLOCATED_MEMORY) + 1));
    ((uint8_t*)result)[size - 1] = CONTROL_BYTE;
    result++;
  }

  return (void *)result;
}

void*
custom_mem_realloc(void *old_ptr, size_t new_size)
{
  W_ALLOCATED_MEMORY *result;
  W_ALLOCATED_MEMORY *old_mem;

  result = (W_ALLOCATED_MEMORY *)custom_mem_alloc(new_size);
  old_mem = (W_ALLOCATED_MEMORY *)old_ptr;

  if ((result != NULL) && (old_ptr != NULL))
  {
    size_t size = old_mem[ -1].size;

    assert(size >= (sizeof(W_ALLOCATED_MEMORY) + 1));

    size -= (sizeof(W_ALLOCATED_MEMORY) + 1);
    if (new_size > size)
      size = new_size;

    assert(result[ -1].size == (new_size + 1 + sizeof(W_ALLOCATED_MEMORY)));

    memcpy(result, old_mem, size);

    custom_mem_free(old_mem);
  }

  return result;
}

void
custom_mem_free(void *ptr)
{
  size_t size = 0;
  W_ALLOCATED_MEMORY *real_ptr = (W_ALLOCATED_MEMORY *)ptr;

  real_ptr--;
  size = real_ptr->size;

  if (((uint8_t*)real_ptr)[size - 1] != CONTROL_BYTE)
    abort(); /* blow it up */
  else
    ((uint8_t*)real_ptr)[size - 1] = CONTROL_BYTE - 1;

  free(real_ptr);

  assert(size >= ((sizeof(W_ALLOCATED_MEMORY) + 1)));
  test_free_used_mem(size - (sizeof(W_ALLOCATED_MEMORY) + 1));
}
