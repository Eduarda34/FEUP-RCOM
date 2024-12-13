## Experiência feita na Bancada 10:

### Experiência 4 - mexe-se no cabo cinza (6.12 para eth1 (router)):
- tuxY4:
**GTK ROUTER:**
```bash
    ip address add address=172.16.1.109/24 interface=ether1
```

**GTK:**
```bash
    interface bridge port remove [find interface=ether24]
    interface bridge port add interface=ether24 bridge=bridge101
```

**GTK ROUTER:**
```bash
    ip address add address=172.16.101.254/24 interface=ether2
    ip address print
```

-tuxY2:
   /ip address add address=172.16.1.109/24 interface=ether1
   /ip address add address=172.16.101.254/24 interface=ether2

- tuxY4:
```bash
    route add default gw 172.16.101.254
```

- tuxY3:
```bash
    route add default gw 172.16.100.254
```

- tuxY2:
```bash
    route add default gw 172.16.101.254
```

**GTK ROUTER**
```bash
    /ip route add dst-address=172.16.100.0/24 gateway=172.16.101.253
    /ip route add dst-address=0.0.0.0/0 gateway=172.16.1.254 
```

3. Using ping commands and Wireshark, verify if tuxY3 can ping all the network interfaces of tuxY2, tuxY4 and Rc

- tuxY3:
```bash
   ping 172.16.100.254 
   ping 172.16.101.1   
   ping 172.16.101.254 
```