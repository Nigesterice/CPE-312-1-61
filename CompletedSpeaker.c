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

#define E_06 (uint16_t)1318
#define MUTE (uint16_t)	1
#define TIMx_PSC			3 
#define ARR_CALCULATE(N) (SystemCoreClock) / ((TIMx_PSC) * (N))

void SystemClock_Config(void);
void SPEAKER_TIM_BASE_Config(uint16_t);
void SPEAKER_TIM_OC_GPIO_Config(void);
void SPEAKER_TIM_OC_Config(uint16_t);
void SPEAKER_Enable(void) ;
void SPEAKER_Disable(void) ;

int main()
{
	SystemClock_Config() ;
	SPEAKER_TIM_OC_Config(ARR_CALCULATE(E_06)) ;
	
	while(1)
	{
		//if(reach_max_time)
			SPEAKER_Enable() ;
		//if(1_second_passed)
			SPEAKER_Disable() ;	
	}
}

void SPEAKER_TIM_BASE_Config(uint16_t ARR)
{
	LL_TIM_InitTypeDef timbase_initstructure;
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	//Time-base configure
	timbase_initstructure.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	timbase_initstructure.CounterMode = LL_TIM_COUNTERMODE_UP;
	timbase_initstructure.Autoreload = ARR - 1;
	timbase_initstructure.Prescaler =  TIMx_PSC- 1;
	LL_TIM_Init(TIM4, &timbase_initstructure);
	
	LL_TIM_EnableCounter(TIM4); 
}

void SPEAKER_TIM_OC_GPIO_Config(void)
{
	LL_GPIO_InitTypeDef gpio_initstructure;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	
	gpio_initstructure.Mode = LL_GPIO_MODE_ALTERNATE;
	gpio_initstructure.Alternate = LL_GPIO_AF_2;
	gpio_initstructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	gpio_initstructure.Pin = LL_GPIO_PIN_6;
	gpio_initstructure.Pull = LL_GPIO_PULL_NO;
	gpio_initstructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	LL_GPIO_Init(GPIOB, &gpio_initstructure);
}
void SPEAKER_Enable(void)
{
	LL_TIM_EnableCounter(TIM4);
}

void SPEAKER_Disable(void)
{
	LL_TIM_DisableCounter(TIM4);
}

void SPEAKER_TIM_OC_Config(uint16_t note)
{
	LL_TIM_OC_InitTypeDef tim_oc_initstructure;
	
	SPEAKER_TIM_OC_GPIO_Config();
	SPEAKER_TIM_BASE_Config(note);
	
	tim_oc_initstructure.OCState = LL_TIM_OCSTATE_DISABLE;
	tim_oc_initstructure.OCMode = LL_TIM_OCMODE_PWM1;
	tim_oc_initstructure.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	tim_oc_initstructure.CompareValue = LL_TIM_GetAutoReload(TIM4) / 2;
	LL_TIM_OC_Init(TIM4, LL_TIM_CHANNEL_CH1, &tim_oc_initstructure);
	/*Interrupt Configure*/
	NVIC_SetPriority(TIM4_IRQn, 1);
	NVIC_EnableIRQ(TIM4_IRQn);
	LL_TIM_EnableIT_CC1(TIM4);
	
	/*Start Output Compare in PWM Mode*/
	LL_TIM_CC_EnableChannel(TIM4, LL_TIM_CHANNEL_CH1);
}

