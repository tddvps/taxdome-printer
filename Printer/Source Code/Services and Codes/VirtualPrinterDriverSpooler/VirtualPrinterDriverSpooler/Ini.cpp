/*
 * Project Name: TaxDome Printer 2.0
 * Copyright (C) 2021 TaxDome LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Author: TaxDome LLC
 */


#include "stdafx.h" // include if you got "fatal error C1010: unexpected end of file..."
#include "Ini.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define DEF_PROFILE_NUM_LEN		64 // numeric string length, could be quite long for binary format
#define DEF_PROFILE_THRESHOLD	512 // temporary string length
#define DEF_PROFILE_DELIMITER	_T(",") // default string delimiter
#define DEF_PROFILE_TESTSTRING	_T("{63788286-AE30-4D6B-95DF-3B451C1C79F9}") // Uuid for internal use

// struct used to be passed to __KeyPairProc as a LPVOID parameter
struct STR_LIMIT
{
	LPTSTR lpTarget;
	DWORD dwRemain;
	DWORD dwTotalCopied;
};

/////////////////////////////////////////////////////////////////////////////////
// Constructors & Destructor
/////////////////////////////////////////////////////////////////////////////////
CIni::CIni()
{
	m_pszPathName = NULL;
}

CIni::CIni(LPCTSTR lpPathName)
{
	m_pszPathName = NULL;
	SetPathName(lpPathName);
}

CIni::~CIni()
{
	if (m_pszPathName != NULL)
		delete [] m_pszPathName;
}

/////////////////////////////////////////////////////////////////////////////////
// Ini File Path Access
/////////////////////////////////////////////////////////////////////////////////

// Assign ini file path name
void CIni::SetPathName(LPCTSTR lpPathName)
{
	if (lpPathName == NULL)
	{
		if (m_pszPathName != NULL)
			*m_pszPathName = _T('\0');
	}
	else
	{
		if (m_pszPathName != NULL)
			delete [] m_pszPathName;

		m_pszPathName = _tcsdup(lpPathName);
	}
}

// Write a string value to the ini file
BOOL CIni::WriteString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue) const
{
	if (lpSection == NULL || lpKey == NULL)
		return FALSE;

	return ::WritePrivateProfileString(lpSection, lpKey, lpValue == NULL ? _T("") : lpValue, m_pszPathName);
}

/////////////////////////////////////////////////////////////////////////////////
// Raw String Access
/////////////////////////////////////////////////////////////////////////////////

// Get a profile string value, if the buffer size is not large enough, the result
// may be truncated.
DWORD CIni::GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDefault) const
{
	if (lpBuffer != NULL)
		*lpBuffer = _T('\0');

	LPTSTR psz = __GetStringDynamic(lpSection, lpKey, lpDefault);
	DWORD dwLen = _tcslen(psz);

	if (lpBuffer != NULL)
	{
		lstrcpy(lpBuffer, psz);
		dwLen = min(dwLen, dwBufSize);
	}

	delete [] psz;
	return dwLen;
}

#ifdef __AFXWIN_H__
CString CIni::GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault) const
{
	LPTSTR psz = __GetStringDynamic(lpSection, lpKey, lpDefault);
	CString str(psz);
	delete [] psz;
	return str;
}
#endif


/////////////////////////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////////////////////////

// Get a profile string value, return a heap pointer so we do not have to worry
// about the buffer size, however, this function requires the caller to manually
// free the memory.
// This function is the back-bone of all "Getxxx" functions of this class.
LPTSTR CIni::__GetStringDynamic(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault) const
{
	TCHAR* psz = NULL;
	if (lpSection == NULL || lpKey == NULL)
	{
		// Invalid section or key name, just return the default string
		if (lpDefault == NULL)
		{
			// Empty string
			psz = new TCHAR[1];
			*psz = _T('\0');
		}
		else
		{
			psz = new TCHAR[_tcslen(lpDefault) + 1];
			lstrcpy(psz, lpDefault);
		}
		
		return psz;
	}
	
	// Keep enlarging the buffer size until being certain on that the string we
	// retrieved was original(not truncated).
	DWORD dwLen = DEF_PROFILE_THRESHOLD;
	psz = new TCHAR[dwLen + 1];
	DWORD dwCopied = ::GetPrivateProfileString(lpSection, lpKey, lpDefault == NULL ? _T("") : lpDefault, psz, dwLen, m_pszPathName);
	while (dwCopied + 1 >= dwLen)
	{		
		dwLen += DEF_PROFILE_THRESHOLD;
		delete [] psz;
		psz = new TCHAR[dwLen + 1];
		dwCopied = ::GetPrivateProfileString(lpSection, lpKey, lpDefault == NULL ? _T("") : lpDefault, psz, dwLen, m_pszPathName);
	}
	
	return psz; // !!! Requires the caller to free this memory !!!
}
