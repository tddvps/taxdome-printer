
// VirtualPrinterDriverSpoolerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VirtualPrinterDriverSpooler.h"
#include "VirtualPrinterDriverSpoolerDlg.h"
#include "afxdialogex.h"
#include <WtsApi32.h>
#include <UserEnv.h>
#include <tlhelp32.h>
#include <io.h>
long GetFileSize(const TCHAR* fileName);
#pragma comment(lib,"WtsApi32.lib")
#pragma comment(lib,"UserEnv.lib")


CString g_sFullPath, g_sUserName;
CString g_strDocumentName, g_sTotalPages;
CString LoadSpoolFile(LPWSTR lpzFileName);
#include <winsvc.h>

CLogFile glog;
CString g_sDebugType;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <WinSpool.h>

CString g_strVal;
int g_nNumber = 0;
#include <Winspool.h >
#include <io.h>
//#include "base64.h"
#define MAX_COMMAND 32768


#include <windows.h>
#include <exception>
 /// <summary>
///  Generic Function - GetLastErrorStdStr - Used to record if any error while calling system APIs
/// </summary>
/// <param name="filename">No param required</param>
/// <returns>String - Full error description</returns>
 // Create a string with last error message
CString GetLastErrorStdStr(DWORD error)
{
	if (error)
	{
		LPVOID lpMsgBuf;
		DWORD bufLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&lpMsgBuf,
			0, NULL);
		if (bufLen)
		{
			LPWSTR lpMsgStr = (LPWSTR)lpMsgBuf;
			CString result(lpMsgStr);

			LocalFree(lpMsgBuf);

			return result;
		}
	}
	return L"";
}

/// <summary>
///  Generic Function - Is_WinXPOrAbove - Get OS version information according to the return value we will call the required APIS
/// </summary>
/// <param name="filename">No param required</param>
/// <returns>BOOL true or false</returns>
/// 
BOOL Is_WinXPOrAbove()
{
	return true;// Since this EXE will only run Above Windows 7 (because it is compiled in VS 2019
   /*OSVERSIONINFOEXW osvi = {0};

   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

   GetVersionEx((LPOSVERSIONINFOW)&osvi);

   return (
	   osvi.dwMajorVersion > 5 || (
		   osvi.dwMajorVersion == 5 &&
		   osvi.dwMinorVersion >= 1)
	   );*/
}

typedef BOOL(WINAPI* PFNWTSQUERYUSERTOKEN)(ULONG, PHANDLE);

static PFNWTSQUERYUSERTOKEN fnWTSQueryUserToken = NULL;

/// <summary>
///  Generic Function - GetWTSQueryUserToken - Getting User token to call the application with this user token - since we have system token 
/// </summary>
/// <param name="filename">No param required</param>
/// <returns>user token</returns>
/// 
static PFNWTSQUERYUSERTOKEN GetWTSQueryUserToken()
{
	if (fnWTSQueryUserToken)
		return fnWTSQueryUserToken;

	HMODULE hMod = GetModuleHandleW(L"wtsapi32.dll");
	if (!hMod)
		hMod = LoadLibraryW(L"wtsapi32.dll");
	if (!hMod)
		return NULL;

	fnWTSQueryUserToken = (PFNWTSQUERYUSERTOKEN)GetProcAddress(hMod, "WTSQueryUserToken");

	return fnWTSQueryUserToken;
}

