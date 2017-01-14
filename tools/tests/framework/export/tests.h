/*
 * tests.h
 *
 *  Created on: Jan 28, 2013
 *      Author: erik
 */

#ifndef TESTS_H_
#define TESTS_H_

#include <check/check.h>
#include <common/k_types.h>

#define ARRAY_SIZE(x)  (sizeof(x) / sizeof(x[0]))
#define ENV_RESULTS_DIR (const char * const) "RESULTS_DIR"

void frameworkInit(void);
K_Status_e frameworkAddSuite(Suite * const suite);
void frameworkRun(const char * const name);
TCase * frameworkCreateTestCase(const char * const name);
TCase * frameworkCreateValgrindTestCase(const char * const name);

#endif /* TESTS_H_ */
