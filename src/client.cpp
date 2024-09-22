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

#include "util.h"

char ** find_targeted_files(char *name, time_t target_time, char **files_path, int *path_cnt);
int start_connection(struct sockaddr_in serverAddr);
int send_to_server(int socket_fd, char *buf);
void close_connection(int socket_fd);

int main(int argc, char **argv) {
    char* target_dir = argv[1];
    printf("%sTarget dir: %s\n", SEP_LINE, target_dir);

    time_t target_time = atoi(argv[2]);
    char target_time_buff[100];
    strftime(target_time_buff, sizeof(target_time_buff), "%Y-%m-%d %H:%M:%S", localtime(&target_time));
    printf("Target time: %s\n%s", target_time_buff, SEP_LINE);

    int *path_cnt = (int*)malloc(sizeof(int));
    *path_cnt = 0;
    char **files_path = (char**)malloc(0);

    files_path = find_targeted_files(target_dir, target_time, files_path, path_cnt);

    struct sockaddr_in serverAddr = {
        AF_INET,
        htons(serverPort),
        inet_addr(serverIP),
    };

    print_debug("\n\n");
    int socket_fd = start_connection(serverAddr);

    for (int i=0; i<*path_cnt; i++) {
        send_to_server(socket_fd, files_path[i]);
    }

    print_debug("%s\n", SEP_LINE);
    close_connection(socket_fd);

    return 0;
}

char ** find_targeted_files(char *name, time_t target_time, char **files_path, int *path_cnt) {
    DIR *dir;
    struct dirent *ent;
    struct stat states;

    dir = opendir(name);
    if (!dir) {
        perror(name);
        return NULL;
    }

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

        print_debug("%20s | %-s\n", "Last Modified Time", "Filename");
        print_debug("%20s | %-s\n\n", date, full_path);

        if (states.st_ctime > target_time && ent->d_type != DT_DIR) {
            print_debug("Is file & modified after target time!\n");

            files_path = (char**)realloc(files_path, (*path_cnt+1) * sizeof(full_path));
            files_path[*path_cnt] = (char*)malloc(sizeof(full_path));
            strcpy(files_path[*path_cnt], full_path);
            (*path_cnt)++;
        }
        print_debug(SEP_LINE);

        if (ent->d_type == DT_DIR) files_path = find_targeted_files(full_path, target_time, files_path, path_cnt);
    }

    close(*((int*)dir));

    return files_path;
}

int start_connection(struct sockaddr_in serverAddr) {
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("Create socket fail!");
        return -1;
    }

    int len = sizeof(serverAddr);

    if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
        perror("Connect server failed");
        close(socket_fd);
        exit(0);
    }

    printf("Connect server [%s:%d] success\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    return socket_fd;
}

int send_to_server(int socket_fd, char *buf) {
    struct sockaddr_in serverAddr = {
        AF_INET,
        htons(serverPort),
        inet_addr(serverIP),
    };

    char recvbuf[MAX_PATH_LEN] = {0};
    
    print_debug(SEP_LINE);
    print_debug("Send path: %s\n", buf);

    if (send(socket_fd, buf, strlen(buf), 0) < 0) {
        printf("Send data to %s:%d, failed!\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    }

    memset(&buf, 0, sizeof(buf));

    if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
        printf("Receive data from %s:%d, failed!\n",
                inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
    }

    print_debug("Receive message from [%s:%d] : %s\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port), recvbuf);
    memset(&recvbuf, 0, sizeof(recvbuf));

    return 0;
}

void close_connection(int socket_fd) {
    if (close(socket_fd) < 0) {
        perror("Close socket failed!");
    }
    printf("Close socket success\n");
}