/*
 * @Author: sanjayzhong
 * @Github: https://github.com/sanjayzzzhong
 * @Date: 2019-09-05 14:44:20
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[]){

    if(argc < 2){
        printf("eg: ./client port\n");
        exit(1);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));//清空结构体
    serv.sin_family = AF_INET;
    serv.sin_port = htons(atoi(argv[1]));
    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);
    connect(fd, (struct sockaddr *)&serv, sizeof(serv));

    //通信
    while(1){
        //发送消息
        char buf[1024];
        printf("请输入要发送的英文字符串:\n");
        fgets(buf, sizeof(buf), stdin);//从终端读入数据
        write(fd, buf, strlen(buf));

        //等待接收数据
        int len = read(fd, buf, sizeof(buf));
        if(len == -1){
            perror("read error");
            exit(1);
        }
        else if(len == 0){
            printf("服务器关闭连接\n");
            break;
        }
        else{
            printf("recv buf: %s\n", buf);
        }
    }
    close(fd);

    return 0;
}