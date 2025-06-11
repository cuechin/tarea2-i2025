# Tarea 2: Vulnerabilidad de condición de carrera

**Maestría en Ciberseguridad**

**Principios de Seguridad en Sistemas Operativos**

**Profesor.**
* Kevin Moraga

**Integrantes**
* Roberto Cordoba - 2025800352
* Daniel Araya - 2020207809
* Andrés Mora - 2013241401

---
## 1. Introducción

En los sistemas operativos modernos, la seguridad de los programas con privilegios elevados es fundamental. Una de las amenazas más críticas en este contexto es la **condición de carrera** (*race condition*), la cual ocurre cuando múltiples procesos acceden concurrentemente a recursos compartidos, generando resultados impredecibles según el orden de ejecución [1].

Este problema se vuelve especialmente grave cuando afecta a programas ejecutados con privilegios de root. Tal es el caso del programa analizado en esta tarea, el cual, aunque verifica permisos con `access()` antes de abrir el archivo con `fopen()`, deja una ventana temporal vulnerable. Un atacante puede aprovechar esta brecha para sustituir el archivo por un enlace simbólico que apunte, por ejemplo, a `/etc/passwd` o `/etc/shadow`, logrando modificar archivos sensibles del sistema [2][3].

El objetivo de esta tarea es analizar, explotar y mitigar esta vulnerabilidad mediante tres enfoques:
1. Explotación directa de la condición de carrera.
2. Protección por verificación repetida
3. Aplicación del principio de privilegio mínimo.

Así, se pretende comprender el impacto real de esta falla y explorar estrategias efectivas para reducir su riesgo en la práctica [4][5].
---
## 2. Instrucciones para ejecutar el programa
### Configuración del entorno

