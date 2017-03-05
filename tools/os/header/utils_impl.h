/*
 * utils_impl.h
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#ifndef OS_HEADER_UTILS_IMPL_H_
#define OS_HEADER_UTILS_IMPL_H_

#include <os/utilsIntf.h>

K_Status_e utils_sleep(uint32_t sec);
K_Status_e utils_usleep(uint32_t microseconds);
size_t utils_stringLength(const char_t * s);
K_Status_e utils_trim(const char_t * const src, char_t * dest, size_t size, OsTrimMode_e trimType);
K_Status_e utils_stringWrite(char_t * dest, size_t size, size_t * written, const char_t * format, ...);
K_Status_e utils_stringCopy(char_t ** dest, const char_t * const src);
K_Status_e utils_stringCmp(const char_t * const s1, const char_t * const s2, size_t n);
K_Status_e utils_stringFind(const char_t * haystack, const char_t * needle, size_t * pos);
K_Status_e utils_stringToInt(const char_t * nptr, int32_t base, int64_t * value);

K_Boolean_e utils_fileExists(const char_t * const path, size_t pathSize);
K_Status_e utils_openFile(OsFile_t * file, const char_t * const path, OsFileMode_e mode);
K_Status_e utils_closeFile(OsFile_t file);
K_Status_e utils_flushFile(OsFile_t file);
K_Status_e utils_readFromFile(OsFile_t file, void * data, size_t unitSize, size_t nbrItems, size_t * itemsRead);
K_Status_e utils_writeToFile(OsFile_t file, void * data, size_t unitSize, size_t nbrItems, size_t * itemsWritten);
K_Boolean_e utils_isEOF(OsFile_t file);

#endif /* OS_HEADER_UTILS_IMPL_H_ */
