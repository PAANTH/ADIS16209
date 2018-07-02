#include "system_init.h"
#include "misc.h"
#include "stm32f10x.h"

#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_spi.h"
#include "inttypes.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
uint8_t tx_buff[4];
uint8_t rx_buff[4];
extern uint8_t is_time;
void init_system()
{
/*setup clk*/
	uint32_t timeoutCounter = 0;
	uint32_t hseStatus = 0;

	RCC->CR |= ((uint32_t)RCC_CR_HSEON);// Enable HSE

	while((hseStatus == 0) && (timeoutCounter != HSEStartUp_TimeOut)){// wait until HSE is started
		hseStatus = (RCC->CR & RCC_CR_HSERDY);// Get HSE status
		timeoutCounter++;
	}



	// Configure Flash prefetch, half-cycle access  and wait state
	FLASH->ACR = FLASH_ACR_PRFTBE |FLASH_ACR_LATENCY_2; //FLASH_ACR_HLFCYA |

	// HCLK = SYSCLK/1, PCLK2 = HCLK/1, PCLK1 = HCLK/2. (AHB-72; APB2-72; APB1-36MHz)
	RCC->CFGR |= (RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE2_DIV1 | RCC_CFGR_PPRE1_DIV2);

	// Configure the main PLL
	/* Configure PLLs ------------------------------------------------------*/
	/* Must use PLL2MUL for 25MHz HSE or can not achieve 72MHz See Clocks in RefMan*/
	/* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
	/* PREDIV1 configuration: PREDIV1CLK = PLL2 / 5 = 8 MHz */
	/* SYSCLC = PREDIV1CLK * PLLMUL = 8*9=72 MHz */
	RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |   //RESET
							  RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);

	RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 |
							 RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);

	/* Enable PLL2 */
	RCC->CR |= RCC_CR_PLL2ON;
	/* Wait till PLL2 is ready */
	while((RCC->CR & RCC_CR_PLL2RDY) == 0);



	/* PLL configuration: PLLCLK = PREDIV1_CLK * 9 = 72 MHz */
	RCC->CFGR &= (uint32_t)~( RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL); //RESET
	RCC->CFGR |= (uint32_t)( RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLMULL9);





	/* Enable PLL */
	RCC->CR |= RCC_CR_PLLON;
	while((RCC->CR &RCC_CR_PLLRDY) == 0);



	//Set HSE_PLL as clock source
	RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));// Clear mask
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	// wait untill HSE_PLL is not selected as clk source
	while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);









	/*setup gpio for button*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );
	GPIO_InitTypeDef gpio;

	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&gpio);





	/*setup gpio for led*/
	gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio.GPIO_Pin = GPIO_Pin_0;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&gpio);

	/*timer init*/

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseInitTypeDef tim_base;

	tim_base.TIM_CounterMode = TIM_CounterMode_Up;
	tim_base.TIM_Period = 0xFFFF; //1s
	tim_base.TIM_Prescaler = 720;

	TIM_TimeBaseInit(TIM2,&tim_base);

//	TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE);
//
//	NVIC_SetPriority(TIM2_IRQn , 5);
//	NVIC_EnableIRQ(TIM2_IRQn);

	//TIM_Cmd(TIM2,ENABLE);


/*setup spi*/

	 RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2 , ENABLE);

	    //clk, miso
	    GPIO_InitTypeDef gpio_init_structure;
	    gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
	    gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;

	    gpio_init_structure.GPIO_Pin =  GPIO_Pin_15 | GPIO_Pin_13 ;
	    GPIO_Init(GPIOB, &gpio_init_structure);

	    gpio_init_structure.GPIO_Mode = GPIO_Mode_IN_FLOATING;

		gpio_init_structure.GPIO_Pin = GPIO_Pin_14;
		GPIO_Init(GPIOB, &gpio_init_structure);
	    //nss
	    gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
		gpio_init_structure.GPIO_Mode = GPIO_Mode_Out_PP;
		gpio_init_structure.GPIO_Pin = GPIO_Pin_12;
		GPIO_Init(GPIOB, &gpio_init_structure);
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
	    //cs
	//    gpio_init_structure.GPIO_PuPd = GPIO_PuPd_UP;
	//    gpio_init_structure.GPIO_Mode = GPIO_Mode_IN;
	//    gpio_init_structure.GPIO_Pin = GPIO_Pin_4;
	//    GPIO_Init(GPIOA, &gpio_init_structure);


