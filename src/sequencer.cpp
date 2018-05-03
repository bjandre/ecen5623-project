/* ========================================================================== */
/*                                                                            */
// Sam Siewert, December 2017
//
// Sequencer Generic
//
// The purpose of this code is to provide an example for how to best
// sequence a set of periodic services for problems similar to and including
// the final project in real-time systems.
//
// For example: Service_1 for camera frame aquisition
//              Service_2 for image analysis and timestamping
//              Service_3 for image processing (difference images)
//              Service_4 for save time-stamped image to file service
//              Service_5 for save processed image to file service
//              Service_6 for send image to remote server to save copy
//              Service_7 for elapsed time in syslog each minute for debug
//
// At least two of the services need to be real-time and need to run on a single
// core or run without affinity on the SMP cores available to the Linux
// scheduler as a group.  All services can be real-time, but you could choose
// to make just the first 2 real-time and the others best effort.
//
// For the standard project, to time-stamp images at the 1 Hz rate with unique
// clock images (unique second hand / seconds) per image, you might use the
// following rates for each service:
//
// Sequencer - 30 Hz
//                   [gives semaphores to all other services]
// Service_1 - 3 Hz  , every 10th Sequencer loop
//                   [buffers 3 images per second]
// Service_2 - 1 Hz  , every 30th Sequencer loop
//                   [time-stamp middle sample image with cvPutText or header]
// Service_3 - 0.5 Hz, every 60th Sequencer loop
//                   [difference current and previous time stamped images]
// Service_4 - 1 Hz, every 30th Sequencer loop
//                   [save time stamped image with cvSaveImage or write()]
// Service_5 - 0.5 Hz, every 60th Sequencer loop
//                   [save difference image with cvSaveImage or write()]
// Service_6 - 1 Hz, every 30th Sequencer loop
//                   [write current time-stamped image to TCP socket server]
// Service_7 - 0.1 Hz, every 300th Sequencer loop
//                   [syslog the time for debug]
//
// With the above, priorities by RM policy would be:
//
// Sequencer = RT_MAX   @ 30 Hz
// Servcie_1 = RT_MAX-1 @ 3 Hz
// Service_2 = RT_MAX-2 @ 1 Hz
// Service_3 = RT_MAX-3 @ 0.5 Hz
// Service_4 = RT_MAX-2 @ 1 Hz
// Service_5 = RT_MAX-3 @ 0.5 Hz
// Service_6 = RT_MAX-2 @ 1 Hz
// Service_7 = RT_MIN   0.1 Hz
//
// Here are a few hardware/platform configuration settings on your Jetson
// that you should also check before running this code:
//
// 1) Check to ensure all your CPU cores on in an online state.
//
// 2) Check /sys/devices/system/cpu or do lscpu.
//
//    Tegra is normally configured to hot-plug CPU cores, so to make all
//    available, as root do:
//
//    echo 0 > /sys/devices/system/cpu/cpuquiet/tegra_cpuquiet/enable
//    echo 1 > /sys/devices/system/cpu/cpu1/online
//    echo 1 > /sys/devices/system/cpu/cpu2/online
//    echo 1 > /sys/devices/system/cpu/cpu3/online
//
// 3) Check for precision time resolution and support with cat /proc/timer_list
//
// 4) Ideally all printf calls should be eliminated as they can interfere with
//    timing.  They should be replaced with an in-memory event logger or at
//    least calls to syslog.
//
// 5) For simplicity, you can just allow Linux to dynamically load balance
//    threads to CPU cores (not set affinity) and as long as you have more
//    threads than you have cores, this is still an over-subscribed system
//    where RM policy is required over the set of cores.

// This is necessary for CPU affinity macros in Linux
// #define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <semaphore.h>

#include <syslog.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#include <errno.h>

#include "utils.hpp"
#include "constants.hpp"
#include "globals.hpp"

#include "thread_context.hpp"

#include "plog.hpp"

static const int MAX_MSG_LEN = 1024;
static const bool debug = false;

extern int abortS1;
extern sem_t semS1;
extern int abortS2;
extern sem_t semS2;
extern int abortS3;
extern sem_t semS3;
extern struct timeval start_time_val;

