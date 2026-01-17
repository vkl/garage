#ifdef TRACE
#include "log.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>

static char buf[64];

void
log_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    USART_SendStr(buf);
}
#endif
