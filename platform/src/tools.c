/*
 * tools.c
 *
 *  Created on: Mar 4, 2017
 *      Author: erik
 */

#include <os/utilsIntf.h>
#include <logging/logging.h>

K_Status_e platform_sysWrite(const char_t * dir, const char_t * file, const char_t * data)
{
  K_Status_e rc = K_Status_General_Error;
  const IUtils_t * const utils = getUtilsIntf();
  char_t buffer[255];

  if (utils->stringWrite(buffer, sizeof buffer, NULL, "%s/%s", dir, file) == K_Status_OK)
  {
    if (utils->fileExists(buffer, sizeof(buffer)) == K_True)
    {
      OsFile_t file;
      if (utils->openFile(&file, buffer, F_WRITE_CREATE) == K_Status_OK)
      {
        rc = utils->writeToFile(file, (void *) data, 1, utils->stringLength(data), NULL);
        utils->closeFile(file);
      }
      else
      {
        WARNING("Could not open %s for writing\n", buffer);
      }
    }
    else
    {
      WARNING("%s does not exists\n", buffer);
    }
  }

  return rc;
}
