#include <stdio.h>
#include "stats.h"
#include "log.h"

FILE *statfile=NULL;

void stats_init(char *statfname)
{
   if((statfile=fopen(statfname, "a+"))==NULL)
	  Warn("can't open %s", statfname);
   STATS.requests=
   STATS.unk_clients=
   STATS.duplicates=
   STATS.accounts=
   STATS.gc=
   STATS.qsize=0;

}

void sig_usr1(int unused)
{
   if(statfile)
   fprintf(statfile, "\nrequests=%d\nduplicates=%d\nunk_clients=%d\naccounts=%d\nqsize=%d\ngarbage=%d\n",
		STATS.requests,
		STATS.duplicates,
		STATS.unk_clients,
		STATS.accounts,
		STATS.qsize,
		STATS.gc);
	fflush(statfile);
}
