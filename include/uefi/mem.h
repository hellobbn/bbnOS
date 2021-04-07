///===- mem.h - Kernel memory management --------------------------------===///
/// Functions for kernel memory management
///===-------------------------------------------------------------------===///
///
/// This file declares functions for the kernel to manage the memory, such as
/// setting up paging, and other functions.
///
///===-------------------------------------------------------------------===///

#ifndef MEM_H
#define MEM_H

#include "efi_mem.h"
#include "types.h"

void freePages(void *addr, size_t num_pages);

void lockPages(void *addr, size_t num_pages);

uint64_t getFreeMemSize();

uint64_t getUsedMemSize();

uint64_t getReservedMemSize();

void *requestPage();

void readEFIMemoryMap(EFI_MEMORY_DESCRIPTOR *mmap, size_t mmap_size,
                      size_t mmap_desc_size);

uint64_t getMemSize(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t num_mmap_entries,
                    uint64_t mmap_desc_size);

#endif // MEM_H
