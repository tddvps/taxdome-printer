#include "stdafx.h"
#include "InstallPrinter.h"
#include "InstallPrinterDlg.h"
#include <windows.h>
#include <winspool.h>
#include ".\installprinterdlg.h"
#include <io.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CLogFile g_log;
// CAboutDlg dialog used for App About

CString m_strAppPath;

char sysdir[256];
char exepath[256];
char source[256];
char destination[256];

char buffer[4096];
DWORD needed, returned;
unsigned int i;
MONITOR_INFO_1 *mi;
MONITOR_INFO_2 mi2;
HKEY hkey, hsubkey;
LONG rc;

TCHAR title[256];
TCHAR monitorname[64];
char *p;
//BOOL is_winnt = FALSE;
//BOOL is_win4 = FALSE;

#define MONITORNAME TEXT("VirtualPrinterDriver Monitor")
#define MONITORDLLNT TEXT("VirtualPrinterDriverQBnt64.dll")
#define MONITORDLLNT_X86 TEXT("VirtualPrinterDriverQBnt.dll")

#define MONITORENVNT TEXT("Windows NT x86")

#define MONITORKEY TEXT("VirtualPrinterDriver Port Monitor")
 
#define UNINSTALLPROG TEXT("UNVirtualPrinterDriverQB.exe")
#define UNINSTALLKEY TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall")

#define PORT_KEY TEXT("SYSTEM\\CurrentControlSet\\Control\\Print\\Monitors\\VirtualPrinterDriver Monitor\\Ports\\UTVirtualPrinterDriverPort:")
#define UNINSTALLSTRINGKEY TEXT("UninstallString")
#define DISPLAYNAMEKEY TEXT("DisplayName")

#define GHOSTSCRIPT_KEY1 TEXT("SOFTWARE\\AFPL Ghostscript")
#define GHOSTSCRIPT_KEY2 TEXT("SOFTWARE\\AFPL Ghostscript\\8.53")

#define GHOSTSCRIPT_KEY3 TEXT("SOFTWARE\\Wow6432Node\\AFPL Ghostscript")
#define GHOSTSCRIPT_KEY4 TEXT("SOFTWARE\\Wow6432Node\\AFPL Ghostscript\\8.53")


CString INSTALL = "TRUE"; // Comment for Remove64.exe 
//CString INSTALL = "FALSE";	// Comment for InstallPrinter64.exe


char strshortPath[MAX_PATH];
#define Environment  TEXT("Windows x64")//Windows x64
#define EnvironmentX86  TEXT("Windows NT x86")//Windows x64

/// <summary>
/// Generic Function - Used to Add Monitor in the Printer Driver 
/// </summary>
/// <returns>BOOL true or false</returns>
BOOL AddMonitorLocal(void)
{
	/*DWORD version = GetVersion();
	if ((HIWORD(version) & 0x8000)==0)
		is_winnt = TRUE;
	if (LOBYTE(LOWORD(version)) >= 4)
		is_win4 = TRUE;*/

	lstrcpy(title,"VirtualPrinterDriver Port Monitor");
	lstrcpy(monitorname,"VirtualPrinterDriver Monitor");


	mi2.pName = monitorname;
#if defined( _WIN64 )
	mi2.pEnvironment = Environment;
#else
	mi2.pEnvironment = EnvironmentX86;
#endif

	

#if defined( _WIN64 )
	mi2.pDLLName = MONITORDLLNT;
#else
	mi2.pDLLName = MONITORDLLNT_X86;
#endif

	

	/* Check if already installed */
	if (EnumMonitors(NULL, 1, (LPBYTE)buffer, sizeof(buffer), 	&needed, &returned)) 
	{
		mi = (MONITOR_INFO_1 *)buffer;
		for (i=0; i<returned; i++) {
			if (lstrcmp(mi[i].pName, monitorname) == 0)
			{
				g_log.AddRecord("Monitor already available");
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}

	/* copy files to Windows system directory */
	if (!GetSystemDirectory(sysdir, sizeof(sysdir)))
	{
		//return FALSE; // In 64 Bit System Permission error. So we have copied this in the installer itself 
	}

	lstrcat(sysdir, "\\");

	CString strmonitorPath;
	strmonitorPath = m_strAppPath + "Xtra\\Prerequisite\\kbmon\\";

	lstrcpy(exepath,strmonitorPath);//

	lstrcpy(destination, sysdir);
	lstrcat(destination, mi2.pDLLName);
	lstrcpy(source, exepath);
	lstrcat(source, mi2.pDLLName);
	
	if (!CopyFile(source, destination, FALSE))
	{	
	
	}

	lstrcpy(destination, sysdir);
	lstrcat(destination, UNINSTALLPROG);
	lstrcpy(source, exepath);
	lstrcat(source, UNINSTALLPROG);
	
	if (!AddMonitor(NULL, 2, (LPBYTE)&mi2)) {
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL );
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf );
		

		return FALSE;
	}

	return TRUE;
}


