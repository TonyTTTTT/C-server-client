#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <omp.h>

#include "word_occur.h"
#include "util.h"

int server(int num_of_threads);

int main(int argc, char **argv) {
    int num_of_threads = atoi(argv[1]);
    printf("Target num_of_threads: %d\n", num_of_threads);
    printf("Max threads availble: %d\n", omp_get_max_threads());

    server(num_of_threads);

    return 0;
}

int server(int num_of_threads) {
    char buf[MAX_PATH_LEN] = {0};

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) perror("Fail to create a socket.");
    
    struct sockaddr_in serverAddr = {
        AF_INET,
        htons(serverPort),
        INADDR_ANY
    };

    if (bind(socket_fd, (const struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    if (listen(socket_fd, 5) == -1) {
        printf("Socket %d listen failed!\n", socket_fd);
        close(socket_fd);
        exit(0);
    }

    printf("Server [%s:%d] --- ready\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while (1) {
        int reply_sockfd;
        struct sockaddr_in clientAddr;
        unsigned int client_len = sizeof(clientAddr);

        printf("%s\n\nWaiting for client request...\n", SEP_LINE);
        reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
        printf("Accept connect request from [%s:%d]\n",
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        int files_cnt = 0;
        while (recv(reply_sockfd, buf, sizeof(buf), 0)) {
            if (strcmp(buf, "exit") == 0) {
                memset(&buf, 0, sizeof(buf));
                break;
            }

            print_debug(SEP_LINE);
            print_debug("Get message from [%s:%d]: %s\n", inet_ntoa(clientAddr.sin_addr),
                    ntohs(clientAddr.sin_port), buf);

            word_occurrence_count(buf, num_of_threads);

            char *respond = (char*)malloc(strlen(buf) + strlen(ACK_MSG) + 1);
            strcpy(respond, buf);
            strcat(respond, ACK_MSG);

            if (send(reply_sockfd, respond, strlen(respond), 0) < 0) {
                printf("Send data to %s:%d, failed!\n",
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                memset(&buf, 0, sizeof(buf));
                free(respond);
                break;
            }

            memset(&buf, 0, sizeof(buf));
            free(respond);
            files_cnt++;
        }

        print_occurrence();

        printf("%s%d files been count.\n", SEP_LINE, files_cnt);

        if (close(reply_sockfd) < 0) {
            perror("Close socket failed!");
        }
    }



    if (close(socket_fd)) {
        perror("Close socket failed!");
    }


    return 0;
}