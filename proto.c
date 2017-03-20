/*
 * $Id: proto.c,v 1.1 1998/06/29 11:18:26 vadik Exp vadik $ $Log: proto.c,v $
 * Revision 1.1  1998/06/29 11:18:26  vadik Initial revision
 * 
 */

/* #include <sys/queue.h> */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>

#include "misc.h"
#include "proto.h"
#include "conf.h"
#include "md5.h"
#include "radd.h"
#include "stats.h"
#include "log.h"
#include "xmalloc.h"

/*
 * SLIST_HEAD(procshead, procs) head; struct procshead *headp;
 */

struct procs   *first_one = NULL;

/*
 * struct procs { u_long ip; u_char ident; u_short port; u_char *authkey;
 * AUTH_HDR *request; AUTH_HDR *reply; Pid_t pid; SLIST_ENTRY(procs) procsss;
 * };
 */

void 
proto_init(void)
{
	struct sigaltstack sigstk;

	DEBUG("proto_init: init()");
	/* SLIST_INIT(&head); */
#define NEWSIGSTKSZ    1024*128
	if ((sigstk.ss_sp = Xmalloc(NEWSIGSTKSZ)) == NULL)
		/* error return */
		sigstk.ss_size = NEWSIGSTKSZ;
	sigstk.ss_flags = 0;
	if (sigaltstack(&sigstk, 0) < 0)
		Err("sigaltstack");
	first_one = NULL;
}

void 
chld_sig(int sig)
{
	/* remove child from queue */
	int             status;
	pid_t           pid;
	struct procs   *np;

	while (1) {

		/* pid = wait(&status);  */
		pid = waitpid((pid_t) - 1, &status, WNOHANG);
		/* Warn("terminated %d", pid); */
		if (pid == -1)
			return;

		/* TODO IFEXITED */
		for (np = first_one; np != NULL; np = np->next)
			if (np->pid == pid) {
				np->pid = -1;
				Xfree(np->request);
				Xfree(np->reply);
				/* free(np); */
			}
	}
}

void 
null_procs(struct procs * unused)
{
	Warn("***************************************************");
	Warn("INTERNAL SERVER ERROR");
	Warn("this message should never be displayed");
	Warn("if it is, report me, please");
	Warn("\t\t\tvadik@sensi.org");
	Warn("***************************************************");
}