/// <summary>
/// Generic Function - Used to add port to the Printer Driver 
/// </summary>
/// <returns>BOOL true or false</returns>
// Adding the Port 
BOOL AddPortLocal(void)
{
	DWORD cbneed,cbstate;
    PBYTE pOutputData;
	HANDLE hXcv = INVALID_HANDLE_VALUE;
	PRINTER_DEFAULTS Defaults = { NULL,NULL,SERVER_ACCESS_ADMINISTER };		
	
	WCHAR pszPortName[]=L"UTVirtualPrinterDriverPort:"; 
    pOutputData=(PBYTE)malloc(MAX_PATH); 

	if(!OpenPrinter(",XcvMonitor VirtualPrinterDriver Monitor",&hXcv,&Defaults ))
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL );
			free(pOutputData);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf );
		return FALSE;

   	}
	if(!XcvData(hXcv,L"AddPort",(PBYTE)pszPortName,sizeof(pszPortName),(PBYTE)pOutputData,MAX_PATH,&cbneed,&cbstate))
	{
		LPVOID lpMsgBuf; 
		SetLastError(cbstate);
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL ); 
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf ); 
		free(pOutputData);

	} 

	//free(pOutputData);
    ClosePrinter(hXcv);
	
	return TRUE;
}
/// <summary>
/// Generic Function - CopyPrintDriverFiles2System - Used to Copy returied files to system folder for Printer Driver creation
/// </summary>
/// <returns>BOOL true or false</returns>
BOOL CopyPrintDriverFiles2System(void)
{
	CString source, dest;
	
	char lpBuffer[MAX_PATH]; 
	DWORD uSize=0; 

	BOOL b;
#if defined( _WIN64 )

	// Windows 64 bit code here

	b = GetPrinterDriverDirectory(NULL, Environment, 1, (LPBYTE)lpBuffer, MAX_PATH, &uSize);

#else
	
	b = GetPrinterDriverDirectory(NULL, EnvironmentX86, 1, (LPBYTE)lpBuffer, MAX_PATH, &uSize);

#endif

	dest = lpBuffer;
#if defined( _WIN64 )
	source = m_strAppPath +  "Xtra\\Prerequisite\\Driver64" ; 
#else
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver";
#endif
	dest += TEXT("\\PSCRIPT5.DLL");
	source += TEXT("\\PSCRIPT5.DLL");

	if (!CopyFile(source, dest, FALSE))
		return FALSE;

	dest = lpBuffer;
#if defined( _WIN64 )
	source = m_strAppPath +  "Xtra\\Prerequisite\\Driver64" ;
#else
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver";
#endif
	dest += TEXT("\\PSCRIPT.NTF");
	source += TEXT("\\PSCRIPT.NTF");
	if (!CopyFile(source, dest, FALSE))
		return FALSE;

	dest = lpBuffer;

#if defined( _WIN64 )
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver64";
#else
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver";
#endif
	dest += TEXT("\\PS5UI.DLL");
	source += TEXT("\\PS5UI.DLL");
	if (!CopyFile(source, dest, FALSE))
		return FALSE;

	dest = lpBuffer;


	
#if defined( _WIN64 )
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver64";
#else
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver";
#endif

	dest += TEXT("\\PSCRIPT.HLP");
	source += TEXT("\\PSCRIPT.HLP");
	if (!CopyFile(source, dest, FALSE))
		return FALSE;
		

	dest = lpBuffer;

#if defined( _WIN64 )
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver64";
#else
	source = m_strAppPath + "Xtra\\Prerequisite\\Driver";
#endif
	dest += TEXT("\\max_english.PPD");
	source += TEXT("\\max_english.PPD");
	if (!CopyFile(source, dest, FALSE))
		return FALSE;
	return TRUE;

}
/// <summary>
/// Generic Function - AddPrinterDriver - Used to add and inform the OS that printer driver for our printer
/// </summary>
/// <returns>BOOL true or false</returns>
BOOL AddPrinterDriver(void)
{

	DRIVER_INFO_3 di3; 
	char lpDriverPath[MAX_PATH]; 
	char lpBuffer[MAX_PATH]; 
	DWORD uSize=0; 

	BOOL b;
#if defined( _WIN64 )
	b= ::GetPrinterDriverDirectory(NULL, Environment, 1, (LPBYTE)lpBuffer, MAX_PATH, &uSize);
#else
	b = ::GetPrinterDriverDirectory(NULL, EnvironmentX86, 1, (LPBYTE)lpBuffer, MAX_PATH, &uSize);
#endif
	lstrcpy(TEXT(lpDriverPath),TEXT(lpBuffer)); 
	lstrcat(TEXT(lpDriverPath),TEXT("\\PSCRIPT5.DLL")); 

	char lpDataFilePath[MAX_PATH]; 
	lstrcpy(TEXT(lpDataFilePath),TEXT(lpBuffer)); 
	lstrcat(TEXT(lpDataFilePath),TEXT("\\max_english.PPD")); 

	char lpConfigFilePath[MAX_PATH]; 
	lstrcpy(TEXT(lpConfigFilePath),TEXT(lpBuffer)); 
	lstrcat(TEXT(lpConfigFilePath),TEXT("\\PS5UI.DLL"));
 
	char lpHelpFilePath[MAX_PATH]; 
	lstrcpy(TEXT(lpHelpFilePath),TEXT(lpBuffer)); 
	lstrcat(TEXT(lpHelpFilePath),TEXT("\\PSCRIPT.HLP"));; 
	
	ZeroMemory(&di3, sizeof(DRIVER_INFO_3)); 
	di3.cVersion = 0x03; 
	di3.pConfigFile =lpConfigFilePath; 
	di3.pDataFile =lpDataFilePath; 	
	di3.pDependentFiles = TEXT("PSCRIPT.NTF\0\0");
	di3.pDriverPath = lpDriverPath; 
#if defined( _WIN64 )
	di3.pEnvironment = Environment;
#else
	di3.pEnvironment = EnvironmentX86;
#endif
	
	di3.pHelpFile = lpHelpFilePath;
	di3.pMonitorName = NULL; 
	di3.pName = TEXT("UTVirtualPrinterDriverDriver");
	di3.pDefaultDataType = TEXT("RAW");

	if(!AddPrinterDriver(NULL, 3, (LPBYTE)&di3))
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL ); 
		//::MessageBox(NULL,(LPCTSTR)lpMsgBuf,"ERROR",MB_OK|MB_ICONINFORMATION);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf ); 
		
		return FALSE;
	}
	return TRUE;
}


