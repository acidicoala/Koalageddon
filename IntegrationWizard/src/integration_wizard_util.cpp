#include "pch.h"
#include "integration_wizard_util.h"
#include "Logger.h"
#include <accctrl.h>
#include <aclapi.h>
#include <sddl.h>

// Source: https://docs.microsoft.com/en-us/windows/win32/secauthz/enabling-and-disabling-privileges-in-c--
bool setPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	LUID luid;

	if(!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		logger->error("Failed to LookupPrivilegeValue. Error code: 0x{:X}", GetLastError());
		return false;
	}

	TOKEN_PRIVILEGES tp = { NULL };
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.
	if(!AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		(PTOKEN_PRIVILEGES) NULL,
		(PDWORD) NULL))
	{
		logger->error("Failed to AdjustTokenPrivileges. Error code: 0x{:X}", GetLastError());
		return false;
	}

	if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		logger->error("The token does not have the specified privilege");
		return false;
	}

	return true;
}

bool changeOwnership(LPCWSTR objectPath, WELL_KNOWN_SID_TYPE wellKnownSidType)
{
	DWORD result = NULL;

	WCHAR objPath[MAX_PATH]; // Copy string into a buffer, since some functions do not accept const ptr
	wcscpy_s(objPath, objectPath);
#pragma warning(disable : 6054) // VS incorrectly warns that the string might not be terminated.


	// Open a handle to the access token for the calling process.
	HANDLE hToken = NULL;
	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		logger->error("Failed to OpenProcessToken. Error code: 0x{:X}", GetLastError());
		return false;
	}

	// Enable the SE_RESTORE_NAME privilege.
	if(!setPrivilege(hToken, SE_RESTORE_NAME, TRUE))
	{
		logger->error("Failed to enable 'SE_RESTORE_NAME' privilege. Error code: 0x{:X}", GetLastError());
		return false;
	}

	// Enable the SE_TAKE_OWNERSHIP_NAME privilege.
	if(!setPrivilege(hToken, SE_TAKE_OWNERSHIP_NAME, TRUE))
	{
		logger->error("Failed to enable 'SE_TAKE_OWNERSHIP_NAME' privilege. Error code: 0x{:X}", GetLastError());
		return false;
	}

	// Initialize SID
	PSID pSIDOwner = malloc(MAX_SID_SIZE);
	DWORD cbSid = MAX_SID_SIZE;
	if(!CreateWellKnownSid(wellKnownSidType, NULL, pSIDOwner, &cbSid))
	{
		logger->error("Failed to CreateWellKnownSid. Error code: 0x{:X}", GetLastError());
		return false;
	}

	// Get existing permissions. Source: https://stackoverflow.com/a/696357/3805929
	ACL* pOldDACL;
	SECURITY_DESCRIPTOR* pSD = NULL;
	result = GetNamedSecurityInfo(objPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, &pOldDACL, 0, (void**) &pSD);
	if(result != ERROR_SUCCESS)
	{
		logger->error("Failed to GetSecurityInfo. Error code: {}", result);
		return false;
	}
	EXPLICIT_ACCESS ea = { NULL };
	ea.grfAccessMode = GRANT_ACCESS;
	ea.grfAccessPermissions = GENERIC_ALL;
	ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.ptstrName = (LPTSTR) pSIDOwner;

	// Create new DACL from existing permissions
	ACL* pNewDACL = 0;
	result = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
	if(result != ERROR_SUCCESS)
	{
		logger->error("Failed to SetEntriesInAcl. Error code: {}", result);
		return false;
	}

	// Finally, apply the permissions
	result = SetNamedSecurityInfo(objPath, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, pNewDACL, 0);
	if(result != ERROR_SUCCESS)
	{
		logger->error("Failed to SetNamedSecurityInfo with DACL_SECURITY_INFORMATION. Error code: {}", result);
		return false;
	}

	logger->debug(L"Successfully changed owner & permissions of '{}'", objectPath);

	return true;
}


void fatalError(string message)
{
	message = fmt::format("{}. Error code: 0x{:X}", message, GetLastError());
	MessageBoxA(NULL, message.c_str(), "Fatal Error", MB_ICONERROR | MB_OK);
	exit(1);
}

wstring getEnvVar(wstring key)
{
	TCHAR buffer[MAX_PATH];
	GetEnvironmentVariable(key.c_str(), buffer, MAX_PATH);

	return absolute(buffer);
}

path getProgramDataPath()
{
	LPWSTR wszPath = NULL;
	HRESULT result = SHGetKnownFolderPath(FOLDERID_ProgramData, NULL, NULL, &wszPath);
	if(result == S_OK)
	{
		return absolute(wszPath);
	}
	else
	{
		fatalError(fmt::format("Failed to get ProgramData path. Error code: {}", result));
		return "";
	}
}

path getDesktopPath()
{
	return absolute(getEnvVar(L"UserProfile")) / "Desktop";
}

HRESULT createShortcut(wstring targetLocation, wstring shortcutLocation, wstring description)
{
	HRESULT hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(SUCCEEDED(hres))
	{
		IShellLink* psl = NULL;

		// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
		// has already been called.
		hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*) &psl);
		if(SUCCEEDED(hres))
		{
			IPersistFile* ppf = NULL;

			// Set the path to the shortcut target and add the description. 
			psl->SetPath(targetLocation.c_str());
			psl->SetDescription(description.c_str());

			// Query IShellLink for the IPersistFile interface, used for saving the 
			// shortcut in persistent storage. 
			hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*) &ppf);

			if(SUCCEEDED(hres))
			{
				// Save the link by calling IPersistFile::Save. 
				hres = ppf->Save(shortcutLocation.c_str(), TRUE);
				ppf->Release();
			}
			psl->Release();
		}
		CoUninitialize();
	}

	return hres;
}
