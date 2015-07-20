#ifndef __TIME_H__
#define __TIME_H__

#include <sys/time.h>

extern struct tm* localtime(const time_t* t);
extern time_t mktime(register struct tm* const t);
extern char *ctime(const time_t *timep);

extern int gettimeofday(struct timeval *tp, void *ignore);

#endif