/// <summary>
///  Generic Function - GetExplorerToken - Enumerate User token to call the application with this user token - since we have system token 
/// </summary>
/// <param name="dwSessionId">currentsessionid</param>
/// <param name="phToken">token will return as pointer</param>
/// <returns>BOOL true or false</returns>
/// 
static BOOL GetExplorerToken(DWORD dwSessionId, HANDLE* phToken)
{
	DWORD winlogonSessId, explorerPid = 0;
	PROCESSENTRY32W procEntry;
	BOOL bRet = FALSE;
	HANDLE hProcess = 0, hPToken = 0, hPTokenDup = 0;
	DWORD dwErr = 0;
	_ASSERT(phToken);
	HANDLE hSnap;


	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE)
	{
		dwErr = GetLastError();
		goto exit;
	}

	procEntry.dwSize = sizeof(PROCESSENTRY32W);

	if (!Process32FirstW(hSnap, &procEntry))
	{
		dwErr = GetLastError();
		goto exit;
	}

	do
	{
		if (_wcsicmp(procEntry.szExeFile, L"explorer.exe") == 0)
		{
			if (ProcessIdToSessionId(procEntry.th32ProcessID, &winlogonSessId)
				&& winlogonSessId == dwSessionId)
			{
				//found explorer.exe running into this session
				explorerPid = procEntry.th32ProcessID;
				break;
			}
		}
	} while (Process32NextW(hSnap, &procEntry));

	if (!explorerPid)
		goto exit;

	if ((hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, explorerPid)) == NULL)
	{
		dwErr = GetLastError();
		goto exit;
	}

	if (!OpenProcessToken(hProcess,
		TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE,
		&hPToken))
	{
		dwErr = GetLastError();
		goto exit;
	}

	if (!DuplicateTokenEx(hPToken, MAXIMUM_ALLOWED, NULL,
		SecurityIdentification, TokenPrimary, &hPTokenDup))
	{
		dwErr = GetLastError();
		goto exit;
	}

	if (!SetTokenInformation(hPTokenDup,
		TokenSessionId, (void*)dwSessionId, sizeof(DWORD)))
	{
		dwErr = GetLastError();
		goto exit;
	}

	*phToken = hPTokenDup;
	bRet = TRUE;

	goto exit;
exit:
	if (dwErr)
	{
		glog.Set_cModuleName(L"Spool");
		glog.Set_cProcName(L"GetExplorerToken");
		glog.AddRecordU(GetLastErrorStdStr(dwErr));
	}

	if (hSnap)
		CloseHandle(hSnap);
	if (hPToken)
		CloseHandle(hPToken);
	if (hProcess)
		CloseHandle(hProcess);

	return bRet;
}

/// <summary>
///  Generic Function - Check for absoulte path 
/// </summary>
/// <param name="szPath">Full path</param>
/// <returns>BOOL true or false</returns>
/// 
static bool isAbsPath(LPCWSTR szPath) {
	if (szPath[0] == L'\\')
		return true;
	if (szPath[0] == L'/')
		return true;
	if (((szPath[0] >= L'A' && szPath[0] <= L'Z') ||
		(szPath[0] >= L'a' && szPath[0] <= L'z')) &&
		szPath[1] == L':')
		return true;
	return false;
}
/// <summary>
///  Generic Function - FindUserSessionId Find and return the session id avaible 
/// </summary>
/// <param name="szUser">User name</param>
/// <param name="pdwSessionId">session id</param>
/// <returns>DWORD true or false</returns>
/// 
static DWORD FindUserSessionId(LPCWSTR szUser, DWORD* pdwSessionId)
{
	PWTS_SESSION_INFOW pSessInfo;
	DWORD dwCount, dwBytes, dwErr = 0;
	LPWSTR lpSessUser;
	BOOL bFound = FALSE;

	_ASSERT(pdwSessionId);

	if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessInfo, &dwCount))
	{
		dwErr = GetLastError();
		goto exit;
	}

	for (DWORD i = 0; i < dwCount; i++)
	{
		if (pSessInfo[i].State == WTSActive)
		{
			if (!WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, pSessInfo[i].SessionId,
				WTSUserName, &lpSessUser, &dwBytes))
			{
				dwErr = GetLastError();
				goto exit;
			}

			if (_wcsicmp(lpSessUser, szUser) == 0)
				bFound = TRUE;

			WTSFreeMemory(lpSessUser);

			if (bFound)
			{
				*pdwSessionId = pSessInfo[i].SessionId;
				break;
			}
		}
	}

exit:
	if (dwErr)
	{
		glog.Set_cModuleName(L"Spool");
		glog.Set_cProcName(L"FindUserSessionID");
		glog.AddRecordU(GetLastErrorStdStr(dwErr));
	}

	if (pSessInfo)
		WTSFreeMemory(pSessInfo);
	return bFound
		? ERROR_SUCCESS
		: (dwErr ? dwErr : 0xFFFFFFFF);
}

