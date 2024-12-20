
#include "pch.h"
#include "CppUnitTest.h"
#include "InstallPrinter.h"
#include "InstallPrinterDlg.h"
#include "InstallPrinterDlg.cpp"
#include "logFile.h"
#include "logFile.cpp"
#include <Afx.h>
extern CString m_strAppPath;

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
/// <summary>
/// InstallPrinterUnitTest - Class to test InstallPrinter64 Unit Testing 
/// </summary>
namespace InstallPrinterUnitTest
{
	TEST_CLASS(InstallPrinterUnitTest)
	{
	public:
		/// <summary>
		/// TEST_METHOD - Method to test Install Functions in InstallPrinter64 module 
		/// </summary>
		/// <param name="Install">No param required</param>
		/// <returns>Returns Pass or Fail in the test explorer </returns>
		TEST_METHOD(Install)
		{
			CInstallPrinterDlg dlg;
			dlg.m_bInstall = true;
			if (!dlg.DoModal())
				throw std::out_of_range("Error Install");
			//throw std::out_of_range("Can't do square roots of negatives");
		}
		/// <summary> 
		/// TEST_METHOD - Method to test Uninstall Functions in InstallPrinter64 module 
		/// </summary>
		/// <param name="Uninstall">No param required</param>
		/// <returns>Returns Pass or Fail in the test explorer </returns>
		TEST_METHOD(Uninstall)
		{
			CInstallPrinterDlg dlg;
			dlg.m_bInstall = false;
			if (!dlg.DoModal())
				throw std::out_of_range("Error Uninstall");
		}
		/// <summary> 
		///CreateRSPFile - Method to test Uninstall Functions in InstallPrinter64 module 
		/// </summary> 
		/// <param name="Uninstall">No param required</param>
		/// <returns>Returns Pass or Fail in the test explorer </returns>
		TEST_METHOD(CreateRSPFileTEST)
		{



			CInstallPrinterDlg dlg;
			BOOL bl = dlg.CreateRSPFile();
			Assert::AreEqual((double)1, (double)bl, L"File not found");
			//ASSERT(bl == false);
			if (bl == false)
				throw std::out_of_range("unable to Create RSPFile");
		}

		/// <summary> 
		///GetINIPath - Method to test getting log file folder in InstallPrinter64 module 
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file path exist ot not </returns>
		TEST_METHOD(GetINIPath)
		{
			CInstallPrinterDlg dlg;
			CString sPath = dlg.GetINIPath();
			Assert::AreEqual((double)0, (double)sPath.IsEmpty(), L"Empty");
			//Assert::AreEquals(sPath)
			//ASSERT(sPath.IsEmpty());
			if (sPath.IsEmpty())
				throw std::out_of_range("unable to find Log Folder");
		}
		/// <summary> 
		///Install - Method to test getting log file folder in InstallPrinter64 module 
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file path exist ot not </returns>
		TEST_METHOD(InstallTest)
		{
			CString as;
			TCHAR adirname[MAX_PATH];
			int at = 0, alen = 0;
			memset(adirname, 0, sizeof(adirname));

			GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, adirname, MAX_PATH);

			CString spath(adirname);
			Assert::AreEqual((double)0, (double)spath.IsEmpty(), L"Empty");
			//ASSERT(spath.IsEmpty());
			AddMonitorLocal();
			AddPortLocal();

			BOOL blRet = CopyPrintDriverFiles2System(); AddPrinterDriver();
			Assert::AreEqual((double)1, (double)blRet, L"CopyPrintDriverFiles2System Failed");
			AddPrinter(); UpdateRegInfo();

			HANDLE     hPrinter = NULL;
			if (!OpenPrinter("TaxDome Printer 2.0", &hPrinter, NULL))
			{
				throw std::out_of_range("unable to find printer");
			}
			ClosePrinter(hPrinter);
			/*if (sPath.IsEmpty())
				throw std::out_of_range("unable to find Log Folder");*/
		}

		/// <summary> 
		///Install - Method to test getting log file folder in InstallPrinter64 module 
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns file path exist ot not </returns>
		TEST_METHOD(UninstallTest)
		{
			CString as;
			TCHAR adirname[MAX_PATH];
			int at = 0, alen = 0;
			memset(adirname, 0, sizeof(adirname));

			GetModuleFileName(NULL/*AfxGetApp()->m_hInstance*/, adirname, MAX_PATH);
			CString spath(adirname);
			//ASSERT(spath.IsEmpty());
			Assert::AreEqual((double)0, (double)spath.IsEmpty(), L"Empty");
			DeletePrinter();
			DeletePrinterDriver();
			DeletePortLocal(NULL);
			DeleteMonitorLocal();
			DeleteRegInfo();
			HANDLE     hPrinter = NULL;
			if (OpenPrinter("TaxDome Printer 2.0", &hPrinter, NULL))
			{
				ClosePrinter(hPrinter);
				throw std::out_of_range("unable to uninstallprinter");
			}
			/*if (sPath.IsEmpty())
				throw std::out_of_range("unable to find Log Folder");*/
		}
		/// <summary> 
		///LogfileTest - Method to test Logfile writing Functions in InstallPrinter64 module 
		/// </summary> 
		/// <param name="NOPARAM">No param required</param>
		/// <returns>Returns log file exist ot not </returns>
		TEST_METHOD(LogfileTest)
		{
			CLogFile log1;
			lstrcpy(log1.cFileName, "testlog.log");


			log1.AddRecord("log test");

			Assert::AreEqual((double)0, (double)_access("testlog.log", 0), L"File not found");

			//ASSERT(_access("testlog.log", 0) == -1);

			if (_access("testlog.log", 0) == -1)
				throw std::out_of_range("unable to Create RSPFile");
		}

	};
}
