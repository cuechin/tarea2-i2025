#!/bin/bash

echo "🔍 Verificando condiciones del entorno para el ataque..."

# 1. Verifica bit SUID en vulp
if [[ -u ./vulp ]]; then
    echo "vulp tiene el bit SUID activado."
else
    echo "vulp NO tiene el bit SUID. Usa: sudo chown root:root vulp && sudo chmod 4755 vulp"
fi

# 2. Verifica propietario y permisos
vulp_info=$(ls -l ./vulp)
echo "🔎 Permisos actuales de vulp: $vulp_info"

# 3. Verifica si AppArmor tiene perfiles activos para vulp
if command -v aa-status &> /dev/null; then
    if sudo aa-status | grep -q vulp; then
        echo "AppArmor está aplicando restricciones a vulp."
    else
        echo "AppArmor NO está restringiendo a vulp."
    fi
else
    echo "AppArmor no está instalado (o el comando aa-status no está disponible)."
fi

# 4. Verifica si el módulo AppArmor está cargado
if lsmod | grep -q apparmor; then
    echo "El módulo AppArmor está cargado."
else
    echo "AppArmor está completamente descargado."
fi

# 5. Verifica si symlink protections están desactivadas
symlinks=$(cat /proc/sys/fs/protected_symlinks)
hardlinks=$(cat /proc/sys/fs/protected_hardlinks)

if [[ "$symlinks" == "0" ]]; then
    echo "fs.protected_symlinks está desactivado."
else
    echo "fs.protected_symlinks está ACTIVADO. Usa: sudo sysctl -w fs.protected_symlinks=0"
fi

if [[ "$hardlinks" == "0" ]]; then
    echo "fs.protected_hardlinks está desactivado."
else
    echo "fs.protected_hardlinks está ACTIVADO. Usa: sudo sysctl -w fs.protected_hardlinks=0"
fi

# 6. Verifica si /tmp/XYZ existe y es symlink
if [[ -L /tmp/XYZ ]]; then
    echo "/tmp/XYZ existe y es un symlink → $(readlink /tmp/XYZ)"
else
    echo "/tmp/XYZ no es un symlink válido (¿está corriendo el attack?)"
fi

# 7. Verifica si el archivo se está ejecutando desde una ruta con nosuid
mount_info=$(findmnt -no OPTIONS "$(pwd)")
if echo "$mount_info" | grep -q nosuid; then
    echo "Esta carpeta está montada con 'nosuid' → el bit SUID será ignorado"
else
    echo "La carpeta NO tiene la opción 'nosuid'"
fi

echo "Verificación completada."