void 
radrecv(u_long addr, u_short port, char *buf, int len)
{
	char           *key;
	AUTH_HDR       *hdr = (AUTH_HDR *) buf;
	AUTH_HDR       *reply;
	struct procs   *np, *p;
	void            (*sfunc) (struct procs *) = null_procs;
	sigset_t        sigs, osigs;

	/* check for presence in CLIENTS */
	if ((key = clnt_getkey(addr)) == NULL) {
		Warn("unknown client %s", lib_hostname(addr));
		STATS.unk_clients++;
		Xfree(buf);
		return;
	};

	DEBUG("received %d bytes from %s, key '%s'", len, lib_hostname(addr), key);
	/* validate key and decrypt passwd */
	switch (hdr->code) {
	case ACCESS_REQUEST:
		/* decrypt passwd, check for validity */
		if (proto_prep_req(&reply, hdr, key)) {
			sfunc = proto_serve;
			STATS.requests++;
			break;
		} else {
			Warn("%s sends us packet with bad auth info", lib_hostname(addr));
			Xfree(buf);
			return;
		}
	case ACCESS_ACCEPT:
	case ACCESS_REJECT:
		Warn("client-side packet skipped");
		Xfree(buf);
		return;

	case ACCOUNTING_REQUEST:
	case ACCOUNTING_RESPONSE:
		if (proto_prep_acct(&reply, hdr, key)) {
			sfunc = proto_acct;
			STATS.accounts++;
			break;
		} else {
			Warn("%s sends us accounting packet with bad auth info",
			     lib_hostname(addr));
			Xfree(buf);
			return;
		}

		/* other rfc-valid packet types -- not supported now */
	case ACCESS_CHALLENGE:
	case STATUS_SERVER:
	case STATUS_CLIENT:
	default:
		Warn("invalid or unsupported action code %d", hdr->code);
		Xfree(buf);
		return;
	}

	sigemptyset(&sigs);
	sigaddset(&sigs, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sigs, &osigs);

	/* garbage collector */
	for (np = first_one; np != NULL; np = np->next) {
		if (np->pid != -1)
			if (np->timestamp < (time(0) - 30)) {
				np->pid = -1;
				Xfree(np->request);
				Xfree(np->reply);
				STATS.gc++;
				continue;
			}
	}
	/* check for duplicate request */
	for (np = first_one; np != NULL; np = np->next)
		if (np->pid != -1) {
			if (np->ip == addr && np->ident == hdr->id) {
				/* duplicate request */
				Warn("dropping duplicate from  %s", lib_hostname(addr));
				STATS.duplicates++;
				sigprocmask(SIG_SETMASK, &osigs, NULL);
				Xfree(buf);
				return;
			}
		}
	/* fill in queue and fork child */
	/* no-free algorythm has sense on static-loaded constructions */
	for (p = first_one; p != NULL; p = p->next)
		if (p->pid == -1)
			break;

	if (p == NULL) {
		np = Xmalloc(sizeof(struct procs));
		STATS.qsize++;
	} else
		np = p;


	np->ip = addr;
	np->timestamp = time(0);
	np->ident = hdr->id;
	np->port = port;
	np->request = hdr;
	np->reply = reply;
	np->authkey = key;
	if (p == NULL) {
		np->next = first_one;
		first_one = np;
	}
	sigprocmask(SIG_SETMASK, &osigs, NULL);

	/* switch (np->pid = rfork(RFPROC | RFMEM)) { */
	switch (np->pid = fork()) {
	case 0:		/* child */
		(*sfunc) (np);
	case -1:		/* error */
		Err("can't fork!:%m");
	default:		/* parent */
		break;
	}
}


int 
proto_prep_req(AUTH_HDR ** new, AUTH_HDR * hdr, char *key)
{
	VALUE          *pass;
	u_char          buf[128];
	u_char          digest[128];
	u_char         *passwd;
	u_char          Password[130];

	int             keylen = strlen(key);
	int             i, j, passwd_len;
/*
	if (debug)
		proto_dumppacket(hdr, "proto_prep_req");
*/

	if ((pass = proto_findtuple(hdr, A_USER_PASSWORD)) == (VALUE *) NULL)
		/* TODO  -- chap support */
		/*
		 * if ((pass = proto_findtuple(hdr, A_CHAP_PASSWORD)) ==
		 * NULL)
		 */
	{
		Warn("no Password nor ChapPassword attribute in packet");
		return 0;
	}
	memset(Password, 0, sizeof(Password));
	passwd = (((char *) pass) + 2);
	passwd_len = pass->len - 2;
	DEBUG("passwd_len=%d", passwd_len);

	if (passwd_len & 0xf) {
		Warn("non-RFC2138 compliant password, %d", passwd_len - 2);
		return 0;
	}
	/* this works (untested) for passwords longer than 16 syms  */

	for (j = 0; j <= (passwd_len >> 4); j++) {
		memset(buf, 0, sizeof(buf));
		strcpy(buf, key);
		memcpy(buf + keylen, j ? (passwd + ((j - 1) << 4)) : hdr->vector, AUTH_VECTOR_LEN);
		md5_calc(digest, buf, keylen + AUTH_VECTOR_LEN);
		for (i = j << 4; i < (j + 1) << 4; i++) {
			Password[i] = digest[i] ^ *(passwd + i);
		}
	}

	DEBUG("pass = %s", Password);

	/* put it back */
	memcpy(passwd, Password, passwd_len);

	if ((*new = Xmalloc(UDP_MAX_DLEN)) == NULL) {
		Warn("can't allocate memory for the reply");
		return 0;
	}
	memcpy((*new)->vector, hdr->vector, AUTH_VECTOR_LEN);
	(*new)->id = hdr->id;
	(*new)->length = htons(AUTH_HDR_LEN);

	return 1;
}

