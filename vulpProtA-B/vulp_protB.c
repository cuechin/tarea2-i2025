/* vulp_protB.c */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define DELAY 10000

int main()
{
    char * fn = "/tmp/XYZ";
    char buffer[60];
    FILE *fp;
    long int i;

    // Drop privileges to real UID
    seteuid(getuid());

    scanf("%50s", buffer );

    if (!access(fn, W_OK)) {
        // Regain root privileges
        seteuid(0);

        for (i = 0; i < DELAY; i++) { int a = i*2; }

        fp = fopen(fn, "a+");
        fwrite("\n", sizeof(char), 1, fp);
        fwrite(buffer, sizeof(char), strlen(buffer), fp);
        fclose(fp);

        // Drop privileges again
        seteuid(getuid());
    }
    else printf("No permission \n");
}