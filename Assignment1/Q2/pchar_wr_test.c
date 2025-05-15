#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>


int main(int argc,char *argv[])
{
    int fd,ret;
    if(argc != 3)
    {
        printf("syntax:%s <devicefile> <string>");
        return 1;
    }
    fd = open(argv[1],O_WRONLY);
    if(fd<0){
        perror("open() failed\n");
        _exit(1);
    }

    ret =  write(fd, argv[2], strlen(argv[2]));
    printf("write() done: %d\n", ret);

    close(fd);
    return 0;
}