# Configure an IP Network

## Context
Context:
Bench: 1
Image:
![alt text](image-2.png)

## Steps
1. Transform tuxY4 (Linux) into a router
    - Configure also eth2 interface of tuxY4 and add it to bridgeY1
    - Enable IP forwarding
    - Disable ICMP echo-ignore-broadcast
2. Observe MAC addresses and IP addresses in tuxY4.eth1 and tuxY4.eth2
3. Reconfigure tuxY3 and tuxY2 so that each of them can reach the other
4. Observe the routes available at the 3 tuxes (route -n)
5. Start capture at tuxY3
6. From tuxY3, ping the other network interfaces (172.16.Y0.254, 172.16.Y1.253, 172.16.Y1.1) and verify if there is connectivity
7. Stop the capture and save the logs
8. Start capture in tuxY4; use 2 instances of Wireshark, one per network interface
9. Clean the ARP tables in the 3 tuxes
10. In tuxY3, ping tuxY2 for a few seconds.
11. Stop captures in tuxY4 and save logs



### Steps 1

### Steps 2

### Steps 3

### Steps 4

### Steps 5

### Steps 6

### Steps 7

### Steps 8

### Steps 9

### Steps 10

### Steps 11



## Questions

**What routes are there in the tuxes? What is their meaning?**
todo

**What information does an entry of the forwarding table contain?**
todo

**What ARP messages, and associated MAC addresses, are observed and why?**
todo

**What ICMP packets are observed and why?**
todo

**What are the IP and MAC addresses associated to ICMP packets and why?**
todo

