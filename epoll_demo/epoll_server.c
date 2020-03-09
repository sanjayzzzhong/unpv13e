/*
 * @Author: sanjayzhong
 * @Github: https://github.com/sanjayzzzhong
 * @Date: 2019-09-09 16:05:22
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/epoll.h>



int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("eg: ./a.out port\n");
        exit(1);
    }
    struct sockaddr_in serv_addr;
    socklen_t serv_len = sizeof(serv_addr);
    int port = atoi(argv[1]);

    //创建套接字
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    //初始化服务器 的结构体
    memset(&serv_addr, 0, serv_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //监听本地所有IP
    serv_addr.sin_port = htons(port);

    //将套接字绑定到服务器上
    bind(lfd, (struct sockaddr *)&serv_addr, serv_len);

    //设置同时监听的最大连接数
    listen(lfd, 36);
    printf("Start accept......\n");


    struct sockaddr_in client_addr; //客户端地址
    socklen_t client_len = sizeof(client_addr);

    //创建epoll树根节点
    int epfd = epoll_create(3000);
    //存储发送变化的fd对应信息
    struct epoll_event all[3000];
    //初始化epoll树
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = lfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);

    while(1){
        //使用epoll通知内核fd 文件IO检测
        int ret = epoll_wait(epfd, all, sizeof(all) / sizeof(all[0]), -1);

        //遍历all数组中的前ret个元素
        for(int i = 0; i < ret; ++i){
            int fd = all[i].data.fd;
            //判断是否有新连接
            if(fd == lfd){
                //接受连接请求
                int cfd = accept(lfd, (struct sockaddr*)&client_addr, &client_len);
                if(cfd == -1){
                    perror("accept err");
                    exit(1);
                }
                //把cfd挂在树上
                struct epoll_event temp;
                temp.events = EPOLLIN;
                temp.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &temp);
                //打印客户端信息
                char ip[64] = {0};
                printf("Net client IP: %s, Port: %d\n\n",
                        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip, sizeof(ip)),
                        ntohs(client_addr.sin_port));
            }
            //已经连接的客户端发消息过来
            else{
                //处理已连接的客户端发送过来的数据,如果不是读的不处理
                if(!(all[i].events & EPOLLIN)){     //!和&优先级是一样的,加不加括号一样,从右到左
                    continue;
                }
                //读数据
                char buf[1024] = {0};
                int len = recv(fd, buf, sizeof(buf), 0);
                if(len == -1){
                    perror("recv err");
                    exit(1);
                }
                else if(len == 0){
                    printf("客户端主动断开连接...\n");
                    // close(fd); //这样子不行,如果关了fd,那么就不能从树上删掉了
                    //把fd从树上删除
                    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    if(ret == -1){
                        perror("epoll_ctl - del error");
                        exit(1); 
                    }
                    close(fd); //要先把fd从树上删除,才可以关掉fd,不然这个fd是无效的
                }
                else{
                    printf("recv buf: %s\n", buf);
                    write(fd, buf, len);
                }
            }
        }
    }
    return 0;
}

