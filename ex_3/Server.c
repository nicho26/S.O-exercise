 #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define MAX_PLAYERS 10
#define MIN_PLAYERS 3

typedef struct {
    char name[50];
    int games_played;
    int score;
} Player;

typedef struct {
    int socket;
    char name[50];
} ConnectedPlayer;

Player players[MAX_PLAYERS];
ConnectedPlayer connected_players[MAX_PLAYERS];
int num_registered = 0;
int num_connected = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void print_players() {
    printf("\n--- Giocatori iscritti ---\n");
    for (int i = 0; i < num_registered; i++) {
        printf("%s - Partite: %d - Punti: %d\n", players[i].name, players[i].games_played, players[i].score);
    }

    printf("\n--- Giocatori connessi ---\n");
    for (int i = 0; i < num_connected; i++) {
        printf("%s\n", connected_players[i].name);
    }
    printf("----------------------------\n");
}

void *handle_client(void *arg) {
    int client_sock = *((int *)arg);
    free(arg);
   
    char player_name[50];
    recv(client_sock, player_name, sizeof(player_name), 0);
   
    pthread_mutex_lock(&lock);
    int found = 0;
   
    // Controlla se il giocatore esiste già
    for (int i = 0; i < num_registered; i++) {
        if (strcmp(players[i].name, player_name) == 0) {
            found = 1;
            break;
        }
    }
   
    // Se il giocatore è nuovo, lo aggiunge
    if (!found) {
        strcpy(players[num_registered].name, player_name);
        players[num_registered].games_played = 0;
        players[num_registered].score = 0;
        num_registered++;
    }

    // Aggiunge ai connessi
    strcpy(connected_players[num_connected].name, player_name);
    connected_players[num_connected].socket = client_sock;
    num_connected++;

    print_players();
   
    // Se il numero minimo di giocatori è raggiunto, avvia la partita
    if (num_connected >= MIN_PLAYERS) {
        printf("\nAvvio della partita...\n");

        // Genera una classifica casuale
        int ranking[num_connected];
        for (int i = 0; i < num_connected; i++) {
            ranking[i] = i;
        }
       
        for (int i = num_connected - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            int temp = ranking[i];
            ranking[i] = ranking[j];
            ranking[j] = temp;
        }

        // Assegna i punteggi ai primi 3
        for (int i = 0; i < num_connected; i++) {
            for (int j = 0; j < num_registered; j++) {
                if (strcmp(players[j].name, connected_players[ranking[i]].name) == 0) {
                    players[j].games_played++;
                    if (i == 0) players[j].score += 3;
                    else if (i == 1) players[j].score += 2;
                    else if (i == 2) players[j].score += 1;
                }
            }
        }

        // Invia i risultati ai client
        for (int i = 0; i < num_connected; i++) {
            char msg[100];
            sprintf(msg, "Classifica: 1° %s, 2° %s, 3° %s\n",
                    connected_players[ranking[0]].name,
                    connected_players[ranking[1]].name,
                    connected_players[ranking[2]].name);
            send(connected_players[i].socket, msg, sizeof(msg), 0);
            close(connected_players[i].socket);
        }

        num_connected = 0;  // Resetta la lista dei connessi
        print_players();
    }

    pthread_mutex_unlock(&lock);
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
    listen(server_sock, MAX_PLAYERS);

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
