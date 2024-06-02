#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Module de monitoring
/*
La supervision de la température du microprocesseur et la gestion automatique de
la vitesse de clignotement de la LED Status devront être réalisées par un module
noyau. Ce module offrira, via le sysfs, une interface permettant de choisir

le mode de fonctionnement automatique ou manuel
la fréquence de clignotement de la LED Status
En mode automatique, la fréquence de clignotement dépendra de la température du
CPU, soit :

Température
 → fréquence de 2Hz
Température
 → fréquence de 5Hz
Température
 → fréquence de 10Hz
Température
 → fréquence de 20Hz
*/

int main()
{
}
