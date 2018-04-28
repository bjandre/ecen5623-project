
/*
** Copyright 2018 Benjamin J. Andre.
** All Rights Reserved.
**
** This Source Code Form is subject to the terms of the Mozilla
** Public License, v. 2.0. If a copy of the MPL was not distributed
** with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

// #define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>

#include <semaphore.h>

#include "globals.hpp"

// FIXME(bja, 2018-04) these need to be protected. should probably be moved into
// modules for each service.

int abortS1 = false;
sem_t semS1;
struct timeval start_time_val;

/**

 */
void private_function(void *context);

uint32_t template_func(void *context)
{
    return 0u;
}

void private_func(void *context)
{
    (void)context;
}

