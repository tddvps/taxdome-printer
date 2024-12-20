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

// LogFile.cpp: implementation of the CLogFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "logging.h"
#include "LogFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CStringA ConvertUnicodeToUTF8(const CStringW& uni);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/// <summary>
/// Constructor of this class 
/// </summary>
/// 
CLogFile::CLogFile()
	:lCurrentRecord(0)
	, bArchiveIfMaxReached(FALSE)
{
	Set_cFileName(L"");
	Set_cArchiveName(L"");
	Set_HeaderRec();
	Set_cModuleName(L"");
	Set_cProcName(L"");
}
/// <summary>
/// Constructor of this class 
/// </summary>
/// <param name="Filename">FilePath</param>
/// 
CLogFile::CLogFile(LPWSTR filename, LPWSTR archivename)
	:lCurrentRecord(0)
	, bArchiveIfMaxReached(FALSE)
{
	Set_cFileName(filename);
	Set_cArchiveName(archivename);
	Set_HeaderRec();
	Set_cModuleName(L"");
	Set_cProcName(L"");
}
/// <summary>
/// Destructor of this class 
/// </summary>
CLogFile::~CLogFile()
{

}
/// <summary>
/// OpenLogFile
/// </summary>
/// <param name="filename">Full File path of the log file</param>
/// <returns>BOOL true or false</returns>
BOOL CLogFile::OpenLogFile(LPWSTR filename)
{
	// I am not keeping the log file open all of the time just 
	// to make sure it always gets closed 
	BOOL bFileOpened = FALSE;

	// we don't want to open the file if there is no file name
	if (cFileName[0] != '\0')
	{
		///AfxMessageBox("2");
		CString strFile(cFileName);

		if ((_waccess(strFile, 0)) == -1)
		{
			FILE* fStream;
			errno_t e = _tfopen_s(&fStream, strFile, _T("w,ccs=UTF-8"));
			if (e)
			{
			}
			if (fStream != NULL)
				fclose(fStream);
		}

		if (Open(strFile, CFile::modeCreate | CFile::modeNoTruncate |
			CFile::modeReadWrite | CFile::shareDenyNone | CStdioFileEx::modeWriteUnicode))
		{
			// return value
			bFileOpened = TRUE;
			SetCodePage(CP_UTF8);
			DWORD length = (DWORD)GetLength();

			if (length == 3)
			{
				// write the header
				AddHeaderRecord();
			}
			else
			{
				// get the current record
				lCurrentRecord = (length / RECORDSIZE);

				// check if we have reached the maximum size of the log
				if (lCurrentRecord == (long)MAXRECORDS)
				{
					SeekToBegin();

					// archive this log file?
					if (bArchiveIfMaxReached)
					{
						CFile archive;
						CString sarchive(cArchiveName);

						if ((_waccess(sarchive, 0)) == -1)
						{
							FILE* fStream;


							errno_t e = _tfopen_s(&fStream, sarchive, _T("w,ccs=UTF-8"));
							if (e)
							{
							}
							if (fStream)
								fclose(fStream);

						}

						// open/create the archive file
						if (archive.Open(sarchive, CFile::modeCreate | CFile::modeNoTruncate |
							CFile::modeReadWrite | CStdioFileEx::modeWriteUnicode))
						{
							// write the header if you have to
							if (archive.SeekToEnd() == 0)
							{



								CStringA strHeader = "DATE\tTIME\tMODULE\tPROCEDURE\tDESCRIPTION\r\n";


								Write(strHeader, strHeader.GetLength());
								//archive.Write(strHeader, strHeader.GetLength());
								//archive.Write(&HeaderRec, sizeof(LogRecord));
								//archive.Write("\r\n", 2);
								archive.SeekToEnd();
							}

							// add the current records to the end of the archive
							for (long line = 1; line < lCurrentRecord; line++)
							{
								char cLine[RECORDSIZE];
								long ln = line * (RECORDSIZE);
								Seek(ln, CFile::begin);
								if (Read(cLine, RECORDSIZE) > 0)
								{
									archive.Write(cLine, RECORDSIZE * sizeof(TCHAR));
								}
							}

							archive.Close();
						}

						// delete the current records and start over
						SetLength(0);
						AddHeaderRecord();
					}
					else
					{
						// use fifo to delete the oldest record
						Seek(2 * RECORDSIZE, CFile::begin);

						// read in all records except the header and oldest record
						long buffsize = (long)(RECORDSIZE * (MAXRECORDS - 2));
						TCHAR* cBuffer = new TCHAR[buffsize];
						memset(cBuffer, 0, buffsize);

						Read(cBuffer, buffsize);

						// truncate the file to 0 length
						SetLength(0);

						// add the header back on
						AddHeaderRecord();

						// add the buffered records back on
						Write(cBuffer, buffsize * sizeof(TCHAR));

						delete[] cBuffer;
					}
				}

				SeekToEnd();
			}
		}
	}

	return bFileOpened;
}
/// <summary>
/// Setting the title text(heading) in the log file 
/// </summary>
void CLogFile::Set_HeaderRec()
{
	memset(&HeaderRec, 0, sizeof(LogRecord));

	lstrcpy(HeaderRec.cDate, L"DATE\t");
	lstrcpy(HeaderRec.cTime, L"TIME\t");
	lstrcpy(HeaderRec.cModuleName, L"MODULE\t");
	lstrcpy(HeaderRec.cProcName, L"PROCEDURE\t");
	lstrcpy(HeaderRec.cDescription, L"DESCRIPTION                                                                          ");
}
/// <summary>
/// Adding the title text(heading) in the log file 
/// </summary>
BOOL CLogFile::AddHeaderRecord()
{
	BOOL bRecordAdded = FALSE;

	// log file is already open because this method
	// is only called from OpenLogFile

	CStringA strHeader = "DATE\tTIME\tMODULE\tPROCEDURE\tDESCRIPTION\n";


	Write(strHeader, strHeader.GetLength());

	//Write(&HeaderRec, sizeof(LogRecord));
	//Write(L"\r\n", 4);

	// checking if the file size increased for verification that Write was successful
	if ((long)(GetLength() / RECORDSIZE) > lCurrentRecord)
	{
		// increment the record count
		lCurrentRecord++;
		bRecordAdded = TRUE;
	}

	return bRecordAdded;
}
/// <summary>
/// This Function used to record events in the logfile. This has to use for every log recording
/// </summary>
/// <param name="description">Text to logging</param>
/// <returns>BOOL true or false</returns>
BOOL CLogFile::AddRecord(CString description)
{
	return AddRecordU(description);

	BOOL bRecordAdded = FALSE;
	LogRecord logrec;
	CTime tTime = CTime::GetCurrentTime();	// current time

	memset(&logrec, 0, sizeof(LogRecord));

	CString strA = tTime.Format(L"%m-%d-%Y");
	CString strAH = tTime.Format(L"%H:%M:%S");
	strA += "\t";
	strAH += "\t";

	//CStringA str32;
	//str32 = char(32);

	CString strModule(cModuleName);
	strModule += L"\t";
	//strModule.Replace(str32,"");

	CString strProc(cProcName);
	strProc += L"\t";

	//strProc.Replace(str32,"");

	CString strDesc;
	strDesc = description;

	//	strDesc.Replace(str32,"");


	//strcat(cModuleName,
	// fill the record
	lstrcpy(logrec.cDate, strA);
	lstrcpy(logrec.cTime, strAH);
	lstrcpy(logrec.cModuleName, strModule);
	lstrcpy(logrec.cProcName, strProc);
	lstrcpy(logrec.cDescription, strDesc);

	CString strValOut = strA;
	strValOut += strAH + strModule + strProc + strDesc;

	if (OpenLogFile(cFileName))
	{
		// write the record
		CString strVal;
		int size = sizeof(logrec);
		if (size)
		{
		}
		Write(CW2A(strValOut), strValOut.GetLength());

		//Write(&logrec, sizeof(logrec));
		Write("\n", 1);

		// checking if the file size increased for verification that Write was successful
		if ((long)(GetLength() / RECORDSIZE) > lCurrentRecord)
		{
			// increment the record count
			lCurrentRecord++;
			bRecordAdded = TRUE;
		}

		// close the file
		Close();
	}

	return bRecordAdded;
}

