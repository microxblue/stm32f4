#include "board.h"

volatile  uint32_t  SystemCoreTick;

void SysTick_Handler (void)
{
  SystemCoreTick++;
}
