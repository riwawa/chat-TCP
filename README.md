# TCP Chat in C

A simple TCP client-server chat application written in C using BSD sockets.

This project was created to study how network communication works at a lower level, including sockets, TCP connections, file descriptors, system calls, buffers, and the interaction between user space and the operating system kernel.

## Current Features

* TCP server
* TCP client
* Client connection using sockets
* Message sending and receiving
* Basic communication between two processes

## Project Structure

```text
chat-tcp/
├── server.c
├── client.c
├── README.md
└── .gitignore
```

## How It Works

The server follows the standard TCP server flow:

```text
socket()
   ↓
bind()
   ↓
listen()
   ↓
accept()
   ↓
send() / recv()
```

The client follows:

```text
socket()
   ↓
connect()
   ↓
send() / recv()
```

### Server

The server:

1. Creates a TCP socket.
2. Binds the socket to a local IP address and port.
3. Places the socket in listening mode.
4. Waits for a client connection.
5. Accepts the connection.
6. Exchanges messages with the connected client.

### Client

The client:

1. Creates a TCP socket.
2. Connects to the server IP address and port.
3. Exchanges messages with the server.

## Compilation

On macOS or another Unix-like system with `clang`:

```bash
clang server.c -o server
clang client.c -o client
```

Using `gcc`:

```bash
gcc server.c -o server
gcc client.c -o client
```

## Running

Start the server first:

```bash
./server
```

Then open another terminal and run:

```bash
./client
```

The client can then connect to the server and exchange messages through TCP.

## Concepts Studied

This project is being used to study:

* TCP/IP
* BSD sockets
* Client-server architecture
* File descriptors
* Ports and IP addresses
* `struct sockaddr`
* `socket()`
* `bind()`
* `listen()`
* `accept()`
* `connect()`
* `send()`
* `recv()`
* Kernel and user space
* System calls
* Network buffers
* TCP byte streams

## Architecture

```text
CLIENT PROCESS                         SERVER PROCESS

   client.c                              server.c
      |                                      |
   socket()                               socket()
      |                                      |
   connect()                              bind()
      |                                   listen()
      |                                   accept()
      |                                      |
      +---------- TCP connection ------------+
                     |
                  TCP/IP
                     |
                  Kernel
                     |
              Network Interface
```

The application does not implement TCP directly.

Instead, the program uses the socket API to request networking services from the operating system. The kernel contains the TCP/IP implementation and handles tasks such as packet transmission, acknowledgements, retransmissions, ordering, and connection state.

## Next Steps

Planned improvements:

* Better error handling
* Graceful client disconnection
* Message framing
* Custom application protocol
* Multiple simultaneous clients
* Broadcasting messages
* Usernames
* Commands such as `/list`, `/msg` and `/quit`
* I/O multiplexing with `poll()` or `select()`
* Non-blocking sockets
* Per-client buffers
* More robust handling of partial `send()` and `recv()`

## Learning Goal

The goal of this project is not only to create a working chat application, but to understand the complete path of network communication:

```text
C application
     ↓
Socket API
     ↓
System calls
     ↓
Operating system kernel
     ↓
TCP
     ↓
IP
     ↓
Network interface
     ↓
Network
```

Future exploration will also include inspecting socket-related calls at the ARM64 assembly level and observing the interaction between the program, CPU registers, system calls, and the macOS kernel.

## Environment

Currently developed and tested on:

* macOS
* ARM64 / Apple Silicon
* C
* BSD sockets
