# FileTransfer

A cross-platform LAN file-sharing application written in C++23. FileTransfer provides a wxWidgets desktop interface, automatic peer discovery, and direct file offers and transfers over the local network.

## Features

- **LAN discovery:** Discover nearby FileTransfer servers and see their availability.
- **Direct file sharing:** Send files to a selected peer over a TCP connection.
- **Transfer confirmation:** Review and accept or reject an incoming file offer.
- **Transfer progress:** Show progress while receiving files.
- **Desktop UI:** Browse discovered peers and send files with a wxWidgets interface.
- **Protocol Buffers:** Use generated Announce and FileOffer message types for structured communication.

## Technology

- C++23
- CMake 4.2+
- Boost.Asio
- Protocol Buffers
- wxWidgets

## Project Structure

```text
include/file_transfer/  Public C++ headers
  client/               Discovery and file-send interfaces
  common/               Encoding and ID helpers
  server/               Server and connection handling
  ui/                   wxWidgets application and main window
proto/                  Protocol Buffer message definitions
src/
  app/                  Application entry point
  client/               Peer discovery and transfer commands
  common/               Shared implementations
  server/               TCP server and peer announcement
  ui/                   Desktop UI implementation
```

## Requirements

To build and run FileTransfer, install:

- A C++23-capable compiler
- CMake 4.2 or newer
- Boost (including Asio headers)
- Protocol Buffers (`protoc` and development libraries)
- wxWidgets core and base components

## Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Run

Launch the built `app` executable:

```bash
./build/src/app/app
```

The exact executable path may vary depending on your platform and CMake generator.

## Firewall Configuration

FileTransfer uses local-network discovery and a TCP server. A firewall may block discovery broadcasts or reject incoming connections, causing peers not to appear or file transfers to fail.

If this occurs:

- Allow the application through the firewall on your private or local network.
- Allow inbound TCP traffic on port `8080`, or on the custom port used by the server.
- Confirm that discovery traffic is permitted on your network.
- Ensure that Windows, router, VPN, and third-party security software rules are not blocking the application.
- Verify that both devices are on the same LAN and network segment.

Avoid exposing the application directly to public networks or granting broad firewall permissions unless it is necessary.

## Protocol

The application defines two Protocol Buffer message types:

- `Announce`: advertises a server ID, display name, port, and open/closed status.
- `FileOffer` / `FileAnswer`: describes an offered file and carries the receiver's accept/reject response.

## Contributing

Contributions are welcome. Fork the repository, create a focused feature branch, and submit a pull request with a clear description of the change.

## License

No license has been specified for this repository. All rights remain with the repository owner unless a license is explicitly added.
