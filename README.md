*This project has been created as part of the 42 curriculum by edesprez, anle-pag.*

# ft_irc

## Description

This project is an IRC server written in C++98. The goal of the project is to implement the core behavior of an IRC server and allow multiple clients to communicate over TCP/IP.

The server uses non-blocking sockets and a single `poll()` call to handle all network I/O. It implements the IRC features required by the subject: client authentication and registration, channels, private and channel messages, channel operators, and the operator commands `KICK`, `INVITE`, `TOPIC`, and `MODE`.

### Architecture

- `Server` owns the listening socket, all `Client` objects, all `Channel` objects, and the `poll()` descriptor list.
- Each `Client` stores its own registration state and input/output buffers for partial reads and non-blocking writes.
- Each `Channel` stores channel-specific state such as its topic, modes, invitations, and members. Members are identified by their file descriptor, with channel-local information such as operator status. 
- When a channel needs to address one of its members, the `Server` resolves that file descriptor back to the corresponding `Client`.

This keeps ownership of connections centralized while preventing channels from owning or duplicating `Client` objects.


### Implemented commands

- `PASS` — authenticate with the server password.
- `NICK` - set or change a nickname.
- `USER` - set the username and real name.
- `JOIN` - create or join a channel.
- `PRIVMSG` - send a message to a user or channel.
- `TOPIC` - view or change a channel topic.
- `KICK` - remove a user from a channel.
- `INVITE` - invite a user to a channel.
- `MODE` - manage channel modes :
  - `i` - invite-only channel.
  - `t` - restrict topic changes to operators.
  - `k` - set or remove a channel key.
  - `o` - give or remove operator privileges.
  - `l` - set or remove the channel user limit.
- `CAP`, `PING`, `QUIT` - additional client compatibility commands.

## Instructions

### Compilation

```bash
make
```

Other available Makefile rules are `clean`, `fclean`, and `re`.

### Running the server

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mysecretpassword
```

### Connecting with an IRC client

For example, with Irssi (our reference client) :

```bash
irssi -c localhost -p 6667 -w mysecretpassword
```

### Testing with Netcat

Netcat can be used as a low-level TCP testing tool to send IRC commands manually:

```bash
nc -C 127.0.0.1 6667
```

Then, for example:

```text
PASS mysecretpassword
NICK alice
USER alice 0 * :Alice Wonderland
JOIN #42
PRIVMSG #42 :Hello world!
```

## Resources

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Linux TCP manual page](https://man7.org/linux/man-pages/man7/tcp.7.html)
- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [IRC command and numeric reference](https://dd.ircdocs.horse/refs/commands/)
- [Modern IRC documentation](https://modern.ircdocs.horse/)

### AI usage

AI tools were used to help research IRC protocol behavior, clarify networking concepts, assist with debugging, plan tests, and review parts of the code. AI suggestions were reviewed, understood, adapted to fit the project. 
