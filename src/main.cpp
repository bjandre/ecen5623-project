//
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

#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <iostream>

#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <semaphore.h>

#include <syslog.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#include <errno.h>

#include <opencv2/opencv.hpp>
#include "gameutil.hpp"
#include "gameobjects.hpp"

using namespace cv;

#include "utils.hpp"
#include "constants.hpp"
#include "globals.hpp"

#include "thread_context.hpp"
#include "sequencer.hpp"

#define NUM_WORK_THREADS (1)
#define NUM_THREADS (NUM_WORK_THREADS + 1)

extern int abortS1;
extern sem_t semS1;
extern struct timeval start_time_val;

void *Service_1(void *threadp);


int main(int argc, char **argv)
{
    struct timeval current_time_val;
    int rc, scope;
    cpu_set_t threadcpu;
    pthread_t threads[NUM_THREADS];
    threadParams_t threadParams[NUM_THREADS];
    pthread_attr_t rt_sched_attr[NUM_THREADS];
    int rt_max_prio, rt_min_prio;
    struct sched_param rt_param[NUM_THREADS];
    struct sched_param main_param;
    pthread_attr_t main_attr;
    pid_t mainpid;
    cpu_set_t allcpuset;

    std::cout << "red laser pointer cursor game" << std::endl;

    gettimeofday(&start_time_val, (struct timezone *)0);
    gettimeofday(&current_time_val, (struct timezone *)0);
    syslog(LOG_CRIT, "Sequencer @ sec=%d, msec=%d\n",
           (int)(current_time_val.tv_sec - start_time_val.tv_sec),
           (int)current_time_val.tv_usec / USEC_PER_MSEC);

    printf("System has %d processors configured and %d available.\n",
           get_nprocs_conf(), get_nprocs());

    CPU_ZERO(&allcpuset);

    for (uint32_t i = 0; i < NUM_CPU_CORES; i++) {
        CPU_SET(i, &allcpuset);
    }

    printf("Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));


    // initialize the sequencer semaphores
    //
    if (sem_init (&semS1, 0, 0)) {
        printf ("Failed to initialize S1 semaphore\n");
        exit (-1);
    }

    mainpid = getpid();

    rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    rt_min_prio = sched_get_priority_min(SCHED_FIFO);

    rc = sched_getparam(mainpid, &main_param);
    main_param.sched_priority = rt_max_prio;
    rc = sched_setscheduler(mainpid, SCHED_FIFO, &main_param);
    if (rc < 0) {
        printf("ERROR: can not set SCHED_FIFO. Did you run with sudo?\n");
        perror("main_param");
    }
    print_scheduler();


    pthread_attr_getscope(&main_attr, &scope);

    if (scope == PTHREAD_SCOPE_SYSTEM) {
        printf("PTHREAD SCOPE SYSTEM\n");
    } else if (scope == PTHREAD_SCOPE_PROCESS) {
        printf("PTHREAD SCOPE PROCESS\n");
    } else {
        printf("PTHREAD SCOPE UNKNOWN\n");
    }

    printf("rt_max_prio=%d\n", rt_max_prio);
    printf("rt_min_prio=%d\n", rt_min_prio);

    for (uint32_t i = 0; i < NUM_THREADS; i++) {

        CPU_ZERO(&threadcpu);
        CPU_SET(3, &threadcpu);

        rc = pthread_attr_init(&rt_sched_attr[i]);
        rc = pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
        rc = pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
        //rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

        rt_param[i].sched_priority = rt_max_prio - i;
        pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);

        threadParams[i].threadIdx = i;
    }

    printf("Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));

    // Create Service threads which will block awaiting release for:
    //

    // Servcie_1 = RT_MAX-1 @ 3 Hz
    //
    rt_param[1].sched_priority = rt_max_prio - 1;
    pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
    rc = pthread_create(&threads[1],             // pointer to thread descriptor
                        &rt_sched_attr[1],         // use specific attributes
                        //(void *)0,               // default attributes
                        Service_1,                 // thread function entry point
                        (void *) & (threadParams[1]) // parameters to pass in
                       );
    if (rc < 0) {
        perror("pthread_create for service 1");
    } else {
        printf("pthread_create successful for service 1\n");
    }


    // Wait for service threads to initialize and await release by sequencer.
    //
    // Note that the sleep is not necessary of RT service threads are created wtih
    // correct POSIX SCHED_FIFO priorities compared to non-RT priority of this main
    // program.
    //
    // usleep(1000000);

    // Create Sequencer thread, which like a cyclic executive, is highest prio
    printf("Start sequencer\n");
    threadParams[0].sequencePeriods = 9000;

    // Sequencer = RT_MAX   @ 30 Hz
    //
    rt_param[0].sched_priority = rt_max_prio;
    pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
    rc = pthread_create(&threads[0], &rt_sched_attr[0], sequencer,
                        (void *) & (threadParams[0]));
    if (rc < 0) {
        perror("pthread_create for sequencer service 0");
    } else {
        printf("pthread_create successful for sequeencer service 0\n");
    }


    for (uint32_t i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nTEST COMPLETE\n");
}

void *Service_1(void *threadp)
{
    struct timeval current_time_val;
    unsigned long long S1Cnt = 0;

    gettimeofday(&current_time_val, (struct timezone *)0);
    syslog(LOG_CRIT, "Frame Sampler thread @ sec=%d, msec=%d\n",
           (int)(current_time_val.tv_sec - start_time_val.tv_sec),
           (int)current_time_val.tv_usec / USEC_PER_MSEC);
    printf("Frame Sampler thread @ sec=%d, msec=%d\n",
           (int)(current_time_val.tv_sec - start_time_val.tv_sec),
           (int)current_time_val.tv_usec / USEC_PER_MSEC);

    Mat src;
    VideoCapture cap;

    init_camera(&cap, 640, 480);
    Goal goal(Point(75, 75) , 40);
    Obstacle o(Point(165, 55) , 40, Point(-2, 0));
    cvNamedWindow("Video");

    cap >> src;
    Mat bgr[3];
    split(src, bgr);
    Mat acc = Mat::zeros(bgr[2].size(), CV_32FC1);
    Mat accScaled, sub;


    int i = 0;
    while (!abortS1) {
        sem_wait(&semS1);
        S1Cnt++;
	i++;
        cap >> src;
        split(src, bgr);
        Mat red = bgr[2];
        //write_ui(red, i++);
        //update the background model
        accumulateWeighted(red, acc, 0.1);

        // Scale it to 8-bit unsigned
        convertScaleAbs(acc, accScaled);
        absdiff(red, accScaled, sub);

        Mat red_threshold;
        int threshold_type_binary = THRESH_BINARY;
        int max_binary_value = 255;
        int threshold_cutoff = 220;
        threshold(sub, red_threshold, threshold_cutoff, max_binary_value, threshold_type_binary);
        //write_ui(sub, i++);


        goal.draw(sub);

        o.move();
        o.draw(sub);

        if (detect_collision(goal, o)) {
            putText(src, "Collision!", Point(40, 40), FONT_HERSHEY_COMPLEX_SMALL, 5,
                    Scalar(100, 100, 100), 1, CV_AA);
        }

        imshow("Video", sub);
        int c = cvWaitKey(10);
        //If 'ESC' is pressed, break the loop
        if ((char)c == 27 ) {
            break;
        }



        gettimeofday(&current_time_val, (struct timezone *)0);
        syslog(LOG_CRIT, "Frame Sampler release %llu @ sec=%d, msec=%d\n", S1Cnt,
               (int)(current_time_val.tv_sec - start_time_val.tv_sec),
               (int)current_time_val.tv_usec / USEC_PER_MSEC);
    }

    cvDestroyAllWindows();
    pthread_exit((void *)0);
}


