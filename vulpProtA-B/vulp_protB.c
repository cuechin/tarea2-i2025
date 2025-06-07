/* vulp_protB.c */
#include <stdio.h>       // Funciones estándar de entrada/salida
#include <unistd.h>      // Para access(), seteuid(), getuid()
#include <string.h>      // Para strlen()
#include <sys/types.h>   // Para tipos como uid_t

#define DELAY 10000      // Retardo simulado

int main()
{
    char * fn = "/tmp/XYZ";           // Ruta del archivo objetivo
    char buffer[60];                  // Buffer para entrada del usuario
    FILE *fp;                         // Puntero al archivo
    long int i;                       // Variable para el retardo

    seteuid(getuid());               // Bajar privilegios al UID real

    scanf("%50s", buffer );          // Leer entrada del usuario

    if (!access(fn, W_OK)) {         // Verificar si el usuario tiene permiso de escritura
        seteuid(0);                  // Recuperar privilegios de root

        for (i = 0; i < DELAY; i++) { int a = i*2; }  // Simular ventana de ataque (retardo)

        fp = fopen(fn, "a+");                      // Abrir archivo en modo append
        fwrite("\n", sizeof(char), 1, fp);         // Escribir nueva línea
        fwrite(buffer, sizeof(char), strlen(buffer), fp);  // Escribir contenido del buffer
        fclose(fp);                                // Cerrar archivo

        seteuid(getuid());         // Bajar privilegios nuevamente
    }
    else printf("No permission \n");  // Mostrar error si no hay permisos
}