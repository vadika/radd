#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mysql.h>

#include "../conf.h"
#include "../proto.h"
#include "../log.h"
#include "../radd.h"

#ifdef HAVEOWNMAIN
#define Warn printf
#define DEBUG printf
#endif


static char    *
getaddr(MYSQL * my, char *name, char *addr, char *addrpool)
{
	MYSQL_RES      *result;
	MYSQL_ROW      *row;
	char           *query;

	DEBUG("address pool is %s\n", addrpool);
	if (strcmp(addrpool, "fixed") == 0) {
		if (strlen(addr))
			return addr;

		mysql_query(my, "LOCK TABLES ifa_pool_fixed,online WRITE");
		mysql_query(my,
			    "SELECT ifaddr from ifa_pool_fixed where ifstatus = 'A' limit 1");

		result = mysql_store_result(my);

		if (mysql_num_rows(result) == 0) {
			DEBUG("out of address space");
			goto fuck1;
		}
		row = mysql_fetch_row(result);
		addr = (char *) row[0];

		asprintf(&query,
			 "UPDATE online SET ifaddr_d='%s' WHERE login='%s'",
			 addr, name);
		mysql_query(my, query);
		free(query);
		asprintf(&query,
			 "UPDATE ifa_pool_fixed SET ifstatus='B', lastuse=%d WHERE ifaddr='%s'",
			  time(0), addr);
		mysql_query(my, query);
		mysql_query(my, "UNLOCK TABLES");
		return addr;
	} else
		/* anonymous users */
		/* TODO */
	{
fuck1:
		mysql_query(my, "UNLOCK TABLES");
		return "255.255.255.252";
	}
}

int
mysql_auth(char *name, char *passwd, struct procs * ps,
	   char *dbhost, char *dbuser, char *dbpasswd)
{
	char           *crypt();
	char           *encpw;

	MYSQL           mysql;
	MYSQL_RES      *result;
	MYSQL_ROW      *row;
	char           *query;
	char           *addr;


	if (!mysql_connect(&mysql, dbhost, dbuser, dbpasswd)) {
		Warn("Failed to connect to database: Error: %s\n",
		     mysql_error(&mysql));
		return 1;
	}
	if (mysql_select_db(&mysql, "tech")) {
		Warn("cant select database: %s", mysql_error(&mysql));
		return 1;
	}
	asprintf(&query,
	 "SELECT ifaddr_d, ifaddr_pool FROM online WHERE login='%s'", name);
	mysql_query(&mysql, query);
	free(query);

	result = mysql_store_result(&mysql);

	if (mysql_num_rows(result) == 0) {
#ifdef CITYLINE
		Message("%s not found in database\n", name);
#endif
		goto nafig;
	}
	if ((row = mysql_fetch_row(result))) {
		addr = getaddr(&mysql, name, (char *) row[0], (char *) row[1]);
		DEBUG("assigned %s\n", addr);
		mysql_free_result(result);
		mysql_close(&mysql);
		return 0;
	} else {
		Warn("query returned 0 rows\n");
		goto nafig;
	}

nafig:
	mysql_free_result(result);
	mysql_close(&mysql);
	return 1;
}

#ifdef HAVEOWNMAIN

int
main(int argc, char **argv)
{
	struct procs    q;
    if (argc <2){
		printf("need a username\n");
		return 1;
		}
	mysql_auth(argv[1], "", &q, "195.9.214.8", "fil", "AAA");
	return 0;
}
#endif
