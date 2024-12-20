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

// InstallPrinterDlg.h : header file
//

#pragma once
#include "LogFile.h"

// CInstallPrinterDlg dialog
/// <summary>
/// This class will be called from Main to handle the install/remove printer - m_bInstall = true for install - m_bInstall = false for uninstall
/// </summary>
class CInstallPrinterDlg : public CDialog
{
// Construction
public:
	
	BOOL m_bInstall;
	CInstallPrinterDlg(CWnd* pParent = NULL);	// standard constructor
	BOOL CreateRSPFile();
	CString GetINIPath();
// Dialog Data
	enum { IDD = IDD_INSTALLPRINTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	

// Implementation
protected:
	HICON m_hIcon;
	
	// Generated message map functions
	virtual BOOL OnInitDialog();
	
	DECLARE_MESSAGE_MAP()
public:
	
	afx_msg void OnBnClickedButtonStep1();
	afx_msg void OnBnClickedButtonStep2();
	afx_msg void OnBnClickedButtonStep3();
	afx_msg void OnBnClickedButtonStep4();
	afx_msg void OnBnClickedButtonStep7();
	afx_msg void OnBnClickedButtonStep8();
	afx_msg void OnBnClickedButtonStep5();
	afx_msg void OnBnClickedButtonStep6();
};
