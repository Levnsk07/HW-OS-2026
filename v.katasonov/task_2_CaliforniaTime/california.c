#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main(void)
{
    time_t now;
    struct tm *tm_info;
    const time_t OFFSET = 15 * 3600;

    if (time(&now) == (time_t)-1) {
        perror("time");
        return 1;
    }

    now -= OFFSET;

    printf("%s", ctime(&now));

    tm_info = localtime(&now);
    if (!tm_info) {
        perror("localtime");
        return 1;
    }

    printf("%d/%d/%d %d:%02d %s\n",
           tm_info->tm_mon + 1,      
           tm_info->tm_mday,        
           tm_info->tm_year + 1900,  
           tm_info->tm_hour,
           tm_info->tm_min,
           (tm_info->tm_isdst >= 0) ? tzname[tm_info->tm_isdst] : "?");

    return 0;
}