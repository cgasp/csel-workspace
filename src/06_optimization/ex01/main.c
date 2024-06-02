#include <stdint.h>

#define SIZE 5000

static int32_t array[SIZE][SIZE];

<<<<<<< HEAD
int main(void)
{
    int i, j, k;

    for (k = 0; k < 10; k++) {
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
=======
int main (void)
{
    int i, j, k;

    for (k = 0; k < 10; k++)
    {
        for (i = 0; i < SIZE; i++)
        {
            for (j = 0; j < SIZE; j++)
            {
>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
                array[j][i]++;
            }
        }
    }
    return 0;
}
<<<<<<< HEAD
=======

>>>>>>> 72cc4f3ca84b2ff8589571b2587b55e131fddb2a
