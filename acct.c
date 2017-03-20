/* accounting packets process
 * $Id: acct.c,v 1.1 1998/07/27 12:03:43 vadik Exp vadik $
 * $Log: acct.c,v $
 * Revision 1.1  1998/07/27 12:03:43  vadik
 * Initial revision
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <time.h>
#include <string.h>
/* #include <mysql.h> */
#include "conf.h"
#include "md5.h"
#include "proto.h"
#include "radd.h"
#include "misc.h"
#include "log.h"

#ifdef CITYLINE
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <utmp.h>
#include "acct.h"
#include "util/own_wtmp.h"

char *system_utmp_file_name = "/var/adm/utmp";
char *system_wtmp_file_name = "/var/adm/wtmp";
char *system_ttys_file_name = "/var/adm/ttys";
/* char *radius_log_file = "/usr/local/statserv/log/radiuslog"; */
char *radius_log_file = "/tech/radius/radiuslog";
extern char *radacct_dir;
char In[256];

void touch(char *path, char *host, int port)
{
  struct stat sb;
  int fd;
  char *str;

  asprintf(&str,"%s:%d",host,port);
  if (lstat(path, &sb))
	/*
	  fd = open(path, O_WRONLY | O_CREAT, DEFFILEMODE);
	*/
	
	fd = symlink(str,path);
  if (fd == -1 )  {
	Warn("%s", path);
  }
  free(str);
}

#endif
void acct_roamsave(AUTH_HDR *, char *);

/* void acct_cidsave(AUTH_HDR *hdr) */

char *acct_cidsave(AUTH_HDR *hdr)
{
 u_char *tuple;
 FILE *file;
 VALUE *v;
 time_t ttime;
 char Uname[256];
 extern char In[256];
 char Out[256];

 bzero(Uname,sizeof(Uname));
 bzero(In, sizeof(In));
 bzero(Out,sizeof(Out));

 if ((file=fopen("/tech/radius/CID", "a+"))==NULL)
 {
	 Warn("can't open /tech/radius/CID");
	 return (NULL);
 }
	if ((v = proto_findtuple(hdr, dict_attrbyname("User-Name"))) != NULL) 
	strncpy(Uname, VAL(v), LEN(v));
	if ((v = proto_findtuple(hdr, dict_attrbyname("Calling-Station-Id"))) != NULL) 
	strncpy(In, VAL(v), LEN(v));
	if ((v = proto_findtuple(hdr, dict_attrbyname("Called-Station-Id"))) != NULL) 
	strncpy(Out, VAL(v), LEN(v));
    ttime=time(0);
    fprintf (file,"%.24s %s %s %s\n",ctime(&ttime),Uname, In, Out);
return(In);
}

