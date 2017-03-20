#include <sys/queue.h>
#include <sys/types.h>

#define AUTH_HDR_LEN            20
#define AUTH_VECTOR_LEN         16

typedef struct pw_auth_hdr {
	u_char code;
	u_char id;
	u_short length;
	u_char vector[AUTH_VECTOR_LEN];
	u_char data[2];
}   AUTH_HDR;

typedef struct value {
	u_char code;
	u_char len;
	u_long value;
}   VALUE;

/* access macros */
#define VAL(x) (((char *)(x))+2)
/*#define VALI(x) ntohl(*((u_long *)((char *)(x))+2)) */
#define VALI(x,y) {bcopy(VAL(x),&y,4);y=ntohl(y);}
/* #define VALI(x) ntohl((x)->value) */
#define LEN(x) ((x)->len-2)

/* proto codes */

#define ACCESS_REQUEST 1
#define ACCESS_ACCEPT 2
#define ACCESS_REJECT 3
#define ACCOUNTING_REQUEST 4
#define ACCOUNTING_RESPONSE 5
#define ACCESS_CHALLENGE  11
#define STATUS_SERVER 12
#define STATUS_CLIENT 13

void radrecv(u_long, u_short, char *, int);

VALUE *proto_findtuple(AUTH_HDR *, int);
void proto_inserttuple(AUTH_HDR *, int, void *, int);
void proto_dumppacket(AUTH_HDR *, char *);

int proto_prep_req(AUTH_HDR **, AUTH_HDR *, char *);
int proto_prep_acct(AUTH_HDR **, AUTH_HDR *, char *);

struct procs;			/* forward declaration */
void proto_serve(struct procs *);
void proto_reject(struct procs *, char *);
void proto_accept(struct procs *);
void proto_acct(struct procs *);
void proto_acct_responce(struct procs *);

/* required attributes */

#define A_USER_NAME	1
#define A_USER_PASSWORD 2
#define A_CHAP_PASSWORD 3

struct procs {
	u_long ip;
	u_char ident;
	time_t timestamp;
	u_short port;
	u_char *authkey;
	AUTH_HDR *request;
	AUTH_HDR *reply;
	pid_t pid;
/*	    SLIST_ENTRY(procs) procsss; */
	struct procs *next;
};
