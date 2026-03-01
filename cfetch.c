#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <dirent.h>

#define BUF 4096
static char buffer[BUF];

// ------------------- utilidades -------------------
void trim_newline(char *s) {
    char *p = strchr(s, '\n');
    if (p) *p = 0;
}

int read_file(const char *path, char *out, int size) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    int n = read(fd, out, size - 1);
    close(fd);
    if (n > 0) out[n] = 0;
    return n;
}

// ------------------- obtener info -------------------
void get_distro(char *out) {
    if (read_file("/etc/os-release", buffer, BUF) < 0) { strcpy(out, "Linux"); return; }
    char *id = strstr(buffer, "ID=");
    if (!id) { strcpy(out, "Linux"); return; }
    id += 3;
    char *end = strchr(id, '\n');
    if (end) *end = 0;
    strcpy(out, id);
}

void get_cpu(char *out) {
    if (read_file("/proc/cpuinfo", buffer, BUF) < 0) return;
    char *model = strstr(buffer, "model name");
    if (!model) model = strstr(buffer, "Processor"); // fallback
    if (!model) return;
    model = strchr(model, ':');
    if (!model) return;
    model += 2;
    trim_newline(model);
    strcpy(out, model);
}

void get_ram(char *out) {
    if (read_file("/proc/meminfo", buffer, BUF) < 0) return;
    long total=0, avail=0;
    char *t = strstr(buffer, "MemTotal:");
    char *a = strstr(buffer, "MemAvailable:");
    if (t) sscanf(t, "MemTotal: %ld", &total);
    if (a) sscanf(a, "MemAvailable: %ld", &avail);
    snprintf(out, 128, "%.2f GB / %.2f GB", (total-avail)/1024.0/1024.0, total/1024.0/1024.0);
}

void get_uptime(char *out) {
    if (read_file("/proc/uptime", buffer, BUF) < 0) return;
    double up = atof(buffer);
    int h = up/3600;
    int m = ((int)up %3600)/60;
    snprintf(out, 64, "%dh %dm", h, m);
}

// ------------------- GPU con mapeo de IDs -------------------
void get_gpu(char *out) {
    DIR *dp = opendir("/sys/bus/pci/devices/");
    if (!dp) { strcpy(out,"Unknown GPU"); return; }

    struct dirent *entry;
    while ((entry = readdir(dp))) {
        if (entry->d_name[0]=='.') continue;

        char path[BUF], class_buf[16], vendor_buf[16], device_buf[16];
        FILE *f;

        // Leer clase
        snprintf(path, sizeof(path), "/sys/bus/pci/devices/%s/class", entry->d_name);
        f = fopen(path,"r"); if(!f) continue;
        if(!fgets(class_buf,sizeof(class_buf),f)){ fclose(f); continue; }
        fclose(f);

        unsigned int class_code; sscanf(class_buf,"%x",&class_code);
        if((class_code>>16)!=0x03) continue; // VGA o 3D

        // Leer vendor y device
        snprintf(path,sizeof(path),"/sys/bus/pci/devices/%s/vendor", entry->d_name);
        if(read_file(path,vendor_buf,sizeof(vendor_buf))<0) continue;
        trim_newline(vendor_buf);
        snprintf(path,sizeof(path),"/sys/bus/pci/devices/%s/device", entry->d_name);
        if(read_file(path,device_buf,sizeof(device_buf))<0) continue;
        trim_newline(device_buf);

        unsigned int v=0,d=0;
        sscanf(vendor_buf,"0x%x",&v);
        sscanf(device_buf,"0x%x",&d);

        // Tabla mínima de GPUs
        if(v==0x1002 && d==0x15d8) { // AMD Vega 3
            snprintf(out,256,"%s : VGA compatible controller : Advanced Micro Devices, Inc. Picasso/Raven 2", entry->d_name);
            closedir(dp);
            return;
        }
        if(v==0x8086) { snprintf(out,256,"%s : VGA compatible controller : Intel Integrated Graphics", entry->d_name); closedir(dp); return; }
        if(v==0x10de) { snprintf(out,256,"%s : VGA compatible controller : NVIDIA GPU", entry->d_name); closedir(dp); return; }

        // si no coincide, seguir buscando
    }
    closedir(dp);
    strcpy(out,"Unknown GPU");
}

// ------------------- imprimir ASCII + info -------------------
void print_ascii_info(const char *distro, const char *user, const char *host,
                      const char *kernel, const char *uptime, const char *cpu,
                      const char *ram, const char *gpu) {
    const char *ascii[] = {
        "       /\\",
        "      /  \\",
        "     /\\   \\",
        "    /      \\",
        "   /   ,,   \\",
        "  /   |  |  -\\",
        " /_-''    ''-_\\"
    };
    int lines = sizeof(ascii)/sizeof(ascii[0]);
    for(int i=0;i<lines;i++){
        printf("\033[1;36m%s\033[0m",ascii[i]); // cyan
        switch(i){
            case 0: printf("   \033[1;33m%s@%s\033[0m",user,host); break;
            case 1: printf("   \033[1;32mOS:\033[0m %s",distro); break;
            case 2: printf("   \033[1;34mKernel:\033[0m %s",kernel); break;
            case 3: printf("   \033[1;35mUptime:\033[0m %s",uptime); break;
            case 4: printf("   \033[1;31mCPU:\033[0m %s",cpu); break;
            case 5: printf("   \033[1;36mRAM:\033[0m %s",ram); break;
            case 6: printf("   \033[1;33mGPU:\033[0m %s",gpu); break;
        }
        printf("\n");
    }
}

// ------------------- main -------------------
int main(){
    char distro[64], cpu[256], ram[128], gpu[256], uptime[64], kernel[128], host[64];

    gethostname(host,sizeof(host));
    char *user = getenv("USER");

    struct utsname uts;
    uname(&uts);
    snprintf(kernel,sizeof(kernel),"%s %s",uts.sysname,uts.release);

    get_distro(distro);
    get_cpu(cpu);
    get_ram(ram);
    get_gpu(gpu);
    get_uptime(uptime);

    print_ascii_info(distro,user,host,kernel,uptime,cpu,ram,gpu);

    return 0;
}

