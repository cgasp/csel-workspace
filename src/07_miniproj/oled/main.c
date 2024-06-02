<<<<<<< HEAD
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ssd1306.h"

/*
Un daemon en espace utilisateur offrira les services pour une gestion manuelle.
Ce daemon proposera deux interfaces de gestion distinctes, soit :

Interface physique via les boutons poussoir et LED Power de la carte d’extension
S1 pour augmenter la vitesse de rotation du ventilateur, la pression du S1 devra
être signalisée sur la LED Power S2 pour diminuer la vitesse de rotation du
ventilateur, la pression du S2 devra être signalisée sur la LED Power S3 pour
changer du mode automatique au mode manuel et vice versa. Interface IPC, au
choix du développeur, permettant de dialoguer avec une application pour choisir
le mode de fonctionnement et spécifier la fréquence de clignotement Le daemon
utilisera l’écran OLED pour indiquer le mode de fonctionnement, la température
actuelle du microprocesseur ainsi que la fréquence de clignotement de la LED
Status.

Une application fournira une interface utilisateur, une ligne de commande, pour
piloter le système via l’interface IPC choisie.
*/

// creer un fifo
// int mkfifo(const char* pathname, mode_t mode)

=======
#include "ssd1306.h"

>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
int main()
{
    ssd1306_init();

<<<<<<< HEAD
    ssd1306_set_position(0, 0);
    ssd1306_puts("CSEL1a - SP.07");
    ssd1306_set_position(0, 1);
    ssd1306_puts("  Demo - SW");
    ssd1306_set_position(0, 2);
    ssd1306_puts("--------------");

    ssd1306_set_position(0, 3);
    ssd1306_puts("Temp: 35'C");
    ssd1306_set_position(0, 4);
    ssd1306_puts("Freq: 1Hz");
    ssd1306_set_position(0, 5);
    ssd1306_puts("Duty: 50%");

    int i = 10;
    for (i = 10; i == 0; i--) {
        // ssd1306_set_position(0, 4);
        char buffer[strlen("Freq:   Hz")];

        // Format the string
        // sprintf(buffer, "Freq: %dHz", i);
        // ssd1306_puts(buffer);
        printf("%s", buffer);
        // fflush(stdout);
        // sleep(1);
    }
    printf("finished");
    return 0;
}
=======
    ssd1306_set_position (0,0);
    ssd1306_puts("CSEL1a - SP.07");
    ssd1306_set_position (0,1);
    ssd1306_puts("  Demo - SW");
    ssd1306_set_position (0,2);
    ssd1306_puts("--------------");

    ssd1306_set_position (0,3);
    ssd1306_puts("Temp: 35'C");
    ssd1306_set_position (0,4);
    ssd1306_puts("Freq: 1Hz");
    ssd1306_set_position (0,5);
    ssd1306_puts("Duty: 50%");

    return 0;
}

>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
