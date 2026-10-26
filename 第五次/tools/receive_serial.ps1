param([string]$PortName='COM3',[int]$TimeoutSeconds=180)
$ErrorActionPreference='Stop'
$labRoot=Split-Path $PSScriptRoot -Parent
$labSerial=[IO.Ports.SerialPort]::new($PortName,115200,[IO.Ports.Parity]::None,8,[IO.Ports.StopBits]::One)
$labSerial.ReadBufferSize=1048576
$labBuffer=[Text.StringBuilder]::new()
try {
    $labSerial.Open()
    [void]$labSerial.ReadExisting()
    $labSerial.Write("printf '\nLAB5_DATA_BEGIN\n'; cat /home/root/lab5/evidence.b64; printf '\nLAB5_DATA_END\n'`r")
    $labTimer=[Diagnostics.Stopwatch]::StartNew()
    while($labTimer.Elapsed.TotalSeconds -lt $TimeoutSeconds){
        [void]$labBuffer.Append($labSerial.ReadExisting())
        if($labBuffer.ToString() -match '(?s)\r?\nLAB5_DATA_BEGIN\r?\n([A-Za-z0-9+/=\r\n]+)\r?\nLAB5_DATA_END\r?\n'){
            $labMatch=$Matches[1] -replace '\s',''
            [IO.File]::WriteAllBytes((Join-Path $labRoot '实测记录\evidence.zlib'),[Convert]::FromBase64String($labMatch))
            Write-Output ('SERIAL_TRANSFER_OK base64_bytes='+$labMatch.Length)
            return
        }
        Start-Sleep -Milliseconds 100
    }
    throw 'Serial transfer timed out; no complete evidence archive received.'
} finally {
    if($labSerial.IsOpen){$labSerial.Close()}
}
