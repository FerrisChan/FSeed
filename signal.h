#ifndef SIGNAL_INCLUDED
#define SIGNAL_INCLUDED

 #include <signal.h>
 #include "http.h"
/**
 * Linux信号的封装
*/
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

#endif // SIGNAL_INCLUDED
