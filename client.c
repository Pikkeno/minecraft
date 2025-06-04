// client.c - Cliente do jogo Aviãozinho
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/socket.h>

#include "protocol.h"
#include "connection.h"

#define MAX_NICK_LEN 13

int sockfd;
bool aposta_feita = false;
bool apostas_encerradas = false;
float valor_apostado = 0;

void usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <server_ip> <port> -nick <nickname>\n", prog_name);
    exit(EXIT_FAILURE);
}

void *listener_thread(void *arg) {
    aviator_msg msg;
    while (recv(sockfd, &msg, sizeof(msg), 0) > 0) {
        if (strcmp(msg.type, "start") == 0) {
            printf("\nRodada aberta! Digite o valor da aposta ou [Q] para sair (10 segundos restantes): ");
            fflush(stdout);
            aposta_feita = false;
            apostas_encerradas = false;
        } else if (strcmp(msg.type, "closed") == 0) {
            printf("\nApostas encerradas! Não é mais possível apostar nesta rodada.\n");
            apostas_encerradas = true;
            if (aposta_feita) {
                printf("Digite [C] para sacar.\n");
            }
        } else if (strcmp(msg.type, "multiplier") == 0) {
            printf("Multiplicador atual: %.2fx\n", msg.value);
        } else if (strcmp(msg.type, "explode") == 0) {
            printf("Aviãozinho explodiu em: %.2fx\n", msg.value);
        } else if (strcmp(msg.type, "payout") == 0) {
            printf("Você sacou em %.2fx e ganhou R$ %.2f!\n", msg.value, msg.value * valor_apostado);
        } else if (strcmp(msg.type, "profit") == 0) {
            printf("Profit atual: R$ %.2f\n", msg.player_profit);
            printf("Profit da casa: R$ %.2f\n", msg.house_profit);
        } else if (strcmp(msg.type, "bye") == 0) {
            printf("O servidor caiu, mas sua esperança pode continuar de pé. Até breve!\n");
            break;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Error: Invalid number of arguments\n");
        usage(argv[0]);
    }

    if (strcmp(argv[3], "-nick") != 0) {
        fprintf(stderr, "Error: Expected '-nick' argument\n");
        usage(argv[0]);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *nickname = argv[4];

    if (strlen(nickname) > MAX_NICK_LEN) {
        fprintf(stderr, "Error: Nickname too long (max 13)\n");
        exit(EXIT_FAILURE);
    }

    int is_ipv6 = strchr(server_ip, ':') != NULL;
    sockfd = conectar_ao_servidor(server_ip, port, is_ipv6);

    printf("Conectado ao servidor. Seu nick: %s\n", nickname);

    pthread_t listener;
    pthread_create(&listener, NULL, listener_thread, NULL);

    char buffer[256];
    while (1) {
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        if ((buffer[0] == 'Q' || buffer[0] == 'q') && buffer[1] == '\n') {
            aviator_msg bye_msg = {0};
            strncpy(bye_msg.type, "bye", STR_LEN);
            send(sockfd, &bye_msg, sizeof(bye_msg), 0);
            break;
        } else if ((buffer[0] == 'C' || buffer[0] == 'c') && buffer[1] == '\n') {
            if (aposta_feita && apostas_encerradas) {
                aviator_msg msg = {0};
                strncpy(msg.type, "cashout", STR_LEN);
                send(sockfd, &msg, sizeof(msg), 0);
            } else {
                printf("Cashout inválido.\n");
            }
        } else {
            float aposta = atof(buffer);
            if (apostas_encerradas) {
                printf("Apostas já foram encerradas.\n");
            } else if (aposta <= 0) {
                printf("Error: Invalid bet value\n");
            } else if (!aposta_feita) {
                aviator_msg msg = {0};
                msg.value = aposta;
                strncpy(msg.type, "bet", STR_LEN);
                send(sockfd, &msg, sizeof(msg), 0);
                valor_apostado = aposta;
                aposta_feita = true;
                printf("Aposta recebida: R$ %.2f\n", aposta);
            } else {
                printf("Você já apostou nesta rodada.\n");
            }
        }
    }

    pthread_cancel(listener);
    pthread_join(listener, NULL);
    close(sockfd);
    printf("Desconectado. Até logo, %s.\n", nickname);
    return 0;
}
