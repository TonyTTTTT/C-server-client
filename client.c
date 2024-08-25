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
#define MAX_PATH_LEN 256


char ** traverse_directory(char *name, time_t target_time, char **files_path, int *path_cnt) {
    DIR *dir;
    struct dirent *ent;
    struct stat states;

    dir = opendir(name);
    if (!dir) {
        perror(name);
        return;
    }

    // printf("%10s %25s %15s %8s %25s %-s\n", "d_ino", "d_off", "d_reclen", "len", "last modified time", "filename");
    while ((ent=readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".")==0 || strcmp(ent->d_name, "..")==0) continue;

        // the lenght of path is limit to 256
        char full_path[MAX_PATH_LEN];
        strcpy(full_path, name);
        strcat(full_path, "/");
        strcat(full_path, ent->d_name);

        memset(&states, 0, sizeof(states));
        if (stat(full_path, &states) == -1) {
            perror("stat() fail!");
        }

        char date[80];
        strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", localtime(&states.st_ctime));

        // printf("%10lu %25ld %15u %8ld %25s %-s\n", ent->d_ino, ent->d_off, ent->d_reclen, strlen(ent->d_name), date, full_path);
        printf("%20s %-s\n", "last modified time", "filename");
        printf("%20s %-s\n\n", date, full_path);

        if (states.st_ctime > target_time && ent->d_type != DT_DIR) {
            printf("is file & modified after target time!\n");

            // send_to_server(full_path);

            files_path = realloc(files_path, (*path_cnt+1) * sizeof(full_path));
            files_path[*path_cnt] = malloc(sizeof(full_path));
            strcpy(files_path[*path_cnt], full_path);
            (*path_cnt)++;
        }
        printf("========================\n");

        if (ent->d_type == DT_DIR) files_path = traverse_directory(full_path, target_time, files_path, path_cnt);
    }

    close(dir);

    return files_path;
}

int start_connection(struct sockaddr_in serverAddr) {
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    int len = sizeof(serverAddr);

    if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
        printf("Connect server failed\n");
        close(socket_fd);
        exit(0);
    }

    printf("Connect server [%s:%d] success\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    return socket_fd;
}

int send_to_server(int socket_fd, char *buf) {
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(serverPort),
        .sin_addr.s_addr = inet_addr(serverIP),
    };

    char recvbuf[MAX_PATH_LEN] = {0};
    
    printf("==============================\n");
    printf("path to be send: %s\n", buf);

    if (send(socket_fd, buf, strlen(buf), 0) < 0) {
        printf("send data to %s:%d, failed!\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    }

    memset(buf, 0, sizeof(buf));

    if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
        printf("recv data from %s:%d, failed!\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    }

    printf("get receive message from [%s:%d] : %s\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
    memset(recvbuf, 0, sizeof(recvbuf));

    return 0;
}

void close_connection(int socket_fd) {
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
}

int main(int argc, char **argv) {
    char path_buf[100] = "./directory_big";
    printf("path: %s\n", path_buf);
    // scanf("%s", path_buf);

    time_t target_time = 1724157950;
    char target_time_buff[100];
    strftime(target_time_buff, sizeof(target_time_buff), "%Y-%m-%d %H:%M:%S", localtime(&target_time));
    printf("target_time: %s\n=========================\n", target_time_buff);

    int *path_cnt = malloc(sizeof(int));
    *path_cnt = 0;
    char **files_path = malloc(0);

    files_path = traverse_directory(path_buf, target_time, files_path, path_cnt);

    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_port = htons(serverPort),
        .sin_addr.s_addr = inet_addr(serverIP),
    };

    int socket_fd = start_connection(serverAddr);

    for (int i=0; i<*path_cnt; i++) {
        send_to_server(socket_fd, files_path[i]);
    }

    close_connection(socket_fd);

    return 0;
}