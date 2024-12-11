## Experiência feita na Bancada 6:

### Experiência 4 - mexe-se no cabo cinza (6.12 para eth1 (router)):
- tuxY4:
**GTK ROUTER:**
```bash
    ip address add address=172.16.2.69/24 interface=ether1
```

**GTK:**
```bash
    interface bridge port remove [find interface=ether24]
    interface bridge port add interface=ether24 bridge=bridge61
```

**GTK ROUTER:**
```bash
    ip address add address=172.16.61.254/24 interface=ether2
    ip address print
```

- tuxY4:
```bash
    route add default gw 172.16.60.254
    route add default gw 172.16.61.254
```