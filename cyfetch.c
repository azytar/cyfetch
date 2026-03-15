#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <vulkan/vulkan.h>

#define BUF 4096
static char buffer[BUF];

static VkInstance instance = VK_NULL_HANDLE;

// ------------------- utilidades -------------------
void trim_newline(char *s){
    char *p=strchr(s,'\n');
    if(p)*p=0;
}

int read_file(const char *path,char *out,int size){
    int fd=open(path,O_RDONLY);
    if(fd<0)return -1;
    int n=read(fd,out,size-1);
    close(fd);
    if(n>0)out[n]=0;
    return n;
}

// ------------------- Vulkan GPU -------------------
int init_vulkan(){
    VkApplicationInfo appinfo={
        .sType=VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName="fetch",
        .applicationVersion=VK_MAKE_VERSION(1,0,0),
        .apiVersion=VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo createInfo={
        .sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo=&appinfo
    };

    return vkCreateInstance(&createInfo,NULL,&instance);
}

void get_gpu(char *out){
    uint32_t deviceCount=0;
    vkEnumeratePhysicalDevices(instance,&deviceCount,NULL);

    if(deviceCount==0){
        strcpy(out,"No GPU found");
        return;
    }

    VkPhysicalDevice devices[8];
    vkEnumeratePhysicalDevices(instance,&deviceCount,devices);

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(devices[0],&props);

    snprintf(out,256,"%s",props.deviceName);
}

// ------------------- obtener info -------------------
void get_distro(char *out){
    if(read_file("/etc/os-release",buffer,BUF)<0){strcpy(out,"Linux");return;}
    char *id=strstr(buffer,"ID=");
    if(!id){strcpy(out,"Linux");return;}
    id+=3;
    char *end=strchr(id,'\n');
    if(end)*end=0;
    strcpy(out,id);
}

void get_cpu(char *out){
    if(read_file("/proc/cpuinfo",buffer,BUF)<0)return;
    char *model=strstr(buffer,"model name");
    if(!model)model=strstr(buffer,"Processor");
    if(!model)return;
    model=strchr(model,':');
    if(!model)return;
    model+=2;
    trim_newline(model);
    strcpy(out,model);
}

void get_ram(char *out){
    if(read_file("/proc/meminfo",buffer,BUF)<0)return;
    long total=0,avail=0;
    char *t=strstr(buffer,"MemTotal:");
    char *a=strstr(buffer,"MemAvailable:");
    if(t)sscanf(t,"MemTotal: %ld",&total);
    if(a)sscanf(a,"MemAvailable: %ld",&avail);
    snprintf(out,128,"%.2f GB / %.2f GB",(total-avail)/1024.0/1024.0,total/1024.0/1024.0);
}

void get_uptime(char *out){
    if(read_file("/proc/uptime",buffer,BUF)<0)return;
    double up=atof(buffer);
    int h=up/3600;
    int m=((int)up%3600)/60;
    snprintf(out,64,"%dh %dm",h,m);
}

// ------------------- imprimir ASCII + info -------------------
void print_ascii_info(const char *distro,const char *user,const char *host,
                      const char *kernel,const char *uptime,const char *cpu,
                      const char *ram,const char *gpu){

    const char *ascii[]={
        "       /\\",
        "      /  \\",
        "     /\\   \\",
        "    /      \\",
        "   /   ,,   \\",
        "  /   |  |  -\\",
        " /_-''    ''-_\\"
    };

    int lines=sizeof(ascii)/sizeof(ascii[0]);

    for(int i=0;i<lines;i++){
        printf("\033[1;36m%s\033[0m",ascii[i]);

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

    char distro[64],cpu[256],ram[128],gpu[256],uptime[64],kernel[128],host[64];

    gethostname(host,sizeof(host));
    char *user=getenv("USER");

    struct utsname uts;
    uname(&uts);
    snprintf(kernel,sizeof(kernel),"%s %s",uts.sysname,uts.release);

    if(init_vulkan()!=VK_SUCCESS){
        strcpy(gpu,"Vulkan init failed");
    }else{
        get_gpu(gpu);
    }

    get_distro(distro);
    get_cpu(cpu);
    get_ram(ram);
    get_uptime(uptime);

    print_ascii_info(distro,user,host,kernel,uptime,cpu,ram,gpu);

    vkDestroyInstance(instance,NULL);

    return 0;
}

