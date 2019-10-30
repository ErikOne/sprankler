/*
 * utils.c
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#include "utils_impl.h"

#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <os/memIntf.h>

static const char_t * localOsModeToSysMode(OsFileMode_e mode);
static K_Boolean_e localIsWhiteSpace(char_t character);

K_Status_e utils_sleep(uint32_t sec)
{
  K_Status_e rc = K_Status_Invalid_Param;
  rc = (sleep(sec) == 0)?K_Status_OK:K_Status_General_Error;

  return rc;
}

K_Status_e utils_usleep(uint32_t microseconds)
{
  K_Status_e rc = K_Status_Invalid_Param;
  if (microseconds < 1000000)
  {
    rc = (usleep(microseconds) == 0)?K_Status_OK:K_Status_General_Error;
  }

  return rc;
}

size_t utils_stringLength(const char_t * const s)
{
  size_t len = 0;

  if (s != NULL)
  {
    len = strlen(s);
  }

  return len;
}

K_Status_e utils_trim(const char_t * const src, char_t * dest, size_t size, OsTrimMode_e trimType)
{
  K_Status_e rc = K_Status_General_Error;

  if ((src != NULL) && (dest != NULL))
  {
    char_t * tmp;
    const char_t * start = src;
    switch (trimType)
    {
      case TRIM_FRONT:
      case TRIM_FRONT_BACK:
        for (; (*start != '\0') && (localIsWhiteSpace(*start) == K_True); start++) ;
        break;
      default:
        break;
    }

    if (utils_stringWrite(dest, size, NULL, "%s", start) == K_Status_OK)
    {
      size_t length = strlen(dest);
      if (length > 0)
      {
        switch (trimType)
        {
          case TRIM_BACK:
          case TRIM_FRONT_BACK:
            for (tmp = dest + length - 1; (tmp != dest) && (localIsWhiteSpace(*tmp) == K_True); tmp--)
            {
              *tmp = '\0';
            }
            rc = K_Status_OK;
            break;
          default:
            rc = K_Status_OK;
            break;
        }
      }
      else
      {
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}

K_Status_e utils_stringWrite(char_t * dest, size_t size, size_t * written, const char_t * format, ...)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dest != NULL) && (size > 0))
  {
    va_list arglist;
    va_start(arglist, format);
    int len = vsnprintf(dest, size, format, arglist);
    va_end(arglist);

    if (len < (int32_t) size)
    {
      if (written != NULL)
      {
        *written = len;
      }
      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Status_e utils_stringCopy(char_t ** dest, const char_t * const src)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dest != NULL) && (src != NULL))
  {
    size_t len = strlen((char *) src) + 1;

    *dest = (char_t *) getMemIntf()->malloc(len);
    if (*dest != NULL)
    {
      if (strncpy((char *) *dest, (char *) src, len) == (char *) *dest)
      {
        rc = K_Status_OK;
      }

    }
  }

  return rc;
}

K_Status_e utils_stringCmp(const char_t * const s1, const char_t * const s2, size_t n)
{
  K_Status_e rc = K_Status_General_Error;

  if (s1 != NULL && s2 != NULL)
  {
    if (n > 0)
    {
      if (strncmp((char *) s1, (char *) s2, n) == 0)
      {
        rc = K_Status_OK;
      }
    }
    else
    {
      if (strcmp((char *) s1, (char *) s2) == 0)
      {
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}

K_Status_e utils_stringFind(const char_t * haystack, const char_t * needle, size_t * pos)
{
  K_Status_e rc = K_Status_General_Error;

  if ((haystack != NULL) && (needle != NULL) && (pos != NULL))
  {
    const char_t * tmp;
    if ((tmp = strstr(haystack, needle)) != NULL)
    {
      *pos = tmp - (const char_t *) haystack;
      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Status_e utils_stringToInt(const char_t * nptr, int32_t base, int64_t * value)
{
  K_Status_e status = K_Status_General_Error;

  if ((nptr != NULL) && (value != NULL))
  {
    if (strlen(nptr) > 0)
    {
      errno = 0;
      char_t * endptr;
      *value = strtoll(nptr, &endptr, base);

      if ((errno == 0) && (*endptr == '\0'))
      {
        status = K_Status_OK;
      }
    }
  }

  return status;
}

K_Boolean_e utils_fileExists(const char_t * const path, size_t pathSize)
{
  K_Boolean_e exists = K_False;

  /* I'm in os abstraction so I can use strlen without any issue */
  if ((path != NULL) && (strnlen(path, pathSize) > 0))
  {
    struct stat pathInfo;
    if (stat(path, &pathInfo) == 0)
    {
      if (S_ISREG(pathInfo.st_mode) == 1)
      {
        exists = K_True;
      }
    }
  }

  return exists;
}

