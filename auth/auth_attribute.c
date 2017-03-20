/* $Id$ */
/* $Log$ */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdlib.h>
#include "../conf.h"
#include "../proto.h"
#include "../radd.h"
#include "../log.h"

int attribute_auth(char *name, char *passwd, struct procs * ps, char *attribute, char *val)
{
	AUTH_HDR *resp = ps->reply;
	u_char value[128];
	int vali;
	unsigned long ina;
	short length=0;
	int attr = dict_attrbyname(attribute);

	switch (dict_typebyname(attribute)) {
	case T_STR:
		length = strlen(val) > 127 ? 127 : strlen(val);
		strncpy(value, val, length);
		break;
	case T_INT:
		if ((vali = dict_valbyinam(attr, val)) == -1)
			vali = atoi(val);
		vali=htonl(vali);
		length = 4;
		memcpy(value, &vali, length);
		break;
	case T_IP:
		ina = inet_addr(val);
		length = 4;
		memcpy(value, &ina, length);
		break;
	case T_DATE:
		/* TODO, but who needs it? :-) */
		break;
	default:
    	Warn("##### attribute %s not found", attribute);
		return 1;
	}

	if(length){
    DEBUG("$$$$$$ attribute %s added", attribute);
	proto_inserttuple(resp, dict_attrbyname(attribute), value, length);

	return 0;		/* success */
	}else{
	Warn("can't insert zero length tuple");
	return 1;
	}
}
