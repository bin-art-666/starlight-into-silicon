$ErrorActionPreference = 'Stop'
Start-Transcript -LiteralPath (Join-Path $PSScriptRoot '实测记录\Windows网络配置.log') -Force
try {
    $labAdapter = Get-NetAdapter | Where-Object InterfaceDescription -eq 'Realtek USB GbE Family Controller'
    if (@($labAdapter).Count -ne 1) { throw 'Cannot uniquely identify experiment USB Ethernet adapter.' }
    Get-NetIPAddress -InterfaceIndex $labAdapter.ifIndex -AddressFamily IPv4 | Format-List *
    if (!(Get-NetIPAddress -InterfaceIndex $labAdapter.ifIndex -AddressFamily IPv4 | Where-Object IPAddress -eq '192.168.1.100')) {
        New-NetIPAddress -InterfaceIndex $labAdapter.ifIndex -IPAddress 192.168.1.100 -PrefixLength 24 -AddressFamily IPv4
    }
    $labCreator = '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}'
    foreach ($labProtocol in @('TCP','UDP')) {
        $labRule = 'EmbeddedLab3-' + $labProtocol
        if (!(Get-NetFirewallHyperVRule -Name $labRule -ErrorAction SilentlyContinue)) {
            New-NetFirewallHyperVRule -Name $labRule -DisplayName $labRule -Direction Inbound -VMCreatorId $labCreator -Protocol $labProtocol -LocalPorts @('69','111','2049','20048','50000-50020') -RemoteAddresses '192.168.1.4' -Action Allow
        }
        if (!(Get-NetFirewallRule -Name $labRule -ErrorAction SilentlyContinue)) {
            New-NetFirewallRule -Name $labRule -DisplayName $labRule -Direction Inbound -Action Allow -Protocol $labProtocol -LocalPort @('69','111','2049','20048','50000-50020') -RemoteAddress 192.168.1.4 -InterfaceAlias $labAdapter.Name
        }
    }
    'SUCCESS' | Set-Content -LiteralPath (Join-Path $PSScriptRoot '实测记录\Windows配置结果.txt')
} catch {
    $_ | Out-String | Set-Content -LiteralPath (Join-Path $PSScriptRoot '实测记录\Windows配置结果.txt')
    throw
} finally { Stop-Transcript }
