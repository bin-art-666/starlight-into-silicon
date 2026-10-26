$ErrorActionPreference='Stop'
try {
    New-NetFirewallRule -Name 'EmbeddedLab3-Return' -DisplayName 'EmbeddedLab3-Return' -Direction Inbound -Action Allow -RemoteAddress @('192.168.1.101','172.18.30.133') -LocalAddress '192.168.1.4' -InterfaceAlias 'vEthernet (WSL (Hyper-V firewall))'
    New-NetFirewallRule -Name 'EmbeddedLab3-Ping' -DisplayName 'EmbeddedLab3-Ping' -Direction Inbound -Action Allow -Protocol ICMPv4 -RemoteAddress 192.168.1.4 -InterfaceAlias (Get-NetAdapter | Where-Object InterfaceDescription -eq 'Realtek USB GbE Family Controller').Name
    New-NetFirewallHyperVRule -Name 'EmbeddedLab3-HostTCP' -DisplayName 'EmbeddedLab3-HostTCP' -Direction Inbound -VMCreatorId '{40E0AC32-46A5-438A-A0B2-2B479E8F2E90}' -Protocol TCP -LocalPorts @('111','2049','20048') -RemoteAddresses '172.18.16.1' -Action Allow
    'SUCCESS' | Set-Content (Join-Path $PSScriptRoot 'return-rule-result.txt')
} catch { $_ | Out-String | Set-Content (Join-Path $PSScriptRoot 'return-rule-result.txt') }
