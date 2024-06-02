#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void useCPU()
{
    while (1) {
        // Boucle infinie pour utiliser 100% de la CPU
        // il ne sortera pas.
    }
}

int main()
{
    pid_t pid1, pid2;

    // premier processus
    pid1 = fork();
    if (pid1 == 0) {  // sp.06 slide 24
        // Code de l'enfant
        useCPU();
    } else if (pid1 > 0) {
        // code du parent
        // deuxième processus
        pid2 = fork();
        if (pid2 == 0) {
            // Code de l'enfant
            useCPU();
        } else if (pid2 > 0) {
            // Code du parent pour le deuxième processus

            // Le parent attend que les enfants se terminent (ce qui n'arrivera
            // pas)
            wait(NULL);  // sp.06 slide 23
            wait(NULL);
        } else {
            // Erreur lors de la création du deuxième processus
            perror("Erreur lors de la création du deuxième processus");
            exit(EXIT_FAILURE);
        }
    } else {
        // Erreur lors de la création du premier processus
        perror("Erreur lors de la création du premier processus");
        exit(EXIT_FAILURE);
    }

    return 0;
}
