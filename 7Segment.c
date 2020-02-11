/*Base register adddress header file*/
#include "stm32l1xx.h"
/*Library related header files*/
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_pwr.h"
#include "stm32l1xx_ll_rcc.h"
#include "stm32l1xx_ll_bus.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx_ll_tim.h"
#include "stm32l152_glass_lcd.h"

#include "stdio.h"

void SystemClock_Config(void) ;
void TIMBase_Config(void) ;
void GPIO_Config(void) ;

uint32_t OR_GPIOC_EnablePins = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 ;
uint32_t OR_GPIOB_EnablePins = LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15 ;
uint32_t GPIOC_LCD_Digits[4] = {LL_GPIO_PIN_0, LL_GPIO_PIN_1, LL_GPIO_PIN_2, LL_GPIO_PIN_3} ;
uint32_t GPIOB_LCD_Numbers[10] = {LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14, //0
													 LL_GPIO_PIN_10 | LL_GPIO_PIN_11,	//1
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_15,	//2
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_15,	//3
													 LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15,	//4
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15, //5
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15, //6
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11,	//7
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15, //8
													 LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15 //9
													} ;

int main()
{
	SystemClock_Config() ;
	GPIO_Config() ;
	
	uint32_t GPIOB_LCD_ShowNumbers[4] = {GPIOB_LCD_Numbers[0], GPIOB_LCD_Numbers[2], GPIOB_LCD_Numbers[4], GPIOB_LCD_Numbers[8]} ;
		
	while(1)
	{
		for(uint8_t i = 0 ; i < 4 ; i++)
		{
			LL_GPIO_ResetOutputPin(GPIOB, OR_GPIOB_EnablePins) ;
			LL_GPIO_ResetOutputPin(GPIOC, OR_GPIOC_EnablePins) ;
			
			LL_GPIO_SetOutputPin(GPIOB, GPIOB_LCD_ShowNumbers[i]) ;
			LL_GPIO_SetOutputPin(GPIOC, GPIOC_LCD_Digits[i]) ;
		}
	}
}

void GPIO_Config()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB) ;
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC) ;
	
	LL_GPIO_InitTypeDef gpio_initstruture ;
	
	gpio_initstruture.Pin =  OR_GPIOB_EnablePins ;
	gpio_initstruture.Mode = LL_GPIO_MODE_OUTPUT ;
	gpio_initstruture.OutputType = LL_GPIO_OUTPUT_PUSHPULL ;
	gpio_initstruture.Pull = LL_GPIO_PULL_NO ;
	gpio_initstruture.Speed = LL_GPIO_SPEED_FREQ_HIGH ;
	LL_GPIO_Init(GPIOB, &gpio_initstruture) ;
	
	gpio_initstruture.Pin =  OR_GPIOC_EnablePins ;
	LL_GPIO_Init(GPIOC, &gpio_initstruture) ;
	
	
}
/*void TIMBase_Config(void)
{
	LL_TIM_InitTypeDef timebase_initstructure ;
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2) ;
	
	timebase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1 ;
	timebase_initstructure.CounterMode = LL_TIM_COUNTERMODE_UP ;
	timebase_initstructure.Autoreload = 1000 - 1 ;
	timebase_initstructure.Prescaler = 32000 - 1 ;
	
	LL_TIM_Init(TIM2, &timebase_initstructure) ;
	
	LL_TIM_EnableIT_UPDATE(TIM2) ;
	NVIC_SetPriority(TIM2_IRQn, 0) ;
	NVIC_EnableIRQ(TIM2_IRQn) ;
	
	LL_TIM_EnableCounter(TIM2) ;
}*/
void SystemClock_Config(void)
{
  /* Enable ACC64 access and set FLASH latency */ 
  LL_FLASH_Enable64bitAccess();; 
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

  /* Set Voltage scale1 as MCU will run at 32MHz */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  
  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (LL_PWR_IsActiveFlag_VOSF() != 0)
  {
  };
  
  /* Enable HSI if not already activated*/
  if (LL_RCC_HSI_IsReady() == 0)
  {
    /* HSI configuration and activation */
    LL_RCC_HSI_Enable();
    while(LL_RCC_HSI_IsReady() != 1)
    {
    };
  }
  
	
  /* Main PLL configuration and activation */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3);

  LL_RCC_PLL_Enable();
  while(LL_RCC_PLL_IsReady() != 1)
  {
  };
  
  /* Sysclk activation on the main PLL */
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  };
  
  /* Set APB1 & APB2 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

  /* Set systick to 1ms in using frequency set to 32MHz                             */
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}
