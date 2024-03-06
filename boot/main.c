#include "efi.h"

void efi_main(void *ImageHandle __attribute__ ((unused)),
              EFI_SYSTEM_TABLE *SystemTable) {
        SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello UEFI!\n");
        while (1);
}
