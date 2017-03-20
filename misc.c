/*
$Id: misc.c,v 1.1 1998/06/27 15:57:13 vadik Exp vadik $
$Log: misc.c,v $
Revision 1.1  1998/06/27 15:57:13  vadik
Initial revision

*/

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include <arpa/inet.h>

#include	<stdio.h>
#include	<stdlib.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<time.h>
#include	<ctype.h>
#include 	<string.h>

#include "conf.h"
#include "misc.h"

static int good_ipaddr(char *);

char * lib_strtolower(char *s)
{
  char *q=s;
  while(*q){ *q=tolower(*q); q++;}
  return s; 
}

int lib_sepwords(char *words[], int num, char **inputstring)
{
	char **ap;
	int nwords = 0;

	for (ap = words; (*ap = strsep(inputstring, " \t\n")) != NULL;)
		if (**ap != '\0') {
			++ap;
			nwords++;
		}
	return nwords;
}

/*
 *	Return a printable host name (or IP address in dot notation)
 *	for the supplied IP address.
 */
char *lib_hostname(ulong ipaddr)
{
	struct hostent *hp = NULL;
	static char hstname[128];
	ulong n_ipaddr;

	n_ipaddr = htonl(ipaddr);
#ifdef  RESOLVHOSTNAMES
	hp = gethostbyaddr((char *) &n_ipaddr, sizeof(struct in_addr), AF_INET);
#endif
	if (hp == 0) {
		lib_ntoa(hstname, ipaddr);
		return (hstname);
	}
	return (char *) hp->h_name;
}


/*
 *	Return an IP address in host long notation from a host
 *	name or address in dot notation.
 */
ulong lib_aton(char *host)
{
	struct hostent *hp;

	if (good_ipaddr(host) == 0) {
		return (ntohl(inet_addr(host)));
	} else if ((hp = gethostbyname(host)) == (struct hostent *) NULL) {
		return ((ulong) 0);
	}
	return (ntohl(*(ulong *) hp->h_addr));
}

/*
 *	Return an IP address in standard dot notation for the
 *	provided address in host long notation.
 */
void lib_ntoa(char *buffer, u_long ipaddr)
{
	u_long addr_byte[4];
	int i;
	u_long xbyte;

	for (i = 0; i < 4; i++) {
		xbyte = ipaddr >> (i * 8);
		xbyte = xbyte & (ulong) 0x000000FF;
		addr_byte[i] = xbyte;
	}
	sprintf(buffer, "%lu.%lu.%lu.%lu", addr_byte[3], addr_byte[2],
		addr_byte[1], addr_byte[0]);
}



char *lib_ntoab(u_long ipaddr)
{
	u_long addr_byte[4];
	static char buffer[20];
	int i;
	u_long xbyte;

	bzero(buffer, sizeof(buffer));
	for (i = 0; i < 4; i++) {
		xbyte = ipaddr >> (i * 8);
		xbyte = xbyte & (ulong) 0x000000FF;
		addr_byte[i] = xbyte;
	}
	sprintf(buffer, "%lu.%lu.%lu.%lu", addr_byte[3], addr_byte[2],
		addr_byte[1], addr_byte[0]);
	return buffer;
}


/*
 *	Check for valid IP address in standard dot notation.
 */
static int good_ipaddr(char *addr)
{
	int dot_count;
	int digit_count;

	dot_count = 0;
	digit_count = 0;
	while (*addr != '\0' && *addr != ' ') {
		if (*addr == '.') {
			dot_count++;
			digit_count = 0;
		} else if (!isdigit(*addr)) {
			dot_count = 5;
		} else {
			digit_count++;
			if (digit_count > 3) {
				dot_count = 5;
			}
		}
		addr++;
	}
	if (dot_count != 3) {
		return (-1);
	} else {
		return (0);
	}
}

char *strnchr(char *str, char c, int len)
{
	int i;

	for(i=0;i<len;i++)
		if(*(str+i)==c)
			return str+i;
	return NULL;
}
 
char *strencode(char *str,int len)
{
 int i;
 static char buff[4096];
 char *q=buff;

 bzero(buff,sizeof(buff));

	for(i=0;i<len;i++)
	 if (*(str+i)<33 || *(str+i)>126)
	 {
	   *q='\\'; q++;
	   *q=((*(str+i)&0300)>>6)+48; q++;
	   *q=((*(str+i)&070)>>3)+48; q++;
	   *q=(*(str+i)&07)+48; q++;
	 }
	 else
	  if((*str+i)=='\\')
	  {*q='\\';q++;*q='\\';q++;}
	  else
	  { *q=*(str+i);q++;}

 return buff;
}

int lib_notgarbage(char *c)
{
 char *q=c;
 int i=0;
  for(;*q && i<127;q++, i++)
  {
	if(!isprint(*q))
		return 0;
  }
  *q='\0';
  return 1;
}
