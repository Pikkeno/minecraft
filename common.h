#pragma once

#ifndef COMMON_H
#define COMMON_H

#include "protocol.h"

int gerar_id_aleatorio();
void log_event(const char *evento, int id, float m, float me, int N, float V, float bet, float payout, float player_profit, float house_profit);
void enviar_profit(int socket, float player_profit, float house_profit);
void inicializar_random();

#endif // COMMON_H