El repositorio del proyecto puede ser clonado desde el siguiente enlace: [Condición de carrera](https://github.com/cuechin/tarea2-i2025.git)

Para el entorno del escenario se utilizan las siguientes sistemas operativos en la máquina virtual VMware Workstation Pro 17, aquí los siguientes enlaces para descargar el .iso:
* #### Target OS
  * [Debian 12.11.0](https://cdimage.debian.org/debian-cd/current/amd64/iso-cd/debian-12.11.0-amd64-netinst.iso)

### Tarea 1: Explotar las vulnerabilidades de la condición de carrera

Para llevar a cabo la explotación de la vulnerabilidad de condición de carrera descrita en esta tarea, se preparó un entorno controlado en Linux. A continuación se describen los pasos ejecutados:

#### Archivos necesarios

Ubicar en el mismo directorio los siguientes archivos:

- `vulp.c` – Programa vulnerable.
- `race_attack.c` – Código en C que crea/elimina symlinks rápidamente.
- `run_attack.sh` – Script bash que automatiza los intentos de explotación.
- `input.txt` – Entrada a redirigir al programa vulnerable.

#### Compilación
```bash
gcc -o attack race_attack.c
gcc -o vulp vulp.c
chmod +x run_attack.sh
```
#### Asignar privilegios (activar bit SUID)
```
sudo chown root:root vulp
sudo chmod 4755 vulp
```

#### Configuración del entorno
Para permitir la creación de enlaces simbólicos peligrosos:
```
sudo sysctl -w fs.protected_symlinks=0
sudo sysctl -w fs.protected_hardlinks=0
```

#### Desactivar AppArmor:
```
sudo systemctl stop apparmor
sudo service apparmor status   # Confirmar que diga "inactive (dead)"
```

#### **Ejecución del ataque**

**Terminal 1:**
```
./attack
```

**Terminal 2:**
```
./run_attack.sh
```

Este script automatiza múltiples ejecuciones de `vulp < input.txt` para intentar interceptar la ventana entre `access()` y `fopen()`.

**Consideraciones clave**
* El archivo `/tmp/XYZ` debe ser un **symlink válido** a `/etc/passwd `o `/etc/shadow`.
* Si durante el ataque el symlink se rompe y `fopen()` se ejecuta, se creará un archivo regular. Cuando esto sucede, el ataque queda neutralizado hasta que se elimine `/tmp/XYZ`.
* Se recomienda agregar al inicio de run_attack.sh una verificación como:
```
[ -e /tmp/XYZ ] && [ ! -L /tmp/XYZ ] && sudo rm /tmp/XYZ
```

**Script de verificación del entorno**
Se utilizó un script auxiliar que valida:
* Bit SUID en vulp
* Permisos del archivo
* Estado de AppArmor
* Desactivación de protecciones de symlinks
* Validez de /tmp/XYZ como symlink
* Que el sistema de archivos permita suid

Este script ayuda a asegurar que el entorno está correctamente preparado antes de lanzar los ataques.

**Resultado esperado**
Cuando el ataque es exitoso, `/etc/passwd` (o `/etc/shadow`) es modificado con el contenido del archivo `input.txt.` En caso contrario, si `/tmp/XYZ` se convierte en archivo regular (no symlink), se debe eliminar manualmente para reiniciar el intento.

![](Tarea%202%20Vulnerabilidad%20de%20condicio%CC%81n%20de%20carrera/image.png)

### Tarea 2: Mecanismo de protección A - Repetición

La ejecución de esta tarea es similar a la **Tarea 1**, pero utilizando una versión modificada del programa vulnerable (`vulp_protA.c`).

El programa vulnerable original suele ser explotable porque, entre el momento en que
verifica los permisos de un archivo (por ejemplo, usando `access()`) y el momento en que lo
abre para escribir (`fopen()`), un atacante puede cambiar el archivo (por ejemplo,
reemplazarlo por un symlink a un archivo root-only). Así, el proceso termina escribiendo en
un archivo que el usuario normal no debería poder modificar.

En vulp_protA.c, la protección clave es la comparación de i-nodos antes y después de abrir
el archivo:
1. Verifica permisos con `access(fn, W_OK)`:
Comprueba si el usuario tiene permisos de escritura sobre `/tmp/XYZ`.
2. Obtiene el i-nodo del archivo con `stat(fn, &statbuf1)`:
Guarda la información del archivo, incluyendo el número de i-nodo, antes de la posible
carrera.
3. Introduce un retardo (`for (i = 0; i < DELAY; i++)`):
Esto simula el tiempo en el que un atacante podría intentar cambiar el archivo.
4. Abre el archivo con `fopen(fn,"a+")`:
Aquí es donde, en un programa vulnerable, el atacante podría haber cambiado el archivo
por un symlink.
5. Obtiene el i-nodo del archivo abierto con `fstat(fileno(fp), &statbuf2)`:
Ahora compara el i-nodo del archivo abierto con el i-nodo que obtuvo antes.
6. Compara los i-nodos (`statbuf1.st_ino != statbuf2.stino`):
Si los i-nodos son diferentes, significa que el archivo fue cambiado entre la verificación y el
uso (es decir, hubo una condición de carrera). En ese caso, el programa detecta la carrera y aborta la operación sin escribir nada.

**¿Por qué esto evita el ataque?**
El atacante puede intentar reemplazar `/tmp/XYZ` por un symlink a un archivo root-only justo después de la verificación de permisos, pero antes de que el archivo sea abierto. Sin embargo, si el archivo cambia, su número de i-nodo también cambia (o el descriptor apunta a un archivo diferente). La comparación de i-nodos detecta este cambio y evita que el programa escriba en el archivo, bloqueando así la explotación de la condición de carrera.

**En resumen**
La protección en `vulp_protA.c` consiste en comparar el i-nodo del archivo antes y después de abrirlo. Si detecta que el archivo cambió (lo que ocurre en un ataque de TOCTOU), aborta la operación y no escribe nada, evitando así la explotación.

### Tarea 3: Mecanismo de Protección B - Principio de Privilegio Mínimo

La ejecución de esta tarea es similar a la **Tarea 1**, pero utilizando una versión modificada del programa vulnerable (`vulp_protB.c`).

El ataque clásico de TOCTOU ocurre cuando un programa SUID root verifica permisos o existencia de un archivo como root, pero luego, antes de abrirlo, un atacante cambia el archivo (por ejemplo, reemplazándolo por un symlink a un archivo root-only). Así, el programa termina abriendo y escribiendo en un archivo que el usuario normal no debería poder modificar.

En vulp_protB.c, la protección se basa en el manejo cuidadoso de los privilegios con ` seteuid()` :
1. Al inicio, el programa baja sus privilegios a los del usuario real con ` seteuid(getuid())` : Esto significa que, aunque el binario tenga el bit SUID root, el proceso ya no tiene privilegios de root para las operaciones siguientes.
2. Verifica si el usuario tiene permisos de escritura sobre el archivo con ` access(fn, W _ OK)` : Como el proceso está corriendo con los privilegios del usuario real, esta verificación refleja exactamente lo que el usuario podría hacer por sí mismo.
3. Si el usuario tiene permisos, el programa recupera privilegios de root con ` seteuid(0)` : Solo en este momento, y solo si el usuario ya tenía permisos, el proceso vuelve a ser root.
4. Abre el archivo y escribe en él: Ahora, el proceso puede abrir y escribir en el archivo, pero solo porque el usuario ya tenía permisos de escritura. 
5. Después de escribir, el programa vuelve a bajar privilegios con `seteuid(getuid())` Esto minimiza el tiempo en que el proceso corre como root. 

**¿Por qué esto evita el ataque?**
El atacante no puede aprovechar una ventana de tiempo entre la verificación y el uso, porque:
- La verificación de permisos y la apertura del archivo se hacen con los privilegios del usuario real.
- Solo si el usuario ya tenía permisos, el proceso se eleva a root para realizar la operación.
- No hay oportunidad para el atacante de cambiar el archivo entre la verificación y el uso, porque no hay cambio de contexto de privilegios en ese intervalo. En otras palabras, el programa nunca realiza una operación privilegiada (como abrir un archivo root-only) basándose en una verificación hecha con privilegios diferentes. Esto elimina la clásica ventana de TOCTOU y bloquea la explotación.

**En resumen**
`vulp_protB.c` evita la condición de carrera asegurando que la verificación de permisos y la apertura del archivo se hagan siempre con los mismos privilegios (los del usuario real), y solo eleva privilegios después de comprobar que el usuario ya tenía acceso legítimo. Así, el atacante no puede engañar al programa para que escriba en archivos root-only

## 3. Descripción del Ataque
La vulnerabilidad explotada en este trabajo es una condición de carrera del tipo **Time-of-Check to Time-of-Use (TOCTOU)**. En este escenario, el programa verifica inicialmente si tiene permisos sobre un recurso (usando access()), pero luego existe una ventana temporal en la cual el recurso podría cambiar antes de que se use efectivamente (con fopen()). Un atacante aprovecha esta ventana crítica para alterar el recurso apuntado mediante un enlace simbólico.
En nuestro ataque específico:
* Utilizamos el programa atacante attack, que constantemente crea y elimina un enlace simbólico en `/tmp/XYZ` apuntando a un archivo crítico del sistema (como `/etc/passwd o /etc/shadow`).
* En paralelo, ejecutamos repetidamente el programa vulnerable (vulp) con permisos elevados, aprovechando que este realiza una comprobación (access()) seguida de un retardo simulado antes de abrir el archivo (fopen()).
* El objetivo es sincronizar estos dos procesos de modo que la creación del enlace simbólico malicioso coincida exactamente con el momento crítico entre la verificación y la apertura del archivo por parte de vulp.

Esta técnica permite potencialmente que un usuario sin privilegios sobrescriba archivos críticos del sistema, facilitando la escalada de privilegios o la creación de usuarios arbitrarios con privilegios de administrador (root) [2][3].
Existen variantes de este ataque que implican diferentes mecanismos de temporización y técnicas de sincronización, como ajustar artificialmente el retardo (DELAY) en el programa vulnerable, aumentar los intentos automatizados, o emplear métodos avanzados de sincronización (eventos, señales del sistema operativo, etc.) [4][5].

---
## 4. Documentación del ataque
A continuación, se describe brevemente el funcionamiento general del ataque ejecutado en esta tarea:
1. Se inicia el programa `attack`, generando continuamente enlaces simbólicos en `/tmp/XYZ`.
2. Simultáneamente, se ejecuta repetidamente el script `run_attack.sh`, que invoca al programa vulnerable `vulp` con una entrada controlada mediante redirección desde input.txt.
3. El programa vulnerable verifica con `access()` los permisos sobre `/tmp/XYZ`. Durante el retardo simulado posterior a la comprobación, el programa `attack` modifica continuamente el enlace simbólico, intentando hacer que el enlace apunte al archivo objetivo (como `/etc/passwd`) exactamente en el momento en que `fopen()` se ejecute.
4. Cuando el ataque es exitoso, `vulp` abre y modifica el archivo crítico, utilizando los privilegios elevados que posee.
5. El ataque requiere múltiples intentos debido a su naturaleza probabilística. Es habitual que durante el proceso el archivo `/tmp/XYZ` deje de ser un enlace simbólico, en cuyo caso se deben reiniciar los intentos tras eliminar este archivo.

---
## 5. Autoevaluación
**Estado final del programa:**
* El programa `vulp.c` fue compilado exitosamente, funcionando correctamente en términos de reproducir la condición de carrera.
* El ataque fue ejecutado con éxito varias veces, demostrando claramente la vulnerabilidad presente.

⠀**Problemas encontrados:**
* La naturaleza probabilística del ataque requirió numerosos intentos antes de lograr el éxito.
* En múltiples ocasiones, `/tmp/XYZ` dejó de ser un enlace simbólico, transformándose en archivo regular debido a la apertura simultánea, requiriendo intervención manual o scripts auxiliares para mantener la automatización.

⠀**Limitaciones adicionales:**
* El entorno requiere configuración específica (desactivar protecciones de enlaces simbólicos y AppArmor), limitando su aplicabilidad inmediata a entornos altamente protegidos.

⠀**Autoevaluación según rúbrica:**
* **Tarea 1 (25%)**: Completada exitosamente (25%)
* **Tarea 2 (25%)**: Aplicación del mecanismo de repetición claramente documentado y probado (25%)
* **Tarea 3 (25%)**: Implementación efectiva del principio de privilegio mínimo (seteuid) documentado y probado (25%)
* **Documentación del Ataque (25%)**: Completa, detallada y organizada, con referencias claras (25%)

⠀**Calificación autoasignada: 100%**

---
## 6. Lecciones Aprendidas
* **Importancia de sincronización**: Se aprendió que las vulnerabilidades por condición de carrera son difíciles de explotar consistentemente debido a la necesidad de sincronización precisa entre procesos paralelos.
* **Configuración del entorno**: Fue esencial aprender cómo desactivar mecanismos de protección del sistema operativo (AppArmor, symlinks protegidos) para simular escenarios reales de explotación.
* **Mitigación efectiva**: Se comprendió la eficacia del principio de privilegio mínimo y verificaciones repetidas para reducir la probabilidad de explotación exitosa.
* **Automatización y persistencia**: Este tipo de vulnerabilidades requieren automatización y persistencia debido a su carácter probabilístico, resaltando la importancia de herramientas y scripts de apoyo.
* **Buenas prácticas**: Se confirmó la importancia de buenas prácticas de programación, como evitar la separación temporal entre chequeo y uso de recursos críticos (TOCTOU).


---
## 7. Video
* [Race Condition Attack - TEC - Sistemas Operativos](https://youtu.be/QLINzdeqCyg)
## 8. Bibliografía
- [1] MITRE. *CWE-362: Race Condition*. Disponible en: [https://cwe.mitre.org/data/definitions/362.html](https://cwe.mitre.org/data/definitions/362.html)
- [2] MITRE. *CWE-367: Time-of-check Time-of-use (TOCTOU)*. Disponible en: [https://cwe.mitre.org/data/definitions/367.html](https://cwe.mitre.org/data/definitions/367.html)
- [3] Wikipedia. *Symlink Race*. Disponible en: [https://en.wikipedia.org/wiki/Symlink_race](https://en.wikipedia.org/wiki/Symlink_race)
- [4] Wikipedia. *Dirty COW*. Disponible en: [https://en.wikipedia.org/wiki/Dirty_COW](https://en.wikipedia.org/wiki/Dirty_COW)
- [5] SEI CERT. *FIO45-C: Avoid TOCTOU race conditions while accessing files*. Disponible en: [https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=87151941](https://wiki.sei.cmu.edu/confluence/pages/viewpage.action?pageId=87151941)

