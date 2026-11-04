#define _POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main() {
    time_t now;
    struct tm *sp;
    setenv("TZ", "PST8PDT", 1);
    void tzset(void);
    (void) time( &now );

    printf("%s", ctime( &now ) );
   
    sp = localtime(&now);
    sp->tm_hour -= 1;

    mktime(sp);

    printf("%d/%d/%02d %d:%02d %s\n",
         sp->tm_mon + 1, sp->tm_mday,
         sp->tm_year + 1900, sp->tm_hour,
         sp->tm_min, tzname[sp->tm_isdst-1]);
     exit(0);
}
