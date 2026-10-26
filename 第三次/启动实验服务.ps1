$ErrorActionPreference='Stop'
$labBase=$PSScriptRoot
$labPython='C:\Users\wuwei\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
$labCurrent=@{}
$labPidFile=Join-Path $labBase '实测记录\本机服务进程.json'
if(Test-Path -LiteralPath $labPidFile) {
    $labSaved=Get-Content -LiteralPath $labPidFile -Raw | ConvertFrom-Json
    foreach($labKind in @('NFS','TFTP')) {
        $labProcess=Get-Process -Id $labSaved.$labKind -ErrorAction SilentlyContinue
        if($labProcess -and (($labKind -eq 'NFS' -and $labProcess.Path -eq (Join-Path $labBase 'tools\WinNFSd.exe')) -or ($labKind -eq 'TFTP' -and $labProcess.Path -eq $labPython))) { $labCurrent[$labKind]=$labProcess.Id }
    }
}
if(!$labCurrent.ContainsKey('NFS')) {
    $labExisting=Get-Process -Name WinNFSd -ErrorAction SilentlyContinue | Where-Object Path -eq (Join-Path $labBase 'tools\WinNFSd.exe') | Select-Object -First 1
    if($labExisting) { $labCurrent.NFS=$labExisting.Id }
}
if(!$labCurrent.ContainsKey('TFTP')) {
    $labExisting=Get-CimInstance Win32_Process -Filter "Name='python.exe'" | Where-Object { $_.ExecutablePath -eq $labPython -and $_.CommandLine -like '*start_tftp.py*' } | Select-Object -First 1
    if($labExisting) { $labCurrent.TFTP=$labExisting.ProcessId }
}
if(!$labCurrent.ContainsKey('NFS')) {
    $labProcess=Start-Process -FilePath (Join-Path $labBase 'tools\WinNFSd.exe') -ArgumentList '-addr 192.168.1.100 -log on . /lab3' -WorkingDirectory (Join-Path $labBase 'nfs') -WindowStyle Hidden -RedirectStandardOutput (Join-Path $labBase '实测记录\Windows_NFS.log') -RedirectStandardError (Join-Path $labBase '实测记录\Windows_NFS_error.log') -PassThru
    $labCurrent.NFS=$labProcess.Id
}
if(!$labCurrent.ContainsKey('TFTP')) {
    $labProcess=Start-Process -FilePath $labPython -ArgumentList ('"'+(Join-Path $labBase 'start_tftp.py')+'"') -WorkingDirectory $labBase -WindowStyle Hidden -RedirectStandardError (Join-Path $labBase '实测记录\Windows_TFTP_error.log') -PassThru
    $labCurrent.TFTP=$labProcess.Id
}
$labCurrent | ConvertTo-Json | Set-Content -LiteralPath $labPidFile
Write-Output 'Server: 192.168.1.100; TFTP: tftp/; NFS: /lab3'
