#include "board.h"

void _init (void)
{
}

uint32_t dwt_init(void)
{
    /* Disable TRC */
    CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
    /* Enable TRC */
    CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

    /* Disable clock cycle counter */
    DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
    /* Enable  clock cycle counter */
    DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

    /* Reset the clock cycle counter value */
    DWT->CYCCNT = 0;

    /* 3 NO OPERATION instructions */
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");
    __ASM volatile ("NOP");

    /* Check if clock cycle counter has started */
    if(DWT->CYCCNT)
    {
       return 0; /*clock cycle counter started*/
    }
    else
    {
      return 1; /*clock cycle counter not started*/
    }
}

void uart_init ()
{
  USART_InitTypeDef      USART_InitStructure;
  USART_ClockInitTypeDef USART_ClockInitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

  USART_ClockStructInit(&USART_ClockInitStructure);
  USART_ClockInit(UART_CONSOLE, &USART_ClockInitStructure);

  USART_InitStructure.USART_BaudRate   = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

  USART_Init(UART_CONSOLE, &USART_InitStructure);
  USART_Cmd(UART_CONSOLE, ENABLE);
}

int main(void)
{
  dwt_init ();

  uart_init ();

  while (1);
}
