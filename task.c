#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>

void timer_fast_callback(union sigval sv) {
    FILE *fp = fopen("timer_fast.log", "a");
    if (fp) {
        time_t now = time(NULL);
        fprintf(fp, "[FAST TIMER] Tick at %s", ctime(&now));
        fclose(fp);
    }
}

void timer_slow_callback(union sigval sv) {
    FILE *fp = fopen("timer_slow.log", "a");
    if (fp) {
        time_t now = time(NULL);
        fprintf(fp, "[SLOW TIMER] Tick at %s", ctime(&now));
        fclose(fp);
    }
}

int main() {
    timer_t timer_id_fast;
    timer_t timer_id_slow;

    struct sigevent sev_fast;
    sev_fast.sigev_notify = SIGEV_THREAD;
    sev_fast.sigev_notify_function = timer_fast_callback;
    sev_fast.sigev_value.sival_ptr = NULL;
    sev_fast.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_REALTIME, &sev_fast, &timer_id_fast) == -1) {
        perror("Failed to create fast timer");
        return 1;
    }

    struct sigevent sev_slow;
    sev_slow.sigev_notify = SIGEV_THREAD;
    sev_slow.sigev_notify_function = timer_slow_callback;
    sev_slow.sigev_value.sival_ptr = NULL;
    sev_slow.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_REALTIME, &sev_slow, &timer_id_slow) == -1) {
        perror("Failed to create slow timer");
        return 1;
    }

    struct itimerspec its_fast;
    its_fast.it_value.tv_sec = 1;
    its_fast.it_value.tv_nsec = 0;
    its_fast.it_interval.tv_sec = 1;
    its_fast.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id_fast, 0, &its_fast, NULL) == -1) {
        perror("Failed to start fast timer");
        return 1;
    }

    struct itimerspec its_slow;
    its_slow.it_value.tv_sec = 3;
    its_slow.it_value.tv_nsec = 0;
    its_slow.it_interval.tv_sec = 3;
    its_slow.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id_slow, 0, &its_slow, NULL) == -1) {
        perror("Failed to start slow timer");
        return 1;
    }

    printf("[*] Both POSIX timers started successfully.\n");
    printf("[*] Logging to 'timer_fast.log' (every 1s) and 'timer_slow.log' (every 3s).\n");
    printf("[*] Main thread entering idle loop. Press Ctrl+C to exit.\n");

    while (1) {
        sleep(1);
    }

    return 0;
}