/// <summary>
/// Generic Function - AddPrinter - Final stage of the Driver creation. This will create the given Printer name in the control panel
/// </summary>
/// <returns>BOOL true or false</returns>
BOOL AddPrinter(void)
{
	DWORD  dwValue =0;
	PRINTER_INFO_2 pi2;
	HANDLE pHD;
	
	ZeroMemory(&pi2, sizeof(PRINTER_INFO_2)); 
	pi2.pServerName= NULL; 
	
	pi2.pPrinterName = TEXT("TaxDome Printer 2.0");  
	pi2.pPortName = TEXT("UTVirtualPrinterDriverPort:"); 
	pi2.pDriverName = TEXT("UTVirtualPrinterDriverDriver");
	pi2.pPrintProcessor = TEXT("WinPrint"); 
	pi2.pDatatype = NULL; 
	if((pHD=AddPrinter(NULL, 2, (LPBYTE)&pi2))==NULL)
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL ); 	
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf ); 
		

		return FALSE;
	}
	ClosePrinter(pHD);

	if ((rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, PORT_KEY, 0, KEY_ALL_ACCESS, &hkey)) != ERROR_SUCCESS) 
	{
		/* failed to open key, so try to create it */
		rc = RegCreateKey(HKEY_LOCAL_MACHINE, PORT_KEY, &hkey);
	}
	if (rc == ERROR_SUCCESS) 
	{
		//m_strAppPath
		CString strrspfile; 
		CString strshortpath = strshortPath;
		strrspfile = "@" +strshortpath + "Xtra\\Prerequisite\\kbgs\\pdfwrite.rsp -";	
		lstrcpy(buffer, strrspfile);
		RegSetValueEx(hkey, TEXT("Arguments"), 0, REG_SZ, (CONST BYTE *)buffer, lstrlen(buffer)+1);

		strrspfile = m_strAppPath + "Xtra\\Prerequisite\\kbgs\\kbbin\\bin\\gswin32c.exe";	

		lstrcpy(buffer, strrspfile);
		RegSetValueEx(hkey, TEXT("Command"), 0, REG_SZ,(CONST BYTE *)buffer, lstrlen(buffer)+1);

		dwValue =2;
		RegSetValueEx(hkey, TEXT("ShowWindow"), 0, REG_DWORD,(CONST BYTE *)&dwValue, 4);

		dwValue =0;
		RegSetValueEx(hkey, TEXT("RunUser"), 0, REG_DWORD,(CONST BYTE *)&dwValue, 4);

		dwValue =300;
		RegSetValueEx(hkey, TEXT("Delay"), 0, REG_DWORD,(CONST BYTE *)&dwValue, 4);

		RegCloseKey(hkey);
	}
	else
		return FALSE;

	return TRUE;

}

