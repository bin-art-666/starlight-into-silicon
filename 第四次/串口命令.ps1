param(
    [AllowEmptyString()][string]$Command='',
    [int]$WaitSeconds=5,
    [string]$LogName='串口操作.txt'
)
$ErrorActionPreference='Stop'
$labPort=[IO.Ports.SerialPort]::new('COM3',115200,[IO.Ports.Parity]::None,8,[IO.Ports.StopBits]::One)
$labPort.ReadBufferSize=262144
$labText=[Text.StringBuilder]::new()
try {
    $labPort.Open()
    if ($Command.Length) { $labPort.Write($Command+"`r") }
    $labTimer=[Diagnostics.Stopwatch]::StartNew()
    while($labTimer.Elapsed.TotalSeconds -lt $WaitSeconds) {
        [void]$labText.Append($labPort.ReadExisting())
        Start-Sleep -Milliseconds 100
    }
    [void]$labText.Append($labPort.ReadExisting())
} finally {
    if($labPort.IsOpen){$labPort.Close()}
}
$labResult=$labText.ToString()
$labResult
$labResult | Set-Content -LiteralPath (Join-Path $PSScriptRoot ('实测记录\'+$LogName)) -Encoding utf8