int 
proto_prep_acct(AUTH_HDR ** new, AUTH_HDR * hdr, char *key)
{
	u_char          buf[AUTH_VECTOR_LEN];
	u_char          old[AUTH_VECTOR_LEN];

	memcpy(old, hdr->vector, AUTH_VECTOR_LEN);
	bzero(hdr->vector, AUTH_VECTOR_LEN);
	memcpy(((char *) hdr) + ntohs(hdr->length), key, strlen(key));
	md5_calc(buf, (void *) hdr, ntohs(hdr->length) + strlen(key));
	DEBUG("len = %d", ntohs(hdr->length));

	if (bcmp(buf, old, AUTH_VECTOR_LEN))
		return 0;	/* incorrect key */

	if ((*new = Xmalloc(UDP_MAX_DLEN)) == NULL) {
		Warn("can't allocate memory for the reply");
		return 0;
	}
	memcpy((*new)->vector, old, AUTH_VECTOR_LEN);
	(*new)->id = hdr->id;
	(*new)->length = htons(AUTH_HDR_LEN);

	return 1;
}

/* this runs as a separate process */
void 
proto_serve(struct procs * procs)
{
	AUTH_HDR       *in = procs->request;
	AUTH_HDR       *out = procs->reply;
	VALUE          *name = proto_findtuple(in, A_USER_NAME);
	VALUE          *passwd = proto_findtuple(in, A_USER_PASSWORD);
	REALM          *rlm = NULL;
	char            cuser[128];
	char            cpasswd[128];
	char           *ruser = NULL;
	char           *msg = NULL;
	int             i, res;

	if (name == NULL || passwd == NULL) {
		msg = "Name and/or password not specified in authentication request";
		goto out;
	}
	bzero(cuser, 128);
	bzero(cpasswd, 128);
	memcpy(cuser, VAL(name), LEN(name));
	memcpy(cpasswd, VAL(passwd), LEN(passwd));
	if(!lib_notgarbage(cuser) || !lib_notgarbage(cpasswd))
	{
	 msg = "Input garbed; check flow and communication settings";
	 goto out;
	}
	Warn("user %s, password %s", cuser, cpasswd);

	for (i = 0; i < realm_num; i++) {
		if ((strcasecmp(realms[i].name, "DEFAULT") == 0) && rlm == NULL)
			rlm = &realms[i];

		ruser = strstr(cuser, realms[i].name);

		if (ruser == NULL)
			continue;	/* look for next matching realm or
					                           the default */
		if (ruser == cuser)
			if (strncasecmp(realms[i].name, cuser,strlen(realms[i].name)) == 0) 
			{	/* username */
				rlm = &realms[i];
				break;
			} else
				continue;

		/* split realm name from username */
		if (*ruser == '@') {
			rlm = &realms[i];
			*ruser = '\0';
			ruser++;
			break;
		}
	}

	if (rlm == NULL) {
		msg = "unspecified realm -- can't authenticate";
		goto out;
	}
	res = 0;
	for (i = 0; i < rlm->nauths; i++) {
#define _ auth_types[rlm->authmethods[i]].
#define __ rlm->authargs[i].

		res += (_ func) (cuser, cpasswd, procs, __ arg1, __ arg2, __ arg3, __ arg4);

#undef _
#undef __
	}

#ifdef CITYLINE
	Message("user %s, realm %s authentication %s", cuser, rlm->name, res ? "failed" : "ok");
#endif

	if (res)
		goto out;
	else {
		proto_accept(procs);
		exit(0);
	}
out:
	proto_reject(procs, msg);
	exit(0);
}


