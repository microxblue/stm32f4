#include "board.h"

// NOTE:
//   It is important to add "-DHSE_VALUE=8000000" in Makefile compiling flags
//   in order to use external crystal 8M clock

#define CLOCK_CRYSTAL_HZ		    8000000UL
#define CLOCK_CPU_TARGET_HZ		168000000UL

#define PLL_M  8
#define PLL_N  336
#define PLL_P  2
#define PLL_Q  7

extern volatile uint32_t  SystemCoreTick;

uint32_t  SystemCoreClock = CLOCK_CPU_TARGET_HZ;

uint32_t get_sys_clock (void)
{
  return SystemCoreClock;
}


uint32_t get_sys_tick ()
{
  return SystemCoreTick;
}

void delay_us (uint32_t us)
{
  uint32_t initial_ticks = DWT->CYCCNT;
  uint32_t ticks = (SystemCoreClock / 1000000);
  us *= ticks;
  while ((DWT->CYCCNT - initial_ticks) < us - ticks);
}

void delay_ms (uint32_t ms)
{
  uint32_t tickstart = get_sys_tick();
  while((get_sys_tick() - tickstart) < ms);
}

void set_sys_clock (void)
{
	int		HSE, N = 0;

	/* Enable HSI.
	 * */
	RCC->CR |= RCC_CR_HSION;

	/* Reset RCC.
	 * */
	RCC->CR &= ~(RCC_CR_PLLON | RCC_CR_CSSON | RCC_CR_HSEBYP | RCC_CR_HSEON);
	RCC->CFGR = 0;
	RCC->CIR = 0;

	/* Enable HSE.
	 * */
	RCC->CR |= RCC_CR_HSEON;

	/* Wait till HSE is ready.
	 * */
	do {
		HSE = RCC->CR & RCC_CR_HSERDY;
		N++;

		__NOP();
	}
	while (HSE == 0 && N < 20000UL);

	/* Enable power interface clock.
	 * */
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	/* Regulator voltage scale 1 mode.
	 * */
	PWR->CR |= PWR_CR_VOS;

	/* Set AHB/APB1/APB2 prescalers.
	 * */
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV4 | RCC_CFGR_PPRE2_DIV2 | (RCC_CFGR_SWS_PLL | RCC_CFGR_SWS_HSE);

  /* From HSE.
   * */
  RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE;

	RCC->PLLCFGR |= (PLL_Q << 24)
		| ((PLL_P / 2UL - 1UL) << 16)
		| (PLL_N << 6) | (PLL_M << 0);

	/* Enable PLL.
	 * */
	RCC->CR |= RCC_CR_PLLON;

	/* Wait till the main PLL is ready.
	 * */
	while ((RCC->CR & RCC_CR_PLLRDY) == 0)
		__NOP();


	/* Configure Flash.
	 * */
	FLASH->ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;

	/* Select PLL.
	 * */
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Wait till PLL is used.
	 * */
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		__NOP();
}

void system_init (void)
{
  extern char _flash_start[];

  __disable_irq ();

  /* FPU settings ------------------------------------------------------------*/
  SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

  /* Reset the RCC clock configuration to the default reset state ------------*/
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset CFGR register */
  RCC->CFGR = 0x00000000;

  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset PLLCFGR register */
  RCC->PLLCFGR = 0x24003010;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;

  set_sys_clock ();

  /* Configure the Vector Table location add offset address ------------------*/
  memcpy ((void *)ISR_VECT_BASE, (void *)_flash_start, ISR_VECT_SIZE);
  SCB->VTOR = ISR_VECT_BASE;
}
