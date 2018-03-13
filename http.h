#ifndef HTTP_H_INCLUDED
#define HTTP_H_INCLUDED

#include <stdio.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "rio.h"


#define LISTENQ 1024 // listen 队列长度,见P655
#define MAXLINE 1024 // 字符串的长度
#define MAXBUF 8192  // 最大IO缓冲大小

extern char **environ; /* 通过libc定义,用于execve()函数 */
/* 简化bind(), connect()和accept()函数调用 */
typedef struct sockaddr SA;

// Close :APSIX close() 的封装
void Close(int fd);
// Open_listenfd 打开连接fd listenfd
int Open_listenfd(char* port);
// 错误打印函数
void error_die(const char *sc);
// Accept 封装,成功返回connfd
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
// handle_http处理http的函数,成功返回0,失败返回-1
extern int handle_http(int connf);

// clienterror Fseed的http请求错误处理函数
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg);

// read_requesthdrs 读取并忽略报头函数
void read_requesthdrs(rio_t *rp);

// prase_uri 分析uri的请求
int prase_uri( char* uri, char* filename, char* cgiargs);

/**
 * serve_static为
 * 静态文件的处理函数
 * 无返回值
 */

void serve_static(int fd, char* filename, int filesize);


/**
 * serve_dynamic
 * 动态文件的处理函数
 * 无返回值
 */
void serve_dynamic (int fd, char* filename, char* cgiargs);

// 获取文件类型
void get_filetype(char  *filename, char *filetype);

#endif // HTTP_H_INCLUDED
