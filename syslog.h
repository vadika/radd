#ifndef _MYSYSLOG_H_
#define _MYSYSLOG_H_

#include <machine/ansi.h>
#include <sys/cdefs.h>

__BEGIN_DECLS
void	mysyslog __P((int, const char *, ...)) __printflike(2, 3);
void	vmysyslog __P((int, const char *, _BSD_VA_LIST_)) __printflike(2, 0);
__END_DECLS

#endif
