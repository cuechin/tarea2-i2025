/* vulp_protA.c */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#define DELAY 10000
#define REPEAT 5

int main()
{
    char * fn = "/tmp/XYZ";
    char buffer[60];
    FILE *fp;
    long int i;
    struct stat statbuf1, statbuf2;
    int success = 1;

    scanf("%50s", buffer );

    for (int j = 0; j < REPEAT; j++) {
        if (access(fn, W_OK) != 0) {
            printf("No permission\n");
            return 1;
        }
        if (stat(fn, &statbuf1) != 0) {
            printf("stat error\n");
            return 1;
        }
        for (i = 0; i < DELAY; i++) { int a = i*2; }
        fp = fopen(fn, "a+");
        if (!fp) {
            printf("fopen error\n");
            return 1;
        }
        if (fstat(fileno(fp), &statbuf2) != 0) {
            printf("fstat error\n");
            fclose(fp);
            return 1;
        }
        if (statbuf1.st_ino != statbuf2.st_ino) {
            printf("Race condition detected! Aborting.\n");
            fclose(fp);
            success = 0;
            break;
        }
        fclose(fp);
    }

    if (success) {
        fp = fopen(fn, "a+");
        fwrite("\n", sizeof(char), 1, fp);
        fwrite(buffer, sizeof(char), strlen(buffer), fp);
        fclose(fp);
    }
}