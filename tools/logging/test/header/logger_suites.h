/*
 * logging_suites.h
 *
 *  Created on: Jul 15, 2014
 *      Author: erik
 */

#ifndef LOGGING_SUITES_H_
#define LOGGING_SUITES_H_

#include <check/check.h>

#define LOGGER_TESTS ((const char_t *) "LOGGER_TESTS")

Suite * loggerSuite(void);

#endif /* LOGGING_SUITES_H_ */