/// <summary>
/// Generic Function - UpdateRegInfo - This will add the required registry information for all users
/// </summary>
/// <returns>BOOL true or false</returns>
/// 
BOOL UpdateRegInfo(void)
{
	//#define GHOSTSCRIPT_KEY1 TEXT("SOFTWARE\\AFPL Ghostscript")
	if ((rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY1, 0, KEY_ALL_ACCESS, &hkey)) != ERROR_SUCCESS) 
	{
		/* failed to open key, so try to create it */
		rc = RegCreateKey(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY1, &hkey);
	}

	//#define GHOSTSCRIPT_KEY2 TEXT("SOFTWARE\\AFPL Ghostscript\\8.53")
	if ((rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY2, 0, KEY_ALL_ACCESS, &hkey)) != ERROR_SUCCESS) 
	{
		/* failed to open key, so try to create it */
		rc = RegCreateKey(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY2, &hkey);
	}

	if (rc == ERROR_SUCCESS) 
	{

		CString strrspfile = m_strAppPath + "Xtra\\PD\\gsdll32.dll";

		lstrcpy(buffer, strrspfile);
		
		RegSetValueEx(hkey, TEXT("GS_DLL"), 0, REG_SZ, (CONST BYTE *)buffer, lstrlen(buffer)+1);

		strrspfile = m_strAppPath + "Xtra\\Prerequisite\\kbgs\\kbbin\\lib;" +m_strAppPath + "Xtra\\Prerequisite\\kbgs\\fonts;"+m_strAppPath + "Xtra\\Prerequisite\\kbgs\\kbbin\\Resource" ;	
		lstrcpy(buffer, strrspfile);
		RegSetValueEx(hkey, TEXT("GS_LIB"), 0, REG_SZ,(CONST BYTE *)buffer, lstrlen(buffer)+1);

		RegCloseKey(hkey);
	}
	else
		return FALSE;


	if ((rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY3, 0, KEY_ALL_ACCESS, &hkey)) != ERROR_SUCCESS) 
	{
		/* failed to open key, so try to create it */
		rc = RegCreateKey(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY3, &hkey);
	}

	if ((rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY4, 0, KEY_ALL_ACCESS, &hkey)) != ERROR_SUCCESS) 
	{
		/* failed to open key, so try to create it */
		rc = RegCreateKey(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY4, &hkey);
	}

	if (rc == ERROR_SUCCESS) 
	{

		CString strrspfile = m_strAppPath + "Xtra\\PD\\gsdll32.dll";

		lstrcpy(buffer, strrspfile);
		
		RegSetValueEx(hkey, TEXT("GS_DLL"), 0, REG_SZ, (CONST BYTE *)buffer, lstrlen(buffer)+1);

		strrspfile = m_strAppPath + "Xtra\\Prerequisite\\kbgs\\kbbin\\lib;" +m_strAppPath + "Xtra\\Prerequisite\\kbgs\\fonts;"+m_strAppPath + "Xtra\\Prerequisite\\kbgs\\kbbin\\Resource" ;	
		lstrcpy(buffer, strrspfile);
		RegSetValueEx(hkey, TEXT("GS_LIB"), 0, REG_SZ,(CONST BYTE *)buffer, lstrlen(buffer)+1);

		RegCloseKey(hkey);
	}
	else
		return FALSE;

	return TRUE;
}

