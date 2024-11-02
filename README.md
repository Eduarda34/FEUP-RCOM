# RCOM_PROJ1

Projects for the RCOM course unit at FEUP.

&nbsp;

## Unit Info

- Name: Redes de Computadores (Computer Networks)
- Date: Year 3, Semester 1, 2024/25
- [More info](https://sigarra.up.pt/feup/pt/ucurr_geral.ficha_uc_view?pv_ocorrencia_id=541890)

&nbsp;

## Project Description

This is a C application designed to learn about Data Link Protocol.

&nbsp;

## Installation

Extra package: termios, unistd, socat

To run our application, follow these simple steps:

1. Open a terminal window
2. Navigate to the src folder using the cd command: `cd code`
3. Execute the following command to compile and run the program: `make`

&nbsp;

### Usage at Home

#### Simulate Serial Port 

0. Clean terminal <br> ```clean``
1. Clean workspace <br> ```make clean```
2. Compile program <br> ``make all```
3. Run cable <br> ````sudo make run_cable```

The project uses two virtual serial ports to simulate communication:

#### Receiver

1. Run the Receiver <br> ```./bin/main /dev/ttyS11 9600 rx penguin-received.gif```

#### Transmitter 

1. Run the Transmitter <br> ```./bin/main /dev/ttyS10 9600 tx penguin.gif```


## Milestones

- [✓] M1: Exchange strings over serial connections;
- [✓] M2: Sending and receiving control frame (SET/UA) and state machine in llopen;
- [✓] M3: Implement the Stop & Wait protocol in llwrite and llread;
- [✓] M4: Timer and retransmission;
- [✓] M5: Application layer implementation and correct API operation of llopen, llclose, llwrite, llread;

&nbsp;

## Collaboration

| Name                                | Up Number | Git                                         |
| ----------------------------------- | --------- | ------------------------------------------- |
| Clarisse Maria Teixeira de Carvalho | 202008444 | [MitsukiS16](https://github.com/MitsukiS16) |
| Maria Eduarda Pacheco Mendes Araújo | 202004473 | [Eduarda34](https://github.com/Eduarda34)   |z