/// <summary>
///  Generic Function -StartExe - calling the EXE from system token to user token 
/// </summary>
/// <param name="szExeName">Full path</param>
/// <param name="szWorkingDir">working path</param> 
/// <param name="szCmdLine">commandline param if any</param> 
/// <param name="dwSessionId">session id</param> 
/// <returns>NULL</returns>
/// 
void StartExe(LPCWSTR szExeName, LPCWSTR szWorkingDir, LPCWSTR szCmdLine,
	BOOL bTSEnabled, DWORD dwSessionId)
{

	LPWSTR szCommand;
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	DWORD dwFlags = 0;
	HANDLE htok = NULL, huser = NULL;
	BOOL bIsXp, bRet;
	LPVOID lpEnv = NULL;
	BOOL bTokOk = FALSE;
	DWORD dwErr = 0;

	glog.Set_cProcName(L"StartExe");

	bIsXp = Is_WinXPOrAbove();

	if (bIsXp)
	{
		if (!GetWTSQueryUserToken())
		{
			dwErr = GetLastError();
			glog.Set_cProcName(L"StartExe_GetWTSQueryUserToken");
			goto exit;
		}

		if (!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &huser))
		{
			dwErr = GetLastError();
			glog.Set_cProcName(L"StartExe_OpenThreadToken_Warn");
			glog.AddRecordU(GetLastErrorStdStr(dwErr));
			dwErr = 0;
		}

		RevertToSelf();
	}

	if (bTSEnabled)
	{
		//Terminal Server present
		if (bIsXp)
		{
			//we have WTSQueryUserToken
			if ((bTokOk = fnWTSQueryUserToken(dwSessionId, &htok)) == FALSE)
			{
			}
			//	g_pLog->Log(LOGLEVEL_ERRORS, L"fnWTSQueryUserToken failed: 0x%0.8X", GetLastError());
		}
		else
		{
			//we DON'T have WTSQueryUserToken; look for a running
			//explorer.exe into this session, and grab token from it
			bTokOk = GetExplorerToken(dwSessionId, &htok);
		}
	}
	else
	{
		//Windows 2000 Pro or Windows 2000 Server w/o TS
		bTokOk = TRUE;
	}

	if (bTokOk)
	{
		szCommand = new WCHAR[MAX_COMMAND];

		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));

		si.cb = sizeof(si);
		si.lpDesktop = L"winsta0\\default";
		si.dwFlags |= STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOWNORMAL;

		if (!isAbsPath(szExeName))
		{
			//Not a full path, prefix with szWorkingDir
			swprintf_s(szCommand, MAX_COMMAND, L"\"%s", szWorkingDir);
			size_t pos = wcslen(szCommand);
			if (pos == 0 || szCommand[pos - 1] != L'\\')
				wcscat_s(szCommand, MAX_COMMAND, L"\\");
		}
		else
		{
			//Full path
			swprintf_s(szCommand, MAX_COMMAND, L"\"");
		}

		wcscat_s(szCommand, MAX_COMMAND, szExeName);
		wcscat_s(szCommand, MAX_COMMAND, L"\" ");
		wcscat_s(szCommand, MAX_COMMAND, szCmdLine);


		if (CreateEnvironmentBlock(&lpEnv, htok, FALSE))
			dwFlags |= CREATE_UNICODE_ENVIRONMENT;

		if (htok)
			bRet = CreateProcessAsUserW(htok, NULL, szCommand, NULL,
				NULL, FALSE, dwFlags, lpEnv, szWorkingDir, &si, &pi);
		else
			bRet = CreateProcessW(NULL, szCommand, NULL,
				NULL, FALSE, dwFlags, lpEnv, szWorkingDir, &si, &pi);

		if (!bRet)
		{
			dwErr = GetLastError();
			return;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);


		delete[] szCommand;
	}

exit:
	if (dwErr)
	{
		glog.Set_cModuleName(L"Spool");

		glog.AddRecordU(GetLastErrorStdStr(dwErr));
	}

	if (lpEnv)
		DestroyEnvironmentBlock(lpEnv);

	if (htok)
		CloseHandle(htok);

	if (huser)
	{
		bool bl = SetThreadToken(NULL, huser);

		if (!bl)
		{
			dwErr = GetLastError();

			glog.Set_cModuleName(L"Spool");
			glog.Set_cProcName(L"StartExe_SetThreadToken");
			glog.AddRecordU(GetLastErrorStdStr(dwErr));
		}

		CloseHandle(huser);
	}
}