CStringA ConvertUnicodeToUTF8(const CStringW& uni)
{
	if (uni.IsEmpty()) return ""; // nothing to do
	CStringA utf8;
	int cc = 0;
	// get length (cc) of the new multibyte string excluding the \0 terminator first
	if ((cc = WideCharToMultiByte(CP_UTF8, 0, uni, -1, NULL, 0, 0, 0) - 1) > 0)
	{
		// convert
		char* buf = utf8.GetBuffer(cc);
		if (buf) WideCharToMultiByte(CP_UTF8, 0, uni, -1, buf, cc, 0, 0);
		utf8.ReleaseBuffer();
	}
	return utf8;
}

/// <summary>
/// This Function used to record events in the logfile. This has to use for every log recording
/// </summary>
/// <param name="description">Text to logging</param>
/// <returns>BOOL true or false</returns>
BOOL CLogFile::AddRecordU(CString description)
{
	BOOL bRecordAdded = FALSE;
	LogRecord logrec;
	CTime tTime = CTime::GetCurrentTime();	// current time

	memset(&logrec, 0, sizeof(LogRecord));

	CString strA = tTime.Format(L"%m-%d-%Y");
	CString strAH = tTime.Format(L"%H:%M:%S");
	strA += "\t";
	strAH += "\t";

	//CStringA str32;
	//str32 = char(32);

	CString strModule(cModuleName);
	strModule += L"\t";
	//strModule.Replace(str32,"");

	CString strProc(cProcName);
	strProc += L"\t";

	//strProc.Replace(str32,"");

	CString strDesc;
	strDesc = description;

	//	strDesc.Replace(str32,"");

	CStringA strDecA = ConvertUnicodeToUTF8(description);

	//strcat(cModuleName,
	// fill the record
	lstrcpy(logrec.cDate, strA);
	lstrcpy(logrec.cTime, strAH);
	lstrcpy(logrec.cModuleName, strModule);
	lstrcpy(logrec.cProcName, strProc);
	lstrcpy(logrec.cDescription, strDesc);

	CString strValOut = strA;
	strValOut += strAH + strModule + strProc;

	if (OpenLogFile(cFileName))
	{
		// write the record
		CString strVal;
		Write(CW2A(strValOut), strValOut.GetLength());

		Write(strDecA, strDecA.GetLength());

		//Write(&logrec, sizeof(logrec));
		Write("\n", 1);

		// checking if the file size increased for verification that Write was successful
		if ((long)(GetLength() / RECORDSIZE) > lCurrentRecord)
		{
			// increment the record count
			lCurrentRecord++;
			bRecordAdded = TRUE;
		}

		// close the file
		Close();
	}

	return bRecordAdded;
}