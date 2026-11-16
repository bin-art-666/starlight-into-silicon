$ErrorActionPreference='Stop'
$port=[System.IO.Ports.SerialPort]::new('COM3',115200,[System.IO.Ports.Parity]::None,8,[System.IO.Ports.StopBits]::One)
try {
    $port.Open()
    $null=$port.ReadExisting()
    $port.WriteLine('echo LAB8_PHOTO_BEGIN; base64 /home/root/lab8/photos/20210722_052042_655_4e402c2d.png; echo LAB8_PHOTO_END')
    $buffer=[System.Text.StringBuilder]::new()
    $timer=[System.Diagnostics.Stopwatch]::StartNew()
    while($timer.Elapsed.TotalSeconds -lt 90) {
        $null=$buffer.Append($port.ReadExisting())
        $raw=$buffer.ToString()
        if($raw -match '(?s)\r?\nLAB8_PHOTO_BEGIN\r?\n(.*?)\r?\nLAB8_PHOTO_END\r?\n') {
            $bytes=[Convert]::FromBase64String(($Matches[1] -replace '\s',''))
            $target=Join-Path $PSScriptRoot '..\实测记录\20210722_052042_655_4e402c2d.png'
            [System.IO.File]::WriteAllBytes([System.IO.Path]::GetFullPath($target),$bytes)
            Write-Output "Photo downloaded: $($bytes.Length) bytes"
            exit 0
        }
        Start-Sleep -Milliseconds 100
    }
    throw 'Photo transfer timed out; no image saved.'
} finally {
    if($port.IsOpen){$port.Close()}
    $port.Dispose()
}
