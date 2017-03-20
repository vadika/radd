/*
	$Id$ 
	$Log$
*/

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <db.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* malloc wrapper */

void *_xmalloc(size_t size, char *file, int line)
{ 
	DB *mdb;
	DBT data, key;
	char path[48];
	char out[128];
	void *ret;

	sprintf(path, "/tmp/Xmalloc%d.db", getpid());
	mdb=dbopen(path,O_RDWR|O_CREAT|O_SHLOCK,0664,DB_HASH,NULL);

	ret=malloc(size);

	if(mdb == NULL || ret == NULL)
		return NULL;

	sprintf(out,"%s:%d(%d)M",file,line,size);			
    key.data = &ret;
	key.size = sizeof(void *);		
	data.data = out;
	data.size = strlen(out);
	
	(mdb->put)(mdb, &key, &data, 0);

	(mdb->close)(mdb);
	return ret;
}

void *_xcalloc(size_t number,size_t size, char *file, int line )
{return NULL;}

void *_xrealloc(void *ptr, size_t size, char *file, int line)
{
 return NULL;
}

void _xfree(void *ptr, char *file, int line)
{
	DB *mdb;
	DBT key;
	char path[48];

	sprintf(path, "/tmp/Xmalloc%d.db", getpid());
	mdb=dbopen(path,O_RDWR|O_CREAT|O_SHLOCK,0664,DB_HASH,NULL);

	free(ptr);

	if(mdb == NULL)
		return;

    key.data = &ptr;
	key.size = sizeof(void *);		

   	(mdb->del)(mdb,&key,0); 
	(mdb->close)(mdb);
}


