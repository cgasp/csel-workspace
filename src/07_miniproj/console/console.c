#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FIFO_PATH "/tmp/miniproj_fifo"

// Converti tout les char d'une str (inplace) en minuscules. 
char* toLower(char* str)
{
    char* p = str;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return str;
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf("Usage: %s <type> <valeur>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* type   = argv[1];
    char* valeur = argv[2];
    char commande[256];

    // Validation 
    if (strncmp(type, "freq", 4) == 0) {
        // Vérifie la valeur de la fréquence
        int freq_valeur = atoi(valeur);
        if (freq_valeur < 0 || freq_valeur > 100) {
            printf("La valeur doit être comprise entre 0 et 100.\n");
            exit(EXIT_FAILURE);
        }

        // Prépare la commande à envoyer au FIFO
        snprintf(commande, sizeof(commande), "freq %d", freq_valeur);

    } else if (strncmp(type, "mode", 4) == 0) {
        // Convertir la valeur en minuscules pour comparaison
        toLower(valeur);

        // Vérifie la valeur correcte du mode
        if (strcmp(valeur, "auto") == 0) {
            snprintf(commande, sizeof(commande), "mode auto");
        } else if (strcmp(valeur, "manual") == 0) {
            snprintf(commande, sizeof(commande), "mode manual");
        } else {
            printf("Mode invalide. Utilisez 'auto' ou 'manual'.\n");
            exit(EXIT_FAILURE);
        }

    } else {
        printf("Type invalide. Utilisez 'freq' ou 'mode'.\n");
        exit(EXIT_FAILURE);
    }

    // Execution 
    //
    // Ouvre le FIFO en écriture
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("ouvrir fifo");
        exit(EXIT_FAILURE);
    }

    // Écrire la commande dans le FIFO
    if (write(fifo_fd, commande, strlen(commande)) == -1) {
        perror("écrire fifo");
        close(fifo_fd);
        exit(EXIT_FAILURE);
    }

    // Ferme le FIFO
    close(fifo_fd);

    printf("Commande '%s' envoyée au FIFO.\n", commande);

    return 0;
}
