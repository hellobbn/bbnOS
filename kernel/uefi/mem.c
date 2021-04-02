#include "mem.h"
#include "bitmap.h"
#include "efi_mem.h"
#include "fb.h"
#include "types.h"

static BitMap MemBitMap;
uint64_t free_memory;
uint64_t reserved_memory;
uint64_t used_memory;

/// Allocate a bitmap with the given size at the given address. This should be
/// private to set up paging. Nothing outside should call this.
///
/// \param size The size of the bitmap to be allocated
/// \param addr The address of the bitmap
/// \return The address of the bitmap (usually equal to addr), NULL otherwise
static void *PGinitBitMap(size_t size, void *addr) {
  MemBitMap.size = size;
  MemBitMap.buffer = addr;

  for (size_t i = 0; i < size; i++) {
    *((uint8_t *)addr + i) = 0;
  }

  printf("Total pages: %lu\n", size * 8);

  return addr;
}

static void _ReservePage(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (bitmapGetVal(&MemBitMap, index) == true) {
    printf("!! %s: Trying to reserve a used page, addr = %X, index = %d\n", __func__, addr, index);
    return;
  }

  bitmapSetVal(&MemBitMap, index, true);

  free_memory -= 4096;
  reserved_memory += 4096;
}

// Not used for now, commenting it out first
// static void _UnreservePage(void *addr) {
//   uint64_t index = (uint64_t)addr / 4096;
//   if (bitmapGetVal(&MemBitMap, index) == false) {
//     printf("!! %s: Trying to un-reserve a not used page!\n", __func__);
//     return;
//   }

//   bitmapSetVal(&MemBitMap, index, false);

//   free_memory += 4096;
//   reserved_memory -= 4096;
// }

static void _FreePage(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (bitmapGetVal(&MemBitMap, index) == false) {
    printf("!! %s: Trying to free a not allocated page!\n", __func__);
    return;
  }

  bitmapSetVal(&MemBitMap, index, false);

  free_memory += 4096;
  used_memory -= 4096;
}

static void _LockPage(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (bitmapGetVal(&MemBitMap, index) == true) {
    printf("!! %s: Trying to lock a used page!\n", __func__);
    return;
  }

  bitmapSetVal(&MemBitMap, index, true);

  free_memory -= 4096;
  used_memory += 4096;
}

static void ReservePages(void *addr, size_t num_pages) {
  for (size_t i = 0; i < num_pages; i++) {
    _ReservePage((void *)((uint64_t)addr + (i * 4096)));
  }
}

// Unused for now, comment it out to avoid warinings
// static void UnreservePages(void *addr, size_t num_pages) {
//   for (size_t i = 0; i < num_pages; i++) {
//     _UnreservePage((void *)((uint64_t)addr + (i * 4096)));
//   }
// }

void freePages(void *addr, size_t num_pages) {
  for (size_t i = 0; i < num_pages; i++) {
    _FreePage((void *)((uint64_t)addr + (i * 4096)));
  }
}

void lockPages(void *addr, size_t num_pages) {
  for (size_t i = 0; i < num_pages; i++) {
    _LockPage((void *)((uint64_t)addr + (i * 4096)));
  }
}

uint64_t getFreeMemSize() { return free_memory; }

uint64_t getUsedMemSize() { return used_memory; }

uint64_t getReservedMemSize() { return reserved_memory; }

void readEFIMemoryMap(EFI_MEMORY_DESCRIPTOR *mmap, size_t mmap_size,
                      size_t mmap_desc_size) {
  static bool initialized = false;

  if (initialized) {
    printf("> %s: WARNING: called more than once, possible ERROR!\n", __func__);
    return;
  }

  initialized = true;
  uint64_t num_mmap_entries = mmap_size / mmap_desc_size;

  uint64_t largest_mem_seg_size = 0;
  void *largest_mem_seg = NULL;

  for (size_t i = 0; i < num_mmap_entries; i++) {
    EFI_MEMORY_DESCRIPTOR *desc =
        (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_desc_size));
    if (desc->type == EfiConventionalMemory) {
      if (desc->num_pages * 4096 > largest_mem_seg_size) {
        largest_mem_seg = desc->phys_addr;
        largest_mem_seg_size = desc->num_pages * 4096;
      }
    }
  }

  uint64_t mem_size = getMemSize(mmap, num_mmap_entries, mmap_desc_size);
  free_memory = mem_size;

  uint64_t bitmap_size = mem_size / 4096 / 8 + 1;

  // initialize bitmap
  PGinitBitMap(bitmap_size, largest_mem_seg);

  // lock pages of bitmap
  lockPages(MemBitMap.buffer, MemBitMap.size / 4096 + 1);

  // reverse pages of unused/reserved memory
  for (size_t i = 0; i < num_mmap_entries; i++) {
    EFI_MEMORY_DESCRIPTOR *desc =
        (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_desc_size));
    // FIXME: The memory mapped I/Os resides in addresses below 4GB, we might
    // want to rule them out first
    if (desc->type != EfiConventionalMemory &&
        desc->type != EfiMemoryMappedIO && desc->type != EfiMemoryMappedIOPortSpace) {
      ReservePages(desc->phys_addr, desc->num_pages);
    }
  }
}

uint64_t getMemSize(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t num_mmap_entries,
                    uint64_t mmap_desc_size) {
  static uint64_t memory_size_bytes = 0;
  if (memory_size_bytes > 0) {
    return memory_size_bytes;
  }

  for (uint64_t i = 0; i < num_mmap_entries; i++) {
    EFI_MEMORY_DESCRIPTOR *desc =
        (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_desc_size));
    if (desc->type > 15) {
      printf("!!! ERROR: This memory type: %d is not viable !!!\n", desc->type);
      // A small blocking loop
      while (1) {
      }
    }
    // FIXME: The memory mapped I/Os resides in addresses below 4GB, we might
    //        want to rule them out first. The detected memory is surely more
    //        than the actual Memory size on this machine, but we do this so
    //        that higher memory address will eventually be included in the
    //        table regardless of how many `holes` are there in the lower
    //        address space.
    memory_size_bytes += desc->num_pages * 4096;
  }

  return memory_size_bytes;
}
