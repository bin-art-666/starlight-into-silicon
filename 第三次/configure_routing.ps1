$ErrorActionPreference='Stop'
Start-Transcript -LiteralPath (Join-Path $PSScriptRoot 'routing.log') -Force
try {
    foreach($labAlias in @('以太网 3','vEthernet (WSL (Hyper-V firewall))')) {
        Get-NetIPInterface -InterfaceAlias $labAlias -AddressFamily IPv4 | Select-Object InterfaceAlias,Forwarding
        Set-NetIPInterface -InterfaceAlias $labAlias -AddressFamily IPv4 -Forwarding Enabled
    }
    if (!(Get-NetFirewallHyperVRule -Name 'EmbeddedLab3-ICMP' -ErrorAction SilentlyContinue)) {
        New-NetFirewallHyperVRule -Name 'EmbeddedLab3-ICMP' -DisplayName 'EmbeddedLab3-ICMP' -Direction Inbound -VMCreatorId '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -Protocol 1 -RemoteAddresses '192.168.1.4' -Action Allow
    }
    'SUCCESS' | Set-Content (Join-Path $PSScriptRoot 'routing-result.txt')
} catch { $_ | Out-String | Set-Content (Join-Path $PSScriptRoot 'routing-result.txt'); throw }
finally { Stop-Transcript }