/// <summary>
///  Generic Function - GetDocumentName - Will open the Printer and check the user name, document name from the current spooler 
/// </summary>
/// <param name="m_strFriendlyName">Printer Driver name</param>
/// <returns>String - documentname</returns>
/// 
CString GetDocumentName(LPWSTR m_strFriendlyName)
{
	CString strDocName = L"";
	HANDLE hPrinter;
	DWORD dwErr = 0;
	PRINTER_INFO_2* pinfo2 = 0;
	JOB_INFO_1* pJobInfoInitial = 0;

	CString strAppPath = g_sFullPath;
	CString strFailLog = strAppPath + _T("LOG\\");
	if (_waccess(strFailLog, 0) == -1)
		CreateDirectory(strFailLog, NULL);
	CTime tm = CTime::GetCurrentTime();
	CString strTime = tm.Format(L"%Y%m%d");
	strFailLog += L"VirtualPrinterDriver-spool_" + strTime + L".log";
	glog.Set_cFileName(strFailLog);
	glog.Set_cModuleName(L"Spool");
	glog.Set_cProcName(L"GetDocumentName");

	if (OpenPrinter(m_strFriendlyName, &hPrinter, NULL) == 0)
	{
		dwErr = GetLastError();
		glog.Set_cProcName(L"GetDocumentName_OpenPrinter");
		goto exit;
	}

	DWORD dwBufsize = 0;
	GetPrinter(hPrinter, 2, (LPBYTE)pinfo2, dwBufsize, &dwBufsize); //Get dwBufsize	
	pinfo2 = (PRINTER_INFO_2*)malloc(dwBufsize); //Allocate with dwBufsize
	if (!GetPrinter(hPrinter, 2, (LPBYTE)pinfo2, dwBufsize, &dwBufsize))
	{
		dwErr = GetLastError();
		glog.Set_cProcName(L"GetDocumentName_GetPrinter");
		goto exit;
	}

	DWORD numJobs = pinfo2->cJobs;

	JOB_INFO_1* pJobInfo = 0;
	DWORD bytesNeeded = 0, jobsReturned = 0;

	//Get info about jobs in queue.
	EnumJobs(hPrinter, 0, numJobs, 1, (LPBYTE)pJobInfo, 0, &bytesNeeded, &jobsReturned);
	pJobInfo = (JOB_INFO_1*)malloc(bytesNeeded);
	if (!EnumJobs(hPrinter, 0, numJobs, 1, (LPBYTE)pJobInfo, bytesNeeded, &bytesNeeded, &jobsReturned))
	{
		dwErr = GetLastError();
		glog.Set_cProcName(L"GetDocumentName_EnumJobs");
		goto exit;
	}

	pJobInfoInitial = pJobInfo;

	DWORD count = 0;
	//for (unsigned short count = 0; count < jobsReturned; count++)
	while (count++ < jobsReturned && pJobInfo != NULL)
	{
		// Start TEST 
		CString strSta;
		strSta = pJobInfo->pStatus;
		//pJobInfo->
		DWORD dw2 = pJobInfo->Status;

		if ((dw2 == 8208) || (dw2 == 16))
		{
			g_strDocumentName = pJobInfo->pDocument;
			g_sTotalPages.Format(L"%d", pJobInfo->TotalPages);
			strDocName = pJobInfo->pUserName;//pJobInfo->pDocument; // Document name
			g_sUserName = pJobInfo->pUserName;
			CString strDName = strDocName;
			g_nNumber = pJobInfo->JobId;
			g_strVal.Format(L"%d", pJobInfo->JobId);

			if (g_sDebugType != "0")
			{
				glog.Set_cModuleName(L"Spool");
				glog.Set_cProcName(L"GetDocumentName");
				CString sTemp = L"Printed File username :" + strDName;
				glog.AddRecordU(sTemp);
				sTemp = L"Job ID :" + g_strVal;
				glog.AddRecord(sTemp);
				//	glog.AddRecord("UserName :" + strDName);

				if (g_sDebugType == "2")
				{
					sTemp = L"Title :" + g_strDocumentName;
					glog.AddRecordU(sTemp);
				}
			}
		}

		pJobInfo++;
	}

exit:
	if (dwErr)
	{
		glog.Set_cModuleName(L"Spool");
		glog.AddRecordU(GetLastErrorStdStr(dwErr));
	}

	if (pinfo2 != NULL)
		free(pinfo2);

	if (pJobInfoInitial != NULL)
		free(pJobInfoInitial);

	if (hPrinter)
		ClosePrinter(hPrinter);

	return strDocName;
}

