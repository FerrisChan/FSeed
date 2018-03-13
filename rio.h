#ifndef RIO_H_INCLUDED
#define RIO_H_INCLUDED

#include <sys/types.h>
#include <stdio.h>

#define RIO_BUFSIZE (8192)

typedef struct {
    int rio_fd;     // 内置的文件描述符
    int rio_cnt;    // unread文件描述符的字节数大小
    char *rio_bufptr;   // 缓冲区指针
    char rio_buf[RIO_BUFSIZE];    // 内置缓冲区
} rio_t;

// 初始化rio包
void rio_readinitb(rio_t *rp, int fd);

//
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);


/*
    * RiO的写入n个char 并带有 buff
    * 成功返回写入的n的个数
    * 失败返回-1
*/
ssize_t rio_writen(int fd, void *usrbuf, size_t n);


#endif // RIO_H_INCLUDED
