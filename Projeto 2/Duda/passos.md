Passos:
Y = Bancada em que estamos
X = Portas que estão com as luzes verdes

- Verificar o pc que está ligado -- ScrLk 2x + escolher pc (4)
- GTK:
  - configurações - port - baud - rate = 115200
  - /system reset-configuration

- Terminal:
  - ifconfig
  - route -n

---

- GTK (PC4):
``` bash
  - /interface bridge add name=bridgeY0
  - /interface bridge add name=bridgeY1
  - /interface bridge print
  - /interface bridge port print brief
  - /interface bridge port remove [find interface=etherX1]
  - /interface bridge port remove [find interface=etherX2]
  - /interface bridge port remove [find interface=etherX3]
  - /interface bridge port remove [find interface=etherX4]
  - /interface bridge port add interface=etherX1 bridge=bridgeY0
  - /interface bridge port add interface=etherX2 bridge=bridgeY0
  - /interface bridge port add interface=etherX3 bridge=bridgeY1
  - /interface bridge port add interface=etherX4 bridge=bridgeY1
  - /interface bridge print brief
```

---

- Terminal (PC4):
```bash
  - sysctl net.ipv4.ip_forward=1 (Enable IP)
  - sysctl net.ipv4.icmp_echo_ignore_broadcasts=0
  - ifconfig
```
---

# Bancada 6:
## Exp.1
- GTK (PC4):
  - configurações - port - baud - rate = 115200
  - /system reset-configuration

- Terminal (PC4):
  - ifconfig
  - route -n

1. Disconnect switch from netlab and connect tux computers
2. Configure tuxy3 and tuxy4 using ifconfig and route commands
3. Register the IP and MAC addresses of network interfaces
- Ligar os cabos (tuxY3 E0 and tuxY4 E0)
- tuxY3:
```bash
   systemctl restart networking
   ifconfig eth1 172.16.Y0.1/24
   ifconfig
```
- tuxY4:
```bash
   systemctl restart networking
   ifconfig eth1 172.16.Y0.254/24
   ifconfig
```
tuxY4:
MAC: 00:01:02:a1:35:69
IP: 172.16.60.254

tuxY3:
MAC: 00:c0:df:25:40:66
IP: 172.16.60.1

4. Use ping command to verify connectivity between these computers
- tuxY3:
```bash
  ping 172.16.Y0.254 (deu certo)
```
- tuxY4:
```bash
  ping 172.16.Y0.1 (deu certo)
```

5. Inspect forwarding (route –n) and ARP (arp–a) tables (ignoramos este passo basicamente)
```bash
  route -n (associa endereço IP à porta para saber por onde mandar)
  arp -a (associa endereço IP a um endereço MAC)
```
6. Delete ARP table entries in tuxy3 (arp–d ipaddress)
```bash
  arp -d 172.16.Y0.254
  arp -a (não retorna nada)
```

7. Start Wireshark in tuxy3.eth0 and start capturing packets (experiencia1-7)
8. In tuxy3, ping tuxy4 for a few seconds
- tuxY3
```bash
  ping 172.16.Y0.254
```
9. Stop capturing packets
10. Save log study it at home


## Exp.2
1. Configure tuxy2 and register its IP and MAC addresses
- tuxY2:
```bash
  systemctl restart networking
  ifconfig eth1 172.16.Y1.1/24
  ifconfig
```
tuxY2 
IP: 172.16.61.1/24
MAC: 00:e0:7d:b5:8c:8e

2. Create two bridges in the switch: bridgeY0 and bridgeY1
``` bash
  - /interface bridge add name=bridge60
  - /interface bridge add name=bridge61
  - /interface bridge print
  - /interface bridge port print brief
  
```

3. Remove the ports where tuxY3, tuxY4 and tuxY2 are connected from the default bridge (bridge) and add them the corresponding ports to bridgeY0 and bridgeY
```bash
  - /interface bridge port remove [find interface=ether1]
  - /interface bridge port remove [find interface=ether3]
  - /interface bridge port remove [find interface=ether5]
  - /interface bridge port add interface=ether1 bridge=bridge60
  - /interface bridge port add interface=ether3 bridge=bridge60
  - /interface bridge port add interface=ether5 bridge=bridge61
  - /interface bridge print brief
```

4. Start capture at tuxY3.eth0 (Wireshark)
5. In tuxy3, ping tuxy4 and then ping tuxy2
- tuxY3
```bash
  ping 172.16.60.254 (tuxY4) (IT WORKS)
  ping 172.16.61.1 (tuxY2) (Network Unreachable)
```

6. Stop capture and save log
7. Start new captures in tuxy3.eth0, tuxy4.eth0, and tuxy2.eth0
- Open Wireshark in tuxY2, tuxY3 and tuxY4 and start to capture

8. In tuxy3, do ping broadcast (ping –b 172.16.y0.255) for a few seconds
9. Observe results, stop captures and save logs
10. Repeat steps 7, 8 and 9, but now do ping broadcast in tuxy2 (ping –b 172.16.y1.255)


## Exp.3
tuxy4
ifconfig eth2 172.16.61.253/24
ETH1:
IP: 172.16.60.254
MAC: 00:01:02:a1:35:69
ETH2:
IP: 172.16.61.253
MAC: 00:c0:df:04:20:8c

tux2
/interface bridge port remove [find interface=ether7]
/interface bridge port add interface=ether7 bridge=bridge61
route add default gw 172.16.61.253
route -n (foto c/Raquel)

tux3:
route add default de 172.16.60.254
route -n (foto c/Raquel)

tux4:
route -n (foto c/Raquel)

APAGAR ARP TABLES TODAS NOS 3 PCS E WIRESHARK A CORRER (eth1 e eth2)
tux4:
arp -a
arp -d 172.16.60.1
arp -d 172.16.61.253
arp -d 10.227.20.254
arp -d 172.16.61.1

tux3:
arp -a
arp -a
arp -d 172.16.60.1
arp -d 10.227.20.254

tux2:
arp -a
arp -d 10.227.20.254

tux3:
ping 172.16.60.254
ping 172.16.61.253
ping 172.16.61.1
ping google.com
ping netlab.fe.up.pt


## Exp.4 (cabo cinza - 6.12 para eth1 router)
tux4:
ip address add address=172.16.2.69/24 interface=ether1 (router)
interface bridge port remove [find interface=ether24] (voltar a meter o cabo no sitio certo)
interface bridge port add interface=ether24 bridge=bridge61

ip address add address=172.16.61.254/24 interface=ether2 (router)
ip address print

tux4:
route add default gw 172.16.60.254
route add default gw 172.16.61.254
