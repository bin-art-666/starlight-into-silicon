$ErrorActionPreference='Stop'
netsh int ipv4 set dynamicport udp start=49152 num=16384
foreach($labAdapter in Get-NetAdapter | Where-Object { $_.InterfaceDescription -eq 'Realtek USB GbE Family Controller' -or $_.Name -eq 'vEthernet (WSL (Hyper-V firewall))' }) {
    Set-NetIPInterface -InterfaceIndex $labAdapter.ifIndex -AddressFamily IPv4 -Forwarding Disabled
}
Get-NetRoute -DestinationPrefix '192.168.1.101/32' -ErrorAction SilentlyContinue | Where-Object NextHop -eq '172.18.30.133' | Remove-NetRoute -Confirm:$false
Get-NetFirewallRule -Name 'EmbeddedLab3-Return' -ErrorAction SilentlyContinue | Remove-NetFirewallRule
