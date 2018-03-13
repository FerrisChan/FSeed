#include "http.h"
#include "signal.h"
#include "threadpool.h"


//#define _RELEASE   // 如果是正式版release,请把__RELEASE f反注释掉
//#define RUN_PROC      // 如果是线程版,RUN_PROC f反注释掉

// 定义线程的一些debug打印设置,release 版本不需要
#ifndef  _RELEASE
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

  pthread_t tid;        // 线程处理
  extern void *thread_doit( void* vargp); // 子线程处理的函数

int main(int argc, char **argv) {

  // 检测输入参数的正确性
#ifdef _RELEASE
    if (argc != 2){
        fprintf(stderr, "USAGE: %s <PORT> \n", argv[0]);
        exit(1);
    }
  char *port = argv[1];
#else
  char* port = "8888";  // release输入 argv[1];
#endif // _RELEAS

  int listenfd = 1, connfd; //连接fd 和 监听fd
  struct sockaddr_storage clientaddr;
  socklen_t clientlen;

  thread_conn_args *conn;


  /**
   * Sianal()为 信号处理函数
   * 这里忽略SIGPIPE 信号
   * SIGPIPE 为向一个没有读用户的管道做写操作
  */
   Signal(SIGPIPE, SIG_IGN);

  /// fd listenfd为打开连接的fd
  listenfd = Open_listenfd(port);
  dbg_printf("listenfd = %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n",listenfd);

  static int count = 1;
  // 主循环处理到来的fd
  while (1) {
    clientlen = sizeof(clientaddr);
    conn = (thread_conn_args*) malloc(sizeof(thread_conn_args));   // 每次分配堆上
    // TODO: 记得在访问完的时候free掉
    conn ->connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    dbg_printf("connfd = %d\n",connfd);
    conn ->count = count;
    count ++;

   /// DNS功能
    char hostname[MAXLINE], clientport[MAXLINE];
    getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, clientport, MAXLINE,0);
    printf("Accepted connection from (%s, %s)\n", hostname, clientport);
 #ifdef RUN_PROC
    // 进程版本
    handle_http(connfd);
    Close(connfd);
 #else
     pthread_create(&tid,NULL,thread_doit,conn);
 #endif // RUN_PROC
  }
  system("pause");
  return 0;
}

void *thread_doit( void* vargp) // Thread rountine
{
  dbg_printf("New process: PID: %d, TID: %lu\n", getpid(), pthread_self());
  thread_conn_args * conn = (thread_conn_args *) vargp;
  int connfd = conn->connfd;
  int count = conn ->count;
  pthread_detach(pthread_self());
  dbg_printf("THE process: PID: %d has been detached !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n", getpid());
  free(vargp);
  handle_http(connfd);
  Close(connfd);
  return ;
}
