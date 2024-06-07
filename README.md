# Chatroom Application

This repository contains the source code for a Chatroom application developed in C++. The application supports user login, registration, adding friends, blocking/deleting users, private and group chats, real-time notifications, and file uploads. The main technology stack includes Redis, CMake, and epoll with multithreading.

## Features

- **User Authentication:** Users can register and log in securely.
- **Friend Management:** Users can add, delete, and block friends.
- **Private and Group Chats:** Users can engage in both private and group conversations.
- **Real-time Notifications:** The application provides real-time notifications for new messages and updates.
- **File Upload:** Users can upload and share files within the chat.
- **Multithreading:** Efficiently handles multiple connections using epoll and multithreading.

## Getting Started

To run the Chatroom application locally, follow these steps:

1. **Clone the repository:**
    ```sh
    git clone https://github.com/ShawnJeffersonWang/Messenger.git
    cd Messenger
    ```

2. **Install dependencies:**
    Ensure you have Redis installed and running. Install CMake and any other necessary libraries for the project.

3. **Compile the source code:**
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```

4. **Run the application:**
    ```sh
    ./chatroom_executable
    ```

5. **Connect to the application:**
    Access the Chatroom application through your preferred client interface.

## Dependencies

The Chatroom application relies on the following dependencies:

- **Redis:** For data storage and message queueing.
- **CMake:** For build configuration.
- **epoll:** For handling multiple I/O events efficiently.
- **Multithreading:** To handle concurrent connections.

Please ensure that you have these dependencies installed and properly configured before running the application.

## Usage

1. **Launch the Chatroom application.**
2. **Create an account or log in** with your existing credentials.
3. **Explore the user interface** and navigate through the available features.
4. **Manage your friends**: add, delete, or block users.
5. **Start conversations**: engage in private or group chats.
6. **Send notifications and share files** in real-time with other users.

## Contributing

Contributions to the Chatroom application are welcome. If you encounter any issues or have suggestions for improvements, please submit a pull request or open an issue on this repository.

When contributing, please adhere to the existing code style and follow the established guidelines.

## License

This Chatroom application is licensed under the [MIT License](LICENSE). You are free to modify and distribute the application as per the license terms.
