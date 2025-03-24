 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    char agent_name[30];
    int quantity, price, min_price;
   
    printf("Nome agente: ");
    scanf("%s", agent_name);
    printf("Quantità: ");
    scanf("%d", &quantity);
    printf("Prezzo: ");
    scanf("%d", &price);
    printf("Prezzo minimo: ");
    scanf("%d", &min_price);

    sprintf(buffer, "AGENTE %s %d %d %d", agent_name, quantity, price, min_price);
    send(sock, buffer, sizeof(buffer), 0);
   
    recv(sock, buffer, sizeof(buffer), 0);
    printf("Esito: %s\n", buffer);

    close(sock);
    return 0;
}
