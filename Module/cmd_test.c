#include <module.h>

COMMON_API *gCommonApi;

int  main();

__attribute__((section (".entry")))
int module_start ()
{
  ZERO_BSS ();
  gCommonApi = (COMMON_API *)COMMON_API_BASE;

  main ();

  return 0;
}

int main (void)
{
  printf ("Hello\n");

  return 0;
}