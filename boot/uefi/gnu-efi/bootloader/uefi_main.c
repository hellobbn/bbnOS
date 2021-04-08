///===- uefi_main.c -----------------------------------------------------===///
/// The UEFI Boot Loader for bbnOS
///===-------------------------------------------------------------------===///
//
// TODO: Fix font styles.
//
//===-------------------------------------------------------------------===///
#include <efi.h>
#include <efibind.h>
#include <efidef.h>
#include <efierr.h>
#include <efilib.h>
#include <efiprot.h>
#include <elf.h>

typedef unsigned long long size_t;

/// The framebuffer descriptor
typedef struct {
  void *BaseAddress;
  size_t BufferSize;
  unsigned int Width;
  unsigned int Height;
  unsigned int PixelsPerScanline;
} Framebuffer;

/// PSF1 Font Related structures and Macros
///{
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
typedef struct {
  unsigned char magic[2];
  unsigned char mode;
  unsigned char charsize;
} PSF1_HEADER;

typedef struct {
  PSF1_HEADER *psf1_header;
  void *glyph_buffer;
} PSF1_FONT;
///}

typedef struct {
  Framebuffer *framebuffer;
  PSF1_FONT *PSF1_Font;
  EFI_MEMORY_DESCRIPTOR *MMap;
  UINTN MMapSize;
  UINTN MMapDescSize;
} BootInfo;

Framebuffer framebuffer;

Framebuffer *InitializeGOP() {
  EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  EFI_STATUS status;

  status =
      uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void **)&gop);
  if (EFI_ERROR(status)) {
    Print(L"==> ERROR: Unable to locate GOP \n\r");
    return NULL;
  }

  Print(L"==> GOP Located \n\r");

  framebuffer.BaseAddress = (void *)gop->Mode->FrameBufferBase;
  framebuffer.BufferSize = gop->Mode->FrameBufferSize;
  framebuffer.Width = gop->Mode->Info->HorizontalResolution;
  framebuffer.Height = gop->Mode->Info->VerticalResolution;
  framebuffer.PixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

  return &framebuffer;
}

/// Load the file in `dir/path` in the EFI partition, if the directory is NULL,
/// the file will be loaded from Root of the partition.
///
/// \param Directory The Directory where the file resides
/// \param Path The file name
/// \param ImageHandle ImageHandle from main, needed by some functions
/// \param SystemTable SystemTable from main
EFI_FILE *LoadFile(EFI_FILE *Directory, CHAR16 *Path, EFI_HANDLE ImageHandle,
                   EFI_SYSTEM_TABLE *SystemTable) {
  EFI_FILE *LoadedFile = NULL;

  // Load the image from the image handle
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
  SystemTable->BootServices->HandleProtocol(
      ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&LoadedImage);

  // Get the file system from the device handle in the image handle
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle,
                                            &gEfiSimpleFileSystemProtocolGuid,
                                            (void **)&FileSystem);

  // If the directory is NULL, the file is in root, we open the volume
  // at root
  if (Directory == NULL) {
    Directory = LibOpenRoot(LoadedImage->DeviceHandle);
  }

  // Open the file
  EFI_STATUS ret =
      uefi_call_wrapper(Directory->Open, 5, Directory, &LoadedFile, Path,
                        EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
  if (ret != EFI_SUCCESS) {
    return NULL;
  }

  return LoadedFile;
}

/// Load the PSF1 font specified.
PSF1_FONT *LoadPSF1Font(EFI_FILE *Directory, CHAR16 *Path,
                        EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  EFI_FILE *font = LoadFile(Directory, Path, ImageHandle, SystemTable);
  if (font == NULL) {
    Print(L"  > %s: ERROR: Unable to load font file \n\r", __func__);
    return NULL;
  }

  // Load the header, and verify it
  PSF1_HEADER *font_header;
  SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER),
                                          (void **)&font_header);
  UINTN header_size = sizeof(PSF1_HEADER);
  font->Read(font, &header_size, font_header);

  if (font_header->magic[0] != PSF1_MAGIC0 ||
      font_header->magic[1] != PSF1_MAGIC1) {
    Print(L"  > %s: ERROR: Invalid font format \n\r", __func__);
    return NULL;
  }

  // Load the glyphs
  UINTN glyph_buffer_size = font_header->charsize * 256;
  if (font_header->mode == 1) {
    // 512 glyph mode
    glyph_buffer_size = font_header->charsize * 512;
  }

  void *glyph_buffer;
  {
    font->SetPosition(font, sizeof(PSF1_HEADER));
    SystemTable->BootServices->AllocatePool(EfiLoaderData, glyph_buffer_size,
                                            (void **)&glyph_buffer);
    font->Read(font, &glyph_buffer_size, glyph_buffer);
  }

  // Fill the font structure
  PSF1_FONT *finished_font;
  SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT),
                                          (void **)&finished_font);
  finished_font->psf1_header = font_header;
  finished_font->glyph_buffer = glyph_buffer;

  return finished_font;
}

