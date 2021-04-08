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

#include "bitmap.h"
#include "efi_mem.h"
#include "types.h"

/// A structure describing the whole memory structure for the kernel
typedef struct MemDesc {
  BitMap MemBitMap;
  uint64_t free_memory;
  uint64_t reserved_memory;
  uint64_t used_memory;

  uint64_t mem_size_bytes;
} MemDesc;

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

/// Page Directory Entry Descriptor
typedef struct PageDirEntry {
  /// Present bit: If set, the page is present
  bool Present : 1;

  /// Read/Write perssions flag: If set, the page is read/write, otherwise
  /// readonly
  bool ReadWrite : 1;

  /// User/Supervisor: Control access to the page based on privilege level
  /// If set, then the page may be accessed by all
  bool UserSuper : 1;

  /// Write-Through abilities of the page: If set, write-through caching is
  /// enabled, otherwise write-back is used
  bool WriteThrough : 1;

  /// Cache Disable: If set, the page will not be cached
  bool CacheDisabled : 1;

  /// Discover whethter a page has beed read or written to. If set, the page
  /// has been read or written to
  bool Accessed : 1;

  /// always 0
  bool IgnoreZero : 1;

  /// Page Size: If set, the pages are 4MiB in size, otherwize 4KiB
  bool LargerPages : 1;

  /// Ignored
  bool IgnoreOne : 1;

  /// Available for OS implementation
  uint8_t Available : 3;
  uint64_t Address : 52;
} PageDirEntry;

typedef struct PageTable {
  PageDirEntry entries[512];
} __attribute__((aligned(0x1000))) PageTable;

typedef struct PageMapIndexer {
  uint64_t PDP_i;
  uint64_t PD_i;
  uint64_t PT_i;
  uint64_t P_i;
} PageMapIndexer;
void initPageMapIndexer(PageMapIndexer *PMI, uint64_t virtualAddress);

typedef struct PageTableManager {
  PageTable *PML4;
} PageTableManager;

void initPageTableManager(PageTableManager *pml, PageTable *pml4_addr);

void pageTableManagerMapMemory(PageTableManager *pml, void *virt_mem,
                               void *phys_mem);
#endif // MEM_H
