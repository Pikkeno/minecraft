// connect_functions.c - Funções de conexão comuns a cliente e servidor
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "connection.h"

int iniciar_socket_servidor(int is_ipv6, int port) {
    int sockfd = socket(is_ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (!is_ipv6) {
        struct sockaddr_in addr4 = {0};
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = INADDR_ANY;
        addr4.sin_port = htons(port);

        if (bind(sockfd, (struct sockaddr *)&addr4, sizeof(addr4)) < 0) {
            perror("bind");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    } else {
        struct sockaddr_in6 addr6 = {0};
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons(port);

        if (bind(sockfd, (struct sockaddr *)&addr6, sizeof(addr6)) < 0) {
            perror("bind");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

int aguardar_conexao(int server_sock) {
    struct sockaddr_storage client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addrlen);
    if (client_sock < 0) {
        perror("accept");
        return -1;
    }
    return client_sock;
}

int conectar_ao_servidor(const char *ip, int port, int is_ipv6) {
    int sockfd = socket(is_ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (!is_ipv6) {
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
            perror("inet_pton");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    } else {
        struct sockaddr_in6 server_addr = {0};
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_port = htons(port);
        if (inet_pton(AF_INET6, ip, &server_addr.sin6_addr) <= 0) {
            perror("inet_pton");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }

    return sockfd;
}
