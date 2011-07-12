#pragma once

#include "xboxtools.h"
#include "XeCrypt.h"

typedef unsigned __int64		u64;
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;

typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;
typedef signed __int64          s64;

typedef unsigned __int64		QWORD;

typedef long			NTSTATUS;

#define STATUS_SUCCESS	0
#define NT_EXTRACT_ST(Status)			((((ULONG)(Status)) >> 30)& 0x3)
#define NT_SUCCESS(Status)              (((NTSTATUS)(Status)) >= 0)
#define NT_INFORMATION(Status)          (NT_EXTRACT_ST(Status) == 1)
#define NT_WARNING(Status)              (NT_EXTRACT_ST(Status) == 2)
#define NT_ERROR(Status)                (NT_EXTRACT_ST(Status) == 3)

// Valid values for the Attributes field
#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_VALID_ATTRIBUTES    0x000001F2L

typedef struct _STRING {
	USHORT Length;
	USHORT MaximumLength;
	PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _OBJECT_ATTRIBUTES {
    HANDLE RootDirectory;
    PSTRING ObjectName;
    ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } st;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef VOID (NTAPI *PIO_APC_ROUTINE) (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
);

typedef struct _CONSOLE_PUBLIC_KEY {
   BYTE PublicExponent[0x04];
   BYTE Modulus[0x80];
} CONSOLE_PUBLIC_KEY, *PCONSOLE_PUBLIC_KEY;

typedef struct _XE_CONSOLE_CERTIFICATE {
   WORD				  CertSize;
   BYTE				  ConsoleId[0x05];
   BYTE				  ConsolePartNumber[0x0B];
   BYTE				  Reserved[0x04];
   WORD				  Privileges;
   DWORD			  ConsoleType;
   BYTE				  ManufacturingDate[0x08];
   CONSOLE_PUBLIC_KEY ConsolePublicKey;
   BYTE				  Signature[0x100];
} XE_CONSOLE_CERTIFICATE, *PXE_CONSOLE_CERTIFICATE;

typedef struct _VERSION_KERNEL {
	WORD Major;
	WORD Minor;
	WORD Build;
	BYTE ApprovalType;
	BYTE QFE;
} VERSION_KERNEL, *PVERSION_KERNEL;

EXTERN_C {

	extern PVERSION_KERNEL XboxKrnlVersion;

	DWORD XexLoadImage(CHAR* XexName, DWORD TypeInfo, DWORD Version, HANDLE* modHandle);
	VOID KeSetCurrentProcessType(DWORD ProcessType);
	void RtlInitAnsiString(PSTRING DestinationString, const char*  SourceString);

	NTSTATUS
	NTAPI
	NtOpenFile(
		OUT		PHANDLE FileHandle,
		IN		ACCESS_MASK DesiredAccess,
		IN		POBJECT_ATTRIBUTES ObjectAttributes,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		ULONG ShareAccess,
		IN		ULONG OpenOptions
		);

	NTSTATUS
	NTAPI
	NtDeviceIoControlFile(
		IN		HANDLE FileHandle,
		IN		HANDLE Event OPTIONAL,
		IN		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN		PVOID ApcContext OPTIONAL,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		ULONG IoControlCode,
		IN		PVOID InputBuffer OPTIONAL,
		IN		ULONG InputBufferLength,
		OUT		PVOID OutputBuffer OPTIONAL,
		IN		ULONG OutputBufferLength
		);

	NTSTATUS
	NTAPI
	NtClose(
		IN		HANDLE Handle
		);

	int ObCreateSymbolicLink( STRING*, STRING*);
	int ObDeleteSymbolicLink( STRING* );

	void HalReturnToFirmware(int);

	BOOL MmIsAddressValid(ULONGLONG Address);

	int VdGetCurrentDisplayInformation(BYTE* DisplayInformation);

	// Xex exports
	UINT32 __stdcall NtSetSystemTime( __int64*  ,__int64* );
	HRESULT __stdcall ExGetXConfigSetting(u16 categoryNum, u16 settingNum, void* outputBuff, s32 outputBuffSize, u16* settingSize);
	HRESULT __stdcall ExSetXConfigSetting(u16 categoryNum, u16 settingNum, void* outputBuff, u16 settingSize);

	DWORD ExCreateThread(PHANDLE pHandle, DWORD dwStackSize, LPDWORD lpThreadId, VOID* apiThreadStartup, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlagsMod);
	void  XapiThreadStartup(void (__cdecl *StartRoutine)(void*), void* StartContext);
	UINT32 XexGetModuleHandle(char* module, PVOID hand); //ie XexGetModuleHandle("xam.xex", &hand);// uint32 hand, returns 0 on success
	UINT32 XexGetProcedureAddress(UINT32 hand, UINT32, PVOID);// ie XexGetProcedureAddress(hand ,0x50, &addr); // uint32 addr, returns 0 on success

	int XeKeysGetConsoleType(WORD* ConsoleType);
	
	// Profile Exports
	int XeKeysUnObfuscate(int ObfuscationKey, char* encryptedBuffer, int encryptedLength, char* outputBuffer, int* outputLength);
	int XeKeysObfuscate(int ObfuscationKey, char* decryptedBuffer, int decryptedLength, char* outputBuffer, int* outputLength);
	DWORD XeKeysGetKey(DWORD keyNum, VOID* buffer, DWORD* bufferSize);
	
	// Compression
	typedef struct _LZX_DECOMPRESS {
		LONG WindowSize;
		LONG CpuType;
	} LZX_DECOMPRESS, *PLZX_DECOMPRESS;

	DWORD LDICreateDecompression(DWORD* pcbDataBlockMax, 
		LZX_DECOMPRESS* pvConfiguration,
		DWORD pfnma, DWORD pfnmf,
		VOID* pcbSrcBufferMin,
		DWORD* unknow, DWORD* pcbDecompressed);
	DWORD LDIDecompress(DWORD context, BYTE* pbSrc, WORD cbSrc,
		BYTE* pdDst, DWORD* pcbDecompressed);
	DWORD LDIDestroyDecompression(DWORD contect);
/*
	// XeCrypt RSA
	typedef struct _XECRYPT_RSA {
		DWORD           cqw;
		DWORD           dwPubExp;
		QWORD           qwReserved;
	} XECRYPT_RSA, *PXECRYPT_RSA;
	typedef struct _XECRYPT_RSAPUB_2048 {
		XECRYPT_RSA     Rsa;
		QWORD           aqwM[32];
	} XECRYPT_RSAPUB_2048, *PXECRYPT_RSAPUB_2048;
	typedef struct _XECRYPT_SIG {
		BYTE Sig[0x100];
	} XECRYPT_SIG, *PXECRYPT_SIG;

	QWORD XeCryptBnQwNeModInv(QWORD qw);
	VOID  XeCryptBnQw_Copy(QWORD * pqwInp, QWORD * pqwOut, DWORD cqw);
	VOID  XeCryptBnQwNeModMul(QWORD * pqwA, QWORD * pqwB, 
		QWORD * pqwC, QWORD qwMI, QWORD * pqwM, DWORD cqw);
	VOID  XeCryptBnQwBeSigFormat(XECRYPT_SIG * pSig, BYTE * pbHash, 
		BYTE * pbSalt);

	// XeCrypt AES
	typedef struct _XECRYPT_AES_STATE{
		BYTE                keytabenc[11][4][4];
		BYTE                keytabdec[11][4][4];
	} XECRYPT_AES_STATE, *PXECRYPT_AES_STATE;
	VOID  XeCryptAesKey(XECRYPT_AES_STATE * pAesState, BYTE * pbKey);
	VOID  XeCryptAesCbc(XECRYPT_AES_STATE * pAesState, BYTE * pbInp, 
		DWORD cbInp, BYTE * pbOut, BYTE * pbFeed, BOOL fEncrypt);

	// XeCrypt SHA
	VOID  XeCryptSha(BYTE * pbInp1, DWORD cbInp1, 
		BYTE * pbInp2, DWORD cbInp2, 
		BYTE * pbInp3, DWORD cbInp3, 
		BYTE * pbOut, DWORD cbOut);
	VOID  XeCryptRotSumSha(BYTE * pbInp1, DWORD cbInp1, BYTE * pbInp2, 
		DWORD cbInp2, BYTE * pbOut, DWORD cbOut);
		*/
}