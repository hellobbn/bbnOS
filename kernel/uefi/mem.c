#include "mem.h"
#include "bitmap.h"
#include "efi_mem.h"
#include "fb.h"
#include "types.h"

static MemDesc mem_info = {.free_memory = 0,
                           .reserved_memory = 0,
                           .used_memory = 0,
                           .mem_size_bytes = 0};
uint64_t getFreeMemSize() { return mem_info.free_memory; }
uint64_t getUsedMemSize() { return mem_info.used_memory; }
uint64_t getReservedMemSize() { return mem_info.reserved_memory; }

/// Allocate a bitmap with the given size at the given address. This should be
/// private to set up paging. Nothing outside should call this.
///
/// \param size The size of the bitmap to be allocated
/// \param addr The address of the bitmap
/// \return The address of the bitmap (usually equal to addr), NULL otherwise
static void *PGinitBitMap(size_t size, void *addr) {
  mem_info.MemBitMap.size = size;
  mem_info.MemBitMap.buffer = addr;

  for (size_t i = 0; i < size; i++) {
    *((uint8_t *)addr + i) = 0;
  }

  printf("Total pages: %lu\n", size * 8);

  return addr;
}

static void _ReservePage(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (bitmapGetVal(&(mem_info.MemBitMap), index) == true) {
    printf("!! %s: Trying to reserve a used page, addr = %X, index = %d\n",
           __func__, addr, index);
    return;
  }

  bitmapSetVal(&(mem_info.MemBitMap), index, true);

  mem_info.free_memory -= 4096;
  mem_info.reserved_memory += 4096;
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
  if (bitmapGetVal(&(mem_info.MemBitMap), index) == false) {
    printf("!! %s: Trying to free a not allocated page!\n", __func__);
    return;
  }

  bitmapSetVal(&(mem_info.MemBitMap), index, false);

  mem_info.free_memory += 4096;
  mem_info.used_memory -= 4096;
}

static void _LockPage(void *addr) {
  uint64_t index = (uint64_t)addr / 4096;
  if (bitmapGetVal(&(mem_info.MemBitMap), index) == true) {
    printf("!! %s: Trying to lock a used page!\n", __func__);
    return;
  }

  bitmapSetVal(&(mem_info.MemBitMap), index, true);

  mem_info.free_memory -= 4096;
  mem_info.used_memory += 4096;
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

static void _descGetLargestPage(EFI_MEMORY_DESCRIPTOR *desc) {}

static void _descReservePage(EFI_MEMORY_DESCRIPTOR *desc) {
  if (desc->type != EfiConventionalMemory && desc->type != EfiMemoryMappedIO &&
      desc->type != EfiMemoryMappedIOPortSpace) {
    ReservePages(desc->phys_addr, desc->num_pages);
  }
}

static void _descAddMemSize(EFI_MEMORY_DESCRIPTOR *desc) {
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
  mem_info.mem_size_bytes += desc->num_pages * 4096;
}

/// Given a function pointer, do this function to every mmap descs
static void _doForAllMMapDesc(EFI_MEMORY_DESCRIPTOR *mmap,
                              size_t num_mmap_entries, size_t mmap_desc_size,
                              void (*func)(EFI_MEMORY_DESCRIPTOR *desc)) {
  for (size_t i = 0; i < num_mmap_entries; i++) {
    EFI_MEMORY_DESCRIPTOR *desc =
        (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_desc_size));
    func(desc);
  }
}

void *requestPage() {
  for (uint64_t index = 0; index < mem_info.MemBitMap.size * 8; index++) {
    if (bitmapGetVal(&(mem_info.MemBitMap), index) == true) {
      continue;
    }

    _LockPage((void *)(index * 4096));
    return (void *)(index * 4096);
  }

  // TODO: Page frame swap
  return NULL;
}

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
  {
    for (size_t i = 0; i < num_mmap_entries; i++) {
      EFI_MEMORY_DESCRIPTOR *desc =
          (EFI_MEMORY_DESCRIPTOR *)((uint64_t)mmap + (i * mmap_desc_size));
      if (desc->type == EfiConventionalMemory) {
        printf("Conventional Mem: From %lX to %lX\n", desc->phys_addr,
               desc->phys_addr + desc->num_pages * 4096);
        if (desc->num_pages * 4096 > largest_mem_seg_size) {
          largest_mem_seg = desc->phys_addr;
          largest_mem_seg_size = desc->num_pages * 4096;
        }
      }
    }
    uint64_t mem_size = getMemSize(mmap, num_mmap_entries, mmap_desc_size);
    mem_info.free_memory = mem_size;
    uint64_t bitmap_size = mem_size / 4096 / 8 + 1;

    // initialize bitmap
    PGinitBitMap(bitmap_size, largest_mem_seg);

    // lock pages of bitmap
    lockPages(mem_info.MemBitMap.buffer, mem_info.MemBitMap.size / 4096 + 1);
  }

  // reverse pages of unused/reserved memory
  _doForAllMMapDesc(mmap, num_mmap_entries, mmap_desc_size, _descReservePage);
}

uint64_t getMemSize(EFI_MEMORY_DESCRIPTOR *mmap, uint64_t num_mmap_entries,
                    uint64_t mmap_desc_size) {
  if (mem_info.mem_size_bytes > 0) {
    return mem_info.mem_size_bytes;
  }

  _doForAllMMapDesc(mmap, num_mmap_entries, mmap_desc_size, _descAddMemSize);

  return mem_info.mem_size_bytes;
}
