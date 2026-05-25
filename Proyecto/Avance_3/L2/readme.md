# Configuración de L2

## Comandos de configuración

```bash
enable
configure terminal

hostname SW-Isla5

no ip domain-lookup
service dhcp

vlan 250
 name ISLA5
 ip default-gateway 172.16.123.81

interface range fastEthernet0/1 - 12
 switchport mode access
 switchport access vlan 250
 no shutdown

interface fastEthernet0/24
 switchport mode trunk
 no shutdown

interface vlan 250
 ip address 172.16.123.82 255.255.255.240
 no shutdown

ip default-gateway 172.16.123.81

ip dhcp excluded-address 172.16.123.81 172.16.123.82

ip dhcp pool ISLA5
 network 172.16.123.80 255.255.255.240
 default-router 172.16.123.81
 dns-server 8.8.8.8

end
write memory
```

---

## show running-config

```bash
show running-config
```

---

## vlan brief

```bash
show vlan brief
```
