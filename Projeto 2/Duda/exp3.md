## Experiência feita na Bancada 6:

### Experiência 3:
1. Transform tuxY4 (Linux) into a router
    - Configure also tuxY4.eth1 and add it to bridgeY1
    - Enable IP forwarding
    - Disable ICMP echo-ignore broadcast

- tuxY4:
```bash
    ifconfig eth2 172.16.61.253/24
    ifconfig
```
|     |        eth1       |         eth2      |
| --- |        ---        |         ---       |
| IP  |   172.16.60.254   |    172.16.61.253  |
| MAC | 00:01:02:a1:35:69 | 00:c0:df:04:20:8c |

- tuxY2:
**GTK:**
```bash
    /interface bridge port remove [find interface=ether7]
    /interface bridge port add interface=ether7 bridge=bridge61
```
**TERMINAL:**
```bash
    route add default gw 172.16.61.253
```
![](docs\img\exp3-term2.jpg)

- tuxY3:
```bash
    route add default de 172.16.60.254
```
![](docs\img\exp3-term3.jpg)

- tuxY4:
![](docs\img\exp3-term4.jpg)

4. Observe the routes available at the 3 tuxe
```bash
    route -n
```

5. Start the capture at tuxY3
6. From tuxY3, ping the other network interfaces and verify if there is connectivity
- tuxY3:
```bash
    ping 172.16.60.254 (IT WORKS)
    ping 172.16.61.253 (IT WORKS)
    ping 172.16.61.1 (IT WORKS)
```

7. Stop the capture and save the logs
8. Start the capture in tuxY4; use 2 instances of wireshark, one per network interface
9. Clean the ARP tables in the 3 tuxes
- Apagar as ARP tables todas nos 3 PCs enquanto colocamos o wireshark a correr (eth1 e eth2):

- tux4:
```bash
    arp -a
    arp -d 172.16.60.1
    arp -d 172.16.61.253
    arp -d 10.227.20.254
    arp -d 172.16.61.1
```

- tuxY3:
```bash
    arp -a
    arp -d 172.16.60.1
    arp -d 10.227.20.254
```

- tuxY2:
```bash
    arp -a
    arp -d 10.227.20.254
    COLOCAR AS QUE FALTAM!!!
```

10. In tuxY3, ping tuxY2 for a few seconds
- tux3:
```bash
    ping 172.16.60.254
    ping 172.16.61.253
    ping 172.16.61.1
```
11. Stop the captures in tuxY4 and save log