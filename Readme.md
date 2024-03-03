# Async Chat Room

The Async Chat Room is an open-source project demonstrating the capabilities of asynchronous communication between a server and multiple clients in a chat room environment. Built with modern C++ and utilizing asynchronous programming paradigms, this project showcases a scalable approach to handling concurrent client connections and real-time message exchange.

## Features

- **Asynchronous Server**: Capable of handling multiple client connections simultaneously without blocking I/O operations.
- **Client-Server Communication**: Utilizes custom packet structures for efficient data transmission over the network.
- **GUI Client Application**: A graphical user interface for clients to interact with the chat room, built using GLFW.
- **Cross-Platform Compatibility**: Designed to run on multiple platforms, leveraging CMake for build configuration.

## How It Works

### Server

The server is responsible for managing client connections, broadcasting messages to all connected clients, and handling asynchronous read and write operations. It uses a thread-safe queue to manage incoming and outgoing packets, ensuring thread safety across client sessions.

### Client

Clients can connect to the server, send messages to the chat room, and receive messages from other clients. The client-side application includes a simple GUI for user interaction, built with GLFW, and handles asynchronous read and write operations to the server.

### Connection Handling

Both server and client use a set of custom classes for managing connections, including asynchronous packet reading and writing, and serializing data structures for network transmission.

## Getting Started

### Prerequisites

- CMake (for building the project)
- A C++17 compatible compiler
- GLFW (for the client GUI)

### Building the Project

1. **Clone the Repository**: `git clone https://github.com/smwbalfe/async-chat-room.git`
2. **Navigate to the Project Directory**: `cd async-chat-room`
3. **Generate Build System**: `cmake .`
4. **Build the Project**: `cmake --build .`

### Running the Application

- **Start the Server**: `./server` (You may need to navigate to the server's build directory)
- **Launch a Client**: `./client` (Navigate to the client's build directory)

## Contributing

We welcome contributions to the Async Chat Room project! Whether it's adding new features, fixing bugs, or improving documentation, your help is appreciated. Please read through the CONTRIBUTING.md file for guidelines on how to make a contribution.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

This README provides a starting point for documenting the Async Chat Room project. Depending on the project's evolution, it may be necessary to update sections to reflect new features or changes.
