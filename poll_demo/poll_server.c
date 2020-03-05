#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <string.h>


int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("eg: ./a.out port\n");
    exit(1);
  }
  struct sockaddr_in serv_addr;
  socklen_t serv_len = sizeof(serv_addr);
  int port = atoi(argv[1]);

  // 创建套接字
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  // 初始化服务器的结构体
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  bind(lfd, (struct sockaddr*)&serv_addr, serv_len);

  listen(lfd, 36);
  printf("Start accept...\n");

  struct pollfd allfd[1024];
  int max_index = 0;

  // 初始化
  for(int i = 0; i < 1024; ++i) {
    allfd[i].fd = -1;   // 设置为-1代表这个fd可用
    allfd[i].events = POLLIN; // 监听读
  }

  allfd[0].fd = lfd;

  while(1) {
    int i = 0;
    int ret = poll(allfd, max_index + 1, -1);   // -1代表阻塞
    if(ret == -1) {
      perror("poll error");
      exit(1);
    }
    // 监听描述符是否准备好
    if(allfd[0].revents & POLLIN) {
      // 说明数据在内核准备好了
      int cfd = accept(lfd, NULL, NULL);
      printf("===============\n");

      // 把cfd添加到poll数组
      for(i = 0; i < 1024; ++i) {
        if(allfd[i].fd == -1) {
          allfd[i].fd = cfd;
          break;
        }
      }

      max_index = max_index < i ? i : max_index;
    }

    // 还要遍历数组，消耗太多资源
    for(int i = 1; i < max_index + 1; ++i) {
      int fd = allfd[i].fd;
      if(fd == -1) {
        continue;
      }
      if(allfd[i].revents & POLLIN) {
        char buf[1024] = {0};
        int len = recv(fd, buf, sizeof(buf), 0);
        if(len == -1) {
          perror("recv err");
          exit(1);
        }
        else if(len == 0) {
          printf("客户端断开连接\n");
          allfd[i].fd = -1;
          close(fd);
        }
        else {
          printf("recv buf = %s", buf);
          printf("长度为%d, strlen(buf) + 1 = %d\n", len, strlen(buf) + 1);
          send(fd, buf, strlen(buf) + 1, 0);
        }
      }
    }
  }


  return 0;
}