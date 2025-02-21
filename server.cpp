#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

// #include "src/files.h"

const int PORT = 1234;
const int MAX_CONNECTIONS = 4096; // def value on Linux
const size_t k_max_msg = 4096;

void error_msg(std::string msg) {
    std::cerr << msg << std::endl;
}

void msg(std::string msg) {
    std::cout << msg << std::endl;
}


static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error, or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    
    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1;  // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t one_request(int connfd) {
    // 4 bytes header
    char rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    if (err) {
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }
    uint32_t len = 0;
    memcpy(&len, rbuf, 4);  // Copies the first 4 bytes of rbuf into len, interpreting it as a 32-bit integer.
    if (len > k_max_msg) {
        msg("too long");
        return -1;
    }
    // request body
    err = read_full(connfd, &rbuf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }
    // do something
    printf("client says: %.*s\n", len, &rbuf[4]);
    // reply using the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(connfd, wbuf, 4 + len);
}

int main() {
    // obtain server file descriptor
    // AF_INET is for IPv4. Use AF_INET6 for IPv6 or dual-stack sockets.
    // SOCK_STREAM is for TCP. Use SOCK_DGRAM for UDP.
    // The 3rd argument is 0 and useless for our purposes.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        error_msg("Failed to create server socket");
        return 1;
    }

    int reuse = 1;
    // The effect of SO_REUSEADDR is important: if it’s not set to 1, a server program cannot bind to the same IP:port 
    // it was using after a restart. This is generally undesirable TCP behavior. You should enable SO_REUSEADDR for all listening sockets!
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        close(server_fd);
        error_msg("setsockopt failed");
        return 1;
    }

    // struct sockaddr_in holds an IPv4:port pair stored as big-endian numbers, converted by htons() and htonl(). 
    // For example, 1.2.3.4 is represented by htonl(0x01020304).
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // wildcard IP 0.0.0.0
    server_addr.sin_port = htons(PORT);
    // htonl() reads “Host to Network Long”. 
    // “Host” means the CPU endian. “Network” means big-endian. 
    // “Long” actually means uint32_t, not the long type. On little-endian CPUs, it’s a byte swap. On big-endian CPUs, it does nothing. 

    // bind to PORT
    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
        close(server_fd);
        error_msg("Failed to bind to port " + std::to_string(PORT));
        return 1;
    }

    if (listen(server_fd, MAX_CONNECTIONS) != 0) {
        close(server_fd);
        error_msg("listen failed");
        return 1;
    }

    msg("Redis server running on port: " + std::to_string(PORT)); 
    msg("Waiting for a client to connect...");

    while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int conn_fd = accept(server_fd, (struct sockaddr *)&client_addr, &socklen);
        if (conn_fd < 0) {
            error_msg("client failed connecting");
            continue;   // error
        }
        // only serves one client connection at once
        while (true) {
            int32_t err = one_request(conn_fd);
            if (err) {
                break;
            }
        }
        close(conn_fd);
    }

    return 0;
}