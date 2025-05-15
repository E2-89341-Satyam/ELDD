#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>


int main(int argc,char *argv[])
{
    int ret,fd;
    if(argc != 3){
        printf("syntax : %s <devicename> <string>\n",argv[0]);
        return 1;
    }
    fd = open(argv[1],O_WRONLY);
    if(fd < 0)
    {
        perror("open() failed\n");
        _exit(1);
    }

    ret = write(fd,argv[2],strlen(argv[2]));

    printf("write() done:%d\n",ret);
    sleep(10);
    close(fd);
    return 0;
}