/// <summary>
/// Generic Function - DeletePrinter - This will be called when we unistall the application
/// </summary>
/// <returns>BOOL true or false</returns>
/// 
BOOL DeletePrinter(void)
{
	BOOL bRet = FALSE;
	PRINTER_DEFAULTS  pd;
	HANDLE  pHd;

	ZeroMemory(&pd, sizeof(PRINTER_DEFAULTS)); 
	pd.DesiredAccess = PRINTER_ALL_ACCESS;

	
	bRet = OpenPrinter(TEXT("TaxDome Printer 2.0"), &pHd, &pd);  
	if ( !bRet )
		return bRet;
	bRet = DeletePrinter( pHd );
	
	if ( !bRet)
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			  NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL ); 
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf ); 
		
	}
	ClosePrinter( pHd );
	return bRet;
}
/// <summary>
/// Generic Function - DeletePrinterDriver - This will be called when we unistall the application - Driver files will be uninstall from the OS
/// </summary>
/// <returns>BOOL true or false</returns>
/// 
BOOL DeletePrinterDriver(void)
{
	BOOL bRet = TRUE;

#if defined( _WIN64 )
	if (!DeletePrinterDriver(NULL, Environment/*TEXT("Windows NT x86")*/, TEXT("UTVirtualPrinterDriverDriver")))
	{
		LPVOID lpMsgBuf;
		GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL, (LPTSTR)&lpMsgBuf, 0, NULL);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		bRet = FALSE;
}
#else
	if (!DeletePrinterDriver(NULL, EnvironmentX86/*TEXT("Windows NT x86")*/, TEXT("UTVirtualPrinterDriverDriver")))
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL, (LPTSTR)&lpMsgBuf, 0, NULL);

		LocalFree(lpMsgBuf);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		bRet = FALSE;
	}
#endif


	return bRet;
}

/// <summary>
/// Generic Function - DeletePortLocal - Will remove installed port while uninstall
/// </summary>
/// <returns>BOOL true or false</returns>
/// 
BOOL DeletePortLocal(HWND hd)
{
	BOOL bRet = TRUE;
	if ( !DeletePort(NULL, hd,TEXT("UTVirtualPrinterDriverPort:")))
	{
		LPVOID lpMsgBuf; 
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL,(LPTSTR) &lpMsgBuf, 0, NULL );  
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree( lpMsgBuf ); 
		
		bRet = FALSE;
	}
	return bRet;
}
/// <summary>
/// Generic Function - DeleteMonitor - Will remove installed monitor while uninstall
/// </summary>
/// <returns>BOOL true or false</returns>
/// 
BOOL DeleteMonitorLocal(void)
{
	BOOL bRet = TRUE;
#if defined( _WIN64 )
	if (!DeleteMonitor(NULL, Environment/*TEXT("Windows NT x86")*/, TEXT("VirtualPrinterDriver Monitor")))
	{
		LPVOID lpMsgBuf;
		GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL, (LPTSTR)&lpMsgBuf, 0, NULL);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		bRet = FALSE;
}
#else
	if (!DeleteMonitor(NULL, EnvironmentX86/*TEXT("Windows NT x86")*/, TEXT("VirtualPrinterDriver Monitor")))
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL, (LPTSTR)&lpMsgBuf, 0, NULL);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		bRet = FALSE;
	}
