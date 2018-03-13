#include "signal.h"

/**
 * Linux信号的封装
 * 见CSAPP chap8信号部分
*/
handler_t *Signal(int signum, handler_t *handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask); /// 阻塞正在处理的信号 Block sigs of type being handled
    action.sa_flags = SA_RESTART; /// Restart syscalls if possible

    if (sigaction(signum, &action, &old_action) < 0)
        error_die("Signal error");
    return (old_action.sa_handler);
}
