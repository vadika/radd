PROG=radd
SRCS=radd.c dict.c auth.c log.c clients.c md5.c proto.c acct.c misc.c stats.c malloc.c syslog.c
COPTS+=-g -Wall -DCITYLINE 
LDADD+=  -lcrypt -L auth -lauth -L acct -lacct -lc -L ./util -lsystem  -L /usr/local/lib -lm  
#-lefence
NOMAN=noman
.include <bsd.prog.mk>

xref:
	cxref *.c -Ocxref -xref-all -index-all -html

#
# $Id: Makefile,v 1.1 1998/06/27 15:58:05 vadik Exp vadik $
#