/// <summary>
///  Generic Function - CreateFileName -Create new file name with datetime stamp 
/// </summary>
/// <returns>String - documentname</returns>
CString CreateFileName()
{
	CString strTime;

	SYSTEMTIME st;
	GetLocalTime(&st);
	strTime.Format(L"%2d%2d%4d-%2d%2d%2d-%3d", st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	strTime.Replace(' ', '0');

	return strTime;
}

// CVirtualPrinterDriverSpoolerDlg dialog

/// <summary>
/// Construtor for CVirtualPrinterDriverSpoolerDlg class 
/// </summary>
/// <param name="pParent">Handle for the mainframe</param>
CVirtualPrinterDriverSpoolerDlg::CVirtualPrinterDriverSpoolerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVirtualPrinterDriverSpoolerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
/// <summary>
/// DoDataExchange for CVirtualPrinterDriverSpoolerDlg class - Holds the events
/// </summary>
/// <param name="pParent">Handle for the mainframe</param>
/// <returns>void - NULL returs</returns>
void CVirtualPrinterDriverSpoolerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVirtualPrinterDriverSpoolerDlg, CDialogEx)

END_MESSAGE_MAP()

/// <summary>
///  Generic Function - GetFileSize - get the file size 
/// </summary>
/// <param name="fileName">File name with full path</param>
/// <returns>String - documentname</returns>
/// 
long GetFileSize(const TCHAR* fileName)
{
	BOOL                        fOk;
	WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

	if (NULL == fileName)
		return -1;

	fOk = GetFileAttributesEx(fileName, GetFileExInfoStandard, (void*)&fileInfo);
	if (!fOk)
		return -1;
	//  assert(0 == fileInfo.nFileSizeHigh);
	return (long)fileInfo.nFileSizeLow;
}

