*This project has been created as part of the 42 curriculum by **hsennane**.*

# ft_irc

An **Internet Relay Chat (IRC) server** built in **C++98** as part of the 42 curriculum. It allows multiple clients to communicate in real time through private messages and shared channels using the IRC protocol.

## Description

The project focuses on network programming, client-server communication, and event-driven I/O. The server accepts TCP connections, authenticates users with a connection password, and handles multiple clients through a single `poll()` loop.

It implements the required IRC features: registration, private and channel messaging, channel management, and operator privileges. The implementation follows the relevant IRC protocol conventions without attempting to be a complete IRC network.

### Features

- **Client registration:** `PASS`, `NICK`, and `USER`.
- **Messaging:** `PRIVMSG` and `NOTICE` to users or channels.
- **Channel management:** `JOIN`, `PART`, `TOPIC`, `INVITE`, and `KICK`.
- **Channel operators:** privileges for managing membership and channel settings.
- **Connection commands:** `PING`, `PONG`, and `QUIT`.
- **Non-blocking I/O:** `poll()` monitors multiple clients without one thread per connection.
- **Message buffering:** handles fragmented incoming commands and partial socket writes.
- **Basic bot:** the built-in `bot` target responds to `!hello` and `!help` sent via private message.

### Channel modes

| Mode | Purpose |
| --- | --- |
| `i` | Invite-only channel |
| `t` | Restrict topic changes to channel operators |
| `k` | Set or remove a channel password |
| `o` | Grant or revoke channel operator privileges |
| `l` | Set or remove the maximum number of users |

> **Scope:** The server uses IPv4 TCP without TLS. It does not implement server-to-server connections, IRCv3 capability negotiation, or file transfers. The included bot is intentionally minimal.

## Instructions

### Requirements

- A POSIX-compatible environment (such as Linux)
- A C++ compiler with C++98 support
- GNU Make

No third-party libraries are required. The Makefile compiles with `-Wall -Wextra -Werror -std=c++98`.

### Build

```bash
git clone https://github.com/hajarsen/FT_IRC.git
cd FT_IRC
make
```

This builds the `ircserv` executable.

### Run

```bash
./ircserv <port> <password>
```

For example:

```bash
./ircserv 6667 secret
```

The port must be available and within `1–65535`. Keep this terminal open while clients connect. Press `Ctrl+C` to stop the server.

### Connect with an IRC client

You can use **WeeChat** or another IRC client that supports a password-protected, plaintext TCP connection.

For WeeChat, open the client and enter:

```text
/server add local 127.0.0.1/6667 -notls -password=secret -nicks=alice
/set irc.server.local.capabilities ""
/connect local
/join #room
```

The capabilities setting disables optional IRCv3 negotiation, which this server does not implement. Set it before connecting.

To test chat between two users, connect a second client with the nickname `bob` and join `#room`.

From either client:

```text
/msg #room Hello everyone!
/msg bob Hello Bob!
```

The first user to create a channel becomes its operator. As that operator, you can try:

```text
/topic #room Welcome to our channel
/mode #room +i
/invite bob #room
/mode #room +o bob
/kick #room bob Leaving the channel
```

To test the built-in bot:

```text
/msg bot !hello
/msg bot !help
```

### Test with a raw TCP client

For a quick protocol-level check, connect using Netcat:

```bash
nc 127.0.0.1 6667
```

Then send these lines one at a time:

```text
PASS secret
NICK alice
USER alice 0 * :Alice
JOIN #room
PRIVMSG #room :Hello from Netcat!
PING :test
```

The server should return registration responses after `USER` and respond to `PING` with `PONG`. Netcat is useful for diagnostics; use a full IRC client for the project evaluation.

### Makefile commands

| Command | Action |
| --- | --- |
| `make` | Compile `ircserv` |
| `make clean` | Remove object files |
| `make fclean` | Remove object files and the executable |
| `make re` | Clean and rebuild the project |

## Project structure

```text
FT_IRC/
├── main.cpp              # Program entry point
├── Makefile
├── include/              # Shared classes and declarations
├── network/              # Server loop, sockets, and client I/O
├── parsing/              # IRC command parser
├── core/                 # Command dispatch and server state
├── commands/             # IRC command implementations
└── bonus/                # Built-in bot
```

The network layer receives data and sends queued replies. The parser converts complete input lines into commands; the command handler then applies server and channel rules before creating responses.

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459)
- [RFC 2812 — Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812)
- [Linux `poll(2)`](https://man7.org/linux/man-pages/man2/poll.2.html)
- [Linux `recv(2)`](https://man7.org/linux/man-pages/man2/recv.2.html) and [`send(2)`](https://man7.org/linux/man-pages/man2/send.2.html)
- [WeeChat documentation](https://weechat.org/doc/)

### AI usage

AI tools were used during development to help review the code against the project requirements, design and run regression checks, investigate socket and buffering issues, and improve documentation. The submitted code remains the responsibility of its author and should be understood and explainable during peer evaluation.