K_Status_e utils_openFile(OsFile_t * file, const char_t * const path, OsFileMode_e mode)
{
  K_Status_e rc = K_Status_General_Error;
  if ((file != NULL) && (path != NULL))
  {
    const char_t * sysmode = localOsModeToSysMode(mode);

    if (sysmode != NULL)
    {
      FILE * newFile = fopen(path, sysmode);
      if (newFile != NULL)
      {
        *file = (OsFile_t) newFile;
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}

K_Status_e utils_closeFile(OsFile_t file)
{
  K_Status_e rc = K_Status_General_Error;

  FILE * impl = (FILE *) file;
  if (impl != NULL)
  {
    if (fclose(impl) == 0)
    {
      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Status_e utils_flushFile(OsFile_t file)
{
  K_Status_e rc = K_Status_General_Error;

  FILE * impl = (FILE *) file;
  if (impl != NULL)
  {
    if (fflush(impl) == 0)
    {
      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Status_e utils_readFromFile(OsFile_t file, void * data, size_t unitSize, size_t nbrItems, size_t * itemsRead)
{
  K_Status_e rc = K_Status_General_Error;

  FILE * impl = (FILE *) file;
  if (impl != NULL)
  {
    if (data != NULL)
    {
      size_t result = fread(data, unitSize, nbrItems, impl);
      if (itemsRead != NULL)
      {
        *itemsRead = result;
      }
      if (nbrItems == result)
      {
        rc = K_Status_OK;
      }
      else if (feof(impl) != 0)
      {
        rc = K_Status_OK;
      }
      else
      {
        rc = K_Status_General_Error;
      }
    }
  }

  return rc;
}

K_Status_e utils_writeToFile(OsFile_t file, void * data, size_t unitSize, size_t nbrItems, size_t * itemsWritten)
{
  K_Status_e rc = K_Status_General_Error;

  FILE * impl = (FILE *) file;
  if (impl != NULL)
  {
    if (data != NULL)
    {
      size_t result = fwrite(data, unitSize, nbrItems, impl);
      if (itemsWritten != NULL)
      {
        *itemsWritten = result;
      }

      if (nbrItems == result)
      {
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}

K_Boolean_e utils_isEOF(OsFile_t file)
{
  K_Boolean_e eof = K_True;
  FILE * impl = (FILE *) file;
  if (impl != NULL)
  {
    eof = (feof(impl) == 0)?K_False:K_True;
  }

  return eof;
}

K_Status_e utils_gettimeofday(OsTime_t * now)
{
  K_Status_e rc = K_Status_General_Error;

  if (now != NULL)
  {
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t) == 0)
    {
      now->sec = t.tv_sec;
      now->nano_sec = t.tv_nsec;

      rc = K_Status_OK;
    }
  }

  return rc;
}

static const char_t * localOsModeToSysMode(OsFileMode_e mode)
{
  const char_t * sysmode;

  switch (mode)
  {
    case F_READONLY:
      sysmode = "r";
      break;
    case F_READWRITE:
      sysmode = "r+";
      break;
    case F_WRITE_CREATE:
      sysmode = "w";
      break;
    case F_READWRITE_CREATE:
      sysmode = "w+";
      break;
    case F_WRITE_APPEND:
      sysmode = "a";
      break;
    case F_READWRITE_APPEND:
      sysmode = "a+";
      break;
    default:
      sysmode = NULL;
      break;
  }

  return sysmode;
}

static K_Boolean_e localIsWhiteSpace(char_t character)
{
  return ((isblank(character) != 0) || (isspace(character) != 0))?K_True:K_False;
}
