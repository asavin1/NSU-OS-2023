#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

int sock = -1;

void handle_sigpipe(int sig) {
    (void) sig;
    printf("server was closed\n");
    close(sock);
    exit(0);
}

int main() {
    struct sockaddr_un server_addr;
    const char *socket_path = "unix_socket";

    signal(SIGPIPE, handle_sigpipe);

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock);
        return -1;
    }
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sock);
        return -1;
    }

    printf("enter text to send to server: \n");

    char buffer[BUFFER_SIZE];
    ssize_t read_value, write_value;
    while ((read_value = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        char *buf = buffer;
        while ((write_value = write(sock, buffer, read_value)) < read_value) {
            if (write_value == -1) {
                perror("write failed");
                close(sock);
                return -1;
            }
            read_value -= write_value;
            buf += write_value;
        }
    }

    if (read_value == -1) {
        perror("read failed");
        close(sock);
        return -1;
    }
    close(sock);
    printf("client terminated.\n");
    return 0;
}
