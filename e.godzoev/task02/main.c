#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
extern char *tzname[];

int main(void)
{
    setenv("TZ", "PST8PDT", 1);
    time_t cal;
    struct tm *sp;

    (void) time( &cal );

    printf("%s", ctime( &cal ) );

    sp = localtime(&cal);
    printf("%d/%d/%02d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst - 1]);
    exit(0);
}