///===- efi_mem.h - UEFI Memory Map -------------------------------------===///
/// UEFI Memory Map related functions
///===-------------------------------------------------------------------===///
///
/// This file contains descriptors for UEFI Memory Map
///
///===-------------------------------------------------------------------===///

#ifndef EFI_MEM_H
#define EFI_MEM_H

#include "types.h"

/// A EFI memory descriptor
typedef struct EFI_MEMORY_DESCRIPTOR {
  /// Type of the memory region
  uint32_t type;

  /// Physical address of the first byte of the memory region
  void *phys_addr;

  /// Virtual address of the first byte of the memory region
  void *virt_addr;

  /// Number of `4KB` pages in the memory region
  uint64_t num_pages;

  /// Attributes of memory region that describe the bit mask of capabilities
  /// for that memory region, and not necessarily the current settings for
  /// that memory region
  uint64_t attribs;
} EFI_MEMORY_DESCRIPTOR;

typedef enum {
    EfiReservedMemoryType = 0,
    EfiLoaderCode = 1,
    EfiLoaderData = 2,
    EfiBootServicesCode = 3,
    EfiBootServicesData = 4,
    EfiRuntimeServicesCode = 5,
    EfiRuntimeServicesData = 6,
    EfiConventionalMemory = 7,
    EfiUnusableMemory = 8,
    EfiACPIReclaimMemory = 9,
    EfiACPIMemoryNVS = 10,
    EfiMemoryMappedIO = 11,
    EfiMemoryMappedIOPortSpace = 12,
    EfiPalCode = 13,
    EfiMaxMemoryType = 14,
} EFI_MEMORY_TYPE;

extern const char *EFI_MEMORY_TYPE_STRINGS[];
#endif // EFI_MEM_H
