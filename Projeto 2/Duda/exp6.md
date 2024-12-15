## Experiência feita na Bancada 4:

### Experiência 6:

1. Compile your download application in tuxY3
```bash
    gcc -o download appliication.c
    ./download + (link do servidor)
```

2. In tuxY3, restart capturing with Wireshark and run your application
3. Verify if file has arrived correctly, stop capturing and save the log
4. Using Wireshark, observe packets exchanged including
- TCP control and data connections, and its phases (establishment, data, termination)
- Data transferred through the FTP control connection
- TCP ARQ mechanism
- TCP congestion control mechanism in  action
- Note: use also the Wireshark Statistics tools (menu) to study the TCP phases, ARQ and congestion control mechanism

5. Repeat the download in tuxY3 but now, in the middle of the transfer, start a new download in tuxY2
- Use the Wireshark statistics tools to understand how the throughput of  a TCP connection varies along the time