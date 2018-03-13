#include "rio.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>




/*
    * rio.h 的核心
    *局部的rio_read 函数,linux 内置read函数的带缓存版本
    *
*/
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    size_t cnt;
    while(rp->rio_cnt <= 0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if (rp->rio_cnt < 0){
            if(errno == EAGAIN){
                return -EAGAIN;
            }
            if(errno != EINTR){
                return -1;
            }
        }
        else if(rp->rio_cnt == 0)
            return 0;
        else
            rp->rio_bufptr = rp->rio_buf;
    }
    cnt = n;
    if(rp->rio_cnt < (ssize_t)n)
        cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}
// 无缓冲的输入输出函数
void rio_readinitb(rio_t *rp, int fd)
{
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
    printf("init rio is successful \n");
}

/*
    * RiO的写入n个char 并带有 buff
    * 成功返回写入的n的个数
    * 失败返回-1
*/
ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = (char *)usrbuf;

    while(nleft > 0){
        if((nwritten = write(fd, bufp, nleft)) <= 0){
            if (errno == EINTR)
                nwritten = 0;
            else{
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}



/*
    * RiO的读取n个char 并带有 buff
    * 成功返回读取的n的个数
    * 失败返回-1
*/
ssize_t rio_readnb(int fd,void *usrbuf,size_t n )
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = (char*) usrbuf;
    while(nleft > 0){
        if(nread = rio_read(fd,bufp,nleft) < 0){
            if (errno == EINTR)  // 被信号打断
                nread = 0;       // 重新再读,再调用read函数
            else
                return -1;       //  read 函数的其他错误
        } else if(nread == 0) {
            break; // 读到EOF
        }
        nleft -= nread;
        bufp += nread;

    }
    return (n - nleft);
}

/*
  * 带缓存的IO,读取一行,类似为getline()
*/
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
    size_t n;
    ssize_t rc;
    char c, *bufp = (char *) usrbuf;
    for ( n = 1;n < maxlen; n++){
            // 调用maxlen-1 次rio_read
        if (rc = rio_read(rp,&c, 1) == 1){
            *bufp++ = c;
            if(c == '\n'){
                // n ++;     // 这个 n 可以不用加1
                break;  // 读完一行
            }
        } else if ( rc == 0){
            if ( n == 1)
                return 0;       // EOF,调用该函数没有读到任何文件
            else
                break;         // EOF,调用该函数有读到一些文件
        } else
            return -1 ;      // 函数出错
    }
    * bufp = 0;
    return n -1;
}
