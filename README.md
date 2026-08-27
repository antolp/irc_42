This project was created as part of the 42 curriculum by edesprez and anle-pag.

# ft_irc

An IRC (Internet Relay Chat) server developed in C++98, using non-blocking I/O multiplexing with poll().

---

## Project Overview

ft_irc is a project from the 42 curriculum aiming to re-implement an IRC server from scratch. The server handles multiple concurrent client connections without multithreading, relying exclusively on non-blocking sockets and the poll() system call.

The server complies with the IRC protocol specifications (RFC 1459) as well as modern specifications (Modern IRC Specs), allowing connections from standard IRC clients such as irssi, nc (Netcat), or HexChat.

---

## Implemented Commands

All differents commands :
PASS : Initial client authentication with the server password. 
NICK : Set or change the client nickname. 
USER : Set the username and real name of the client. 
CAP : IRC capability negotiation for modern clients (e.g., CAP LS). 
PING : Connection health check between client and server. 
JOIN : Create or join an IRC channel (#channel or &channel). 
PRIVMSG : Send private messages to a user or broadcast to a channel. 
TOPIC : View or modify the topic of a channel. 
KICK : Eject a member from a channel (channel operator privilege). 
INVITE : Invite a user to join a channel. 
QUIT : Gracefully disconnect from the server with an optional quit message. 

## Compilation and Usage

The project compiles with make using C++98 flags (-Wall -Wextra -Werror -std=c++98):

```bash
make
```

Makefile rules:
- make: Builds the ircserv executable.
- make clean: Removes object files (.obj/).
- make fclean: Removes object files and the ircserv binary.
- make re: Performs a clean rebuild of the project.

### Running the Server

```bash
./ircserv <port> <password>
```

Example:
```bash
./ircserv 6667 mysecretpassword
```

### Connecting to the Server

#### Using Irssi
```bash
irssi -c localhost -p 6667 -w mysecretpassword
```

#### Using Netcat (nc)
```bash
nc localhost 6667
PASS mysecretpassword
NICK alice
USER alice 0 * :Alice Wonderland
JOIN #42
PRIVMSG #42 :Hello world!
```

---

## Resources and References

This project relies on network programming fundamentals and IRC protocol standards:

- [Beej's Guide to Network Programming - System Calls or Bust](https://beej.us/guide/bgnet/html/split/system-calls-or-bust.html#system-calls-or-bust): The guide for POSIX sockets (socket, bind, listen, accept, poll, send, recv).
- [Linux TCP Man Page (tcp.7)](https://man7.org/linux/man-pages/man7/tcp.7.html): TCP protocol specifications and socket options on Linux.
- [RFC 1459 - Internet Relay Chat Protocol](https://www.rfc-editor.org/info/rfc1459/): The original IRC protocol specification.
- [IRC Command Reference (ircdocs)](https://dd.ircdocs.horse/refs/commands/): Comprehensive reference for IRC commands and numeric replies.
- [Modern IRC Documentation](https://modern.ircdocs.horse/#mode-message): Documentation on modern IRC specifications and message handling.

---
