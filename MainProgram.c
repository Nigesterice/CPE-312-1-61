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
#include "dwt_delay.h"

#include "stdio.h"

#define MAX_SEC_TIME 15

//=========================================================//
void GPIO_Config(void) ;
void TIMBase_Config(void);
void SystemClock_Config(void);

//--------------7SEG-USE-ON-GPIOA--------------//
uint32_t All_GPIOA_Pins = LL_GPIO_PIN_0 ;
uint32_t _7SEG_SPECIAL_DIGIT = LL_GPIO_PIN_0 ;

//--------------7SEG-USE-ON-GPIOB--------------//
uint32_t All_GPIOB_Pins = LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15 ;
uint32_t _7SEG_NUMBER[10] =
{
	LL_GPIO_PIN_2 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14, //0
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

uint32_t _7SEG_EMPTY = 0 ;
uint32_t _7SEG_C = LL_GPIO_PIN_2 | LL_GPIO_PIN_12 | LL_GPIO_PIN_13 | LL_GPIO_PIN_14 ;
uint32_t _7SEG_COLON = LL_GPIO_PIN_2 | LL_GPIO_PIN_10 ;
uint32_t _7SEG_UPPER_DOT = LL_GPIO_PIN_11 ;

//--------------7SEG-USE-ON-GPIOC--------------//
uint32_t All_GPIOC_Pins = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 ;
uint32_t _7SEG_DIGIT[4] =
{
	LL_GPIO_PIN_0, 
	LL_GPIO_PIN_1, 
	LL_GPIO_PIN_2, 
	LL_GPIO_PIN_3
} ;
//---------------------------------------------//
void _7SEG_reset(void) ;
void _7SEG_InitDigits(uint32_t[]) ;
void _7SEG_InitSymbol(uint32_t) ;
void _7SEG_Show_Timer(void) ;
void _7SEG_Show_Temperature(void) ;
//=============================================================//

void OW_WriteBit(uint8_t d);
uint8_t OW_ReadBit(void);
void DS1820_GPIO_Configure(void);
uint8_t DS1820_ResetPulse(void);

void OW_Master(void);
void OW_Slave(void);
void OW_WriteByte(uint8_t data);
uint16_t OW_ReadByte(void);

void temperature_value(void);
//==============================================================//

#define E_06 (uint16_t)1318
#define MUTE (uint16_t)	1
#define TIMx_PSC			3 
#define ARR_CALCULATE(N) (SystemCoreClock) / ((TIMx_PSC) * (N))

void SPEAKER_TIM_BASE_Config(uint16_t);
void SPEAKER_TIM_OC_GPIO_Config(void);
void SPEAKER_TIM_OC_Config(uint16_t);
void SPEAKER_Enable(void) ;
void SPEAKER_Disable(void) ;
//==============================================================//

uint32_t count_time = 0 ;
uint32_t count_flag = 0 ;
uint16_t result_16Bit = 0 ;
float temp = 0;

int main()
{
	SystemClock_Config() ;
	
	GPIO_Config() ;
	TIMBase_Config() ;
	DWT_Init();
	DS1820_GPIO_Configure();
	temperature_value();
	SPEAKER_TIM_OC_Config(ARR_CALCULATE(E_06)) ;
	
		while(1)
		{
			if(count_time > MAX_SEC_TIME)
			{
				count_time = 0 ;
				SPEAKER_Enable() ;
			}
			if(count_flag > 12)
				count_flag = 0 ;
			
			if(count_time == 1)
				SPEAKER_Disable() ;
			
			if(count_flag > 10)
				_7SEG_Show_Temperature() ;
			else
				_7SEG_Show_Timer() ;
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

void TIM4_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_CC1(TIM4) == SET)
	{
		LL_TIM_ClearFlag_CC1(TIM4);
	}
}

void _7SEG_reset()
{
	LL_GPIO_ResetOutputPin(GPIOA, All_GPIOA_Pins) ;
	LL_GPIO_ResetOutputPin(GPIOB, All_GPIOB_Pins) ;
	LL_GPIO_ResetOutputPin(GPIOC, All_GPIOC_Pins) ;
}

void _7SEG_Show_Temperature()
{
	uint32_t newTemp = temp ;
	
	uint32_t digits[4] ;
	
	digits[0] = _7SEG_EMPTY ;
	digits[1] = _7SEG_NUMBER[newTemp % 100 / 10] ;
	digits[2] = _7SEG_NUMBER[newTemp % 10] ;
	digits[3] = _7SEG_C ;
	
	_7SEG_InitDigits(digits) ;
	_7SEG_InitSymbol(_7SEG_UPPER_DOT) ;
	
}

void _7SEG_Show_Timer()
{
	uint32_t digits[4] ;
			
	digits[0] = _7SEG_NUMBER[count_time /60 /10] ;
	digits[1] = _7SEG_NUMBER[count_time /60 %10] ;
	digits[2] = _7SEG_NUMBER[count_time %60 /10] ;
	digits[3] = _7SEG_NUMBER[count_time %60 %10] ;
		
	_7SEG_InitDigits(digits) ;
	_7SEG_InitSymbol(_7SEG_COLON) ;
}

void _7SEG_InitDigits(uint32_t digits[])
{
	for(uint8_t i = 0 ; i < 4 ; i++)
	{
		_7SEG_reset() ;
		
		if(digits[i] != _7SEG_EMPTY)
		{
			LL_GPIO_SetOutputPin(GPIOB, digits[i]) ;
			LL_GPIO_SetOutputPin(GPIOC, _7SEG_DIGIT[i]) ;
		}
	}	
}

void _7SEG_InitSymbol(uint32_t symbol)
{
	_7SEG_reset() ;
	
	if(symbol != _7SEG_EMPTY)
	{
		LL_GPIO_SetOutputPin(GPIOB, symbol) ;
		LL_GPIO_SetOutputPin(GPIOA, _7SEG_SPECIAL_DIGIT) ;	
	}
}

void TIMBase_Config(void)
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
}

