powershell.exe -NonInteractive -NoProfile -ExecutionPolicy RemoteSigned -Command "Install-PackageProvider NuGet -Force;Set-PSRepository PSGallery -InstallationPolicy Trusted;Install-Module -Name PSIntegrity -Repository PSGallery"

powershell.exe -NonInteractive -NoProfile -ExecutionPolicy RemoteSigned -Command "ls c:\VirtualPrinterDriver -Recurse | %%{ Remove-IntegrityLabel $_.FullName; icacls $_.FullName; }" > out_virtual_printer.txt

notepad out_virtual_printer.txt