#endif
	
	return bRet;
}

/// <summary>
/// Generic Function - DeleteRegInfo - Will remove added registry information from the system while uninstall
/// </summary>
/// <returns>BOOL true or false</returns>
/// 
BOOL DeleteRegInfo(void)
{
	if ((rc = RegDeleteKey(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY2)) != ERROR_SUCCESS) 
	{
		rc = RegDeleteKey(HKEY_LOCAL_MACHINE, GHOSTSCRIPT_KEY1);
	}

	if (rc == ERROR_SUCCESS) 
		return TRUE;
	else
		return FALSE;

}

// CInstallPrinterDlg dialog


/// <summary>
/// Construtor for CInstallPrinterDlg class 
/// </summary>
/// <param name="pParent">Handle for the mainframe</param>
CInstallPrinterDlg::CInstallPrinterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInstallPrinterDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInstall = true;
}
/// <summary>
/// DoDataExchange for CInstallPrinterDlg class - Holds the events
/// </summary>
/// <param name="pParent">Handle for the mainframe</param>
/// <returns>void - NULL returs</returns>
void CInstallPrinterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CInstallPrinterDlg, CDialog)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_STEP1, OnBnClickedButtonStep1)
	ON_BN_CLICKED(IDC_BUTTON_STEP2, OnBnClickedButtonStep2)
	ON_BN_CLICKED(IDC_BUTTON_STEP3, OnBnClickedButtonStep3)
	ON_BN_CLICKED(IDC_BUTTON_STEP4, OnBnClickedButtonStep4)
	ON_BN_CLICKED(IDC_BUTTON_STEP7, OnBnClickedButtonStep7)
	ON_BN_CLICKED(IDC_BUTTON_STEP8, OnBnClickedButtonStep8)
	ON_BN_CLICKED(IDC_BUTTON_STEP5, OnBnClickedButtonStep5)
	ON_BN_CLICKED(IDC_BUTTON_STEP6, OnBnClickedButtonStep6)
END_MESSAGE_MAP()

/// <summary>
/// CreateRSPFile - Created commandline param for GS while will be passed for each printer driver calling
/// </summary>
/// <returns>BOOL-TRUE or FALSE</returns>
// CInstallPrinterDlg message handlers
BOOL CInstallPrinterDlg::CreateRSPFile()
{
	
	
	FILE *fp;
	CString strFile = m_strAppPath + "Xtra\\Prerequisite\\kbgs\\pdfwrite.rsp";

	CString sFolder = m_strAppPath + "Xtra\\Prerequisite\\kbgs\\";

	if (_access(sFolder, 0) == -1)
	{
		CreateDirectory(m_strAppPath + "Xtra\\", NULL); CreateDirectory(m_strAppPath + "Xtra\\Prerequisite\\", NULL); CreateDirectory(m_strAppPath + "Xtra\\Prerequisite\\kbgs\\", NULL);
	}

	fopen_s(&fp,strFile,"w");

	
	if (fp)
	{
		CString strshortpath1 = strshortPath;
		CString strVal = "-I" + strshortpath1 + "Xtra\\Prerequisite\\kbgs\\kbbin\\lib;" + strshortpath1 + "Xtra\\Prerequisite\\kbgs\\fonts";

		fprintf(fp, "%s\n", strVal.GetBuffer(strVal.GetLength()));

		fprintf(fp, " -sDEVICE=pdfwrite\n");
		fprintf(fp, " -r600\n");
		fprintf(fp, " -dNOPAUSE\n");
		fprintf(fp, " -dSAFER\n");
		fprintf(fp, " -dAutoRotatePages=/All\n");
		fprintf(fp, " -dPDFFitPage\n");

		fprintf(fp, " -sPAPERSIZE=letter\n");

		strVal = " -sOutputFile=";
		strVal += char(34);
		strVal += m_strAppPath + "Xtra\\VPDriver.PDF" + char(34);

		fprintf(fp, "%s\n", strVal.GetBuffer(strVal.GetLength()));

		fclose(fp);
	}
	else
		return false;
	return TRUE;

}


