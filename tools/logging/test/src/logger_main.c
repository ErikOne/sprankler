/*
 * beo_main.c
 *
 *  Created on: Jul 7, 2014
 *      Author: erik
 */

#include <check/check.h>
#include <testframework/tests.h>

#include "logger_suites.h"

int main(int argc, char ** argv)
{
  frameworkInit();

  frameworkAddSuite(loggerSuite());

  frameworkRun(LOGGER_TESTS);

  return 0;
}
