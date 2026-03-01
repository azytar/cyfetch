# cyfetch – Fast System Info in Pure C (Beta)

**cyfetch** es un *fetcher de sistema* ultra rápido, ligero y completamente escrito en **C puro**, diseñado para mostrar tu información del sistema al estilo *Neofetch/Fastfetch* pero **sin depender de herramientas externas**.  

> ⚠️ **Beta:** Actualmente solo probado en **Arch Linux** y derivados. Puede no funcionar correctamente en otras distribuciones.

## 🔹 Características principales

- Detecta **CPU, RAM, Kernel, Distro, Uptime** y más directamente desde `/proc` y `sysfs`.  
- Detecta tu **GPU integrada o dedicada** usando solo **vendor/device IDs**, incluyendo AMD Vega, Intel y NVIDIA.  
- Mantiene un **ASCII artístico + información alineada a la derecha**, personalizable.  
- Todo es **ultra rápido** (~4–6 ms), sin forks ni ejecución de programas externos.  
- Pequeño y portable: solo necesitas **GCC y Arch Linux**.

## 🔹 Por qué cyfetch

- Evita el overhead de Python/Bash (*Neofetch*) o forks de procesos (*lspci, dmidecode*).  
- Ideal para **sistemas ligeros, minimalistas o integrados**.  
- Fácil de ampliar: agrega nuevos **IDs de GPU**, soporte para otros datos del sistema o personaliza el ASCII.

## 🔹 Instalación y uso

```bash
gcc cyfetch.c -o cyfetch
./cyfetch
sudo cp cyfetch /usr/bin/cyfetch
