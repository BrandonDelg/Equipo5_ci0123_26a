
# Configuración de L2

## Comandos de configuración

```bash
enable
configure terminal

hostname isla5

vlan 250
 name ISLA5


ip dhcp excluded-address 172.16.123.81
ip dhcp excluded-address 172.16.123.82

ip dhcp pool isla5
 network 172.16.123.80 255.255.255.240
 default-router 172.16.123.81

spanning-tree mode pvst
spanning-tree extend system-id

interface FastEthernet0/1
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/2
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/3
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/4
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/5
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/6
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/7
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/8
 switchport access vlan 250
 switchport mode access

interface FastEthernet0/9
 switchport access vlan 250
 switchport mode access


interface FastEthernet0/10
 switchport access vlan 250
 switchport mode access


interface FastEthernet0/11
 switchport access vlan 250
 switchport mode access


interface FastEthernet0/12
 switchport access vlan 250
 switchport mode access



interface FastEthernet0/13
 switchport mode trunk

interface Vlan250
 ip address 172.16.123.82 255.255.255.240
 no shutdown

ip default-gateway 172.16.123.81


end
write memory
```

---

## show running-config

```bash
!
version 12.2
no service pad
service timestamps debug uptime
service timestamps log uptime
no service password-encryption
!
hostname isla5
!
!
no aaa new-model
system mtu routing 1500
ip subnet-zero
ip dhcp excluded-address 172.16.123.81
ip dhcp excluded-address 172.16.123.82
!
ip dhcp pool isla5
   network 172.16.123.80 255.255.255.240
   default-router 172.16.123.81
!
no file verify auto
spanning-tree mode pvst
spanning-tree extend system-id
!
vlan internal allocation policy ascending
!
interface FastEthernet0/1
 switchport access vlan 250
 switchport mode access
!
interface FastEthernet0/2
 switchport access vlan 250
 switchport mode access
!
interface FastEthernet0/3
 switchport mode trunk
!
interface Vlan250
 ip address 172.16.123.82 255.255.255.240
 no ip route-cache
!
ip http server
!
control-plane
!
line con 0
line vty 5 15
!
end
```

---

## show vlan brief

```bash
isla5#show vlan brief

VLAN Name                             Status    Ports
---- -------------------------------- --------- -----------------------------
1    default                          active    Fa0/4, Fa0/5, Fa0/6, Fa0/7
                                                Fa0/8, Fa0/9, Fa0/10, Fa0/11
                                                Fa0/12, Fa0/13, Fa0/14, Fa0/15
                                                Fa0/16, Fa0/17, Fa0/18, Fa0/19
                                                Fa0/20, Fa0/21, Fa0/22, Fa0/23
                                                Fa0/24, Gi0/1, Gi0/2

250  VLAN0250                         active    Fa0/1, Fa0/2

1002 fddi-default                     act/unsup
1003 token-ring-default               act/unsup
1004 fddinet-default                  act/unsup
1005 trnet-default                    act/unsup
```

# Configuración de L3

## Comandos de configuración L3

```bash
enable
configure terminal

hostname L3-capa8

ip routing
ip subnet-zero

spanning-tree mode pvst
spanning-tree extend system-id

vlan 200
 name VLAN200

vlan 210
 name VLAN210

vlan 220
 name VLAN220

vlan 230
 name VLAN230

vlan 240
 name VLAN240

vlan 250
 name VLAN250

vlan 260
 name VLAN260

interface range FastEthernet0/1 - 7
 switchport trunk encapsulation dot1q
 switchport mode trunk
 no shutdown

interface Vlan200
 ip address 172.16.123.1 255.255.255.240
 no shutdown

interface Vlan210
 ip address 172.16.123.17 255.255.255.240
 no shutdown

interface Vlan220
 ip address 172.16.123.33 255.255.255.240
 no shutdown

interface Vlan230
 ip address 172.16.123.49 255.255.255.240
 no shutdown

interface Vlan240
 ip address 172.16.123.65 255.255.255.240
 no shutdown

interface Vlan250
 ip address 172.16.123.81 255.255.255.240
 no shutdown

interface Vlan260
 ip address 172.16.123.97 255.255.255.240
 no shutdown

ip classless
ip http server

end
write memory
```

---

## show running-config

```bash
!
hostname L3-capa8
!
no aaa new-model
ip subnet-zero
ip routing
!
no file verify auto
spanning-tree mode pvst
spanning-tree extend system-id
!
vlan internal allocation policy ascending
!
vlan 200
 name VLAN200
!
vlan 210
 name VLAN210
!
vlan 220
 name VLAN220
!
vlan 230
 name VLAN230
!
vlan 240
 name VLAN240
!
vlan 250
 name VLAN250
!
vlan 260
 name VLAN260
!
interface FastEthernet0/1
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/2
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/3
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/4
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/5
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/6
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface FastEthernet0/7
 switchport trunk encapsulation dot1q
 switchport mode trunk
!
interface Vlan1
 no ip address
 shutdown
!
interface Vlan200
 ip address 172.16.123.1 255.255.255.240
!
interface Vlan210
 ip address 172.16.123.17 255.255.255.240
!
interface Vlan220
 ip address 172.16.123.33 255.255.255.240
!
interface Vlan230
 ip address 172.16.123.49 255.255.255.240
!
interface Vlan240
 ip address 172.16.123.65 255.255.255.240
!
interface Vlan250
 ip address 172.16.123.81 255.255.255.240
!
interface Vlan260
 ip address 172.16.123.97 255.255.255.240
!
ip classless
ip http server
!
control-plane
!
line con 0
line vty 0 4
 no login
line vty 5 15
 no login
!
end
```
