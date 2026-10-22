#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main()
{
    time_t now;
    struct tm *sp;

    /* Force California time, no DST (always UTC-8) */
    putenv("TZ=PST8");
    tzset();

    (void) time(&now);

    sp = localtime(&now);

    printf("California time (PST):\n");
    printf("%d/%d/%04d %d:%02d\n",
        sp->tm_mon + 1,        /* month: 0-11 → 1-12 */
        sp->tm_mday,           /* day: 1-31 */
        sp->tm_year + 1900,    /* year: since 1900 → full year */
        sp->tm_hour,           /* hour: 0-23 */
        sp->tm_min);           /* minute: 0-59 */

    return 0;
}
