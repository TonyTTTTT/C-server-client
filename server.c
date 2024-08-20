#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define serverPort 48763

char *convert(char *src) {
    char *iter = src;
    char *result = malloc(sizeof(*src));
    char *it = result;
    if (iter == NULL) return iter;

    while (*iter) {
        *it++ = toupper(*iter++);
    }

    return result;
}

int main() {
    char buf[1024] = {0};

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) printf("Fail to create a socket.");
    
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(serverPort),
        .sin_addr.s_addr = INADDR_ANY
    };

    if (bind(socket_fd, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    if (listen(socket_fd, 5) == -1) {
        printf("socket %d listen failed!\n", socket_fd);
        close(socket_fd);
        exit(0);
    }

    printf("server [%s:%d] --- ready\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while (1) {
        int reply_sockfd;
        struct sockaddr_in clientAddr;
        unsigned int client_len = sizeof(clientAddr);

        reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
        printf("Accept connect request from [%s:%d]\n",
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        while (recv(reply_sockfd, buf, sizeof(buf), 0)) {
            if (strcmp(buf, "exit") == 0) {
                memset(buf, 0, sizeof(buf));
                break;
            }

            char *conv = convert(buf);

            printf("get message from [%s:%d]: ", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            printf("%s -> %s\n", buf, conv);

            if (send(reply_sockfd, conv, strlen(conv), 0) < 0) {
                printf("send data to %s:%d, failed!\n",
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                memset(buf, 0, sizeof(buf));
                free(conv);
                break;
            }

            memset(buf, 0, sizeof(buf));
            free(conv);
        }

        if (close(reply_sockfd) < 0) {
            perror("close socket failed!");
        }
    }



    if (close(socket_fd)) {
        perror("close socket failed!");
    }


    return 0;
}