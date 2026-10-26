$ErrorActionPreference='Stop'
try {
    $labIf=Get-NetIPInterface -InterfaceAlias 'vEthernet (WSL (Hyper-V firewall))' -AddressFamily IPv4
    if (!(Get-NetRoute -DestinationPrefix '192.168.1.101/32' -ErrorAction SilentlyContinue)) {
        New-NetRoute -DestinationPrefix '192.168.1.101/32' -InterfaceIndex $labIf.InterfaceIndex -NextHop '172.18.30.133' -PolicyStore ActiveStore
    }
    'SUCCESS' | Set-Content (Join-Path $PSScriptRoot 'server-route-result.txt')
} catch { $_ | Out-String | Set-Content (Join-Path $PSScriptRoot 'server-route-result.txt') }
