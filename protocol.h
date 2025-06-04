#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define STR_LEN 11
#define MAX_CLIENTS 10

typedef struct aviator_msg {
    int32_t player_id;
    float   value;
    char    type[STR_LEN];
    float   player_profit;
    float   house_profit;
} aviator_msg;

#endif // PROTOCOL_H
