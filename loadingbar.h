#ifndef _LOADINGBAR_H_
#define _LOADINGBAR_H_

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

static inline size_t
__LoadingBar_terminalWidth();

static inline struct timespec
__LoadingBar_currentTime();

static inline struct timespec
__LoadingBar_timediff(struct timespec start, struct timespec end);

static inline long
__LoadingBar_nsec(struct timespec t);

static inline void
__LoadingBar_print(long remaining_nsec, long total_nsec, size_t width);


typedef struct{
    size_t terminalWidth;
    struct timespec previousTimeStamp;
    size_t previous;
    size_t total;
}LoadingBar;

static inline LoadingBar
LoadingBar_new(size_t total){
    return (LoadingBar){
        .terminalWidth = __LoadingBar_terminalWidth(),
        .previousTimeStamp = __LoadingBar_currentTime(),
        .previous = 0,
        .total = total,
    };
}

static inline void
LoadingBar_update(LoadingBar* lb, size_t current){
    struct timespec timediff = __LoadingBar_timediff(
            lb->previousTimeStamp,
            __LoadingBar_currentTime());
    size_t elapse = current - lb->previous;
    size_t remaining = lb->total - current;
    long expectedTotalTime = __LoadingBar_nsec(timediff) * 
        (float)lb->total / (elapse);
    long expectedRemainingTime = __LoadingBar_nsec(timediff)*
        (float)(remaining) / (elapse);

    __LoadingBar_print(
            expectedRemainingTime, expectedTotalTime, lb->terminalWidth);

    lb->previous = current;
    lb->previousTimeStamp = __LoadingBar_currentTime();
}



static inline size_t
__LoadingBar_terminalWidth(){
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

static inline struct timespec
__LoadingBar_currentTime(){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now;
}

static inline struct timespec
__LoadingBar_timediff(struct timespec start, struct timespec end){
    struct timespec temp;
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    return temp;
}

static inline long
__LoadingBar_nsec(struct timespec t) {
    return t.tv_sec * 1e9 + t.tv_nsec;
}

static inline void
__LoadingBar_print(long remaining_nsec, long total_nsec, size_t width){
    struct Time{
        long sec, min, hr;
    };
    struct Time remaining = {
        .sec = (long)((double)remaining_nsec / 1e9) % 60,
        .min = (long)((double)remaining_nsec / 1e9 / 60) % 60,
        .hr = (long)((double)remaining_nsec / 1e9 / 60 / 60),
    };
    struct Time total = {
        .sec = (long)(total_nsec / 1e9) % 60,
        .min = (long)(total_nsec / 1e9 / 60) % 60,
        .hr = (long)(total_nsec / 1e9 / 60 / 60),
    };

    char remaining_str[width+1];
    char total_str[width+1];
    sprintf(remaining_str, "%zuh%zum%zus", 
            remaining.hr, remaining.min, remaining.sec);
    sprintf(total_str, "%zuh%zum%zus", total.hr, total.min, total.sec);
    long totalBarLength = width - strlen(remaining_str) - 
        strlen(total_str) - 3;
    totalBarLength = totalBarLength < 0 ? 0 : totalBarLength;
    long barLength = (double)totalBarLength * 
        (total_nsec - remaining_nsec) / total_nsec;
    printf("\33[2K\r");
    putc('[', stdout);
    long i = 0;
    for (; i < barLength; i++){
        putc('=', stdout);
    }
    for (; i < totalBarLength; i++){
        putc(' ', stdout);
    }
    putc(']', stdout);
    printf("%s/%s", remaining_str, total_str);
    fflush(stdout);
}

#endif
