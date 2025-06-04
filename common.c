// common.c - Funções auxiliares
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include "protocol.h"
#include "common.h"

// Gera um ID único entre 1 e 999
int gerar_id_aleatorio() {
    return rand() % 999 + 1;
}

// Imprime mensagem formatada de log no servidor
void log_event(const char *evento, int id, float m, float me, int N, float V, float bet, float payout, float player_profit, float house_profit) {
    printf("event=%s", evento);
    if (id >= 0) printf(" | id=%d", id);
    if (m >= 0) printf(" | m=%.2f", m);
    if (me >= 0) printf(" | me=%.2f", me);
    if (N >= 0) printf(" | N=%d", N);
    if (V >= 0) printf(" | V=%.2f", V);
    if (bet >= 0) printf(" | bet=%.2f", bet);
    if (payout >= 0) printf(" | payout=%.2f", payout);
    if (player_profit >= -9999) printf(" | player_profit=%.2f", player_profit);
    if (house_profit >= -9999) printf(" | house_profit=%.2f", house_profit);
    printf("\n");
}

// Envia mensagem a um cliente com campos de lucro
void enviar_profit(int socket, float player_profit, float house_profit) {
    aviator_msg msg = {0};
    strncpy(msg.type, "profit", STR_LEN);
    msg.player_profit = player_profit;
    msg.house_profit = house_profit;
    send(socket, &msg, sizeof(msg), 0);
}

// Inicializa o gerador de números aleatórios
void inicializar_random() {
    srand(time(NULL));
}