//	    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI1);
//	    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI1);
//	    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI1);


	    SPI_InitTypeDef spi_init_structure;
	    spi_init_structure.SPI_Direction =          SPI_Direction_2Lines_FullDuplex;
	    spi_init_structure.SPI_Mode =               SPI_Mode_Master;
	    spi_init_structure.SPI_DataSize =           SPI_DataSize_8b;
	    spi_init_structure.SPI_CPOL =               SPI_CPOL_High;
	    spi_init_structure.SPI_CPHA =               SPI_CPHA_2Edge; //2Edge
	    spi_init_structure.SPI_NSS =                SPI_NSS_Soft;
	    spi_init_structure.SPI_BaudRatePrescaler =  SPI_BaudRatePrescaler_64;
	    spi_init_structure.SPI_FirstBit =           SPI_FirstBit_MSB;
	    SPI_Init(SPI2, &spi_init_structure);

	    SPI_NSSInternalSoftwareConfig(SPI2, SPI_NSSInternalSoft_Set);

	    SPI_Cmd(SPI2, ENABLE);

//	    //dma
//	    //mem->spi
//      RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
//	    DMA_InitTypeDef dma_init_structure;
//	    dma_init_structure.DMA_PeripheralBaseAddr =     (uint32_t) &(SPI2->DR);
//	    dma_init_structure.DMA_MemoryBaseAddr =        (uint32_t) &tx_buff;
//	    dma_init_structure.DMA_DIR =                    DMA_DIR_PeripheralDST;
//	    dma_init_structure.DMA_BufferSize =             2;
//
//	    //dma_init_structure.DMA DMA_FIFOMode =               DMA_FIFOMode_Disable;
//	    //dma_init_structure.DMA_FIFOThreshold =          DMA_FIFOThreshold_Full;
//	    //dma_init_structure.DMA_MemoryBurst =            DMA_MemoryBurst_Single;
//	    dma_init_structure.DMA_MemoryDataSize =         DMA_MemoryDataSize_Byte;
//	    dma_init_structure.DMA_MemoryInc =              DMA_MemoryInc_Enable;
//	    dma_init_structure.DMA_Mode =                   DMA_Mode_Normal;
//	    //dma_init_structure.DMA_PeripheralBurst =        DMA_PeripheralBurst_Single;
//	    dma_init_structure.DMA_PeripheralDataSize =     DMA_PeripheralDataSize_Byte;
//	    dma_init_structure.DMA_PeripheralInc =          DMA_PeripheralInc_Disable;
//	    dma_init_structure.DMA_Priority =               DMA_Priority_High;
//	    DMA_Init(DMA1_Channel5,&dma_init_structure);
//
//	    //spi->mem
//	    dma_init_structure.DMA_PeripheralBaseAddr =     (uint32_t) &(SPI2->DR);
//	    dma_init_structure.DMA_MemoryBaseAddr =        (uint32_t) rx_buff;
//	    dma_init_structure.DMA_DIR =                    DMA_DIR_PeripheralSRC;
//	    dma_init_structure.DMA_BufferSize =             2;
////	    dma_init_structure.DMA_Channel =                DMA_Channel_3;
////	    dma_init_structure.DMA_FIFOMode =               DMA_FIFOMode_Disable;
////	    dma_init_structure.DMA_FIFOThreshold =          DMA_FIFOThreshold_Full;
////	    dma_init_structure.DMA_MemoryBurst =            DMA_MemoryBurst_Single;
//	    dma_init_structure.DMA_MemoryDataSize =         DMA_MemoryDataSize_Byte;
//	    dma_init_structure.DMA_MemoryInc =              DMA_MemoryInc_Enable;
//	    dma_init_structure.DMA_Mode =                   DMA_Mode_Normal;
//	    //dma_init_structure.DMA_PeripheralBurst =        DMA_PeripheralBurst_Single;
//	    dma_init_structure.DMA_PeripheralDataSize =     DMA_PeripheralDataSize_Byte;
//	    dma_init_structure.DMA_PeripheralInc =          DMA_PeripheralInc_Disable;
//	    dma_init_structure.DMA_Priority =               DMA_Priority_High;
//	    DMA_Init(DMA1_Channel4,&dma_init_structure);

	  //  SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
	  //  SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);


}



//void TIM2_IRQHandler()
//{
//	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
//	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
//
//	TIM_Cmd(TIM2,DISABLE);
//	TIM_SetCounter(TIM2,0);
//    is_time = 0;
//
//}
