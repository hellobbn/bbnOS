#include <efi.h>
#include <efibind.h>
#include <efidef.h>
#include <efilib.h>
#include <efiprot.h>

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
  Print(L"Root opened \n\r");

  EFI_STATUS ret =
      uefi_call_wrapper(Directory->Open, 5, Directory, &LoadedFile, Path,
                        EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
  if (ret != EFI_SUCCESS) {
    return NULL;
  }

  return LoadedFile;
}

EFI_STATUS
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
  InitializeLib(ImageHandle, SystemTable);

  SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, NULL);

  Print(L"== BBN OS UEFI Loader == \n\r");

  if (LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable) == NULL) {
    Print(L"==> ERROR: Cannot load kernel \n\r");
  } else {
    Print(L"==> Successfully loaded kernel \n\r");
  }

  while (1) {
  }

  return EFI_SUCCESS;
}
