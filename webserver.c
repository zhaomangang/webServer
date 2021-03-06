#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int createlistenfd()
{
    //创建一个监听套接字
    int fd = socket(AF_INET,SOCK_STREAM,0); //创建套接字
    
    int n = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&n,4);    //地址复用
    
    struct sockaddr_in sin; //声明地址结构体
    bzero(&sin,sizeof(sin));    //清空
    sin.sin_family = AF_INET;
    sin.sin_port = htons(8888);   //小端转大段，设置端口号
    sin.sin_addr.s_addr = INADDR_ANY;    //任意地址

    //绑定
    int r = bind(fd,(struct sockaddr *)&sin,sizeof(sin));
    if(r==-1)
    {
        //绑定失败
        puts("bind shibai");
        perror("bind");
        exit(-1);   //直接退出整个程序
    }

    //开始监听
    r = listen(fd,10);
    if(r==-1)
    {
        perror("listen");
        exit(-1);   //直接退出
    }
    return fd;
}

void work(int fd,char *s)
{
    //第一步，分析得到客户端所需的文件名
    char filename[100] = {0};
    sscanf(s,"GET /%s",filename);
    printf("请求里的文件名是%s\n",filename);
    
    //解析mime类型,通过后缀判断
    char *mime;
    if(strstr(filename,".html"))
        mime = "text/html";
    else if(strstr(filename,".jpg"))
        mime = "image/jpeg";
    
    //构建响应头，发给客户端
    char response[100] = {0};
    sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type:%s\r\n\r\n",mime);
    write(fd,response,strlen(response));

    //发送具体文件
    int filefd = open(filename,O_RDONLY);   //只读方式打开
    bzero(response,sizeof(response));   //清空缓冲区
    int rlen = 0;
    while((rlen = read(filefd,response,100))>0)
    {
        //读到东西
        write(fd,response,rlen);
    }



}



int main()
{
    while(1)
    {
        int sockfd = createlistenfd();    //创建监听套接字
        int fd = accept(sockfd,NULL,NULL);    //获取浏览器连接的fd
        char buffer[1024]={0}; //缓冲区
        int rlen = read(fd,buffer,sizeof(buffer));  //读取浏览器发送的东西
        printf("客户端发来的请求是：%s\n",buffer);
        work(fd,buffer);
    
        close(sockfd);  //关闭套接字
    }
}










