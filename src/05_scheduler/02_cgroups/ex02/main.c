#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 1048576  // 1 MB
#define BLOCKS_NUM 50

int main()
{
    void* blocks[BLOCKS_NUM];

    for (int i = 0; i < BLOCKS_NUM; ++i) {
        blocks[i] =
            malloc(BLOCK_SIZE);  // Allouer un nombre défini de blocs de mémoire
                                 // d’un mébibyte1, par exemple 50
        if (blocks[i] == NULL) {  // Tester si le pointeur est non nul
            perror("Allocation Memoire raté");  // Ne fonctionne pas, il est
                                                // killed avant.
            printf("Peut plus alloué de mémoire %d\n", i + 1);
            break;
        }
        memset(blocks[i], 0, BLOCK_SIZE);  // Remplir le bloc avec des 0
        printf("Block %d rempli\n", i + 1);
    }

    return 0;
}
