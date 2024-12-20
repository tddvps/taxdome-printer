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



#ifndef __INI_H__
#define __INI_H__

#include <windows.h>
#include <tchar.h>

// If MFC is linked, we will use CStringArray for great convenience
#ifdef __AFXWIN_H__
	#include <afxtempl.h>
#endif

// Number bases
#define BASE_BINARY			2
#define BASE_OCTAL			8
#define BASE_DECIMAL		10
#define BASE_HEXADECIMAL	16

//---------------------------------------------------------------
//	    Callback Function Type Definition
//---------------------------------------------------------------
// The callback function used for parsing a "double-null terminated string".
// When called, the 1st parameter passed in will store the newly extracted sub
// string, the 2nd parameter is a 32-bit user defined data, this parameter can
// be NULL. The parsing will terminate if this function returns zero. To use
// the callback, function pointer needs to be passed to "CIni::ParseDNTString".
typedef BOOL (CALLBACK *SUBSTRPROC)(LPCTSTR, LPVOID);

class CIni
{
public:		

	//-----------------------------------------------------------
	//    Constructors & Destructor
	//-----------------------------------------------------------
	CIni(); // Default constructor
	CIni(LPCTSTR lpPathName); // Construct with a given file name
	virtual ~CIni();

	//-----------------------------------------------------------
	//    Ini File Path Name Access
	//-----------------------------------------------------------
	void SetPathName(LPCTSTR lpPathName); // Specify a new file name
	
	//------------------------------------------------------------
	//    String Access
	//------------------------------------------------------------	
	DWORD GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPTSTR lpBuffer, DWORD dwBufSize, LPCTSTR lpDefault = NULL) const;
#ifdef __AFXWIN_H__
	CString GetString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault = NULL) const;
#endif
	BOOL WriteString(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpValue) const;
	

	
		
protected:	

	//------------------------------------------------------------
	//    Helper Functions
	//------------------------------------------------------------

	LPTSTR __GetStringDynamic(LPCTSTR lpSection, LPCTSTR lpKey, LPCTSTR lpDefault = NULL) const;
	//------------------------------------------------------------
	//    Member Data
	//------------------------------------------------------------
	LPTSTR m_pszPathName; // Stores path of the associated ini file
};

#endif // #ifndef __INI_H__