void TIM2_IRQHandler(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM2) == SET)
	{
		LL_TIM_ClearFlag_UPDATE(TIM2) ;
		count_time++ ;
		count_flag++ ;
	}
}

void GPIO_Config()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA) ;
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB) ;
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC) ;
	
	LL_GPIO_InitTypeDef gpio_initstruture ;
	
	//GPIOA
	gpio_initstruture.Pin = All_GPIOA_Pins ;
	gpio_initstruture.Mode = LL_GPIO_MODE_OUTPUT ;
	gpio_initstruture.OutputType = LL_GPIO_OUTPUT_PUSHPULL ;
	gpio_initstruture.Pull = LL_GPIO_PULL_NO ;
	gpio_initstruture.Speed = LL_GPIO_SPEED_FREQ_HIGH ;
	LL_GPIO_Init(GPIOA, &gpio_initstruture) ;
	
	//GPIOB
	gpio_initstruture.Pin = All_GPIOB_Pins ;
	LL_GPIO_Init(GPIOB, &gpio_initstruture) ;
	
	//GPIOC
	gpio_initstruture.Pin = All_GPIOC_Pins ;
	LL_GPIO_Init(GPIOC, &gpio_initstruture) ;
}

void temperature_value(void)
{
		DS1820_ResetPulse();//Send reset pulse
		OW_WriteByte(0xCC);//Send 'Skip Rom (0xCC)' command
		OW_WriteByte(0x44);//Send 'Temp Convert (0x44)' command
		LL_mDelay(200);//Delay at least 200ms (typical conversion time)
		
		DS1820_ResetPulse();//Send reset pulse
		OW_WriteByte(0xCC);//Send 'Skip Rom (0xCC)' command
		OW_WriteByte(0xBE);//Send 'Read Scractpad (0xBE)' command
		
		result_16Bit |= OW_ReadByte() ;//Read byte 1 (Temperature data in LSB)
		result_16Bit |= OW_ReadByte() << 8 ;//Read byte 2 (Temperature data in MSB)
		
		temp = (result_16Bit*1.0)/16.0;	//Convert to readable floating point temperature
}
void OW_Master(void)
{
	LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_9,LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinPull(GPIOA,LL_GPIO_PIN_9,LL_GPIO_PULL_NO);
}

void OW_Slave(void)
{
	LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_9,LL_GPIO_MODE_INPUT);
	LL_GPIO_SetPinPull(GPIOA,LL_GPIO_PIN_9,LL_GPIO_PULL_UP);
}


void OW_WriteByte(uint8_t data)
{
	for(int i = 0; i < 8; i++)
	{
		//OW_WriteBit(data & (1 << i) >> i) ;
		OW_WriteBit(data & 0x01);
		data = (data >> 1);
	}
}

uint16_t OW_ReadByte(void)
{
	uint8_t result = 0, bit;
	
	for(int i = 0; i < 8; i++)
	{
		bit = OW_ReadBit();
		if(bit == 1)
		{
			result |= (1<<i);
		}
		DWT_Delay(60);
	}
	return result ;
}

void OW_WriteBit(uint8_t d)
{
	if(d == 1) //Write 1
	{
		OW_Master(); //uC occupies wire bus
		LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_9);
		DWT_Delay(1);
		OW_Slave(); //uC releases wire bus
		DWT_Delay(60);
	}
	else //Write 0
	{
		OW_Master(); //uC occupies wire bus
		DWT_Delay(60);
		OW_Slave(); //uC releases wire bus
	}
}

uint8_t OW_ReadBit(void)
{
	OW_Master();
	LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_9);
	DWT_Delay(2);
	OW_Slave();
	
	return LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_9);	
}


void DS1820_GPIO_Configure(void)
{
	LL_GPIO_InitTypeDef ds1820_io;
	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	
	ds1820_io.Mode = LL_GPIO_MODE_OUTPUT;
	ds1820_io.Pin = LL_GPIO_PIN_9;
	ds1820_io.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	ds1820_io.Pull = LL_GPIO_PULL_NO;
	ds1820_io.Speed = LL_GPIO_SPEED_FREQ_LOW;
	LL_GPIO_Init(GPIOA, &ds1820_io);
}

uint8_t DS1820_ResetPulse(void)
{	
	OW_Master();
	LL_GPIO_ResetOutputPin(GPIOA,LL_GPIO_PIN_9);
	DWT_Delay(480);
	OW_Slave();
	DWT_Delay(80);
	
	if(LL_GPIO_IsInputPinSet(GPIOA,LL_GPIO_PIN_9) == 0)
	{
		DWT_Delay(400);
		return 0;
	}
	else
	{
		DWT_Delay(400);
		return 1;
	}
}
	
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
  /* This frequency can be calculated thraough LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}