/// Compare two memory regions of size \p n
///
/// \param l The left pointer
/// \param r The other memory region
/// \param n The size of region to compare
int memcmp(const void *l, const void *r, size_t n) {
  const unsigned char *lptr = l;
  const unsigned char *rptr = r;

  for (size_t i = 0; i < n; i++) {
    if (lptr[i] < rptr[i]) {
      return -1;
    } else if (lptr[i] > rptr[i]) {
      return 1;
    }
  }

  return 0;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  // Initialize resources
  InitializeLib(ImageHandle, SystemTable);

  // Clear Screen on start
  gST->ConOut->ClearScreen(gST->ConOut);

  Print(L"== BBN OS UEFI Loader == \n\r");

  // Load the kernel from EFI partition
  EFI_FILE *Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
  if (Kernel == NULL) {
    Print(L"==> ERROR: Cannot load kernel \n\r");
  } else {
    Print(L"==> Successfully loaded kernel \n\r");
  }

  // Get the kernel header (ELF Header)
  Elf64_Ehdr header;
  {
    UINTN FileInfoSize;
    EFI_FILE_INFO *FileInfo;

    // First, we need to get the size of `FILE_INFO` struct
    Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);

    // Then we allocate size for FileInfo the get such info
    SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize,
                                            (void **)&FileInfo);
    Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize,
                    (void **)&FileInfo);

    UINTN size = sizeof(header);
    uefi_call_wrapper(Kernel->Read, 3, Kernel, &size, &header);
  }

  // Check if ther kernel can be executed
  if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
      header.e_ident[EI_CLASS] != ELFCLASS64 ||
      header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC ||
      header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
    Print(L"==> Current Format BAD \n\r");
  } else {
    Print(L"==> Kernel Format Verified \n\r");
  }

  // Get the kernel's Program Header, describing segments
  Elf64_Phdr *phdrs;
  {
    Kernel->SetPosition(Kernel, header.e_phoff);
    UINTN size = header.e_phnum * header.e_phentsize;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, size,
                                            (void **)&phdrs);
    Kernel->Read(Kernel, &size, phdrs);
  }

  // For each segments, we need to put them to corresponding positions
  for (Elf64_Phdr *phdr = phdrs;
       (char *)phdr < (char *)phdrs + header.e_phnum * header.e_phentsize;
       phdr = (Elf64_Phdr *)((char *)phdr + header.e_phentsize)) {
    switch (phdr->p_type) {

    // PT_LOAD: Loadable entry in the segment table
    case PT_LOAD: {
      // Calculate how many pages this segment requires
      int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;

      // Get the address to load this segment
      Elf64_Addr segment = phdr->p_paddr;

      // Allocate memory at the given address
      SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData,
                                               pages, &segment);

      // Set the (file pointer) to the beginning of the segment
      Kernel->SetPosition(Kernel, phdr->p_offset);

      // Get the segment size in file
      UINTN size = phdr->p_filesz;

      // Load this segment to memory
      Kernel->Read(Kernel, &size, (void *)segment);
      Print(L"PT_LOAD: %X, size %X \n\r", phdr->p_paddr, phdr->p_filesz);
      break;
    }
    }
  }

  Print(L"==> Kernel Loaded, do other initializations before jumping... \n\r");

  PSF1_FONT *newFont =
      LoadPSF1Font(NULL, L"default_font.psf", ImageHandle, SystemTable);
  if (newFont == NULL) {
    Print(L"ERROR: Loading Default font failed. \n\r");
  } else {
    Print(L"==> Font loaded. char size: %d\n\r",
          newFont->psf1_header->charsize);
  }

  // Get the framebuffer
  Framebuffer *newBuffer = InitializeGOP();
  {
    // If we get the buffer, print the information about the buffer
    if (newBuffer != NULL) {
      Print(L"  > Base 0x%X\n\r  > Size 0x%X\n\r  > Width %d\n\r  > Height "
            L"%d\n\r  > PixelsPerScanline "
            L"%d\n\r",
            newBuffer->BaseAddress, newBuffer->BufferSize, newBuffer->Width,
            newBuffer->Height, newBuffer->PixelsPerScanline);
    } else {
      Print(L"PANIC: No framebuffer available \n\r");

      // we should panic, or quit here
      return EFI_ABORTED;
    }
  }

  EFI_MEMORY_DESCRIPTOR *MMap = NULL;
  UINTN MapSize, MapKey;
  UINTN DescriptorSize;
  UINT32 DescriptorVersion;
  EFI_STATUS GetMMapRet;
  {
    MapSize = 0;
    GetMMapRet = gBS->GetMemoryMap(&MapSize, MMap, &MapKey, &DescriptorSize,
                                   &DescriptorVersion);
    ASSERT(GetMMapRet == EFI_BUFFER_TO_SMALL);

    // Loop here, if the pool is too small, we re-allocate it and re-do the
    // get
    do {
      SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize,
                                              (void **)&MMap);
      ASSERT(MMap != NULL);

      GetMMapRet = SystemTable->BootServices->GetMemoryMap(
          &MapSize, MMap, &MapKey, &DescriptorSize, &DescriptorVersion);
      if (GetMMapRet != EFI_BUFFER_TOO_SMALL && EFI_ERROR(GetMMapRet)) {
        Print(L"!!! GetMemoryMap Error! Code: %d !!! \n\r", GetMMapRet);
        FreePool(MMap);
        return EFI_ABORTED;
      }

      if (GetMMapRet == EFI_BUFFER_TOO_SMALL) {
        Print(L"!! ==> EFI Buffer Too small, retrying... \n\r");
        FreePool(MMap);
      }
    } while (GetMMapRet == EFI_BUFFER_TOO_SMALL);
  }

  Print(L"==> Calling kernel \n\r");

  // Fill bootinfo
  BootInfo bootInfo;
  bootInfo.framebuffer = newBuffer;
  bootInfo.PSF1_Font = newFont;
  bootInfo.MMap = MMap;
  bootInfo.MMapSize = MapSize;
  bootInfo.MMapDescSize = DescriptorSize;

  SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

  // Start the kernel
  // create a function pointer, which is the entry point of kernel
  void (*KernelStart)(BootInfo *) =
      ((__attribute__((sysv_abi)) void (*)(BootInfo *))header.e_entry);

  KernelStart(&bootInfo);

  // The kernel will take over, never here
  while (1) {
  }

  return EFI_SUCCESS;
}
