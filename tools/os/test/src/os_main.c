/*
 * os_main.c
 *
 *  Created on: Jan 6, 2017
 *      Author: erik
 */

#include "memtests.h"
#include "locktests.h"
#include "threadtests.h"

int main(void)
{
  frameworkInit();

  frameworkAddSuite(osMemSuite());
  frameworkAddSuite(osLockSuite());
  frameworkAddSuite(osThreadSuite());

  frameworkRun("OS_TESTS");

  return 0;
}
