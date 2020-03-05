#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>


int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("eg: ./a.out port\n");
    exit(1);
  }
  // 服务器地址结构
  struct sockaddr_in serv_addr;
  socklen_t serv_len = sizeof(serv_addr);
  int port = atoi(argv[1]);

  // 创建套接字
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  // 初始化服务器地址的结构体
  bzero(&serv_addr, serv_len);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  // 绑定套接字
  bind(lfd, (struct sockaddr*)&serv_addr, serv_len);
  // 设置同时坚挺最大连接数
  listen(lfd, 36);
  printf("start accept......\n");


  // 客户端地址
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);


  // 最大文件描述符
  int maxfd = lfd;
  // 创建文件描述符集合
  fd_set reads, temp;
  // init
  FD_ZERO(&reads);
  FD_SET(lfd, &reads);

  while(1) {
    // 委托内核做io检测
    temp = reads;
    int ret = select(maxfd + 1, &temp, NULL, NULL, NULL);
    if(ret == -1) {
      perror("select error");
      exit(1);
    }
    // 如果客户端发了连接
    if(FD_ISSET(lfd, &temp)) {
      // 接收连接请求，因为这个时候内核的IO已经准备好了
      int cfd = accept(lfd, (struct sockaddr*)&client_addr, &client_len);
      if(cfd == -1) {
        perror("accept error");
        exit(1);
      }
      printf("new client conncet succeed\n");
      // 将cfd加入待检测的读集合中
      FD_SET(cfd, &reads);
      // 更新最大文件描述符
      maxfd = maxfd < cfd ? cfd : maxfd;
    }
    // 已经连上的给我发数据
    for(int i = lfd + 1; i <= maxfd; ++i) {
      if(FD_ISSET(i, &temp)) {
        char buf[1024] = {0};
        int len = recv(i, buf, sizeof(buf), 0);
        if(len == -1) {
          perror("recv error");
          exit(1);
        }
        else if(len == 0) {
          printf("客户端已经断开连接\n");
          close(i);
          FD_CLR(i, &reads);
        }
        else {
          printf("recv buf : %s\n", buf);
          send(i, buf, strlen(buf) + 1, 0);
        }
      }
    }
  }

  return 0;
}
