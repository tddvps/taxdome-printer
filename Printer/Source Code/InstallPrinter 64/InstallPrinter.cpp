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

// InstallPrinter.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "InstallPrinter.h"
#include "InstallPrinterDlg.h"

#ifdef _DEBUG 
#define new DEBUG_NEW
#endif


// CInstallPrinterApp

BEGIN_MESSAGE_MAP(CInstallPrinterApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// CInstallPrinterApp construction
/// <summary>
/// Constructor of this class 
/// </summary>
CInstallPrinterApp::CInstallPrinterApp()
{
	
}


// The one and only CInstallPrinterApp object

CInstallPrinterApp theApp;


// CInstallPrinterApp initialization
/// <summary>
/// This will be called when we open the EXE file  - We have to compile with Both Modes x64 and Win32 
/// </summary>
/// <returns>BOOL true or false</returns>
BOOL CInstallPrinterApp::InitInstance()
{
	InitCommonControls();

	
	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CString sCMD = m_lpCmdLine;
	//AfxMessageBox(sCMD);
	CInstallPrinterDlg dlg;
	m_pMainWnd = &dlg;
	if (sCMD == "Install")
		dlg.m_bInstall = true;
	else 
		dlg.m_bInstall = false;

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
