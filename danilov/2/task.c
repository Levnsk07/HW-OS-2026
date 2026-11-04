#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <stdio.h>

extern char *tzname[];

int main()
{
    time_t now;
    struct tm *sp;

    (void)time(&now);

    putenv("TZ=America/Los_Angeles");
    tzset();

    sp = localtime(&now);
    printf("%d/%d/%04d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
        sp->tm_hour, sp->tm_min,
        tzname[sp->tm_isdst]);

    putenv("TZ=PST8");
    tzset();

    sp = localtime(&now);
    printf("%d/%d/%04d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
        sp->tm_hour, sp->tm_min,
        tzname[sp->tm_isdst]);

    return 0;
}
