/*
$Id: dict.c,v 1.1 1998/06/26 23:22:32 vadik Exp vadik $
$Log: dict.c,v $
Revision 1.1  1998/06/26 23:22:32  vadik
Initial revision

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "proto.h"
#include "radd.h"
#include "conf.h"
#include "log.h"

DICT_ATTR dict_attributes[MAX_DICT_SIZE];
DICT_VAL dict_values[MAX_DICT_SIZE];

int dict_attributes_num = 0;
int dict_values_num = 0;

int dict_type(char *type)
{
	char *types[] = {"string", "integer", "ipaddr", "date"};
	int ntypes = 4, i;

	for (i = 0; i < ntypes; i++) {
		if (strncasecmp(types[i], type, strlen(types[i])) == 0)
			return i;
	}
	return -1;
}

static char _cbuff[1024];

char *dict_cat(char *s1, char *s2)
{
	bzero(_cbuff, sizeof(_cbuff));
	strcpy(_cbuff, s1);
	strcat(_cbuff, s2);
	return _cbuff;
}

int dict_attrbyname(char *name)
{
	int i;

	for (i = 0; i < dict_attributes_num; i++)
		if (strncasecmp(dict_attributes[i].name, name, strlen(dict_attributes[i].name)) == 0)
			return dict_attributes[i].value;

	return -1;

}

int dict_typebyname(char *name)
{
	int i;

	for (i = 0; i < dict_attributes_num; i++)
		if (strncasecmp(dict_attributes[i].name, name, strlen(dict_attributes[i].name)) == 0)
			return dict_attributes[i].type;

	return -1;

}


int dict_typebyval(int val)
{
	int i;

	for (i = 0; i < dict_attributes_num; i++)
		if (dict_attributes[i].value == val)
			return dict_attributes[i].type;

	return -1;

}



void dict_init(char *file)
{
	FILE *fin;
	char str[512];
	char state[64], name[64], value[64], type[64];
	int line = 0;

	if ((fin = fopen(dict_cat(DICT_PATH, file), "r")) == NULL)
		Err("dict_init:fopen():%s:%m", file);

	while (!feof(fin)) {
		fgets(str, 512, fin);
		line++;

		if (*str == '#' || *str == '\0' || *str == '\n')
			continue;

		if (strncasecmp(str, "$INCLUDE", 8) == 0) {
			if (sscanf(str, "%s%s", state, value) != 2) {
				Warn("dict_init: include: bad $INCLUDE on line %d", line);
				continue;
			}
			dict_init(value);
			continue;
		}
		if (sscanf(str, "%s%s%s%s", state, name, value, type) != 4) {
			Warn("dict_init:sscanf():invalid line %d", line);
			continue;
		}
		if (strncasecmp(state, "ATTRIBUTE", 9) == 0) {
#define _ dict_attributes[dict_attributes_num].

			strncpy(_ name, name, 32);

			_   value = atoi(value);
			_   type = dict_type(type);

			if (_ type == -1) {
				Warn("dict_init: invalid type %s on line %d", type, line);
				continue;
			}
#undef _
			dict_attributes_num++;
		} else if (strncasecmp(state, "VALUE", 5) == 0) {
			int q;

			if ((q = dict_attrbyname(name)) == -1) {
				Warn("init_dict: attribute %s not found at line %d", name, line);
				continue;
			}
#define _ dict_values[dict_values_num].
			_   attr = q;

			strncpy(_ name, value, 32);
			_   value = atoi(type);

#undef _
			dict_values_num++;
		} else {
			Warn("dict_init:strncasecmp():invalid line %d", line);
			continue;
		}
	}
	DEBUG("dict_init(): attributes %d", dict_attributes_num);
#if 0
	{
		FILE *dbg;
		int i;

		dbg = fopen("/tmp/dict", "w");
		for (i = 0; i < dict_attributes_num; i++)
			fprintf(dbg, "%s %d %d\n", dict_attributes[i].name, dict_attributes[i].value, dict_attributes[i].type);
		fclose(dbg);
	}
#endif
}



char *dict_attrbynum(int num)
{
	int i;

	for (i = 0; i < dict_attributes_num; i++)
		if (dict_attributes[i].value == num)
			return dict_attributes[i].name;
	return NULL;
}

/* TODO */

char *dict_valbynum(char *attr, int num)
{
	return NULL;
}

char *dict_valbyinum(int attr, int num)
{
	return NULL;
}

int dict_valbynam(char *attr, char *nam)
{
	return -1;
}

int dict_valbyinam(int attr, char *nam)
{
	int i;

	for (i = 0; i < dict_values_num; i++) {
		if ((dict_values[i].attr == attr) &&
		    strncasecmp(dict_values[i].name, nam, strlen(dict_values[i].name)) == 0)
			return dict_values[i].value;
	}
	return -1;
}
