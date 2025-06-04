// connection.h - Header para funções de conexão
#ifndef CONNECTION_H
#define CONNECTION_H

int iniciar_socket_servidor(int is_ipv6, int port);
int aguardar_conexao(int server_sock);
int conectar_ao_servidor(const char *ip, int port, int is_ipv6);

#endif // CONNECTION_H