/// <summary>
/// OnInitDialog - this class function when we call install or uninstall
/// </summary>
/// <returns>BOOL-TRUE or FALSE</returns>
BOOL CInstallPrinterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_bInstall == TRUE)
		INSTALL = "TRUE";
	else 
		INSTALL = "FALSE";

	
	char dirname[MAX_PATH];
	long len = -1, t = 0;

	CString strINIPath = GetINIPath();

	if (_access(strINIPath, 0) == -1)
	{
		CreateDirectory(strINIPath, NULL);
	}

	// Create folder with date
	CTime tm = CTime::GetCurrentTime();
	CString time = CTime::GetCurrentTime().Format("%H.%M.%S");
	CString strTime = tm.Format("%m.%d.%Y");

	CString strINIFile = strINIPath + "VirtualPrinterDriver.log";


	g_log.Set_cFileName(strINIFile);

	//AfxMessageBox(strINIFile);


	GetModuleFileName(AfxGetApp()->m_hInstance, dirname, MAX_PATH);
	if (strstr(dirname, "\\") != NULL)
	{
		len = (long)strlen(dirname);
		_strrev(dirname);
		CString sVal = strstr(dirname, "\\");

		t = sVal.GetLength();
		_strrev(dirname);
		dirname[t] = 0;
	}

	m_strAppPath = dirname;


	GetShortPathName( m_strAppPath,strshortPath,MAX_PATH);

	if (INSTALL == "TRUE") 
	{
		g_log.Set_cModuleName("InstallPrinter");
		g_log.Set_cProcName("OnInitDialog");

#if defined( _WIN64 )
		g_log.AddRecord("Installing Print Driver 64 Bit  .... called from the Deployment");
#else
		g_log.AddRecord("Installing Print Driver 32 Bit  .... called from the Deployment");
#endif
		


		g_log.AddRecord("Trying to create PDF creator settings");

		CreateRSPFile();
		OnBnClickedButtonStep1();

		OnBnClickedButtonStep2();

		OnBnClickedButtonStep3();

		OnBnClickedButtonStep4();
	}
	else 
	{
#if defined( _WIN64 )
		g_log.Set_cModuleName("UninstallPrinter 64 Bit");
#else
		g_log.Set_cModuleName("UninstallPrinter 32 Bit");
#endif

		

		g_log.AddRecord("Uninstalling Print Driver .... called from the Deployment");

		OnBnClickedButtonStep5();

		OnBnClickedButtonStep6();

		OnBnClickedButtonStep7();

		OnBnClickedButtonStep8();

	}
	exit (0);

	

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


/// <summary>
/// OnBnClickedButtonStep1 - Creation of the Monitor will be called while installing
/// </summary>
//Install a Monitor
void CInstallPrinterDlg::OnBnClickedButtonStep1()
{
	CString msg="Failed";
	g_log.Set_cProcName("Monitor Creation");

	if (AddMonitorLocal())
	{
		g_log.AddRecord("Monitor creation completed");
		msg="Add Monitor Successfully";
	}
	if (msg == "Failed") 
	{
		g_log.AddRecord("Monitor creation already done or failed");
	}


	return;
}

/// <summary>
/// OnBnClickedButtonStep2 - Creation of the Port will be called while installing
/// </summary>
//Install a Port
void CInstallPrinterDlg::OnBnClickedButtonStep2()
{
	CString msg="Failed";
	g_log.Set_cProcName("Port Creation");

	if (AddPortLocal())
	{
		msg="Add Port Successfully";
		g_log.AddRecord("Port creation completed");
	}
	if (msg == "Failed") 
	{
		g_log.AddRecord("Port already created or failed");
	}
	

	return;
}