void proto_acct(struct procs * ps)
{
	FILE *outfd;
	acctrec account;
	struct utmp u;
	char _path[FILENAME_MAX];
	char *t1 = alloca(64);
	AUTH_HDR *in = ps->request;
	VALUE *v;
	int stop = 0;
	char *myaddr=NULL;
	char *myphone=NULL;
	char *uname=account.uname;

	bzero(&account, sizeof(account));
	account.timestamp = time(0);


	if ((outfd = fopen(radius_log_file, "a")) == (FILE *) NULL) {
		Err("accounting: could not append to file %s\n", radius_log_file);
		/* do not respond if we cannot save record */
	} else {

			proto_acct_responce(ps); /* быстро отвечаем, дальше будем разбираться */

		if ((v = proto_findtuple(in, dict_attrbyname("User-Name"))) != NULL)
		{
			if(strnchr(VAL(v),'@',LEN(v))!=NULL)
			{
				/* save realm accounting */
				char *realm=alloca(512); /* 256 bytes for realm name */
				strncpy(account.uname, VAL(v), LEN(v));

				bzero(realm, 256);
				realm=strncpy(realm, strchr(account.uname, '@'), strlen(strchr(account.uname, '@')) );
				acct_roamsave(in,realm);
			}
			strncpy(account.uname, VAL(v), LEN(v));
		}

		/* TODO set client's IP if zero */
		if ((v = proto_findtuple(in, dict_attrbyname("NAS-IP-Address"))) != NULL)
			VALI(v, account.nas_ip);

		if ((v = proto_findtuple(in, dict_attrbyname("NAS-Port"))))
			VALI(v, account.nasport);

		if ((v = proto_findtuple(in, dict_attrbyname("Acct-Status-Type"))) != NULL) {
			VALI(v, account.type);
//			DEBUG("Acct-Status-Type = %d, %d %d", account.type, v->code, v->len);
//			DEBUG(" value %x %x %x %x", *(VAL(v)), *(VAL(v) + 1), *(VAL(v) + 2), *(VAL(v) + 3));
			lib_strtolower(uname);
			sprintf(_path, "%s/%s", USERSTAT, uname);
//			DEBUG (" pathhh is %s",uname);
			switch (account.type) {
			case 1:/* start */

				Message("user %s logged in", uname);
				touch(_path, lib_hostname(account.nas_ip), account.nasport);
				myphone = acct_cidsave(in);
//				DEBUG("UT: %s %d %d %s %s %s", lib_hostname(account.nas_ip), account.nasport, account.timestamp, uname, myphone, myaddr);
				break;
			case 2:/* stop */
				Message("user %s logged out", uname);
				stop = 1;
				if (unlink(_path)<0)
				   Warn("####%s",strerror(errno));
				break;
			}
		}
		if ((v = proto_findtuple(in, dict_attrbyname("Framed-IP-Address"))) != NULL) 
		{
			struct in_addr filfil;
			VALI(v, account.his_ip);
			filfil.s_addr = ntohl(account.his_ip);
			Warn("IPaddr from NAS: %s",inet_ntoa(filfil));
		}

		if ((v = proto_findtuple(in, dict_attrbyname("Acct-Session-Time"))) != NULL)
			VALI(v, account.session_time);

		if ((v = proto_findtuple(in, dict_attrbyname("Acct-Input-Octets"))) != NULL)
			VALI(v, account.bytes_in);

		if ((v = proto_findtuple(in, dict_attrbyname("Acct-Output-Octets"))) != NULL)
			VALI(v, account.bytes_out);

		if ((v = proto_findtuple(in, dict_attrbyname("Acct-Terminate-Cause"))) != NULL)
			VALI(v, account.terminated_by);

		bzero(t1, 64);
		sprintf(t1, "tty%d", account.nasport);
		strncpy(u.ut_line, t1, UT_LINESIZE);
		strncpy(u.ut_host, lib_ntoab(htonl(account.nas_ip)), UT_HOSTSIZE);
		strncpy(u.ut_name, stop ? "" : uname, UT_NAMESIZE);
		time(&u.ut_time);
		my_login(&u);
		fwrite(&account, sizeof(acctrec), 1, outfd);
		fclose(outfd);

	}
#if 0
	proto_acct_responce(ps);
#endif
	exit(0);
}


#define  ROAMPATH "/var/adm"

void acct_roamsave(AUTH_HDR *hdr, char *realm)
{
	u_char         *tuple;
	u_char         *maxoffset = ((u_char *) hdr) + ntohs(hdr->length);
	FILE           *file;
	time_t 		ttime;
	char	*fpath;
	int q;

	DEBUG("saving accounting from %s\n",realm);
	asprintf(&fpath,"%s/%s",ROAMPATH,realm);
	if ((file=fopen(fpath, "a+"))==NULL)
	{
		Warn("can't open %s\n",fpath);
		goto exit;
	}
	ttime=time(0);
	fprintf (file,"%8lX %.24s",ttime,ctime(&ttime));
	for (tuple = hdr->data;
         tuple < maxoffset; tuple += *(tuple + 1)) {
         VALUE *val=(VALUE *)tuple;	

	   if(dict_attrbynum(val->code))
		fprintf(file, " %s=", dict_attrbynum(val->code));
	   else
		fprintf(file, " %d=", val->code);
	
	    switch(dict_typebyval(val->code)) {
		case T_STR:
			fprintf(file,"%s",strencode(VAL(val),LEN(val)));
			break;
		case T_INT:
			VALI(val,q);
			fprintf(file,"%d",q);
			break;
		case T_IP:
			VALI(val,q);
			fprintf(file,"%d.%d.%d.%d", 
				(q&0xff000000)>>24,
				(q&0x00ff0000)>>16,
				(q&0x0000ff00)>>8,
				q&0x000000ff);
			break;
		default:
			fprintf(file,"???");
			break;
		}
	}
	fputs("\n",file);
	fclose(file);
  exit:
	free(fpath);
}

void proto_acct_responce(struct procs * procs)
{
	struct sockaddr_in sin;

	/* AUTH_HDR *in=procs->request; */
	AUTH_HDR *out = procs->reply;
	u_char newdg[16];
	int tlen;

	out->code = ACCOUNTING_RESPONSE;
	tlen = ntohs(out->length);
	memcpy(((char *) out) + tlen, procs->authkey, strlen(procs->authkey));
	md5_calc(newdg, (char *) out, tlen + strlen(procs->authkey));
	memcpy(out->vector, newdg, AUTH_VECTOR_LEN);

	memset((char *) &sin, '\0', sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(procs->ip);
	sin.sin_port = htons(procs->port);

	sendto(acct_sock, (char *) out, (int) tlen, (int) 0,
	     (struct sockaddr *) & sin, sizeof(struct sockaddr_in));

}
