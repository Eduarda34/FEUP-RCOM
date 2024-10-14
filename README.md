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

1. Compile the Project <br> ```make all```
2. Run the cable <br> ```sudo make run_cable```
3. Check if files are the same <br> ```make check_files``` 
4. Clean the project <br> ```make clean```

The project uses two virtual serial ports to simulate communication:


#### Transmitter 

1. ```ls /dev/ttyS12```
2. Run the Transmitter <br> ```make run_tx```


#### Receiver 

1. ```ls /dev/ttyS13```
3. Run the Receiver <br> ```make run_rx```


### Socat

 --- on           : connect the cable and data is exchanged (default state)

 --- off          : disconnect the cable disabling data to be exchanged

 --- baud <rate>  : set baud rate, between 1200 and 115200 (default=9600)
                   note that 10 bits are sent per byte (8-N-1)

 --- prop <delay> : set the propagation delay in usec (0-1000000, default=0)
                   will be approximated to an integer multiple of the byte
                   delay (10 / baud_rate)

 --- log <file>   : log transmitted data to file

 --- endlog       : stop logging transmitted data


## Milestones

- [ ] M1: Exchange strings over serial connections;
- [ ] M2: Sending and receiving control frame (SET/UA) and state machine in llopen;
- [ ] M3: Implement the Stop & Wait protocol in llwrite and llread;
- [ ] M4: Timer and retransmission;
- [ ] M5: Application layer implementation and correct API operation of llopen, llclose, llwrite, llread;

&nbsp;

## Collaboration

| Name                                | Up Number | Git                                         |
| ----------------------------------- | --------- | ------------------------------------------- |
| Clarisse Maria Teixeira de Carvalho | 202008444 | [MitsukiS16](https://github.com/MitsukiS16) |
| Maria Eduarda Pacheco Mendes Araújo | 202004473 | [Eduarda34](https://github.com/Eduarda34)   |z