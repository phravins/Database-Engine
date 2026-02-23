$packageName = 'v2vdb'
$toolsDir = "$(Split-Path -parent $MyInvocation.MyCommand.Definition)"
$url = 'https://github.com/phravins/Database-Engine/releases/download/v1.0.0/v2vdb-windows.exe'
$checksum = 'REPLACE_WITH_SHA256'

Install-ChocolateyPackage -PackageName "$packageName" `
    -FileType 'exe' `
    -Url "$url" `
    -Checksum "$checksum" `
    -ChecksumType 'sha256' `
    -SilentArgs "/S" `
    -ValidExitCodes @(0)
