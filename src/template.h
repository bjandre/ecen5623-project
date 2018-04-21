/**
   \file template.h

   Template header file
 */

/*
** Copyright 2018 Benjamin J. Andre.
** All Rights Reserved.
**
** This Source Code Form is subject to the terms of the Mozilla
** Public License, v. 2.0. If a copy of the MPL was not distributed
** with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#ifndef ESE_TEMPLATE_H_
#define ESE_TEMPLATE_H_

#include <stdint.h>

/**
   Template enumeration

 */
typedef enum template_enum_t_ {
    template_1,/*!< operation successful */
} template_enum_t;


/**
   Do something interesting

   \param[in,out] pointer to context

   \return status
 */
uint32_t template_func(void *context);


#endif /* ESE_TEMPLATE_H_ */
