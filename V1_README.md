# V1_README.md

 这是第一版,使用的是简单的accept来一个连接就开一个进程或线程去处理连接的原始版本;主要是练习一下网络编程而已,大量参考了CSAPP的tiny webproxy
 同时,也做了一些压力测试和其他测试.
 
 
 - download 好 zip 包
 - make,程序自动运行在8888 端口
 - 浏览器输入http://localhost:8888/home.html
 
 
 然后就可以看到结果了
 
 ## 代码覆盖率
 
 因为测试了下代码覆盖率,如果不需要就把makefile里面的-fprofile-arcs -ftest-coverage 给去掉就可以了
