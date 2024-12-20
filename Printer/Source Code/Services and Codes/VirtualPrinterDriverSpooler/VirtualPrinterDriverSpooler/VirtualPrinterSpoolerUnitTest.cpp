#include "pch.h"
#include "CppUnitTest.h"

//#define _CRT_SECURE_NO_WARNINGS
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler.h"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/VirtualPrinterDriverSpoolerDlg.h"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/VirtualPrinterDriverSpoolerDlg.cpp"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/logFile.h"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/Ini.h"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/logFile.cpp"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/ini.cpp"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/StdioFileEx.h"
#include "../../VirtualPrinterDriverSpooler/VirtualPrinterDriverSpooler/StdioFileEx.cpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
#include <Afx.h>

/// <summary>
/// VirtualPrinterSpoolerUnitTest - Class to test VirtualPrinterSpooler Unit Testing 
/// </summary>
namespace VirtualPrinterSpoolerUnitTest
{
	TEST_CLASS(VirtualPrinterSpoolerUnitTest)
	{
	public:
		/// <summary>
		/// FullRun - Method to test Spooler Full Function in the Spooler module 
		/// </summary>
		/// <param name="FullRun">No param required</param>
		/// <returns>Returns Pass or Fail in the test explorer </returns>
		TEST_METHOD(FullRun)
		{
			CVirtualPrinterDriverSpoolerDlg dlg;
			if (!dlg.DoModal())
				throw std::out_of_range("Error Fulltest");
		}
		/// <summary>
		/// GetDocument - Method to test Spooler GetDocument Generic Function in Spooler module 
		/// </summary>
		/// <param name="GetDocument">No param required</param>
		/// <returns>Returns Pass or Fail in the test explorer </returns>
		TEST_METHOD(GetDocument)
		{

			CString sFile = GetDocumentName(L"TaxDome Printer 2.0");
			Assert::AreEqual((double)0, (double)!sFile.IsEmpty(), L"Empty");
			if (!sFile.IsEmpty())
				throw std::out_of_range("GetDocument Failed");

			//	throw std::out_of_range("GetDocument Failed");
		}

		/// <summary>
		/// CreateFileName - Method to test Creation of the file name without null
		/// </summary>
		/// <param name="CreateFileName">No param required</param>
		/// <returns>String value </returns>
		TEST_METHOD(CreateFileNameTest)
		{

			CString sFileName = CreateFileName();

			Assert::AreEqual((double)0, (double)sFileName.IsEmpty(), L"Empty");
			//ASSERT(sFileName.IsEmpty());

			if (sFileName.IsEmpty())
				throw std::out_of_range("CreatFileName Fails");
		}

		/// <summary>
		/// GetFileSize - Method to Get FileSize
		/// </summary>
		/// <param name="GetFileSize">No param required</param>
		/// <returns>Returns long with file size </returns>
		TEST_METHOD(GetFileSizeTest)
		{

			long lnSize = GetFileSize(L"VirtualPrinterSpoolerUnitTest.dll");

			if (lnSize <= 0)
				throw std::out_of_range("GetFileSize fails");

		}

		/// <summary> 
		///LogfileTest - Method to test Logfile writing Functions in InstallPrinter64 module 
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns log file exist ot not </returns>
		TEST_METHOD(LogfileTest)
		{
			CLogFile log1;
			lstrcpy(log1.cFileName, L"testlog.log");

			DeleteFile(L"testlog.log");
			log1.AddRecord(L"log test");
			log1.AddRecordU(L"您好 test");
			CStringA strUTF = ConvertUnicodeToUTF8(L"test");
			DWORD dwErr = GetLastError();
			CString sErrorstring = GetLastErrorStdStr(dwErr);

			BOOL bl = false;

			if (!strUTF.IsEmpty())
			{
				bl = true;
			}
			Assert::AreEqual((double)1, (double)bl, L"ConvertUnicode Error");

			if (!sErrorstring.IsEmpty())
			{
				bl = true;
			}
			Assert::AreEqual((double)1, (double)bl, L"GetLastErrorStdStr Error");

			Assert::AreEqual((double)0, (double)_access("testlog.log", 0), L"File not found");

			if (_waccess(L"testlog.log", 0) == -1)
				throw std::out_of_range("unable to Create RSPFile");
		}

		/// <summary> 
		///isAbsPath - Method to test getting Path is absolute
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file true or false </returns>
		TEST_METHOD(isAbsPathCheck)
		{
			CString as;
			TCHAR adirname[MAX_PATH];
			//int at = 0, alen = 0;
			memset(adirname, 0, sizeof(adirname));

			GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, adirname, MAX_PATH);

			CString spath(adirname);
			Assert::AreEqual((double)0, (double)spath.IsEmpty(), L"Empty");

			//CVirtualPrinterDriverSpoolerDlg dlg;
			bool  bPath = isAbsPath(adirname);

			if (bPath == false)
				throw std::out_of_range("unable to find Log Folder");
		}

