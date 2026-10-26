$ErrorActionPreference = 'Stop'
$labLinuxDir = '/mnt/d/Develop/Linux/嵌入式/第六次'
Start-Process -FilePath 'wsl.exe' -ArgumentList @('-d', 'sweet', '--cd', "$labLinuxDir/qt_text_io", '--', "$labLinuxDir/bin/qt_text_io") -WindowStyle Hidden
