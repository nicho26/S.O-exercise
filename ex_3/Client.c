#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char player_name[50], buffer[100];

    printf("Inserisci il tuo nome: ");
    scanf("%s", player_name);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    send(sock, player_name, sizeof(player_name), 0);

    recv(sock, buffer, sizeof(buffer), 0);
    printf("\nRisultato partita: %s\n", buffer);

    close(sock);
    return 0;
}
