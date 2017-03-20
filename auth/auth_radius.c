#include <sys/types.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "../conf.h"
#include "../proto.h"
#include "../radd.h"
#include "../md5.h"
#include "../log.h"

/* Number of retries */
#define NTRIES 3

/* Timeout for retransmission in seconds */
#define TIMEOUT 5

/* remote port number */
#define PORT 1645

int sign_packet(AUTH_HDR * hdr, char *key)
{
	VALUE *pass;
	u_char digest[AUTH_VECTOR_LEN];
	u_char *buffer = alloca(256);
	u_char *passwd;
	int i;

	if ((pass = proto_findtuple(hdr, A_USER_PASSWORD)) == (VALUE *) NULL) {
		Warn("auth_radius: no Password attribute in packet");
		return 1;
	}
	passwd = (((char *) pass) + 2);

	bzero(buffer, 256);
/*
	memcpy(buffer, hdr->vector, AUTH_VECTOR_LEN);
	memcpy(buffer + AUTH_VECTOR_LEN, key, strlen(key));
*/
	memcpy(buffer,key, strlen(key));
	memcpy(buffer+strlen(key), hdr->vector,AUTH_VECTOR_LEN);
	md5_calc(digest, (void *) buffer, AUTH_VECTOR_LEN + strlen(key));

	for (i = 0; i < 16; i++)
		*(passwd + i) ^= digest[i];

	hdr->code=ACCESS_REQUEST;

	return 0;
}

int radius_auth(char *name, char *passwd, struct procs * ps,
		    char *server, char *key)
{
	AUTH_HDR *resp = ps->reply;
	struct sockaddr_in addr;
	int addrlen;
	int sock, res;
	int ntries = NTRIES;
	struct timeval timeout;
	fd_set readfds;
	int i;
	AUTH_HDR *answer = alloca(UDP_MAX_DLEN);


	/* name & passwd are zero-filled 128byte strings */
	proto_inserttuple(resp, 1, name, strlen(name));

	/* TODO: align password to %16 */
	proto_inserttuple(resp, 2, passwd, 16);

	if (sign_packet(resp, key)) {
		Warn("auth_radius: can't sign a packet");
		return 1;
	}
	/* open a socket and send it */

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		Err("auth_radius:socket():%m");
		return 1;
	}
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(server);
	addr.sin_port = htons(PORT);
	addrlen = sizeof(addr);

	if (connect(sock, (struct sockaddr *) & addr, addrlen) < 0) {
		Err("auth_radius:connect:%m");
		return 1;
	}
	while (ntries--) {
		bzero(&timeout, sizeof(timeout));
		timeout.tv_sec = TIMEOUT;

		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);

		send(sock, resp, ntohs(resp->length), (int) 0);

		i = select(FD_SETSIZE + 1, &readfds, NULL, NULL, &timeout);
		switch (i) {
		case 0:	/* timeout */
			Warn("auth_radius: tries left %d", ntries);
			break;
		case -1:
			if (errno != EINTR) {
				Err("select");
				return 1;
			}
		default:
			res = recv(sock, answer, UDP_MAX_DLEN, 0);
			if (res < AUTH_HDR_LEN) {
				switch (res) {
				case 0:
					Warn("auth_radius: size == 0?");
					break;
				case -1:
					Warn("auth_radius: recv err %m");
					break;
				default:
					Warn("auth_radius: packet underrun, size %d",res);
					break;
				}
			} else {
				/* TODO check signature */
				if (answer->code == ACCESS_ACCEPT)
				{
					DEBUG("user accepted");
					return 0;
				}
				else
				{
					DEBUG("user declined");
					return 1;
				}
			}
		}
	}
	return 1;
}