		/// <summary> 
		///Is_WinXPOrAbove - Method to check OS above XP or higher
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file true or false </returns>
		TEST_METHOD(Is_WinXPOrAboveCheck)
		{


			bool  bXPAbove = Is_WinXPOrAbove();

			if (bXPAbove == false)
				throw std::out_of_range("unable to find Log Folder");
		}

		/// <summary> 
		///GetSessionID - Method to check current login session id
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file true or false </returns>
		TEST_METHOD(GetSessionID)
		{
			TCHAR username[UNLEN + 1];
			DWORD username_len = UNLEN + 1;
			GetUserName(username, &username_len);
			DWORD dwRet = 0, dwSessionId = 0;

			dwRet = FindUserSessionId(username, &dwSessionId);

			ASSERT(dwRet != ERROR_SUCCESS);

			if (dwRet != ERROR_SUCCESS)
				throw std::out_of_range("unable to find session id");
		}
		/// <summary> 
		///StartEXETEST - Method to test StartEXE
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file true or false </returns>
		TEST_METHOD(StartEXETEST)
		{
			CString as;
			TCHAR adirname[MAX_PATH];
			//int at = 0, alen = 0;
			memset(adirname, 0, sizeof(adirname));

			GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, adirname, MAX_PATH);

			CString spath(adirname);
			Assert::AreEqual((double)0, (double)spath.IsEmpty(), L"Empty");

			//const int INFO_BUFFER_SIZE = 10;
			TCHAR infoBuf[100];
			lstrcpy(infoBuf, L"");
			DWORD bufCharCount = 100;
			CString sUserN;

			if (GetUserName(infoBuf, &bufCharCount))
			{
				sUserN = infoBuf;

			}

			DWORD dwSessionId = 0, dwRet;
			BOOL bTSEnabled = FALSE;

			dwRet = FindUserSessionId(sUserN, &dwSessionId);

			HANDLE htok = NULL;
			BOOL bTokOk = FALSE;
			bTokOk = GetExplorerToken(dwSessionId, &htok);

			Assert::AreEqual((double)0, (double)bTokOk, L"Explorer Token error");

			StartExe(adirname, L"", L"test.pdf", bTSEnabled, dwSessionId);

