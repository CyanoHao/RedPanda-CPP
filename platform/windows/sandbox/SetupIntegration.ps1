
function New-DesktopShortcut() {

  # $WScriptObj = New-Object -ComObject ("WScript.Shell")
  # $Shortcut = $WScriptObj.CreateShortcut("C:\Users\Public\Desktop\Red Panda C++.lnk")
  # $Shortcut.TargetPath = "C:\RedPanda-CPP\RedPanda-CPP.exe"
  # $Shortcut.WorkingDirectory = "C:\Users\WDAGUtilityAccount\Documents"
  # $Shortcut.Save()

  New-Item -ItemType SymbolicLink -Path "C:\Users\Public\Desktop\Red Panda C++.exe" -Target "C:\RedPanda-CPP\RedPanda-CPP.exe"
}

function Set-FileAssociation() {
  param (
      [string]$Extension,
      [string]$Description,
      [string]$IconIndex
  )

  $progId = "DevCpp.$Extension"

  $key = "HKLM:\Software\Classes\.$Extension"
  if (-not (Test-Path $key)) { New-Item -Path $key -Force }
  Set-ItemProperty -Path $key -Name "(Default)" -Value "DevCpp.$progId"

  $key = "HKLM:\Software\Classes\$progId"
  if (-not (Test-Path $key)) { New-Item -Path $key -Force }
  Set-ItemProperty -Path $key -Name "(Default)" -Value $Description

  $key = "HKLM:\Software\Classes\$progId\DefaultIcon"
  if (-not (Test-Path $key)) { New-Item -Path $key -Force }
  Set-ItemProperty -Path $key -Name "(Default)" -Value "C:\RedPanda-CPP\RedPandaIDE.exe,$IconIndex"

  $key = "HKLM:\Software\Classes\$progId\shell\open\command"
  if (-not (Test-Path $key)) { New-Item -Path $key -Force }
  Set-ItemProperty -Path $key -Name "(Default)" -Value "C:\RedPanda-CPP\RedPandaIDE.exe `"%1`""
}

function Set-FileAssociations() {
  Set-FileAssociation -Extension "c" -Description "C/C++ source code" -IconIndex "4"
  Set-FileAssociation -Extension "i" -Description "C source code" -IconIndex "4"
  Set-FileAssociation -Extension "cc" -Description "C++ source code" -IconIndex "5"
  Set-FileAssociation -Extension "cp" -Description "C++ source code" -IconIndex "5"
  Set-FileAssociation -Extension "cpp" -Description "C++ source code" -IconIndex "5"
  Set-FileAssociation -Extension "cxx" -Description "C++ source code" -IconIndex "5"
  Set-FileAssociation -Extension "c++" -Description "C++ source code" -IconIndex "5"
  Set-FileAssociation -Extension "ii" -Description "C++ source code" -IconIndex "5"
  Set-FileAssociation -Extension "h" -Description "C/C++ header" -IconIndex "6"
  Set-FileAssociation -Extension "hh" -Description "C++ header" -IconIndex "7"
  Set-FileAssociation -Extension "hp" -Description "C++ header" -IconIndex "7"
  Set-FileAssociation -Extension "hpp" -Description "C++ header" -IconIndex "7"
  Set-FileAssociation -Extension "hxx" -Description "C++ header" -IconIndex "7"
  Set-FileAssociation -Extension "h++" -Description "C++ header" -IconIndex "7"
  Set-FileAssociation -Extension "tcc" -Description "C++ template" -IconIndex "7"
}

function Restart-Explorer() {
  Stop-Process -Name "explorer.exe"
  Start-Process "explorer.exe"
}

function Add-UninstallerKeys() {
  $key = "HKLM:\Software\Microsoft\Windows\CurrentVersion\Uninstall\RedPanda-CPP"
  if (-not (Test-Path $key)) { New-Item -Path $key -Force }
  Set-ItemProperty -Path $key -Name "UninstallString" -Value "C:\RedPanda-CPP\uninstall.exe"
  Set-ItemProperty -Path $key -Name "InstallLocation" -Value "C:\RedPanda-CPP"
}

function Start-RedPandaCpp() {
  Start-Process "C:\RedPanda-CPP\RedPandaIDE.exe" -WorkingDirectory "C:\Users\WDAGUtilityAccount\Documents"
}

New-DesktopShortcut
Set-FileAssociations
Restart-Explorer
Add-UninstallerKeys
Start-RedPandaCpp
