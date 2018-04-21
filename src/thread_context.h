/**
   \file thread_context.h

   header file for thread context related code
 */

/*
** Copyright 2018 Benjamin J. Andre.
** All Rights Reserved.
**
** This Source Code Form is subject to the terms of the Mozilla
** Public License, v. 2.0. If a copy of the MPL was not distributed
** with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef RTES_THREAD_CONTEXT_H_
#define RTES_THREAD_CONTEXT_H_

#include <stdint.h>

typedef struct {
    int threadIdx;
    unsigned long long sequencePeriods;
} threadParams_t;


#endif /* RTES_THREAD_CONTEXT_H_ */
