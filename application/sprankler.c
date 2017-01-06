#include <os/memIntf.h>
#include <os/threadIntf.h>

#include <stdio.h>

int main(int argc, char **argv)
{
  const IMem_t *const mem = getMemIntf();

  uint8_t * data = (uint8_t *) mem->malloc(1000);
  size_t i;

  for (i= 0 ; i < 1000; i++)
  {
    printf("%u\n",*(data+i));
  }



  mem->free(data);

	return 0;
}
