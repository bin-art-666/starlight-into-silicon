param([Parameter(Mandatory=$true)][string]$Command, [int]$WaitSeconds=2, [string]$Log='')
$ErrorActionPreference='Stop'
$labPort=[System.IO.Ports.SerialPort]::new('COM3',115200,[System.IO.Ports.Parity]::None,8,[System.IO.Ports.StopBits]::One)
try {
    $labPort.Open()
    $labPending=$labPort.ReadExisting()
    $labPort.WriteLine($Command)
    Start-Sleep -Seconds $WaitSeconds
    $labResult=$labPending+$labPort.ReadExisting()
    $labResult
    if($Log){$labResult | Set-Content -LiteralPath $Log -Encoding utf8}
} finally {
    if($labPort.IsOpen){$labPort.Close()}
}
