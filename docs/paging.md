# Paging

This doc introduces the paging mechanism in both legacy and UEFI 64-bit kernel.
But for now only paging in the UEFI kernel is explained.

Some of the materials are copied from _Intel 64 and IA32 Architectures Software
Developer's Manual_ and [osdev](https://wiki.osdev.org/Page_Tables)

## A. Paging

> Paging is a system which allows each process to see a full virtual address
> space, without actually requiring the full amount of physical memory to be
> available or present.

So in general, in an Operating System, each process will see the full address
space of the OS, which, in 32-bit processor's case, is usually 4-GiB, and
256-TiB for a 64-bit processor with 48-bit virtual addressing. The paging
mechanism will translate the memory address a physical address according to the
paging translation mechanism, which will be introduced below.

Generally, the topmost paging structure is some levels of paging directories,
each of which contains 512 entries, and each entry is of 32-bit (in 32-bit
memory) or 64-bit (in 64-bit mode). So as a result, each level of page
directory will be of size (512 * 8 Bytes or 1024 * 4 Bytes) 4KB size. And each
entry of the page directory will look like this:

```
  -----------------------------------------------------------------------------
  | Next Level PageDir Address | Reserved | G | S | 0 | A | D | W | U | R | P |
  -----------------------------------------------------------------------------
  X                            11          9                                  0

  X: total page entry length
```

So the higher [X...12] bit will show the address of next level page directory,
and the hardware(MMU) will automatically translate that for you.

### A.1 32-bit Implementation

In 32-bit processors (without PAE), the virtual memory address will be 4 GiB,
and there are 2 levels: Page table and page directory, both of which have 1024
entries, and the virtual memory will be like:

```
  ------------------------------------------------------
  | PageTable Index | PageEntry Index | Offset in Page |
  ------------------------------------------------------
  0                 9                 19               31
```

The `PageTableIndex` shows the index of the page table in the top level page
directory, and we can get the address of next level page table. And the
`PageEntryIndex` shows the index of the page in the second page table, which
will show the address of the actual page. After we get the address of the page
and the offset in page (in the virtual address), we can finally get the
physical address.

### A.2 64-bit Implementation

The 64-bit processor usually does not have a full 64-bit virtual address space,
instead, the 48-bit virtual address space is implemented (at this time), and
the virtual address looks like this:

```
  -----------------------------------------------------------------
  | PDP Index | PD Index | PT Index | Page Index | Offset in Page |
  -----------------------------------------------------------------
  0           8          17         26           35               47
```

Each index is 9-bit long (512 entries) and the offset in page is 12-bit long.

## B. bbnOS UEFI 64-bit Implementation

This section shows the paging implementation in bbnOS kernel, UEFI, 64-bit
mode.

The memory related declarations of functions reside in _mem.h_ and the
definitions reside in _mem.h_.

The implementation follows the [youtube video](https://www.youtube.com/watch?v=e47SApmmx44&t=649s)

### B.1 Basic initializations

So when the control transfers from our own bootloader to kernel, we will need
to initialize certain data structures on memory. For example, based on the
memory map passed from UEFI, we will need to see the types and size of each
memory region and initialize the `mem_info` structure, which keeps track of
the current memory states.

> __Note:__ The memory detection function, which is actually the `getMemSize`
>       function, adds up the size of all memory regions (even for those types
>       like `EfiMemoryMappedIO`, which might not be necessary. However, based
>       on the observation that even a large memory size will not take much
>       space, we think that will be fine for now, but eventually this will
>       be fixed.

### B.2 Mapping

After getting the required memory info and setting up the data structure, we
will need to initialize the `PageTableManager`, which is a kernel space data
structure used to map the physical and virtual memory. So we initialize the
`pml4` table, which is the top level page table of the paging mechanism in
64-bit processor, and use that to initialize the `PageTableManager`, which only
has one member: the address of the `pml4` table.

Then we will need to map the whole memory, and we use the flat memory model,
where the virtual and physical memory addresses are equal, so we use the
following loop:

```c
  for (uint64_t t = 0;
       t < getMemSize(boot_info->mmap,
                      boot_info->mmap_size / boot_info->mmap_desc_size,
                      boot_info->mmap_desc_size);
       t += 0x1000) {
    pageTableManagerMapMemory(&ptm, (void *)t, (void *)t);
  }
```

Also, to make sure the framebuffer address does not change, we also remaps the
framebuffer (Because the address of the framebuffer might be larget than the
total memory size, and the code above does not map the framebuffer):

```c
  uint64_t fbBase = (uint64_t)boot_info->framebuffer->BaseAddress;
  uint64_t fbSize = (uint64_t)boot_info->framebuffer->BufferSize + 0x1000;
  for (uint64_t t = fbBase; t < fbBase + fbSize; t += 0x1000) {
    pageTableManagerMapMemory(&ptm, (void *)t, (void *)t);
  }
```

And finally, we load the new address of the `pml4` table to the `cr3` register:

```c
  asm("mov %0, %%cr3" : : "r"(pml4));
```

