// server.c - Servidor do jogo Aviãozinho (refatorado)
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#include "protocol.h"
#include "common.h"
#include "client_handler.h"

#define MAX_CLIENTS 10
#define ROUND_DURATION 10
#define BROADCAST_INTERVAL 100000 // 100ms

int client_sockets[MAX_CLIENTS];
int client_ids[MAX_CLIENTS];
float apostas[MAX_CLIENTS];
float player_profits[MAX_CLIENTS];
float house_profit = 0.0;
int client_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
float multiplicador = 1.0;

void broadcast(aviator_msg *msg) {
    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] != 0) {
            send(client_sockets[i], msg, sizeof(*msg), 0);
        }
    }
    pthread_mutex_unlock(&lock);
}

void *rodada_loop(void *arg) {
    while (1) {
        sleep(1);
        if (client_count == 0) continue;

        multiplicador = 1.0f;
        float ponto_explosao = 2.0f;

        aviator_msg start_msg = {0};
        strncpy(start_msg.type, "start", STR_LEN);
        broadcast(&start_msg);
        log_event("start", -1, -1, -1, client_count, -1, -1, -1, -9999, -9999);

        sleep(ROUND_DURATION);

        aviator_msg closed_msg = {0};
        strncpy(closed_msg.type, "closed", STR_LEN);
        broadcast(&closed_msg);
        log_event("closed", -1, -1, -1, -1, -1, -1, -1, -9999, -9999);

        float total = 0;
        int apostadores = 0;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (apostas[i] > 0) {
                total += apostas[i];
                apostadores++;
            }
        }

        ponto_explosao = sqrtf(1 + apostadores + 0.01f * total);
        log_event("explode", -1, -1, ponto_explosao, apostadores, total, -1, -1, -9999, -9999);

        while (multiplicador < ponto_explosao) {
            usleep(BROADCAST_INTERVAL);
            multiplicador += 0.01f;
            aviator_msg m_msg = {0};
            m_msg.value = multiplicador;
            strncpy(m_msg.type, "multiplier", STR_LEN);
            broadcast(&m_msg);
        }

        aviator_msg explode_msg = {0};
        explode_msg.value = multiplicador;
        strncpy(explode_msg.type, "explode", STR_LEN);
        broadcast(&explode_msg);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (apostas[i] > 0) {
                player_profits[i] -= apostas[i];
                house_profit += apostas[i];
                enviar_profit(client_sockets[i], player_profits[i], house_profit);
                log_event("profit", client_ids[i], multiplicador, ponto_explosao, -1, -1, -1, -1, player_profits[i], house_profit);
            }
            apostas[i] = 0;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <v4|v6> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int is_ipv6 = (strcmp(argv[1], "v6") == 0);
    int port = atoi(argv[2]);

    int server_sock = socket(is_ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (!is_ipv6) {
        struct sockaddr_in addr4;
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_addr.s_addr = INADDR_ANY;
        addr4.sin_port = htons(port);

        if (bind(server_sock, (struct sockaddr *)&addr4, sizeof(addr4)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
        }
    } else {
        struct sockaddr_in6 addr6;
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_addr = in6addr_any;
        addr6.sin6_port = htons(port);

        if (bind(server_sock, (struct sockaddr *)&addr6, sizeof(addr6)) < 0) {
            perror("bind");
            exit(EXIT_FAILURE);
        }
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado na porta %d (%s).\n", port, is_ipv6 ? "IPv6" : "IPv4");

    inicializar_random();

    pthread_t rodada_thread;
    pthread_create(&rodada_thread, NULL, rodada_loop, NULL);

    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addrlen);

        if (*client_sock < 0) {
            perror("accept");
            free(client_sock);
            continue;
        }

        pthread_mutex_lock(&lock);
        if (client_count >= MAX_CLIENTS) {
            close(*client_sock);
            free(client_sock);
            pthread_mutex_unlock(&lock);
            continue;
        }

        client_count++;
        pthread_t thread;
        pthread_create(&thread, NULL, client_handler, client_sock);
        pthread_detach(thread);
        pthread_mutex_unlock(&lock);
    }

    close(server_sock);
    return 0;
}
