///===- multiboot.c -----------------------------------------------------===///
/// Multiboot header, in C
///===-------------------------------------------------------------------===///
///
/// This file defines the multiboot headers used in bbnOS kernel to build
/// a multiboot image.
///
///===-------------------------------------------------------------------===///

#include "multiboot.h"
#include <stdint.h>

extern uint64_t _HeaderStart;
extern uint64_t _HeaderEnd;

struct kern_multiboot_header {
  struct multiboot_header header;
  struct multiboot_header_tag end_tag;
} header __attribute__((section(".multiboot_header"))) = {
    // The header
    .header =
        {
            .magic = MULTIBOOT2_HEADER_MAGIC,
            .architecture = MULTIBOOT_ARCHITECTURE_I386,
            .header_length = sizeof(struct kern_multiboot_header),
            // Force cast the `sizeof` to a uint32_t to silence a warning
            .checksum =
                -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT_ARCHITECTURE_I386 +
                  (uint32_t)sizeof(struct kern_multiboot_header)),
        },
    // Terminating tag, size = 0,
    .end_tag = {
        .type = 0,
        .flags = 0,
        .size = 8,
    }};
