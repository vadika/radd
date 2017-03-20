/*
 * $Id: radd.c,v 1.1 1998/06/26 23:23:05 vadik Exp vadik $ $Log: radd.c,v $
 * Revision 1.1  1998/06/26 23:23:05  vadik Initial revision
 * 
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>

#define __MAIN__
#include "conf.h"
#include "proto.h"
#include "radd.h"
#include "stats.h"
#include "log.h"
#include "xmalloc.h"

int             debug = 1;

int             auth_sock, acct_sock;

extern char    *optarg;
extern int      optind;
extern int      optopt;
extern int      opterr;
extern int      optreset;


void 
def_sig(int sig)
{
	Warn("default shandler: caught signal %d", sig);
}

void 
int_sig(int sig)
{
	Warn("shandler: caught SIGINT");
	exit(-1);
}

extern void     chld_sig(int);

void 
hup_sig(int sig)
{
	Warn("shandler: caught SIGHUP, reconfig? ");
}

void 
catch_signals()
{
	signal(SIGHUP, hup_sig);
	signal(SIGINT, int_sig);
	signal(SIGCHLD, chld_sig);
	signal(SIGUSR1, sig_usr1);
	signal(SIGUSR2, def_sig);
}

int 
alloc_sock(int port)
{

	struct sockaddr_in addr;
	int             addrlen;
	int             sock, res;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		Err("alloc_sock:socket():%m");

	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);
	addrlen = sizeof(addr);

	res = bind(sock, (struct sockaddr *) & addr, addrlen);

	if (res < 0)
		Err("alloc_sock:bind():%m");

	return sock;
}


void 
main_loop()
{
	fd_set          readfds;
	struct timeval  timeout;
	int             i;
	char           *buf;
	struct sockaddr_in from;
	int             addrsize = sizeof(struct sockaddr_in);
	int             size;

	while (1) {
		bzero(&timeout, sizeof(timeout));
		FD_ZERO(&readfds);
		FD_SET(auth_sock, &readfds);
		FD_SET(acct_sock, &readfds);

		i = select(FD_SETSIZE + 1, &readfds, NULL, NULL, NULL);

		switch (i) {
		case 0:
			break;
		case -1:
			if (errno != EINTR)
				errx(1, "select");
			break;
		default:
			if (FD_ISSET(auth_sock, &readfds)) {
				buf = Xmalloc(UDP_MAX_DLEN);
				size = recvfrom(auth_sock, buf, UDP_MAX_DLEN,
				  0, (struct sockaddr *) & from, &addrsize);
				if (size < AUTH_HDR_LEN) {
					switch (size) {
					case 0:
						Warn("auth size ==0... hmm. f*cking..");
						break;
					case -1:
						Warn("auth error %s", strerror(errno));
						break;
					default:
						Warn("auth packet underrun, size %d", size);
					}
					Xfree(buf);
				} else
					radrecv(ntohl(from.sin_addr.s_addr), ntohs(from.sin_port), buf, size);
			}
			if (FD_ISSET(acct_sock, &readfds)) {
				buf = Xmalloc(UDP_MAX_DLEN);
				size = recvfrom(acct_sock, buf, UDP_MAX_DLEN,
				  0, (struct sockaddr *) & from, &addrsize);
				if (size < AUTH_HDR_LEN) {
					switch (size) {
					case 0:
						Warn("acct size ==0... hmm. f*cking..");
						break;
					case -1:
						Warn("acct error %s", strerror(errno));
						break;
					default:
						Warn("accounting packet underrun, size %d", size);
					}
					Xfree(buf);

				} else
					radrecv(ntohl(from.sin_addr.s_addr), ntohs(from.sin_port), buf, size);
			}
		}
	}
}

int 
main(int argc, char **argv)
{

	dict_init(DICT_FILE);
	auth_init(REALM_FILE);
	clnt_init(CLIENTS_FILE);
	stats_init(STATS_FILE);
	proto_init();
	init_log();

	/* TODO -- parse commandline */

	/* if (!debug) */
	daemon(0, 0);
	catch_signals();

	/* TODO -- get from /etc/services */

	auth_sock = alloc_sock(1645);
	acct_sock = alloc_sock(1646);

	main_loop();
	return 0;
}
