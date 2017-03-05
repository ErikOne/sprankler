/*
 * utils_intf.c
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#include "utils_impl.h"

static IUtils_t localIntf =
{
#ifndef UNITTESTS
  .sleep        = utils_sleep,
  .usleep       = utils_usleep,
  .getTimeOfDay = utils_gettimeofday,
  .stringLength = utils_stringLength,
  .stringCopy   = utils_stringCopy,
  .stringCmp    = utils_stringCmp,
  .stringFind   = utils_stringFind,
  .stringWrite  = utils_stringWrite,
  .trim         = utils_trim,
  .stringToInt  = utils_stringToInt,
  .openFile     = utils_openFile,
  .closeFile    = utils_closeFile,
  .flushFile    = utils_flushFile,
  .isEOF        = utils_isEOF,
  .readFromFile = utils_readFromFile,
  .writeToFile  = utils_writeToFile,
  .fileExists   = utils_fileExists
#endif
};

IUtils_t * getUtilsIntf(void)
{
  return &localIntf;
}
