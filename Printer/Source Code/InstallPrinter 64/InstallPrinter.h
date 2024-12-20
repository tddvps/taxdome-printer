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

// InstallPrinter.h : main header file for the PROJECT_NAME application
//

#pragma once
#include "stdafx.h"
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


// CInstallPrinterApp:
// See InstallPrinter.cpp for the implementation of this class
//
/// <summary>
/// This is the Main class derived from CWinApp 
/// </summary>
class CInstallPrinterApp : public CWinApp
{
public:
	CInstallPrinterApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CInstallPrinterApp theApp;