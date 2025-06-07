/* vulp_protA.c */
#include <stdio.h>     // Funciones de entrada/salida estándar
#include <unistd.h>    // Para access(), fileno()
#include <string.h>    // Para strlen()
#include <sys/stat.h>  // Para stat() y fstat()

#define DELAY 10000    // Retardo simulado para crear ventana TOCTOU
#define REPEAT 5       // Número de repeticiones de la verificación

int main()
{
    char * fn = "/tmp/XYZ";                  // Ruta del archivo objetivo
    char buffer[60];                         // Buffer para entrada del usuario
    FILE *fp;                                // Puntero a archivo
    long int i;                              // Variable para el bucle de retardo
    struct stat statbuf1, statbuf2;          // Estructuras para almacenar info del archivo
    int success = 1;                         // Bandera para determinar si se escribe al final

    scanf("%50s", buffer );                  // Leer entrada del usuario

    for (int j = 0; j < REPEAT; j++) {       // Repetir el intento varias veces
        if (access(fn, W_OK) != 0) {         // Verificar si se tienen permisos de escritura
            printf("No permission\n");       // Mostrar error si no hay permisos
            return 1;                        // Terminar con error
        }
        if (stat(fn, &statbuf1) != 0) {      // Obtener información del archivo (antes de abrir)
            printf("stat error\n");          // Mostrar error si stat falla
            return 1;                        // Terminar con error
        }
        for (i = 0; i < DELAY; i++) { int a = i*2; }  // Simular retardo (ventana TOCTOU)
        fp = fopen(fn, "a+");                // Abrir archivo en modo lectura/escritura
        if (!fp) {                           // Verificar si fopen falló
            printf("fopen error\n");         // Mostrar error si no se pudo abrir
            return 1;                        // Terminar con error
        }
        if (fstat(fileno(fp), &statbuf2) != 0) {  // Obtener info del archivo ya abierto
            printf("fstat error\n");              // Mostrar error si falla fstat
            fclose(fp);                           // Cerrar archivo si se abrió
            return 1;                             // Terminar con error
        }
        if (statbuf1.st_ino != statbuf2.st_ino) { // Comparar inodos antes y después de abrir
            printf("Race condition detected! Aborting.\n");  // Detectar TOCTOU
            fclose(fp);                          // Cerrar archivo
            success = 0;                         // Marcar que no se debe escribir
            break;                               // Salir del ciclo
        }
        fclose(fp);                              // Cerrar archivo si todo fue bien
    }

    if (success) {                                // Si no se detectó condición de carrera
        fp = fopen(fn, "a+");                     // Reabrir archivo para escribir
        fwrite("\n", sizeof(char), 1, fp);        // Escribir nueva línea
        fwrite(buffer, sizeof(char), strlen(buffer), fp);  // Escribir entrada del usuario
        fclose(fp);                               // Cerrar archivo
    }
}
