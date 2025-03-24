 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/wait.h>

#define PORT 12345
#define NUM_CHILDREN 5  // numero di richieste concorrenti per ogni esecuzione del client

int main() {
    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (fork() == 0) {
            // Processo figlio: ogni istanza usa un seme unico
            srand(time(NULL) + getpid());
            int number = rand() % 100; // genera un numero casuale tra 0 e 99

            int sock = 0;
            struct sockaddr_in serv_addr;
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                perror("socket creation error");
                exit(EXIT_FAILURE);
            }

            serv_addr.sin_family = AF_INET;
            serv_addr.sin_port = htons(PORT);

            // Connetti al server (localhost)
            if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
                perror("Invalid address/ Address not supported");
                exit(EXIT_FAILURE);
            }

            if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("Connection Failed");
                exit(EXIT_FAILURE);
            }

            // Invio del numero al server
            if (write(sock, &number, sizeof(number)) <= 0) {
                perror("write");
                close(sock);
                exit(EXIT_FAILURE);
            }

            int response;
            if (read(sock, &response, sizeof(response)) <= 0) {
                perror("read");
                close(sock);
                exit(EXIT_FAILURE);
            }
            printf("Processo figlio PID %d: Inviato %d, Ricevuto %d\n", getpid(), number, response);
            close(sock);

            sleep(1); // permette di vedere l'output in sequenza
            exit(0);
        }
    }

    // Attende la terminazione di tutti i processi figli
    while (wait(NULL) > 0);
    return 0;
}
