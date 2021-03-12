#pragma once

// Source: https://github.com/blaquee/dllnotif/blob/master/LdrDllNotification/ntapi.h

#include <winnt.h>

typedef __success(return >= 0) LONG NTSTATUS;

#ifndef NT_STATUS_OK
#define NT_STATUS_OK 0
#endif


#define STATUS_SUCCESS ((NTSTATUS)0)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001)
#define STATUS_PROCEDURE_NOT_FOUND ((NTSTATUS)0xC000007A)
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004)
#define STATUS_NOT_FOUND ((NTSTATUS)0xC0000225)
#define STATUS_THREAD_IS_TERMINATING ((NTSTATUS)0xc000004b)
#define STATUS_NOT_SUPPORTED ((NTSTATUS)0xC00000BB)

enum LDR_DLL_NOTIFICATION_REASON
{
	LDR_DLL_NOTIFICATION_REASON_LOADED = 1,
	LDR_DLL_NOTIFICATION_REASON_UNLOADED = 2,
};

typedef struct tag_UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} __UNICODE_STRING, * PCUNICODE_STRING; // Removed * PUNICODE_STRING to avoid conflicts with PLH's PEB.hpp

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA
{
	ULONG Flags;                    //Reserved.
	PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
	PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
	PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
	ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_LOADED_NOTIFICATION_DATA, * PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA
{
	ULONG Flags;                    //Reserved.
	PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
	PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
	PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
	ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, * PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA
{
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, * PLDR_DLL_NOTIFICATION_DATA;
typedef const LDR_DLL_NOTIFICATION_DATA* PCLDR_DLL_NOTIFICATION_DATA;


typedef VOID(CALLBACK* PLDR_DLL_NOTIFICATION_FUNCTION)(
	_In_     ULONG                       NotificationReason,
	_In_     PLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID                       Context
);

typedef NTSTATUS(NTAPI* _LdrRegisterDllNotification)(
	_In_     ULONG                          Flags,
	_In_     PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
	_In_opt_ PVOID                          Context,
	_Out_    PVOID* cookie
);

typedef NTSTATUS(NTAPI* _LdrUnregisterDllNotification)(
	_In_ PVOID cookie
);
