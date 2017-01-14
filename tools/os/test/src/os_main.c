/*
 * os_main.c
 *
 *  Created on: Jan 6, 2017
 *      Author: erik
 */

#include "memtests.h"
#include "locktests.h"

int main(void)
{
  frameworkInit();

  frameworkAddSuite(osMemSuite());
  frameworkAddSuite(osLockSuite());

  frameworkRun("OS_TESTS");

  return 0;
}
