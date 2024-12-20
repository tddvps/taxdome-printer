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
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/// <summary>
/// Constructor of this class 
/// </summary>
/// 
CLogFile::CLogFile()
	:lCurrentRecord(0)
	,bArchiveIfMaxReached(FALSE)
{
	Set_cFileName("");
	Set_cArchiveName("");
	Set_HeaderRec();
	Set_cModuleName("");
	Set_cProcName("");
}
/// <summary>
/// Constructor of this class 
/// </summary>
/// <param name="Filename">FilePath</param>
/// 
CLogFile::CLogFile(LPCSTR filename, LPCSTR archivename)
	:lCurrentRecord(0)
	,bArchiveIfMaxReached(FALSE)
{
	Set_cFileName(filename);
	Set_cArchiveName(archivename);
	Set_HeaderRec();
	Set_cModuleName("");
	Set_cProcName("");
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
BOOL CLogFile::OpenLogFile(LPCSTR filename)
{
	// I am not keeping the log file open all of the time just 
	// to make sure it always gets closed
	BOOL bFileOpened = FALSE;
	
	// we don't want to open the file if there is no file name
	if (cFileName[0] != '\0')
	{
		///AfxMessageBox("2");
		CStringA strFile = cFileName;

		if (Open((LPCTSTR) strFile, CFile::modeCreate | CFile::modeNoTruncate | 
			CFile::modeReadWrite | CFile::shareDenyNone))
		{
			// return value
			bFileOpened = TRUE;

			DWORD length = (DWORD)GetLength();

			if (length == 0)
			{
				// write the header
				AddHeaderRecord();
			}
			else
			{
				// get the current record
				lCurrentRecord = (length/RECORDSIZE);

				// check if we have reached the maximum size of the log
				if (lCurrentRecord == (long)MAXRECORDS)
				{
					SeekToBegin();

					// archive this log file?
					if (bArchiveIfMaxReached)
					{
						CFile archive;
							CStringA sarchive = cArchiveName;

			

						// open/create the archive file
						if (archive.Open((LPCTSTR)sarchive, CFile::modeCreate | CFile::modeNoTruncate | 
							CFile::modeReadWrite))
						{
							// write the header if you have to
							if (archive.SeekToEnd() == 0)
							{

								

								CStringA strHeader = "DATE\tTIME\tMODULE\tPROCEDURE\tDESCRIPTION";


								archive.Write(strHeader, strHeader.GetLength());
								//archive.Write(&HeaderRec, sizeof(LogRecord));
								archive.Write("\r\n", 2);
								archive.SeekToEnd();
							}
							
							// add the current records to the end of the archive
							for (long line = 1; line < lCurrentRecord; line++)
							{
								char cLine[RECORDSIZE];
								long lng = line* (RECORDSIZE);
								Seek(lng, CFile::begin);
								if (Read(cLine, RECORDSIZE) > 0)
								{
									archive.Write(cLine, RECORDSIZE);
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
						Seek(2*RECORDSIZE, CFile::begin);

						// read in all records except the header and oldest record
						long buffsize =  (long)(RECORDSIZE*(MAXRECORDS - 2));
						char* cBuffer = new char[buffsize];
						memset(cBuffer, 0, buffsize);

						Read(cBuffer, buffsize);

						// truncate the file to 0 length
						SetLength(0);

						// add the header back on
						AddHeaderRecord();

						// add the buffered records back on
						Write(cBuffer, buffsize);

						delete [] cBuffer;
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

	lstrcpy(HeaderRec.cDate, "DATE\t");
	lstrcpy(HeaderRec.cTime, "TIME\t");
	lstrcpy(HeaderRec.cModuleName, "MODULE\t");
	lstrcpy(HeaderRec.cProcName, "PROCEDURE\t");
	lstrcpy(HeaderRec.cDescription, "DESCRIPTION                                                                          ");
}
/// <summary>
/// Adding the title text(heading) in the log file 
/// </summary>
BOOL CLogFile::AddHeaderRecord()
{
	BOOL bRecordAdded = FALSE;
	
	// log file is already open because this method
	// is only called from OpenLogFile
	
								CStringA strHeader = "DATE\tTIME\tMODULE\tPROCEDURE\tDESCRIPTION";


								Write(strHeader, strHeader.GetLength());

	//Write(&HeaderRec, sizeof(LogRecord));
	Write("\r\n", 2);

	// checking if the file size increased for verification that Write was successful
	if ((long)(GetLength()/RECORDSIZE) > lCurrentRecord)
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
BOOL CLogFile::AddRecord(LPCSTR description)
{
	BOOL bRecordAdded = FALSE;
	LogRecord logrec;
	CTime tTime = CTime::GetCurrentTime();	// current time

	memset(&logrec, 0, sizeof(LogRecord));

	CString strA = tTime.Format("%m-%d-%Y");
	CString strAH = tTime.Format("%H:%M:%S");
	strA += "\t";
	strAH += "\t";

	//CStringA str32;
	//str32 = char(32);

	CStringA strModule = cModuleName;
	strModule += "\t";
	//strModule.Replace(str32,"");

	CStringA strProc = cProcName;
	strProc += "\t";

	//strProc.Replace(str32,"");

	CStringA strDesc;
	strDesc = description;

	//	strDesc.Replace(str32,"");


	//strcat(cModuleName,
	// fill the record
	lstrcpy(logrec.cDate, strA);
	lstrcpy(logrec.cDate,  strA);
	lstrcpy(logrec.cTime,  strAH);
	lstrcpy(logrec.cModuleName,  strModule);
	lstrcpy(logrec.cProcName,  strProc);
	lstrcpy(logrec.cDescription,  strDesc);

	/*
	strcpy_s(logrec.cDate, strA.GetLength(), strA );
	strcpy_s(logrec.cTime, strAH.GetLength(), strAH );
	strcpy_s(logrec.cModuleName, strModule.GetLength(), strModule );
	strcpy_s(logrec.cProcName, strProc.GetLength(), strProc );
	strcpy_s(logrec.cDescription, strDesc.GetLength(), strDesc );
	*/
	CStringA strValOut = strA;
	strValOut +=  strAH + strModule + strProc + strDesc;
		
	if (OpenLogFile(cFileName))
	{
		// write the record
		CString strVal;
		int size = sizeof(logrec);
		Write(strValOut,strValOut.GetLength());
		
		//Write(&logrec, sizeof(logrec));
		Write("\r\n", 2);

		// checking if the file size increased for verification that Write was successful
		if ((long)(GetLength()/RECORDSIZE) > lCurrentRecord)
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