void 
proto_accept(struct procs * procs)
{
	struct sockaddr_in sin;

	/* AUTH_HDR *in=procs->request; */
	AUTH_HDR       *out = procs->reply;
	u_char          newdg[16];
	int             tlen;

	out->code = ACCESS_ACCEPT;
	tlen = ntohs(out->length);
	memcpy(((char *) out) + tlen, procs->authkey, strlen(procs->authkey));
	md5_calc(newdg, (char *) out, tlen + strlen(procs->authkey));
	memcpy(out->vector, newdg, AUTH_VECTOR_LEN);

	memset((char *) &sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(procs->ip);
	sin.sin_port = htons(procs->port);

	sendto(auth_sock, (char *) out, (int) tlen, (int) 0,
	       (struct sockaddr *) & sin, sizeof(struct sockaddr_in));

}


void 
proto_reject(struct procs * procs, char *message)
{
	struct sockaddr_in sin;

	/* AUTH_HDR *in=procs->request; */
	AUTH_HDR       *out = procs->reply;
	u_char          newdg[16];
	int             tlen;

	out->length = htons(AUTH_HDR_LEN);	/* strip added attributes */

	if (message != NULL) {	/* TODO add the message to the packet */
	}
	out->code = ACCESS_REJECT;
	tlen = ntohs(out->length);
	memcpy(((char *) out) + tlen, procs->authkey, strlen(procs->authkey));
	md5_calc(newdg, (char *) out, tlen + strlen(procs->authkey));
	memcpy(out->vector, newdg, AUTH_VECTOR_LEN);

	memset((char *) &sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(procs->ip);
	sin.sin_port = htons(procs->port);

	sendto(auth_sock, (char *) out, (int) tlen, (int) 0,
	       (struct sockaddr *) & sin, sizeof(struct sockaddr_in));

}


VALUE          *
proto_findtuple(AUTH_HDR * hdr, int code)
{
	u_char         *tuple;
	u_char         *maxoffset = ((u_char *) hdr) + ntohs(hdr->length);

	for (tuple = hdr->data;
	     tuple < maxoffset; tuple += *(tuple + 1)) {
		if (((VALUE *) tuple)->code == code) {
			return (VALUE *) tuple;
		}
	}
	return (VALUE *) NULL;
}

void 
proto_inserttuple(AUTH_HDR * hdr, int code, void *val, int len)
{
	u_char         *offset = (u_char *) hdr + ntohs(hdr->length);
	u_char          ccode = (u_char) (code & 0xff);
	u_char          clen = (u_char) (len & 0xff);

	/* DEBUG("!@#!@#code = %d, len = %d", code, ntohs(hdr->length)); */

	if (code == -1)
		return;

	*offset = ccode;
	*(offset + 1) = clen + 2;

	memcpy(offset + 2, val, len);
	hdr->length = htons(len + ntohs(hdr->length) + 2);
}

void 
proto_dumppacket(AUTH_HDR * hdr, char *stage)
{
	u_char         *tuple;
	u_char         *maxoffset = ((u_char *) hdr) + ntohs(hdr->length);
	FILE           *file;

	file = fopen("/tmp/raddump", "a+");

	if (stage);
	fprintf(file, "******** stage = %s\n", stage);

	for (tuple = hdr->data;
	     tuple < maxoffset; tuple += *(tuple + 1)) {
		 VALUE *val=(VALUE *)tuple;

		fprintf(file, "%s=", dict_attrbynum(val->code));

		switch (dict_typebyval(val->code)) {
		case T_STR:
			fwrite(VAL(val),LEN(val),1,file);
			fputs("\n",file);
			break;
		case T_INT:
	/*		if ((vali = dict_valbyinam(attr, val)) == -1)
				vali = atoi(val);
			vali = htonl(vali);
			length = 4;
			memcpy(value, &vali, length); */
			break;
		case T_IP:
/*			ina = inet_addr(val);
			length = 4;
			memcpy(value, &ina, length); */
			break;
		case T_DATE:
			/* TODO, but who needs it? :-) */
			break;
		default:
			fputs("\n",file);
			return 1;
		}

	}
	fputs("\n", file);
	fclose(file);
}
