#pragma once

typedef char BOOLEAN;
typedef long INTN;
typedef unsigned long UINTN;
typedef char INT8;
typedef unsigned char UINT8;
typedef short INT16;
typedef unsigned short UINT16;
typedef int INT32;
typedef unsigned int UINT32;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef unsigned char CHAR8;
typedef unsigned short CHAR16;
typedef void VOID;

#define IN 
#define OUT
#define OPTIONAL
#define CONST const
#define EFIAPI

struct EFI_GUID {
        UINT32  d1;
        UINT16  d2;
        UINT16  d3;
        UINT8   d4[8];
};
typedef UINTN EFI_STATUS;
typedef void * EFI_HANDLE;

typedef struct {
        UINT64      Signature;
        UINT32      Revision;
        UINT32      HeaderSize;
        UINT32      CRC32;
        UINT32      Reserved;
 } EFI_TABLE_HEADER;

#define EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL_GUID \
        {0x387477c2,0x69c7,0x11d2,\
         {0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b}}

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (*EFI_TEXT_STRING) (
        struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
        CHAR16 *String);

typedef EFI_STATUS (*EFI_TEXT_CLEAR_SCREEN) (
        struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
        unsigned long long      todo1[1];
        EFI_TEXT_STRING         OutputString;
        unsigned long long      todo2[4];
        EFI_TEXT_CLEAR_SCREEN   ClearScreen;
        unsigned long           todo3[3];
};

typedef struct {
        EFI_TABLE_HEADER                Hdr;
        CHAR16                          *FirmwareVendor;
        UINT32                          FirmwareRevision;
        EFI_HANDLE                      ConsoleInHandle;
        void                            *ConIn; // TODO: EFI_SIMPLE_TEXT_INPUT_PROTOCOL
        EFI_HANDLE                      ConsoleOutHandle;
        struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *ConOut;
        EFI_HANDLE                      StandardErrorHandle;
        struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *StdErr;
        void                            *RuntimeServices; // TODO: EFI_RUNTIME_SERVICES
        void                            *BootServices; // TODO: EFI_BOOT_SERVICES
        UINTN                            NumberOfTableEntries;
        void                            *ConfigurationTable; // TODO: EFI_CONFIGURATION_TABLE
} EFI_SYSTEM_TABLE;