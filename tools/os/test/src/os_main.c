/*
 * os_main.c
 *
 *  Created on: Jan 6, 2017
 *      Author: erik
 */

#include "memtests.h"

int main(void)
{
  frameworkInit();

  frameworkAddSuite(osMemSuite());

  frameworkRun("OS_MEMORY_TESTS");

  return 0;
}
