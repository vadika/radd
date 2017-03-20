#ifndef _STATS_H_
#define _STATS_H_
struct {
   int requests;
   int unk_clients;
   int duplicates;
   int accounts;
   int qsize;
   int  gc;
}STATS;

void sig_usr1(int);
#endif /* _STATS_H_ */
