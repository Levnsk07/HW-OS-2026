#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    time_t now;
    struct tm *current_time;

#ifdef _WIN32
    _putenv_s("TZ", "PST8");
    _tzset();
#else
    setenv("TZ", "PST8", 1);
    tzset();
#endif

    time(&now);
    current_time = localtime(&now);

    printf("%04d-%02d-%02d %02d:%02d:%02d PST\n",
           current_time->tm_year + 1900,
           current_time->tm_mon + 1,
           current_time->tm_mday,
           current_time->tm_hour,
           current_time->tm_min,
           current_time->tm_sec);

    return 0;
}
