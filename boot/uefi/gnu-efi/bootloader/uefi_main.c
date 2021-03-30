#include <efi.h>
#include <efibind.h>
#include <efidef.h>
#include <efilib.h>
#include <efiprot.h>
#include <elf.h>

typedef unsigned long long size_t;

EFI_FILE *LoadFile(EFI_FILE *Directory, CHAR16 *Path, EFI_HANDLE ImageHandle,
                   EFI_SYSTEM_TABLE *SystemTable) {
  EFI_FILE *LoadedFile = NULL;

  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
  SystemTable->BootServices->HandleProtocol(
      ImageHandle, &gEfiLoadedImageProtocolGuid, (void **)&LoadedImage);

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle,
                                            &gEfiSimpleFileSystemProtocolGuid,
                                            (void **)&FileSystem);

  if (Directory == NULL) {
    Directory = LibOpenRoot(LoadedImage->DeviceHandle);
  }

  EFI_STATUS ret =
      uefi_call_wrapper(Directory->Open, 5, Directory, &LoadedFile, Path,
                        EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
  if (ret != EFI_SUCCESS) {
    return NULL;
  }

  return LoadedFile;
}

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
  InitializeLib(ImageHandle, SystemTable);

  SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

  Print(L"== BBN OS UEFI Loader == \n\r");

  EFI_FILE *Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);

  if (Kernel == NULL) {
    Print(L"==> ERROR: Cannot load kernel \n\r");
  } else {
    Print(L"==> Successfully loaded kernel \n\r");
  }

  Elf64_Ehdr header;
  {
    UINTN FileInfoSize;
    EFI_FILE_INFO *FileInfo;
    Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
    SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize,
                                            (void **)&FileInfo);
    Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize,
                    (void **)&FileInfo);

    UINTN size = sizeof(header);
    uefi_call_wrapper(Kernel->Read, 3, Kernel, &size, &header);
  }

  if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
      header.e_ident[EI_CLASS] != ELFCLASS64 ||
      header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC ||
      header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
    Print(L"==> Current Format BAD \n\r");
  } else {
    Print(L"==> Kernel Format Verified \n\r");
  }

  Elf64_Phdr *phdrs;
  {
    Kernel->SetPosition(Kernel, header.e_phoff);
    UINTN size = header.e_phnum * header.e_phentsize;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, size,
                                            (void **)&phdrs);
    Kernel->Read(Kernel, &size, phdrs);
  }

  for (Elf64_Phdr *phdr = phdrs;
       (char *)phdr < (char *)phdrs + header.e_phnum * header.e_phentsize;
       phdr = (Elf64_Phdr *)((char *)phdr + header.e_phentsize)) {
    switch (phdr->p_type) {
    case PT_LOAD: {
      int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
      Elf64_Addr segment = phdr->p_paddr;
      SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData,
                                               pages, &segment);

      Kernel->SetPosition(Kernel, phdr->p_offset);
      UINTN size = phdr->p_filesz;
      Kernel->Read(Kernel, &size, (void *)segment);
      break;
    }
    }
  }

  Print(L"==> Kernel Loaded, jump to kernel... \n\r");

  // create a function pointer, which is the entry point of kernel
  int (*KernelStart)() = ((__attribute__((sysv_abi)) int (*)()) header.e_entry);

  Print(L"<==Kernel Returned: %d \n\r", KernelStart());

  while (1) {
  }

  return EFI_SUCCESS;
}
