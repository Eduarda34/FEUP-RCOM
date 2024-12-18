## Experiência feita na Bancada 4:

### Experiência 5

1. Configure DNS at tuxY3, tuxY4, tuxY2 (use DNS server services.netlab.fe.up.pt (10.227.20.3))
```bash
    nano /etc/resolv.conf
    nameserver 10.227.20.3
```

2. Verify if names can be used in these hosts (e.g., ping hostname, use browser)

3. Execute ping (new-hostname-in-the-Internet); observe DNS related packets in Wireshark
```bash
    ping google.com
```