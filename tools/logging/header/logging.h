/*
 * logging.h
 *
 *  Created on: Sep 10, 2013
 *      Author: erik
 */

#ifndef I_LOGGING_H_
#define I_LOGGING_H_

#include <common/k_types.h>
#include <logging/logging.h>

K_Status_e log_init(const char_t * const name);
void log_shutdown(void);
K_Status_e log_setLevel(LogLevel_e level);
K_Boolean_e log_isEnabledFor(LogLevel_e);

#endif /* I_LOGGING_H_ */
