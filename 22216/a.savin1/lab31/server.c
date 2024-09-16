#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 3

const char *socket_path = "unix_socket";
int server_sock = -1;
int client_socks[MAX_CLIENTS] = {0};

void to_uppercase(char *str, int read_value) {
    char character;
    for (int i = 0; i < read_value; i++) {
        character = str[i];
        putchar(toupper(character));
    }
}

void clear() {
    if (server_sock != -1) {
        close(server_sock);
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socks[i] != 0) {
            close(client_socks[i]);
        }
    }
    if (unlink(socket_path) != 0) {
        write(STDERR_FILENO, "\nunlink failed\n", 15);
        _exit(-1);
    }
}

void handle_sigint(int sig) {
    (void) sig;
    write(STDERR_FILENO, "\nserver terminated by SIGINT signal\n", 36);
    clear();
    _exit(0);
}

int free_slot() {
    int slot = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socks[i] == 0) {
            slot = i;
            break;
        }
    }
    return slot;
}

void handle_new_connection() {
    int new_socket = accept(server_sock, NULL, NULL);
    if (new_socket < 0) {
	perror("accept failed");
	clear();
	exit(1);
    } else {
	printf("New connection accepted\n");
	client_socks[free_slot] = new_socket;
    }
}

int main() {
    struct sockaddr_un server_addr;

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket failed");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, socket_path);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind failed");
        close(server_sock);
        return -1;
    }

    if (signal(SIGINT, handle_sigint) == SIG_ERR) {
        perror("failed to handle sigint");
        return -1;
    }

    if (listen(server_sock, MAX_CLIENTS) == -1) {
        perror("listen failed");
        clear();
        return -1;
    }

    printf("server is listening...\n");

    char buffer[BUFFER_SIZE];
    fd_set readfds;
    int max_sd, activity, sd;
    while (1) {
        FD_ZERO(&readfds);
	if (free_slot() != -1) {
	    FD_SET(server_sock, &readfds);
	}
        max_sd = server_sock;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socks[i];
            if (sd > 0) {
                FD_SET(sd, &readfds);
            }
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select error");
            clear();
            return 1;
        }

        if (FD_ISSET(server_sock, &readfds)) {
            handle_new_connection();
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socks[i];

            if (FD_ISSET(sd, &readfds)) {
                int read_value = read(sd, buffer, BUFFER_SIZE);

                if (read_value == 0) {
                    printf("client disconnected\n");
                    close(sd);
                    FD_CLR(sd, &readfds);
                    client_socks[i] = 0;
                } else if (read_value > 0) {
                    to_uppercase(buffer, read_value);
                } else {
                    perror("read failed");
                    close(sd);
                    FD_CLR(sd, &readfds);
                    client_socks[i] = 0;
                }
            }
        }
    }

    clear();
    return 0;
}

