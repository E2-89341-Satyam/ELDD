#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/ioctl.h>
#include"pchar_ioctl.h"


int main(int argc,char *argv[])
{
    int ret,fd;
    if(argc != 3)
    {
        printf("syntax:%s <devicefile> <command>\n",argv[0]);
        return 1;
    }
    fd = open(argv[1],O_RDWR);
    if(fd < 0)
    {
        perror("open() failed");
        _exit(1);
    }
    //if command is clear,send FIFO_CLEAR cmd to device using ioctl()
    if(strcmp(argv[2],"clear") == 0){
        ioctl(fd,FIFO_CLEAR);
        printf("fifo clear command sent\n");
    }
    //if command is info,send FIFO_GET_INFO cmd to device using ioctl()
    if(strcmp(argv[2],"info")==0){
        fifo_info_t info;
        ioctl(fd,FIFO_GET_INFO,&info);
        printf("%s: info size=%d, len = %d, avail = %d\n",argv[1],info.size,info.len,info.avail);

    }
       if(strcmp(argv[2],"resize")==0){
        //fifo_info_t info;
        ioctl(fd,FIFO_RESIZE,64);
        //printf("%s: info size=%d, len = %d, avail = %d\n",argv[1],info.size,info.len,info.avail);

    }

    //if invalid command,show error

    else
    printf("invalid command : %s\n",argv[2]);
    close(fd);
    return 0;
}