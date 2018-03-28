#include "http.h"
#include "signal.h"
#include "threadpool.h"
#include <sys/epoll.h>

// #define _RELEASE   // 如果是正式版release,请把__RELEASE f反注释掉
#define RUN_PROC      // 如果是线程版,RUN_PROC f反注释掉

// 定义线程的一些debug打印设置,release 版本不需要
#ifndef  _RELEASE
# define dbg_printf(...)  printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

void *thread_doit( void* vargp); // 子线程处理的函数

int main(int argc, char **argv) {
  // 检测输入参数的正确性
#ifdef _RELEASE
    if (argc != 2){
        fprintf(stderr, "USAGE: %s <PORT> \n", argv[0]);
        exit(-1);
    }
  char *port = argv[1];
#else
  char* port = "8888";  // release输入 argv[1];
#endif // _RELEAS
    printf("FSeed  runs in %s \n",port);

    int listenfd; // 监听fd
    int connfd; // 连接fd
    struct sockaddr_storage clientaddr;
    socklen_t clientlen;
    pthread_t tid;        // 线程处理
    char hostname[MAXLINE], clientport[MAXLINE];   // DNS 需要的功能

    // 信号处理，忽略SIGPIPE 信号
    Signal(SIGPIPE, SIG_IGN);

    // fd listenfd为打开连接的fd
    listenfd = Open_listenfd(port);

    // 主循环处理到来的fd
    while (1)
    {
        //如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
        clientlen = sizeof(clientaddr);
        printf("listenfd = %d \n",listenfd);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        dbg_printf("connfd = %d\n",connfd );
        // DNS功能
        getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, clientport, MAXLINE,0);
        printf("Accepted connection from (%s, %s)\n", hostname, clientport);
#ifdef RUN_PROC
        // 进程版本
        handle_http(connfd);
        Close(connfd);
        // free(connfd);
#else
        // 线程版本
        Pthread_create(&tid,NULL,thread_doit,&connfd);
#endif // RUN_PROC
    }
  return 0;
}

void *thread_doit( void* vargp) // Thread rountine
{
  dbg_printf("New process: PID: %d, TID: %lu\n", getpid(), pthread_self());
  // threadpool * conn = (threadpool *) vargp;
  int connfd = * ((int *) vargp);
  pthread_detach(pthread_self());
  dbg_printf("THE process: PID: %d has been detached !!!!!!!! \n", getpid());
  handle_http(connfd);
  Close(connfd);
  // exit(500);               // gcov 需要退出才可以测试代码覆盖率？？？
  return ;
}
