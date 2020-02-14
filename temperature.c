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

/*already implemented */
void SystemClock_Config(void);
void OW_WriteBit(uint8_t d);
uint8_t OW_ReadBit(void);
void DS1820_GPIO_Configure(void);
uint8_t DS1820_ResetPulse(void);

/*haven't been implemented yet!*/
void OW_Master(void);
void OW_Slave(void);
void OW_WriteByte(uint8_t data);
uint16_t OW_ReadByte(void);

void temperature_value(void);

uint16_t result_16Bit = 0 ;
float temp = 0;
int main()
{
	SystemClock_Config();
	DWT_Init();
	DS1820_GPIO_Configure();

	while(1)
	{
		temperature_value();
	}
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
  /* This frequency can be calculated through LL RCC macro                          */
  /* ex: __LL_RCC_CALC_PLLCLK_FREQ (HSI_VALUE, LL_RCC_PLL_MUL_6, LL_RCC_PLL_DIV_3); */
  LL_Init1msTick(32000000);
  
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(32000000);
}