#include <iostream>
#include <stdlib.h>
#include <fstream>
using namespace std;
#include <vector>
/// <summary>
/// OnInitDialog - this class function when we call install or uninstall
/// </summary>
/// <returns>BOOL-TRUE or FALSE</returns>
BOOL CVirtualPrinterDriverSpoolerDlg::OnInitDialog()
{
	//CString strNewFIle = newFolder + strTMP + ".txt";

	DWORD dwErr = 0;
	BOOL bResult = FALSE;

	CString as;
	TCHAR adirname[MAX_PATH];
	memset(adirname, 0, sizeof(adirname));

	glog.Set_cProcName(L"OnInit");

	GetModuleFileName(AfxGetApp()->m_hInstance, adirname, MAX_PATH);

	//	CString strAppPath = adirname;
	CString strFile = adirname;
	strFile.Replace(_T("gswin32cM.exe"), _T(""));

	CString strFullPath = strFile;
	CString strAppPath = strFile;
	strAppPath.Replace(L"Xtra\\Prerequisite\\kbgs\\kbbin\\bin\\", L"");

	g_sFullPath = strAppPath;

	CString sFolder = strAppPath + L"Files\\";

	if ((_waccess(sFolder, 0)) == -1)
		CreateDirectory(sFolder, NULL);

	CString sFullPath = strAppPath;
	m_IniUser.SetPathName(strAppPath + L"Settings.ini");

	g_sDebugType = m_IniUser.GetString(_T("Settings"), _T("DEBUG"), _T("0"));

	CString strNameU;
	CString strTMP = GetDocumentName(L"TaxDome Printer 2.0");
	strNameU = strTMP;

	{
		char* user = getenv("username");
		glog.Set_cModuleName(L"Spool");
		glog.Set_cProcName(L"OnInit_username");
		CString sTemp = CString(user);
		glog.AddRecord(sTemp);
	}

	if ((_waccess(g_sFullPath, 0)) == -1)
		CreateDirectory(g_sFullPath, NULL);

	strFile.Replace(L"Prerequisite\\kbgs\\kbbin\\bin\\", L"VPDriver.pdf");

	CString strNewPDFILe = sFolder + g_sUserName + L"_" + CreateFileName() + L".pdf";

	LoadSpoolFile(L"");

	long lnVal = GetFileSize(strFile.GetBuffer(strFile.GetLength()));

	if (lnVal < 760)
	{
		CString sTime = CreateFileName();
		strFile.Replace(L"VPDriver.pdf", L"VirtualPrinterDriver.dat");
		strNewPDFILe = sFolder + g_sUserName + L"_" + sTime + L".dat";
		if (g_sDebugType == L"2")
		{
			glog.Set_cModuleName(L"Spool");
			glog.Set_cProcName(L"OnInit_GetFileSize");
			CString sTemp = L"Raw Printing called";
			glog.AddRecord(sTemp);
		}
	}
	else
	{
		if (g_sDebugType == L"2")
		{
			glog.Set_cModuleName(L"Spool");
			glog.Set_cProcName(L"OnInit_GetFileSize");
			CString sTemp = L"Normal Printing called";
			glog.AddRecord(sTemp);
		}
	}

	glog.Set_cModuleName(L"Spool");
	glog.Set_cProcName(L"OnInit_CopyFile");

	if (!PathFileExists(strFile))
	{
		CString sTemp = L"File not exists: " + strFile;
		glog.AddRecordU(sTemp);
	}
	else
	{
		if (g_sDebugType == L"2")
		{
			CString sTemp = L"File exists: " + strFile;
			glog.AddRecordU(sTemp);
		}
	}

	if (!CopyFile(strFile, strNewPDFILe, false))
	{
		dwErr = GetLastError();
		goto exit;
	}

	glog.Set_cModuleName(L"Spool");
	glog.Set_cProcName(L"OnInit_CopyFile");

	if (!PathFileExists(strNewPDFILe))
	{
		CString sTemp = L"File not exists: " + strNewPDFILe;
		glog.AddRecordU(sTemp);
	}
	else
	{
		if (g_sDebugType == L"2")
		{
			CString sTemp = L"File exists: " + strNewPDFILe;
			glog.AddRecordU(sTemp);
		}
	}

	DWORD dwSessionId = 0, dwRet;
	BOOL bTSEnabled = FALSE;

	dwRet = FindUserSessionId(strTMP, &dwSessionId);

	if (dwRet == ERROR_SUCCESS)
		bTSEnabled = TRUE;
	else
	{
		if (dwRet != ERROR_APP_WRONG_OS)
		{
			if (Is_WinXPOrAbove())
			{
				typedef DWORD(WINAPI* PFNWTSGETACTIVECONSOLESESSIONID)(void);
				PFNWTSGETACTIVECONSOLESESSIONID fnWTSGetActiveConsoleSessionId;

				HMODULE hMod = GetModuleHandleW(L"kernel32.dll");

				if (!hMod)
				{
					dwErr = GetLastError();
					glog.Set_cProcName(L"OnInit_GetModuleHandleW");
					bResult = FALSE;
					goto exit;
				}

				fnWTSGetActiveConsoleSessionId = (PFNWTSGETACTIVECONSOLESESSIONID)GetProcAddress(hMod, "WTSGetActiveConsoleSessionId");

				if (!fnWTSGetActiveConsoleSessionId)
				{
					dwErr = GetLastError();
					glog.Set_cProcName(L"OnInit_GetProcAddress");
					bResult = FALSE;
					goto exit;
				}

				if ((dwSessionId = fnWTSGetActiveConsoleSessionId()) == 0xFFFFFFFF)
				{
					dwErr = GetLastError();
					glog.Set_cProcName(L"OnInit_fnWTSGetActiveConsoleSessionId");
					bResult = FALSE;
					goto exit;
				}

				bTSEnabled = TRUE;
			}
			else
			{
				dwErr = GetLastError();
				glog.Set_cProcName(L"OnInit_Is_WinXPOrAbove");
				bResult = FALSE;
				goto exit;
			}
		}
	}

	{
		CString sEXEName = m_IniUser.GetString(_T("Settings"), _T("Path"), _T(""));

		CString sWorkingDIR = m_IniUser.GetString(_T("Settings"), _T("WORKINGDIR"), _T(""));

		if (!sWorkingDIR.IsEmpty())
		{
			sWorkingDIR.Replace(L"\"", L"");
			sWorkingDIR.Replace(L"=", L"");
			sWorkingDIR.Replace(L"'", L"");
		}

		if (!sEXEName.IsEmpty())
		{
			sEXEName.Replace(L"\"", L"");
			sEXEName.Replace(L"=", L"");
			sEXEName.Replace(L"'", L"");
		}
		if (_waccess(sEXEName, 0) == -1)
		{
			//if (g_sDebugType!=L"0") 
			{
				glog.Set_cProcName(L"OnInit_sEXEName");
				glog.AddRecord(L"Not a vaild calling EXE");
				glog.AddRecord(sEXEName);

				bResult = FALSE;
				goto exit;
			}
		}

		//CString sCurrentPath = sFullPath + L"";

		int nPos = sEXEName.ReverseFind(WCHAR(92));


		CString sPath = sEXEName.Mid(0, nPos);


		if ((sWorkingDIR.IsEmpty()) || (_waccess(sWorkingDIR, 0) == -1))
		{
			glog.Set_cProcName(L"OnInit_sWorkingDIR");
			glog.AddRecord(L"Not a vaild working directory");
			glog.AddRecordU(sWorkingDIR);
			glog.AddRecord(L"Fixing to calling EXE Path");

			bResult = FALSE;
			goto exit;
		}
		else
		{
			sPath = sWorkingDIR;
		}
		CString sFIleName = sEXEName.Mid(nPos + 1, sEXEName.GetLength() - nPos - 1);
		if (g_sDebugType == L"2")
		{
			glog.Set_cProcName(L"OnInit_StartExe_Prepare");
			glog.AddRecordU(strNewPDFILe);
			glog.AddRecordU(sEXEName);
			glog.AddRecordU(sPath);
		}


		CString sOutFile;
		g_strDocumentName.Replace(L"*", L"");
		sOutFile.Format(L"\"%s\" /title=\"%s\"", strNewPDFILe.GetBuffer(strNewPDFILe.GetLength()), g_strDocumentName.GetBuffer(g_strDocumentName.GetLength()));

		StartExe(sEXEName, sPath, sOutFile, bTSEnabled, dwSessionId);

		Sleep(1000);
	}

exit:
	if (dwErr)
	{
		glog.AddRecordU(GetLastErrorStdStr(dwErr));
	}

	exit(0);

	// TODO: Add extra initialization here

	return bResult;  // return TRUE  unless you set the focus to a control
}

