## Experiência feita na Bancada 6:

### Experiência 2:
1. Configure tuxy2 and register its IP and MAC addresses
- tuxY2:
```bash
  systemctl restart networking
  ifconfig eth1 172.16.Y1.1/24
  ifconfig
```
|     |        tuxY2       |
| --- |         ---        |
| IP  |   172.16.61.1/24   |
| MAC |  00:e0:7d:b5:8c:8e |

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
  - /interface bridge port add interface=etherX1 bridge=bridge60
  - /interface bridge port add interface=etherX2 bridge=bridge60
  - /interface bridge port add interface=etherX3 bridge=bridge61
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