int abortTest = false;

void *sequencer(void *context)
{
    struct timeval current_time_val;
    struct timespec delay_time = {0, 33333333}; // delay for 33.33 msec, 30 Hz
    struct timespec remaining_time;
    double residual;
    int rc, delay_cnt = 0;
    unsigned long long seqCnt = 0;
    threadParams_t *threadParams = (threadParams_t *)context;

    plog_buffer_t buff;
    plog_t *curr;

    initPlogBuff(10000, &buff);

    char message[MAX_MSG_LEN];

    gettimeofday(&current_time_val, (struct timezone *)0);
    syslog(LOG_CRIT, "Sequencer thread @ sec=%d, msec=%d\n",
           (int)(current_time_val.tv_sec - start_time_val.tv_sec),
           (int)current_time_val.tv_usec / USEC_PER_MSEC);
    printf("Sequencer thread @ sec=%d, msec=%d\n",
           (int)(current_time_val.tv_sec - start_time_val.tv_sec),
           (int)current_time_val.tv_usec / USEC_PER_MSEC);

    do {
        delay_cnt = 0;
        residual = 0.0;

        gettimeofday(&current_time_val, (struct timezone *)0);
        snprintf(message, MAX_MSG_LEN,
                 "Sequencer thread prior to delay @ sec=%d, msec=%d\n",
                 (int)(current_time_val.tv_sec - start_time_val.tv_sec),
                 (int)current_time_val.tv_usec / USEC_PER_MSEC);
        syslog(LOG_CRIT, "%s", message);
        if (debug) {
            printf("%s", message);
        }
        do {
            rc = nanosleep(&delay_time, &remaining_time);

            if (rc == EINTR) {
                residual = remaining_time.tv_sec + ((double)remaining_time.tv_nsec /
                                                    (double)NANOSEC_PER_SEC);

                if (residual > 0.0) {
                    printf("residual=%lf, sec=%d, nsec=%d\n", residual, (int)remaining_time.tv_sec,
                           (int)remaining_time.tv_nsec);
                }

                delay_cnt++;
            } else if (rc < 0) {
                perror("Sequencer nanosleep");
                exit(-1);
            }

        } while ((residual > 0.0) && (delay_cnt < 100));

        getStartPlog(&buff, &curr, 0);

        seqCnt++;
        gettimeofday(&current_time_val, (struct timezone *)0);
        snprintf(message, MAX_MSG_LEN, "Sequencer cycle %llu @ sec=%d, msec=%d\n",
                 seqCnt,
                 (int)(current_time_val.tv_sec - start_time_val.tv_sec),
                 (int)current_time_val.tv_usec / USEC_PER_MSEC);
        if (debug) {
            printf("%s", message);
        }

        if (delay_cnt > 1) {
            printf("Sequencer looping delay %d\n", delay_cnt);
        }


        // Release each service at a sub-rate of the generic sequencer rate

        // Servcie_1 = RT_MAX-1 @ 3 Hz
        if ((seqCnt % 3) == 0) {
            sem_post(&semS1);
        }

        // Servcie_1 = RT_MAX-1 @ 3 Hz
        if ((seqCnt % 4) == 0) {
            sem_post(&semS2);
        }

        // Servcie_1 = RT_MAX-1 @ 3 Hz
        if ((seqCnt % 5) == 0) {
            sem_post(&semS3);
        }

        gettimeofday(&current_time_val, (struct timezone *)0);
        snprintf(message, MAX_MSG_LEN,
                 "Sequencer release all sub-services @ sec=%d, msec=%d\n",
                 (int)(current_time_val.tv_sec - start_time_val.tv_sec),
                 (int)current_time_val.tv_usec / USEC_PER_MSEC);
        if (debug) {
            printf("%s", message);
        }

        endPlog(curr);

    } while (!abortTest && (seqCnt < threadParams->sequencePeriods));

    abortS1 = true;
    sem_post(&semS1);
    abortS2 = true;
    sem_post(&semS2);
    abortS3 = true;
    sem_post(&semS3);

    csvAppendPlogBuff(&buff, "results.csv");

    pthread_exit((void *)0);
}
