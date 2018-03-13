#include "http.h"
#include <string.h>
/*
* socket 封装函数,返回成功描述符fd
*/
int Socket(int domain, int type, int protocol)
{
    int rc;

    if ((rc = socket(domain, type, protocol)) < 0)
        error_die("Socket error");

    return rc;
}

/* UNIX I/O封装 */
int Open(const char *pathname, int flags, mode_t mode)
{
    int rc;

    if ((rc = open(pathname, flags, mode)) < 0)
        error_die("Open error");

    return rc;
}

/*
 *APSIX close() 的封装
*/
void Close(int fd)
{
    int rc;

    if ((rc = close(fd)) < 0)
        error_die("Close error");
}

/*
 * 错误打印函数
*/
void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}

/* 存储器映射封装 */
void *Mmap(void *addr, size_t len, int port, int flags, int fd, off_t offset)
{
    void *ptr;

    if ((ptr = mmap(addr, len, port, flags, fd, offset)) == ((void *)-1))
        error_die("mmap error");

    return ptr;
}

/*
 * 打开连接fd listenfd,成功返回listenfd的值,失败为-1
*/

int Open_listenfd(char *port)
{
    int listenfd, optval = 1;
    // 打开协议无关的套接字,见P658
    // addrinfo 见p658 图11-16
    struct addrinfo hints, *listp, *p;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_flags |= AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;

    // getaddrinfo见p656-11.4.7,生成套接字结构,可重入
    if (0 != getaddrinfo(NULL, port, &hints, &listp)  )
        error_die("getaddrinfo error1");

    //遍历链表,查找一个可以bind的描述符
    for (p = listp; p; p = p->ai_next)
    {
        if ((listenfd = Socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            // 可在这里使用getNameInfo,获取主机名字,见p659:hostinfo.c
            continue;

        if (0 !=  (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,sizeof(int) )))
        {
            error_die("setaddrinfo error");
        }

        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;   // 打开套接字成功成功

        Close(listenfd);
    }

    // 释放List的资源
    freeaddrinfo(listp);

    if (!p)
        return -1;

    if (listen(listenfd, LISTENQ) < 0)
        return -1;

    return listenfd;
}

/*
 *Accept 封装,成功返回非负connfd
*/
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    int rc;

    if ((rc = accept(s, addr, addrlen)) < 0)
        error_die("Accept error");

    return rc;
}

/*
* 处理http的主要函数,成功返回0,失败返回-1
*/
int handle_http(int fd)
{
    printf("handle_http connfd is %d \n",fd);
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    struct stat sbuf;    // 保存文件filename 的信息,见man 2 statŝ

    int is_static = 0;

    rio_t rio;   //Robust I/O ,见CSAPPP chap10.5

    rio_readinitb(&rio,fd);    // 初始化rio函数

    // 从套接字缓存读取显示请求到buf 中
    if (!rio_readlineb(&rio, buf, MAXLINE))
        return -1;

    printf("%s", buf);

    sscanf(buf,"%s %s %s",method,uri, version);

    if (0 != strcasecmp(method, "GET")  && 0 != strcasecmp(method, "POST") )
    {
        // 忽略大小写的method比较,实现http 的GET 方法
        clienterror(fd, method, "501", "Not Implemented",
                    " does not implement this method");
        return 0;
    }

    // 读取并忽略报头函数
    read_requesthdrs(&rio);
    // 动静态请求的分析
    is_static = prase_uri(uri, filename,cgiargs);

    if (stat(filename, &sbuf) < 0)
    {
        // 获取filename 的文件信息
        clienterror(fd, filename, "404", "Not found",
                    "FSeed couldn't find this file");
        return 0;
    }

    if(is_static)
    {
        // 静态文件的处理
        if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            // 不是常规文件,S_ISREG 见fcntl.h
            clienterror(fd, filename,"404","Not Found", "FSeed could not found this file");
            return 0;
        }

        serve_static(fd, filename,sbuf.st_size);     // 静态文件的处理函数
    }
    else
    {

        // 动态文件CGI的处理
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            clienterror(fd, filename, "403", "Forbidden",
                        "FSeed couldn't run the CGI program");
            return 0;
        }

        serve_dynamic(fd, filename, cgiargs);     // 动态文件的处理函数
    }

    printf("handle_http success\n");
    return 0;
}

/*
* 读取每一行报文并忽略http报头函数
*/
void read_requesthdrs( rio_t *rp)
{
    char buf[MAXLINE];

    rio_readlineb(rp, buf, MAXLINE);  // 读取头部第一行

    while( strcmp(buf, "\r\n") )
    {
        // 当不是"\r\n " 结束时
        rio_readlineb(rp, buf, MAXLINE);
        printf("%s",buf);
    }

    return;
}


/*
 *  FSeed的http请求错误处理函数
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg,
                 char *longmsg)
{
    char buf[MAXLINE], body[MAXBUF];

    sprintf(body, "<html><title>Fseed Error</title>");
    sprintf(body,
            "%s<body bgcolor="
            "ffffff"
            ">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The FSeed Web server</em>\r\n", body);

    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

/*
 * prase_uri
 * 分析uri的请求
 * 返回1如果是静态的请求,返回0 则为cgi通用网关请求
*/

int prase_uri( char* uri, char* filename, char* cgiargs)
{
    char *ptr;

    if (!strstr(uri, "cgi-bin"))
    {
        // 静态的请求
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);

        if (uri[strlen(uri) - 1] == '/')
            strcat(filename, "home.html");   // 默认为home.html

        return 1;
    }
    else
    {
        // 动态的请求,格式为 GET /cgi-bin/prog?args1&args2
        ptr = index(uri, '?');

        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
        {
            strcpy(cgiargs, "");
        }

        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

/*
 * serve_static
 * 静态文件的处理函数
 */
void serve_static(int fd, char *filename, int filesize)
{
    int srcfd;    // 源文件的文件描述符
    char *srcp, filetype[MAXLINE], buf[MAXBUF];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: FSeed Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));
    printf("Response headers:\n");
    printf("%s", buf);

    srcfd = Open(filename, O_RDONLY,0);
    srcp = Mmap(0,filesize, PROT_READ,MAP_PRIVATE, srcfd, 0);  // 内存映射见CSAPP chap9.8
    Close(srcfd);
    rio_writen(fd, srcp, filesize);
    munmap(srcp,filesize);
    /// TODO: 这里是否需要exit返回呢?
    return;
}
/*
 * serve_dynamic
 * 动态文件的处理函数
 */
void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};

    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Server: FSeed Web Server\r\n");
    rio_writen(fd, buf, strlen(buf));

    if (fork() == 0)
    {
        // 分配子进程运行cgi程序
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);
        execve(filename, emptylist, environ);
    }

    wait(NULL);
}
/*
 * get_filetype
 *获取文件类型
 */
void get_filetype(char *filename, char* filetype)
{
    if (strstr(filename, ".html"))
        strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
        strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
        strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
        strcpy(filetype, "image/jpeg");
    else
        strcpy(filetype, "text/plain");
}















