// client_handler.c - Thread de tratamento de cliente
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "protocol.h"
#include "common.h"
#include "client_handler.h"

extern int client_sockets[];
extern int client_ids[];
extern float apostas[];
extern float player_profits[];
extern float house_profit;
extern float multiplicador;
extern pthread_mutex_t lock;

void *client_handler(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    int client_id = gerar_id_aleatorio();
    int index = -1;

    pthread_mutex_lock(&lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
            client_sockets[i] = client_sock;
            client_ids[i] = client_id;
            apostas[i] = 0;
            player_profits[i] = 0;
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&lock);

    aviator_msg msg;
    while (recv(client_sock, &msg, sizeof(msg), 0) > 0) {
        if (strcmp(msg.type, "bye") == 0) {
            log_event("bye", client_id, -1, -1, -1, -1, -1, -1, -9999, -9999);
            break;
        } else if (strcmp(msg.type, "bet") == 0) {
            pthread_mutex_lock(&lock);
            apostas[index] = msg.value;
            pthread_mutex_unlock(&lock);
            log_event("bet", client_id, -1, -1, -1, -1, msg.value, -1, -9999, -9999);
        } else if (strcmp(msg.type, "cashout") == 0) {
            pthread_mutex_lock(&lock);
            float ganho = apostas[index] * multiplicador;
            player_profits[index] += (ganho - apostas[index]);
            house_profit -= (ganho - apostas[index]);
            aviator_msg payout_msg = {0};
            payout_msg.value = multiplicador;
            strncpy(payout_msg.type, "payout", STR_LEN);
            send(client_sock, &payout_msg, sizeof(payout_msg), 0);

            enviar_profit(client_sock, player_profits[index], house_profit);
            apostas[index] = 0;
            pthread_mutex_unlock(&lock);

            log_event("cashout", client_id, multiplicador, -1, -1, -1, -1, ganho, player_profits[index], house_profit);
        }
    }

    close(client_sock);
    pthread_mutex_lock(&lock);
    client_sockets[index] = 0;
    pthread_mutex_unlock(&lock);
    return NULL;
}
