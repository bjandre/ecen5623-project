/**
   \file globals.h

   Globals header file
 */

/*
** Copyright 2018 Benjamin J. Andre.
** All Rights Reserved.
**
** This Source Code Form is subject to the terms of the Mozilla
** Public License, v. 2.0. If a copy of the MPL was not distributed
** with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef RTES_GLOBALS_H_
#define RTES_GLOBALS_H_

#include <stdint.h>

/**
   Globals enumeration

 */
typedef enum globals_enum_t_ {
    globals_1,/*!< operation successful */
} globals_enum_t;


/**
   Do something interesting

   \param[in,out] pointer to context

   \return status
 */
uint32_t globals_func(void *context);


#endif /* RTES_GLOBALS_H_ */
