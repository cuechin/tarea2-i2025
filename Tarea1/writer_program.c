#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define DELAY 10000

int main()
{
    const char *fn = "/tmp/XYZ";
    char buffer[60];
    FILE *fp;
    long int i;

    /* leer entrada del usuario */
    if (scanf("%50s", buffer) != 1) {
        return 1;
    }

    if (!access(fn, W_OK)) {
        /* simulación de retardo */
        for (i = 0; i < DELAY; i++) {
            (void)(i * i);
        }

        fp = fopen(fn, "a+");
        if (fp) {
            fputc('\n', fp);
            fwrite(buffer, sizeof(char), strlen(buffer), fp);
            fclose(fp);
        }
    }
    else {
        printf("No permission\n");
    }

    return 0;
}
