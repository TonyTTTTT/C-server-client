#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define serverPort 48763
#define serverIP "127.0.0.1"

int traverse_directory(char *name, time_t target_time) {
    DIR *dir;
    struct dirent *ent;
    struct stat states;

    dir = opendir(name);
    if (!dir) {
        perror(name);
        return -1;
    }

    // printf("%10s %25s %15s %8s %25s %-s\n", "d_ino", "d_off", "d_reclen", "len", "last modified time", "filename");
    printf("%25s %-s\n", "last modified time", "filename");
    while ((ent=readdir(dir)) != NULL) {
        stat(ent->d_name, &states);
        if (strcmp(ent->d_name, ".")==0 || strcmp(ent->d_name, "..")==0) continue;


        char date[80];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&states.st_ctime));

        char full_path[1024];
        strcpy(full_path, name);
        strcat(full_path, "/");
        strcat(full_path, ent->d_name);

        // printf("%10lu %25ld %15u %8ld %25s %-s\n", ent->d_ino, ent->d_off, ent->d_reclen, strlen(ent->d_name), date, full_path);
        printf("%25s %-s\n", date, full_path);
        if (states.st_ctime > target_time) {
            printf("modified after target time!\n");
            send_to_server(full_path);
        }
        if (ent->d_type == DT_DIR) traverse_directory(full_path, target_time);
    }

    close(dir);
    return 0;
}

int send_to_server(char *buf) {
    char recvbuf[1024] = {0};

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(serverPort),
        .sin_addr.s_addr = inet_addr(serverIP),
    };
    int len = sizeof(serverAddr);

    if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
        printf("Connect server failed\n");
        close(socket_fd);
        exit(0);
    }

    printf("Connect server [%s:%d] success\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    printf("path to be send: %s\n", buf);

    if (send(socket_fd, buf, strlen(buf), 0) < 0) {
        printf("send data to %s:%d, failed!\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
        // memset(buf, 0, sizeof(buf));
    }

    // memset(buf, 0, sizeof(buf));

    if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
        printf("recv data from %s:%d, failed!\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    }

    printf("get receive message from [%s:%d] : %s\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));

    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }

    return 0;
}

int start_communicate() {
    char buf[1024] = {0};
    char recvbuf[1024] = {0};

    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(serverPort),
        .sin_addr.s_addr = inet_addr(serverIP),
    };
    int len = sizeof(serverAddr);

    if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
        printf("Connect server failed\n");
        close(socket_fd);
        exit(0);
    }

    printf("Connect server [%s:%d] success\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while (1) {
        printf("Please input your message: ");
        scanf("%s", buf);

        if (send(socket_fd, buf, sizeof(buf), 0) < 0) {
            printf("send data to %s:%d, failed!\n",
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            break;
        }

        if (strcmp(buf, "exit") == 0) break;

        memset(buf, 0, sizeof(buf));

        if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
            printf("recv data from %s:%d, failed!\n",
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            break;
        }

        printf("get receive message from [%s:%d] : %s\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }

    return 0;
}

int main(int argc, char **argv) {
    char path_buf[100] = "./directory_big";
    printf("Please input your path:\n");
    // scanf("%s", path_buf);

    time_t target_time = 1724136105;
    char target_time_buff[100];
    strftime(target_time_buff, sizeof(target_time_buff), "%Y-%m-%d %H:%M:%S", localtime(&target_time));
    printf("target_time: %s\n", target_time_buff);

    traverse_directory(path_buf, target_time);

    // start_communicate();

    return 0;
}