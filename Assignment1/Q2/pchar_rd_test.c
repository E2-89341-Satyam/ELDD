#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>


int main(int argc,char *argv[])
{
    int ret,fd;
    char buf[512];
    if(argc != 2) {
        printf("syntax: %s <devfile>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1],O_RDONLY);
    if(fd < 0){
        perror("open() failed\n");
        _exit(1);
    }    

    printf("open() done %d\n",fd);

    memset(buf,0,sizeof(buf));
    ret = read(fd,buf,sizeof(buf));
    printf("read() done: %d - %s\n", ret, buf);
    close(fd);
    return 0;
}