#include "board.h"
#include "shell.h"
#include "usbapi.h"

COMMON_API *gCommonApi = (COMMON_API *)COMMON_API_BASE;

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

void hw_gpio_init (void)
{
	GPIO_InitTypeDef  GPIO_InitStruct = {0};

	// GPIO clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | \
	                       RCC_AHB1Periph_GPIOB | \
	                       RCC_AHB1Periph_GPIOC | \
	                       RCC_AHB1Periph_GPIOD | \
	                       RCC_AHB1Periph_GPIOE,  \
	                       ENABLE);

  // PC 10/11 as UART
  GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd   = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed  = GPIO_Fast_Speed;
  GPIO_Init (GPIOC, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);


  // PA   11/12:  USB
  GPIO_InitStruct.GPIO_Pin    = GPIO_Pin_11|GPIO_Pin_12;
  GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd   = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_100MHz;
  GPIO_Init (GPIOA, &GPIO_InitStruct);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_OTG_FS);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_OTG_FS);
}

void sys_tick_init (void)
{
  /*Configure the SysTick to have interrupt in 1ms time basis*/
  SysTick->LOAD  = (uint32_t)(SystemCoreClock / 1000 - 1UL);        /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
  __enable_irq ();
}

void module_init (void)
{
  memset (gCommonApi, 0, sizeof(COMMON_API));
  gCommonApi->HasChar      = haschar;
  gCommonApi->GetChar      = getchar;
  gCommonApi->PutChar      = putchar;
  gCommonApi->Printf       = printf;
  gCommonApi->DelayMs      = delay_ms;
  gCommonApi->DelayUs      = delay_us;

  gCommonApi->UsbGetRxBuf  = usb_get_rx_buf;
  gCommonApi->UsbFreeRxBuf = usb_free_rx_buf;
  gCommonApi->UsbGetTxBuf  = usb_get_tx_buf;
  gCommonApi->UsbAddTxBuf  = usb_add_tx_buf;
}

void switch_part ()
{
  typedef void (*RST_VECT)();
  RST_VECT  reset_vector;
  UINT8     magic;

  magic = *(UINT8 *)USER_REG_BASE;
  *(UINT8 *)USER_REG_BASE = 0x00;
  reset_vector = (RST_VECT)0;
  switch (magic) {
  case 0xAA:
    reset_vector = (RST_VECT)((*(DWORD *)(SCB->VTOR + 4)) ^ SHELL_SIZE);
    break;
  case 0xAB:
    reset_vector = (RST_VECT)((*(DWORD *)(SCB->VTOR + 4)));
    break;
  case 0xAC:
    reset_vector = (RST_VECT)(*(DWORD *)(USER_REG_BASE + 0x04));
    break;
  case 0xAD:
    reset_vector = (RST_VECT)(*(DWORD *)(USER_REG_BASE + 0x04));
    reset_vector = (RST_VECT)(*(DWORD *)((UINT8 *)reset_vector + 0x04));
    break;
  default:
    break;
  }

  if (reset_vector) {
    reset_vector();
  }
}

int main(void)
{
  bool banner;

  banner = false;

  __disable_irq ();

  switch_part ();

  dwt_init ();

  sys_tick_init ();

  module_init ();

  hw_gpio_init ();

  uart_init ();

  usb_init ();

  usb_start ();

  while(1) {

    if (!banner && IS_CONSOLE_READY()) {
      banner = true;
      print_banner ();
    }

    if (get_ser_cmdline (NULL)) {
      parse_cmdline (NULL);
    }

    if (get_usb_cmdline(NULL)) {
      parse_cmdline (NULL);
    }

    if (get_mem_cmdline(NULL)) {
      parse_cmdline (NULL);
    }

  }

}
