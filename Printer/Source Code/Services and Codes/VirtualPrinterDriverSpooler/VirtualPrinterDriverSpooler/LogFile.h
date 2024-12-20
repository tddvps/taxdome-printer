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

// LogFile.h: interface for the CLogFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGFILE_H)
#define _LOGFILE_H

#include <afx.h>
#include "StdioFileEx.h"
#define RECORDSIZE	140		// maximum size of each record
#define MAXRECORDS	9999999		// maximum number of records in the log (including header)
/// <summary>
/// Structure which holds log file with fields Date, Time, ModuleName, Procedure name and Description 
/// </summary>
struct LogRecord
{
	TCHAR cDate[22];			// date of record entry
	TCHAR cTime[20];			// time of record entry
	TCHAR cModuleName[40];	// name of the exe the record is associated with
	TCHAR cProcName[40];		// name of the procedure generating the record
	TCHAR cDescription[1000];	// description of the event being logged
	// leave 2 bytes for \r\n at the end of the record
};
/// <summary>
   /// This class will generate logfile when we set DEBUG>0, for errors it will log automatically
   /// </summary>
class CLogFile : public CStdioFileEx  
{
public:
	CLogFile();											// default contructor
	CLogFile(LPWSTR filename, LPWSTR archivename = L"");	
	virtual ~CLogFile();

	void Set_cFileName(CString filename) { lstrcpy((wchar_t*)cFileName, filename); };
	LPWSTR Get_cFileName() { return cFileName; };

	void Set_cArchiveName(LPWSTR archivename) { 
		lstrcpy(cArchiveName, archivename);

		if (archivename[0] != '\0')
			bArchiveIfMaxReached = TRUE;
	};
	LPWSTR Get_cArchiveName() { return cArchiveName; };	

	void Set_cModuleName(LPWSTR module) { lstrcpy((wchar_t*)cModuleName, module); };

	void Set_cProcName(LPWSTR proc) { lstrcpy((wchar_t*)cProcName, proc); };

	BOOL AddRecord(CString description);
	BOOL AddRecordU(CString description);
	TCHAR cFileName[MAX_PATH + 1];
protected:
	BOOL bArchiveIfMaxReached;			// archive the log file
		// log file full path
	TCHAR cArchiveName[MAX_PATH + 1];	// archive file full path
	TCHAR cModuleName[21];				// name of the module using this log
	TCHAR cProcName[21];					// name of the procedure generating the record
	LogRecord HeaderRec;				// header record
	long lCurrentRecord;				// the number of the last record in the log

	BOOL OpenLogFile(LPWSTR filename);

	void Set_HeaderRec();
	BOOL AddHeaderRecord();
};

#endif
