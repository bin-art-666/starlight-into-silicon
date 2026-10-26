$ErrorActionPreference='Stop'
$labPrograms=@((Join-Path $PSScriptRoot 'tools\WinNFSd.exe'),'C:\Users\wuwei\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe')
$labCounter=0
foreach($labProgram in $labPrograms) {
    $labName='EmbeddedLab3-Native-'+$labCounter
    if (!(Get-NetFirewallRule -Name $labName -ErrorAction SilentlyContinue)) {
        New-NetFirewallRule -Name $labName -DisplayName $labName -Program $labProgram -Direction Inbound -Action Allow -Protocol Any -LocalAddress 192.168.1.100 -RemoteAddress 192.168.1.4
    }
    $labCounter++
}
