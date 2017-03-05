/*
 * logging_suite.c
 *
 *  Created on: Jul 15, 2014
 *      Author: erik
 */

#include <check/check.h>
#include <testframework/tests.h>
#include <logging/logging.h>

#include "testhelper.h"

LogLevel_e localIsEnabledForTestData[] =
{
  LogLevelNoLog,
  LogLevelFatal,
  LogLevelError,
  LogLevelWarning,
  LogLevelInfo,
  LogLevelTrace,
  LogLevelVerbose,
};

START_TEST(test_isEnabledFor)
  {
    const LoggerIntf_t * const log = getLoggerIntf();
    LogLevel_e current = localIsEnabledForTestData[_i];
    log->setLevel(current);

    fail_unless(system_logLevel == current);

    int32_t i;

    for (i = 0; i <= LogLevelVerbose; i += 1000)
    {
      LogLevel_e tmp = (LogLevel_e) i;
      One_Boolean isEnabled = log->isEnabledFor(tmp);
      if ((tmp <= current) && (current != LogLevelNoLog))
      {
        fail_unless(isEnabled == One_TRUE);
      }
      else if (current == LogLevelNoLog)
      {
        fail_unless(isEnabled == One_FALSE);
      }
      else
      {
        fail_unless(isEnabled == One_FALSE);
      }
    }
  }
END_TEST

Suite * loggerSuite(void)
{
  Suite * s = suite_create("Logging tests");
  TCase * tc = frameworkCreateValgrindTestCase("Logging testcase");

  tcase_add_checked_fixture(tc, setupLoggerTests, teardownLoggerTests);
  tcase_set_timeout(tc, 300);

  tcase_add_loop_test(tc, test_isEnabledFor, 0, ARRAY_SIZE(localIsEnabledForTestData));
  suite_add_tcase(s, tc);

  return s;
}
