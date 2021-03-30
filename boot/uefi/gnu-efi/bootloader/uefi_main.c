#include <efi.h>
#include <efibind.h>
#include <efidef.h>
#include <efilib.h>
#include <efiprot.h>
#include <elf.h>

typedef unsigned long long size_t;

typedef struct {
  void *BaseAddress;
  size_t BufferSize;
  unsigned int Width;
  unsigned int Height;
  unsigned int PixelsPerScanline;
} Framebuffer;

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

  // Disable watchdog
  SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

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
      break;
    }
    }
  }

  Print(L"==> Kernel Loaded, jump to kernel... \n\r");

  // create a function pointer, which is the entry point of kernel
  int (*KernelStart)(Framebuffer *) =
      ((__attribute__((sysv_abi)) int (*)(Framebuffer *))header.e_entry);

  Framebuffer *newBuffer = InitializeGOP();

  if (newBuffer != NULL) {
    Print(L"  > Base 0x%X\n\r  > Size 0x%X\n\r  > Width %d\n\r  > Height "
          L"%d\n\r  > PixelsPerScanline "
          L"%d\n\r\n\r",
          newBuffer->BaseAddress, newBuffer->BufferSize, newBuffer->Width,
          newBuffer->Height, newBuffer->PixelsPerScanline);
  }

  Print(L"==> Calling kernel \n\r");
  KernelStart(newBuffer);
  while (1) {
  }

  return EFI_SUCCESS;
}
