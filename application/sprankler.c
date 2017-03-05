#include <os/utilsIntf.h>
#include <platform/platformIntf.h>

#include  <logging/logging.h>

int main(int argc, char ** argv)
{
  const IPlatform_t * const platform = getPlatformIntf();

  if (platform->init() == K_Status_OK)
  {
    INFO("Successfully started system\n");
    const IUtils_t * const utils = getUtilsIntf();

    while (K_True)
    {
      utils->sleep(1);
    }
  }

  return 0;
}