			ASSERT(dwRet != ERROR_SUCCESS);
			if (dwRet != ERROR_SUCCESS)
				throw std::out_of_range("unable to find session id");




		}
		/// <summary> 
		///INIHandling - Method to test ini file
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file true or false </returns>
		TEST_METHOD(INIHandling)
		{
			CString as;
			TCHAR adirname[MAX_PATH];
			//int at = 0, alen = 0;
			memset(adirname, 0, sizeof(adirname));

			GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, adirname, MAX_PATH);

			CIni m_IniUser;

			LPTSTR tempFile = new TCHAR[MAX_PATH];
			GetTempPath(MAX_PATH, tempFile);
			GetTempFileName(tempFile, L"", 0, tempFile);

			m_IniUser.SetPathName(tempFile);

			Assert::IsTrue(m_IniUser.WriteString(_T("Settings"), _T("UNITTESTING"), _T("1")));

			CString sUnitTest = m_IniUser.GetString(_T("Settings"), _T("UNITTESTING"), _T("0"));

			Assert::AreEqual((double)0, (double)sUnitTest.Compare(L"1"), L"INI error");
			//ASSERT(sUnitTest == 0);

			delete tempFile;

			if (sUnitTest == L"0")
				throw std::out_of_range("unable to save read ini file");
		}

		/// <summary> 
	///TESTLoadSpoolFile- Method to Get RAW Filename
	/// </summary> 
	/// <param name="NOPARAM">No param required</param>
	/// <returns>Returns file true or false </returns>
		TEST_METHOD(TESTLoadSpoolFile)
		{
			g_nNumber = 1;
			CString sTarget = LoadSpoolFile(L"");

			BOOL bl = false;

			if (!sTarget.IsEmpty())
			{
				bl = true;
			}


			Assert::AreEqual((double)1, (double)bl, L"Spool file error");
			//ASSERT(sUnitTest == 0);



		}

		/// <summary> 
		///TESTCStdioFileEx- Method to Get TEST FILE OPERATIONS
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file true or false </returns>
		TEST_METHOD(TESTCStdioFileEx)
		{

			FILE* fp;
			char* l = setlocale(LC_ALL, "");
			fopen_s(&fp, "utest.txt", "a+");
			if (fp)
			{
				fprintf(fp, "testing unit test");
				fprintf(fp, "%ls\n", L"您好");
				fclose(fp);
			}
			CStdioFileEx	fileEx;
			fileEx.SetCodePage(CP_UTF8);

			CString rString;


			// Get the current code page
			UINT nLocaleCodePage = CStdioFileEx::GetCurrentLocaleCodePage();
			int nCharCount = 0;
			CString sLine;
			BOOL bIsRead = false;
			if (fileEx.Open(L"utest.txt", CFile::modeReadWrite | CFile::shareExclusive | CFile::typeText))
			{
				BOOL bIsUnicode = fileEx.IsFileUnicodeText();
				nCharCount = fileEx.GetCharCount();
				fileEx.ReadString(sLine);
			}


			if (nCharCount > 0)
			{
				bIsRead = true;
			}
			Assert::AreEqual((double)1, (double)bIsRead, L"unable to Read File");

			UINT unit = fileEx.GetCurrentLocaleCodePage();
			if (nLocaleCodePage <= 0)
				bIsRead = true;

			Assert::AreEqual((double)1, (double)bIsRead, L"unable to get locale information");


			unit = fileEx.GetCurrentLocaleCodePage();
			if (nLocaleCodePage <= 0)
				bIsRead = true;
			fileEx.WriteString(L"Append");

			Assert::AreEqual((double)1, (double)bIsRead, L"unable to get locale information");

			char cMultiBStr[MAX_PATH];
			fileEx.GetMultiByteStringFromUnicodeString(L"TEST", cMultiBStr, 4, -1);
			CStringA strMU(cMultiBStr);
			if (strMU.IsEmpty())
				bIsRead = true;


			Assert::AreEqual((double)1, (double)bIsRead, L"GetMultiByteStringFromUnicodeString Error");
			TCHAR cU2MultiBStr[MAX_PATH];
			fileEx.GetUnicodeStringFromMultiByteString("Test", cU2MultiBStr, 8, -1);
			CString strUM(cU2MultiBStr);
			if (strUM.IsEmpty())
				bIsRead = true;

			Assert::AreEqual((double)1, (double)bIsRead, L"GetUnicodeStringFromMultiByteString Error");


		}
	};
}




