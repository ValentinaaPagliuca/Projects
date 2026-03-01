#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 

#define MAXLEN 255
#define PORT 110


void die(char *);
void receive_response(int);
void close_socket(int); 

int main() {
    int sockfd;
    int numero;
    char sendbuff[MAXLEN], recvbuff[MAXLEN], command[MAXLEN];
    struct sockaddr_in server_ip_port;

    // Pulizia dei buffer
    memset(sendbuff, 0, MAXLEN);
    memset(recvbuff, 0, MAXLEN);

    // Creazione del socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) die("Errore nella creazione del socket");
    printf("Socket creato correttamente.\n");

    // Configurazione dell'indirizzo del server
    server_ip_port.sin_family = AF_INET;
    server_ip_port.sin_addr.s_addr = inet_addr("213.209.0.134");
    server_ip_port.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *) &server_ip_port, sizeof(server_ip_port)) < 0) {
        die("Errore nella connessione al server");
    }

    // Messaggio di benvenuto
    receive_response(sockfd);

    // USER
    printf("Inserisci username: ");
    fgets(sendbuff, MAXLEN, stdin);
    sendbuff[strcspn(sendbuff, "\n")] = 0; // Rimuove il newline finale
    snprintf(command, sizeof(command), "USER %s\r\n", sendbuff);
    if (send(sockfd, command, strlen(command), 0) < 0) {
        die("Errore nell'invio del comando USER");
    }
    receive_response(sockfd);

    // PASS
    printf("Inserisci password: ");
    fgets(sendbuff, MAXLEN, stdin);
    sendbuff[strcspn(sendbuff, "\n")] = 0;
    snprintf(command, sizeof(command), "PASS %s\r\n", sendbuff);
    if (send(sockfd, command, strlen(command), 0) < 0) {
        die("Errore nell'invio del comando PASS");
    }
    receive_response(sockfd);

    // Invio del comando LIST di default
    if (send(sockfd, "LIST\r\n", 6, 0) < 0) {
        die("Errore nell'invio del comando LIST");
    }
    receive_response(sockfd);


    // Loop per i comandi utente LIST RETR DELE QUIT
    while (1) {
        printf("Inserisci comando: LIST o RETR o DELE o QUIT\n");
        fgets(sendbuff, MAXLEN, stdin);
        sendbuff[strcspn(sendbuff, "\n")] = 0;

        if (strcmp(sendbuff, "LIST") == 0) {
            if (send(sockfd, "LIST\r\n", 6, 0) < 0) {
                die("Errore nell'invio del comando LIST");
            }
            receive_response(sockfd);
        } else if (strcmp(sendbuff, "RETR") == 0) {
            printf("Inserisci un numero: ");
            scanf("%d", &numero);
            while (getchar() != '\n'); // Pulisce il buffer di input
            snprintf(command, sizeof(command), "RETR %d\r\n", numero);
            if (send(sockfd, command, strlen(command), 0) < 0) {
                die("Errore nell'invio del comando RETR");
            }
            receive_response(sockfd);
            receive_response(sockfd);
        } else if (strcmp(sendbuff, "DELE") == 0) {
            printf("Inserisci un numero: ");
            scanf("%d", &numero);
            while (getchar() != '\n');
            snprintf(command, sizeof(command), "DELE %d\r\n", numero);
            if (send(sockfd, command, strlen(command), 0) < 0) {
                die("Errore nell'invio del comando DELE");
            }
            receive_response(sockfd);
        } else if (strcmp(sendbuff, "QUIT") == 0) {
            if (send(sockfd, "QUIT\r\n", 6, 0) < 0) {
                die("Errore nell'invio del comando QUIT");
            }
            receive_response(sockfd);
            break;
        } else {
            printf("Comando non riconosciuto. Riprova.\n");
        }
    }

    // Chiusura del socket
    close_socket(sockfd);
    return 0;
}
void receive_response(int sockfd) {
    char buffer[MAXLEN]; 
    char *response = NULL; 
    size_t response_size = 0; 
    int n; 

    while (1) {
        n = recv(sockfd, buffer, MAXLEN - 1, 0);
        
        if (n < 0) {
            die("Errore nella ricezione dei dati"); 
        } else if (n == 0) {
            break; 
        }

        buffer[n] = '\0'; // Aggiunge il terminatore di stringa al buffer

        // Aggiunge il contenuto del buffer alla risposta
        response = realloc(response, response_size + n + 1); 
        if (!response) {
            die("Errore di memoria"); // Gestione dell'errore se realloc fallisce
        }
        memcpy(response + response_size, buffer, n + 1); // Copia i dati dal buffer alla risposta
        response_size += n; 

        
        if (strstr(response, "\r\n.\r\n") != NULL) {
            break; 
        }

        // Per risposte su una sola riga, esce dopo aver letto una riga
        if (n < MAXLEN - 1) {
            break; // Se il numero di byte ricevuti è inferiore alla dime max esce
        }
    }

  
    printf("S: %s\n", response);

    free(response);
}




// Funzione per stampare un messaggio di errore e terminare il programma
void die(char *error) {
    fprintf(stderr, "%s\n", error);
    exit(1);
}

// Funzione per chiudere il socket
void close_socket(int sockfd) {
    if (close(sockfd) < 0) { 
        die("Errore nella chiusura del socket");
    } else {
        printf("Connessione chiusa correttamente.\n");
    }
}


/*https://mail1.libero.it/appsuite/#!!&app=io.ox/mail&folder=default0/INBOX
tabowav621@libero.it
tabo21!_A
*/