/// <summary>
/// OnBnClickedButtonStep3 - Addding the Printer Driver while installing
/// </summary>
//Install Driver
void CInstallPrinterDlg::OnBnClickedButtonStep3()
{
	g_log.Set_cProcName("PrinterDriver");
	CString msg="Failed";
	if (CopyPrintDriverFiles2System() && AddPrinterDriver())
	{
		g_log.AddRecord("Added Printer Driver to the system");
		msg = "Add Printer Driver Successfully";
	}
	if (msg == "Failed") 
	{
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, GetLastError(), NULL, (LPTSTR)&lpMsgBuf, 0, NULL);
		//::MessageBox(NULL, (LPCTSTR)lpMsgBuf, "ERROR", MB_OK | MB_ICONINFORMATION);
		g_log.AddRecord((LPTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		
		g_log.AddRecord("Printer Driver generation failed");
	}

    return ;
}



/// <summary>
/// OnBnClickedButtonStep4 - Creation of the Printer Driver while installing
/// </summary>
//add virtual printer
void CInstallPrinterDlg::OnBnClickedButtonStep4()
{
	CString msg="Failed";
	g_log.Set_cProcName("Printercreation");

	if (AddPrinter() && UpdateRegInfo())
	{
		msg = "Add Printer Successfully";
		g_log.AddRecord("Printer - VirtualPrinterDriver - Created sucessfully");

	}
	if (msg == "Failed") 
	{
		g_log.AddRecord("Printer - VirtualPrinterDriver - Failed");
	}

    return ;
}

/// <summary>
/// OnBnClickedButtonStep7 - Delete Port while uninstalling
/// </summary>
//Delete Port
void CInstallPrinterDlg::OnBnClickedButtonStep7()
{
	CString msg="Failed";
	g_log.Set_cProcName("Delete Port");

	if (DeletePortLocal(GetSafeHwnd()))
	{
		msg="Delete Port Successfully";
		g_log.AddRecord("Port Delete completed");
	}
	if (msg == "Failed") 
	{

	}

	return;
}

/// <summary>
/// OnBnClickedButtonStep8 - Delete Monitor while uninstalling
/// </summary>
//Delete Monitor
void CInstallPrinterDlg::OnBnClickedButtonStep8()
{
	CString msg="Failed";
	g_log.Set_cProcName("DeleteMonitor");

	if (DeleteMonitorLocal())
	{
		g_log.AddRecord("Delete Monitor completed");
		msg="Delete Monitor Successfully";
	}
	if (msg == "Failed") 
	{

	}

	return;
}


/// <summary>
/// OnBnClickedButtonStep5 - Delete Printer while uninstalling
/// </summary>
//Delete Printer
void CInstallPrinterDlg::OnBnClickedButtonStep5()
{
	CString msg="Failed";
	g_log.Set_cProcName("Delete Printer");
	if (DeletePrinter())
	{
		g_log.AddRecord("Delete Printer completed");
		msg="Delete Printer Successfully";
	}
	if (msg == "Failed") {
	
	}
	return;

}
/// <summary>
/// OnBnClickedButtonStep6 - Delete Printer Driver while uninstalling
/// </summary>
//Delete Printer
void CInstallPrinterDlg::OnBnClickedButtonStep6()
{
	CString msg="Failed";
	g_log.Set_cProcName("Uninstall Printer");
	if (DeletePrinterDriver())
		msg="Delete Printer Driver Successfully";
	
	if (msg == "Failed") 
	{
		
	}
	

	return;
}
/// <summary>
/// GetINIPath - Getting the LOG file folder 
/// </summary>
/// <param name="filename">No param required</param>
/// <returns>String - Full log folder path</returns>
CString CInstallPrinterDlg::GetINIPath()
{
	CString strval;
	TCHAR dirname[MAX_PATH];
	int len,t;

	GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, dirname, MAX_PATH);
	if (strstr(dirname, "\\") != NULL)
	{
		len = (long)strlen(dirname);
		_strrev(dirname);
		CString sVal = strstr(dirname, "\\");

		t = sVal.GetLength();
		//t = (long)strlen(strstr(dirname, "\\"));
		_strrev(dirname);
		dirname[t] = 0;
	}
	 
	strval = dirname;

	strval += "LOG\\";
	return strval;
}