/// <summary>
/// LoadSpoolFile - Checking for RAW Printer and get the spooler raw files
/// </summary>
/// <param name="filename">No param required</param>
/// <returns>String - Full spool file path</returns>
CString LoadSpoolFile(LPWSTR lpzFileName)
{
	CFileException e;
	//int	res=0;
	CString strTarget;

	CStringArray strArray;
	CFileFind fileFinder;

	CString  strSystemPath;
	TCHAR chrWid[255];

	CTime ftime1;
	CTime trialTime = CTime::GetCurrentTime();


	CString as;
	TCHAR adirname[MAX_PATH];
	//int at=0, alen=0;
	memset(adirname, 0, sizeof(adirname));

	GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, adirname, MAX_PATH);

	CString strFile;
	CString strFileNew = adirname;


	strFileNew.Replace(_T("gswin32cM.exe"), _T(""));

	//strFileNew.Replace(L"Xtra\\Prerequisite\\kbgs\\kbbin\\bin\\", L"");


	CString strFullPath = strFileNew;


	CString strAppPath;
	CString strINIFile = strAppPath + _T("LOG\\");
	CString strFailLog = strINIFile;



	CTime tm = CTime::GetCurrentTime();
	//CString time = CTime::GetCurrentTime().Format("%H.%M.%S");
	CString strTime = tm.Format("%Y%m%d");



	if (g_sDebugType != "0")
	{

		glog.Set_cModuleName(L"Spool");
		glog.Set_cProcName(L"LoadSpoolFile");
	}
	CString str22;
	str22.Format(L"%d", (DWORD)trialTime.GetTime());


	strFileNew.Replace(L"Prerequisite\\kbgs\\kbbin\\bin\\", L"VirtualPrinterDriver.dat");

	CString strPDFILE = strFullPath;
	strPDFILE.Replace(L"Prerequisite\\kbgs\\kbbin\\bin\\", L"VPDriver.pdf");

	CString strPDFFile2 = strFullPath;
	strPDFFile2.Replace(L"Prerequisite\\kbgs\\kbbin\\bin\\", L"VPDriver1.pdf");

	long lnVal = GetFileSize(strPDFILE.GetBuffer(strPDFILE.GetLength()));


	//	if (lnVal < 760) 
	{
		if (lnVal < 760)
		{
		}
		if ((_waccess(strFileNew, 0)) != -1)
			DeleteFile(strFileNew);

		UINT uint = GetWindowsDirectory(chrWid, 255);
		if (uint)
		{
		}
		strSystemPath = chrWid;
		strSystemPath += L"\\System32\\spool\\PRINTERS\\*.SPL";

		CString strPath = chrWid;
		strPath += L"\\System32\\spool\\PRINTERS\\";
		if (g_nNumber > 0)
		{
			strFile.Format(L"%05d", g_nNumber);
			strFile = strPath + strFile + L".SPL";
		}

		if (_waccess(strFile, 0) == -1)
		{
			//g_nNumber
			strFile = L"";
			BOOL bFile = fileFinder.FindFile(strSystemPath);// List out all files stylles Folder

			if (bFile == 0)
				return L"ERROR";
			CString strPreFileName;
			CString strCurrentFileName;
			while (bFile)
			{
				// Find Next file or Move to Next file
				CString strFileName;
				bFile = fileFinder.FindNextFile();

				if (!fileFinder.IsDirectory())// && (!fileFinder.IsDots())) // Find the type of file  dir or not dir
				{
					strFileName = fileFinder.GetFilePath(); // store the file name in strFileName
					strCurrentFileName = fileFinder.GetFileName();
					//	if (GetFileSize(strFileName.GetBuffer(strFileName.GetLength())) > 0) 
					{
						fileFinder.GetCreationTime(ftime1);
						long timdiff = (long)trialTime.GetTime() - (long)ftime1.GetTime();

						CString str;
						str.Format(L"%d", (DWORD)trialTime.GetTime());

						//CString strTime;
						strTime.Format(L"%d", (DWORD)ftime1.GetTime());



						if ((ftime1 <= trialTime) && (timdiff < 5000))// Checking for latest files 
						{
							if (ftime1 == trialTime)
							{
								if (!strPreFileName.IsEmpty())
								{
									CString strTempPREV = strPreFileName;
									CString strCurrent = strCurrentFileName;
									strTempPREV.MakeLower();
									strTempPREV.Replace(L".spl", L"");
									strCurrent.MakeLower();
									strCurrent.Replace(L".spl", L"");

									int nPrevFile = _wtoi(strTempPREV);
									int nCurrentF = _wtoi(strCurrent);

									if (nPrevFile > nCurrentF)
									{
										trialTime = ftime1;
										strFile = strFileName; // New Latest file

									}
								}
								else
								{
									trialTime = ftime1;
									strFile = strFileName; // New Latest file

								}
							}
							else
							{
								trialTime = ftime1;
								strFile = strFileName; // New Latest file

							}
						}

						strPreFileName = strCurrentFileName;
					}
				}

			}

		}

		strArray.Add(strFile);//strFile = 
		fileFinder.Close();  // Close File 
		TRY
		{
			strTarget = strFileNew;

			CopyFile(strArray.GetAt(0),strTarget,FALSE);
			if (g_sDebugType != "0")
			{
				glog.Set_cProcName(L"LoadSpoolFile");
				glog.AddRecord(strFile);
			}

			Sleep(1000);

			}
			CATCH(CFileException, e)
			{
#ifdef _DEBUG
				afxDump << "File " << lpzFileName << " does not exist or cannot be opened\n";
#endif
			}
			END_CATCH

				if (e.m_cause == CFileException::none)
				{
					return strTarget;
				}
			//delete[] buffer;		
			//delete TheFile;
		}
	return strTarget;
	}