$ErrorActionPreference='Stop'
netsh int ipv4 show dynamicport udp | Out-File (Join-Path $PSScriptRoot 'udp-range-before.txt')
netsh int ipv4 set dynamicport udp start=32768 num=32768
if($LASTEXITCODE -ne 0) { throw 'UDP range change failed' }
netsh int ipv4 show dynamicport udp | Out-File (Join-Path $PSScriptRoot 'udp-range-after.txt')
