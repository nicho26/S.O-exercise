 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define MAX_TITLES 10
#define BUFFER_SIZE 1024

typedef struct {
    char agent[30];
    int quantity;
    int price;
    int min_price;
    int total_earned;
} Stock;

Stock stocks[MAX_TITLES];
int num_stocks = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void print_stocks() {
    printf("\n--- Quotazioni attuali ---\n");
    for (int i = 0; i < num_stocks; i++) {
        printf("%s - Quantità: %d - Prezzo: %d - Min Prezzo: %d\n",
               stocks[i].agent, stocks[i].quantity, stocks[i].price, stocks[i].min_price);
    }
    printf("---------------------------\n");
}

void *handle_client(void *arg) {
    int client_sock = *((int *)arg);
    free(arg);
   
    char buffer[BUFFER_SIZE];
    recv(client_sock, buffer, sizeof(buffer), 0);

    pthread_mutex_lock(&lock);
   
    if (strncmp(buffer, "AGENTE", 6) == 0) {
        // Un agente sta registrando un titolo
        Stock new_stock;
        sscanf(buffer, "AGENTE %s %d %d %d", new_stock.agent, &new_stock.quantity, &new_stock.price, &new_stock.min_price);
        new_stock.total_earned = 0;
       
        stocks[num_stocks++] = new_stock;
        printf("Agente %s ha registrato un titolo.\n", new_stock.agent);
    }
    else if (strncmp(buffer, "INVESTITORE", 11) == 0) {
        // Un investitore chiede la lista dei titoli
        char list[BUFFER_SIZE] = "";
        for (int i = 0; i < num_stocks; i++) {
            char entry[100];
            sprintf(entry, "%s %d %d\n", stocks[i].agent, stocks[i].quantity, stocks[i].price);
            strcat(list, entry);
        }
        send(client_sock, list, sizeof(list), 0);
       
        // Riceve la scelta dell'investitore
        recv(client_sock, buffer, sizeof(buffer), 0);
        char chosen_agent[30];
        sscanf(buffer, "ACQUISTA %s", chosen_agent);
       
        for (int i = 0; i < num_stocks; i++) {
            if (strcmp(stocks[i].agent, chosen_agent) == 0) {
                stocks[i].quantity--;
                stocks[i].price++;
               
                // Prezzo degli altri titoli cala
                for (int j = 0; j < num_stocks; j++) {
                    if (j != i) stocks[j].price--;
                }
               
                // Se il titolo è terminato o sotto il prezzo minimo, informa l'agente
                if (stocks[i].quantity == 0 || stocks[i].price < stocks[i].min_price) {
                    sprintf(buffer, "RICAVO %s %d", stocks[i].agent, stocks[i].total_earned);
                    send(client_sock, buffer, sizeof(buffer), 0);
                   
                    // Rimuove il titolo
                    for (int k = i; k < num_stocks - 1; k++) {
                        stocks[k] = stocks[k + 1];
                    }
                    num_stocks--;
                }
                break;
            }
        }
    }
   
    print_stocks();
    pthread_mutex_unlock(&lock);
    close(client_sock);
    return NULL;
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_TITLES);

    printf("Server in ascolto sulla porta %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, new_sock);
        pthread_detach(thread);
    }

    return 0;
}
