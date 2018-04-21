/**
   \file utils.h

   Utils header file
 */

/*
** Copyright 2018 Benjamin J. Andre.
** All Rights Reserved.
**
** This Source Code Form is subject to the terms of the Mozilla
** Public License, v. 2.0. If a copy of the MPL was not distributed
** with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef RTES_UTILS_H_
#define RTES_UTILS_H_

#include <stdint.h>

/**
   Do something interesting

   \param[in,out] pointer to context

   \return status
 */
double getTimeMsec(void);

/**
   Do something interesting

   \param[in,out] pointer to context

   \return status
 */
void print_scheduler(void);


#endif /* RTES_UTILS_H_ */
