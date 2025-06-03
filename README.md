# tarea2-i2025
Tarea 2 Vulnerabilidad de Carrera

Pasos:

Tener listo los archivos run_attack.sh, vulp.c, race_attack.c e input.txt en el mismo directorio.

Compilar los archivos:

gcc -o attack race_attack.c
gcc -o vulp vulp.c

Volver ejecutable el sh si no lo es:

chmod +x run_attack.sh

Dar permisos a vulp

sudo chown root:root vulp
sudo chmod 4755 vulp

Quitar permisos:

sudo sysctl -w fs.protected_symlinks=0
sudo sysctl -w fs.protected_hardlinks=0

sudo systemctl stop apparmor

Se comprueba que apparmor está detenido con:

sudo service apparmor status

Debe decir "inactive (dead)".

Ejecutar el ataque:
Terminal 1:
./attack

./Terminal 2:
./run_attack.sh
