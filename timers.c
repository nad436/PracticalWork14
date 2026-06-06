#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

struct timer_config {
    const char *filename;
    const char *prefix;
};

void timer_callback(union sigval sv) {
    struct timer_config *config = (struct timer_config *)sv.sival_ptr;
    
    pthread_mutex_lock(&log_mutex);
    FILE *fp = fopen(config->filename, "a");
    if (fp) {
        time_t now = time(NULL);
        fprintf(fp, "[%s] Tick at %s", config->prefix, ctime(&now));
        fclose(fp);
    }
    pthread_mutex_unlock(&log_mutex);
}

int main() {
    timer_t timer_id_fast;
    timer_t timer_id_slow;

    struct timer_config fast_cfg = {"timer_fast.log", "FAST TIMER"};
    struct timer_config slow_cfg = {"timer_slow.log", "SLOW TIMER"};

    struct sigevent sev_fast;
    sev_fast.sigev_notify = SIGEV_THREAD;
    sev_fast.sigev_notify_function = timer_callback;
    sev_fast.sigev_value.sival_ptr = &fast_cfg;
    sev_fast.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_REALTIME, &sev_fast, &timer_id_fast) == -1) {
        perror("Failed to create fast timer");
        return 1;
    }

    struct sigevent sev_slow;
    sev_slow.sigev_notify = SIGEV_THREAD;
    sev_slow.sigev_notify_function = timer_callback;
    sev_slow.sigev_value.sival_ptr = &slow_cfg;
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

    while (1) {
        sleep(1);
    }

    return 0;
}
