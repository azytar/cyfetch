# cyfetch – Fast System Info in Pure C (Beta)

**cyfetch** es un *fetcher de sistema* ultra rápido, ligero y completamente escrito en **C puro**, diseñado para mostrar tu información del sistema al estilo *Neofetch/Fastfetch* pero **sin depender de herramientas externas**.  

> ⚠️ **Beta:** Actualmente solo probado en **Arch Linux** y derivados. Puede no funcionar correctamente en otras distribuciones.

## 🔹 Características principales

- Detecta **CPU, RAM, Kernel, Distro, Uptime** y más directamente desde `/proc` y `sysfs`.  
- Detecta tu **GPU integrada o dedicada**   
- Mantiene un **ASCII artístico + información alineada a la derecha**.  
- Pequeño y portable: solo necesitas **GCC y Arch Linux**.

## 🔹 Por qué cyfetch
 
- Ideal para **sistemas ligeros, minimalistas o integrados**.  

## 🔹 Instalación y uso

```bash
git clone https://github.com/azytar/cyfetch.git
cd cyfetch
gcc cyfetch.c -o cyfetch
./cyfetch
sudo cp cyfetch /usr/bin/cyfetch
