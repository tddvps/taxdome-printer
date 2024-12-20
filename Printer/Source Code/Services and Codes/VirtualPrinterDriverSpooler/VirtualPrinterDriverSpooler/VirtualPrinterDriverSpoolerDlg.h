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


// VirtualPrinterDriverSpoolerDlg.h : header file
//

#pragma once
#include "resource.h"
#include "LogFile.h"
#include "Ini.h"
// CVirtualPrinterDriverSpoolerDlg dialog
/// <summary>
/// This class will be called from Main to handle the the current spool and pass the file to the application
/// </summary>
class CVirtualPrinterDriverSpoolerDlg : public CDialogEx
{
// Construction
	
public:
	CVirtualPrinterDriverSpoolerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VirtualPrinterDriverSpoolerDLG_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	
	CIni m_IniUser;
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	
};