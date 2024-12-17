## Experiência feita na Bancada 4:

### Experiência 4 - mexe-se no cabo cinza (6.12 para eth1 (router)):
- tuxY4:
**GTK ROUTER:**
```bash
    /ip address add address=172.16.1.49/24 interface=ether9
    /ip address add address=172.16.41.254/24 interface=ether2
```

**TERMINAL**
- tuxY4:
```bash
    route add -net 172.16.1.0/24 gw 172.16.41.254
```

- tuxY3:
```bash
    route add -net 172.16.41.0/24 gw 172.16.40.254
    route add -net 172.16.1.0/24 gw 172.16.40.254
```

- tuxY2:
```bash
    route add -net 172.16.40.0/24 gw 172.16.41.253
    route add -net 172.16.1.0/24 gw 172.16.44.254
```

**GTK ROUTER:**
   /ip address add address=172.16.1.49/24 interface=ether1
   /ip address add address=172.16.41.254/24 interface=ether2


**GTK:**
```bash
    /interface bridge port remove [find interface=ether10]
    /interface bridge port add interface=ether24 bridge=bridge41
```

**GTK ROUTER**
```bash
    /ip route add dst-address=172.16.100.0/24 gateway=172.16.41.253
    /ip route add dst-address=0.0.0.0/0 gateway=172.16.1.254 
```

3. Using ping commands and Wireshark, verify if tuxY3 can ping all the network interfaces of tuxY2, tuxY4 and Rc

- tuxY3:
```bash
   ping 172.16.40.254 
   ping 172.16.41.1   
   ping 172.16.41.254 
```

4. In tuxY2
```bash
    sysctl net.ipv4.conf.eth0.accept_redirects=0
    sysctl net.ipv4.conf.all.accept_redirects=0 
```

- Remove the route to 172.16.40.0/24 via tuxY4
```bash
    route delete -net 172.16.40.0/24 gw 172.16.41.253
```

- Ping tuxY3
- Using capture at tuxY2, try to understand the path followed by ICMP ECHO and ECHO-REPLY packets (look at MAC addresses)

- traceroute tuxY3
```bash
    traceroute -n 172.16.40.1
```

- Activate the acceptance of  ICMP redirect at tuxY2 when there is no route to 172.16.Y0.0/24 via tuxY4 and try to understand what happens
```bash
    sysctl net.ipv4.conf.eth0.accept_redirects=1
    sysctl net.ipv4.conf.all.accept_redirects=1
```

5. In tuxY3, ping the FTP server (172.16.1.254) and try to understand what happens
- tuxY3:
```bash
    ping 172.16.1.254
```

6. Disable NAT functionality in router Rc
**GTK**
```bash
    /ip firewall nat disable 0
```

7. In tuxY3 ping 172.16.1.254, verify if there is connectivity, and try to understand what happens
```bash
    ping 172.16.1.254
```