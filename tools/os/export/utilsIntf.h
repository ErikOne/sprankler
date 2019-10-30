/*
 * utilsIntf.h
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#ifndef OS_EXPORT_UTILSINTF_H_
#define OS_EXPORT_UTILSINTF_H_

#include <common/k_types.h>

typedef enum
{
  TRIM_FRONT = 1,
  TRIM_BACK = 2,
  TRIM_FRONT_BACK = 3,
} OsTrimMode_e;

typedef unsigned long int OsFile_t;

typedef enum
{
  F_READONLY = 1,
  F_READWRITE = 2,
  F_WRITE_CREATE = 3,
  F_READWRITE_CREATE = 4,
  F_WRITE_APPEND = 5,
  F_READWRITE_APPEND = 6
} OsFileMode_e;

typedef struct _ostime
{
  uint32_t sec;
  uint32_t nano_sec;
} OsTime_t;

typedef struct _utils_interface
{
  K_Status_e (* sleep)(uint32_t seconds);

  K_Status_e (* usleep)(uint32_t microseconds);

  K_Status_e (* getTimeOfDay)(OsTime_t * time);

  size_t (* stringLength)( const char_t *const string);

  K_Status_e (* stringCopy)(char_t ** dest, const char_t * const src);

  K_Status_e (* stringCmp)(const char_t * const s1, const char_t * const s2, size_t n);

  K_Status_e (* stringFind)(const char_t * const haystack, const char_t * const needle, size_t * pos);

  K_Status_e (* stringWrite)(char_t * dest, size_t size, size_t * written, const char_t * format, ...);

  K_Status_e (* trim)(const char_t * const src, char_t * dest, size_t size, OsTrimMode_e mode);

  K_Status_e (* stringToInt)(const char_t * nptr, int32_t base, int64_t * value);

  K_Status_e (* openFile)(OsFile_t * file, const char_t * const path, OsFileMode_e mode);

  K_Status_e (* closeFile)(OsFile_t file);

  K_Status_e (* flushFile)(OsFile_t file);

  K_Boolean_e (* isEOF)(OsFile_t file);

  /*!
   * \brief reads nbrOfItems each of size unitSize into data.
   *
   * If the file reaches EOF, than itemsRead is set to the number of items that was still read,
   * and One_Success is returned. So after less items than requested no more read requests should
   * be done.
   */
  K_Status_e (* readFromFile)(OsFile_t file, void * data, size_t unitSize, size_t nbrItems, size_t * itemsRead);

  K_Status_e (* writeToFile)(OsFile_t file, void * data, size_t unitSize, size_t nbrItems, size_t * itemsWritten);

  K_Boolean_e (* fileExists)(const char_t * const path, size_t pathSize);

} IUtils_t;

IUtils_t * getUtilsIntf(void);

#endif /* OS_EXPORT_UTILSINTF_H_ */
