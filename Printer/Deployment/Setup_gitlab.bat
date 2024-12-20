@setlocal enableextensions
@cd /d "%~dp0"
"%msbuild%" "..\Source Code\InstallPrinter 64\InstallPrinterUnitTest.vcxproj" /p:configuration=release /p:platform=x64
echo F | xcopy /s/z "..\Source Code\bin\InstallPrinterUnitTest64.dll" InstallPrinterUnitTest64.dll /K /H /Y
CALL :UnitTest "InstallPrinterUnitTest64.dll"
if %errorlevel% neq 0 exit /b %errorlevel%
"%msbuild%" "..\Source Code\InstallPrinter 64\InstallPrinterUnitTest.vcxproj" /p:configuration=release /p:platform=win32
echo F | xcopy /s/z "..\Source Code\bin\InstallPrinterUnitTest.dll" InstallPrinterUnitTest.dll /K /H /Y
CALL :UnitTest "InstallPrinterUnitTest.dll"
if %errorlevel% neq 0 exit /b %errorlevel%
"%msbuild%" "..\Source Code\Services and Codes\VirtualPrinterDriverSpooler\VirtualPrinterDriverSpooler\VirtualPrinterSpoolerUnitTest.vcxproj" /p:configuration=release /p:platform=win32
echo F | xcopy /s/z "..\Source Code\bin\VirtualPrinterSpoolerUnitTest.dll" VirtualPrinterSpoolerUnitTest.dll /K /H /Y
Rem CALL :UnitTest "VirtualPrinterSpoolerUnitTest.dll"
Rem if %errorlevel% neq 0 exit /b %errorlevel%
"%msbuild%" "..\Source Code\Services and Codes\VirtualPrinterDriverSpooler\VirtualPrinterDriverSpooler\VirtualPrinterDriverSpooler.vcxproj" /p:configuration=release /p:platform=win32
echo F | xcopy /s/z "..\Source Code\bin\InstallPrinter64.exe" InstallPrinter64.exe /K /H /Y
echo F | xcopy /s/z "..\Source Code\bin\InstallPrinter.exe" InstallPrinter.exe /K /H /Y
echo F | xcopy /s/z "..\Source Code\bin\gswin32cM.exe" "Xtra\Prerequisite\kbgs\kbbin\bin\gswin32cM.exe" /K /H /Y
CALL :CodeSign "InstallPrinter64.exe"
CALL :CodeSign "InstallPrinter.exe"
CALL :CodeSign "Xtra\Prerequisite\kbgs\kbbin\bin\gswin32cM.exe"
"C:\Program Files (x86)\NSIS\makensis.exe" "Virtual Printer Driver.nsi"
CALL :CodeSign "TaxDome_printer_setup.exe"
copy "TaxDome_printer_setup.exe" "../../Installer/TaxDome_printer_setup.exe"
EXIT /B %ERRORLEVEL%

:CodeSign
"C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe" sign /t http://timestamp.digicert.com /f "c:\Projects\TaxDome\windows_app\Installer\Signature\taxdome win certificate.pfx" /p taxdome %~1
EXIT /B 0

:UnitTest
"C:\testPlatform\tools\net462\Common7\IDE\Extensions\TestPlatform\vstest.console.exe" %~1
echo "Please run with elevated session"
EXIT /B %ERRORLEVEL%
