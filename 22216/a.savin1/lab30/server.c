#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <signal.h>

#define SOCKET_PATH "unix_socket"
#define BUFFER_SIZE 1024

int server_sock = -1, client_sock = -1;

void to_uppercase(char *str, int read_value) {
    char character;
    for (int i = 0; i < read_value; i++) {
        character = str[i];
        putchar(toupper(character));
    }
}

void handle_sigint(int sig) {
    (void) sig;
    if (server_sock != -1) {
        close(server_sock);
    }
    if (client_sock != -1) {
        close(client_sock);
    }
    if (unlink(SOCKET_PATH) != 0) {
        perror("unlink failed");
    }
    printf("\nserver terminated by SIGINT signal\n");
    exit(0);
}

int main() {
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("signal failed");
        return -1;
    }

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sun_family = AF_UNIX;

    strcpy(server_addr.sun_path, SOCKET_PATH);


    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind failed");
        close(server_sock);
        return -1;
    }

    if (listen(server_sock, 5) == -1) {
        perror("listen failed");
        close(server_sock);
        return -1;
    }

    printf("server is listening...\n");

    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock == -1) {
        perror("accept failed");
        close(server_sock);
        return -1;
    }

    int read_value;

    while ((read_value = read(client_sock, buffer, sizeof(buffer))) > 0) {
        to_uppercase(buffer, read_value);
    }

    if (read_value == -1) {
        perror("read failed");
        close(client_sock);
        close(server_sock);
        return -1;
    }

    if (read_value == 0) {
        printf("\connection terminated by client\n");
    }

    close(client_sock);
    close(server_sock);
    if (unlink(SOCKET_PATH) != 0) {
        perror("unlink failed");
    }
    return 0;
}
