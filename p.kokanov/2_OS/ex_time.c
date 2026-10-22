#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char *tzname[];

int main() {
        setenv("TZ", "PST8PST", 1);
        tzset();

        time_t now;
        struct tm *sp;

        (void) time(&now);

	char *time_str = ctime(&now);
	time_str[strcspn(time_str, "\n")] = '\0';

        printf("%s PDT\n", time_str);

        sp = localtime(&now);
	sp->tm_hour -= 1;

	mktime(sp);

        printf("%d/%d/%02d %d:%02d %s\n",
                sp->tm_mday, sp->tm_mon + 1,
                sp->tm_year + 1900, sp->tm_hour,
                sp->tm_min, tzname[sp->tm_isdst]);

        return 0;
}
