## Experiência feita na Bancada 6:

### Experiência 1:
1. Disconnect switch from netlab and connect tux computers
2. Configure tuxy3 and tuxy4 using ifconfig and route commands
3. Register the IP and MAC addresses of network interfaces
- **GTK TUXY4:**
```bash
    - configurações - port - baud - rate = 115200
    - /system reset-configuration
```
- **TERMINAL TUXY4:**
```bash
    - ifconfig
    - route -n
```
- Ligar os cabos (tuxY3 E0 e tuxY4 E0)
- tuxY4:
```bash
   systemctl restart networking
   ifconfig eth1 172.16.Y0.254/24
   ifconfig
```
- tuxY3:
```bash
   systemctl restart networking
   ifconfig eth1 172.16.Y0.1/24
   ifconfig
```
|     |       tuxY4       |        tuxY3      |
| --- |        ---        |         ---       |
| IP  |   172.16.60.254   |    172.16.60.1    |
| MAC | 00:01:02:a1:35:69 | 00:c0:df:25:40:66 |

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