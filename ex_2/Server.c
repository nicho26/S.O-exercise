 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define PORT 12345

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creazione della socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Opzione per il riutilizzo dell'indirizzo
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configurazione dell'indirizzo del server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // ascolta su tutte le interfacce
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server in ascolto sulla porta %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Gestione della connessione con fork: il figlio gestisce il client
        if (fork() == 0) {
            close(server_fd);
            int number;
            if (read(new_socket, &number, sizeof(number)) <= 0) {
                perror("read");
                close(new_socket);
                exit(EXIT_FAILURE);
            }
            printf("Ricevuto %d da %s:%d\n", number, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            number++; // incremento del numero
            if (write(new_socket, &number, sizeof(number)) <= 0) {
                perror("write");
            }
            close(new_socket);
            exit(0);
        }
        close(new_socket);
    }

    return 0;
}
