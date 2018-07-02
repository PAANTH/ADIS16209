#include "spi_prog.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "inttypes.h"
#include "stm32f10x_spi.h"
void ADIS16209_SendData(uint16_t Data);
uint8_t ADIS16209_ReceiveData();
uint16_t ReadReg_ADIS16209(uint8_t Addr);
extern uint8_t rx_buff[4], tx_buff[4];
uint8_t buff[4];
uint8_t is_time=0;
void process_program()
{

	if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13) != SET)
	{




		uint16_t x = ReadReg_ADIS16209(0x0C);
		uint16_t y  = ReadReg_ADIS16209(0x0E);

		float xf=0,yf=0;
		if((x & 0x2000)!=0)
		{
			x = ((~x)+1)&0x3FFF;
			xf = -1*(float)x*0.025;
		}
		else
		{
			xf = (float)(x&0x3FFF)*0.025;
		}

		if((y & 0x2000)!=0)
		{
			y = ((~y)+1)&0x3FFF;
			yf = -1*(float)y*0.025;
		}
		else
		{
			yf = (float)(y&0x3FFF)*0.025;
		}


		xf=0;

	}


}


void ADIS16209_SendData(uint16_t Data)
{char tmp;


	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  /* Write in the DR register the data to be sent */
  SPI2->DR = Data;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	tmp=SPI2->DR;

}


uint8_t ADIS16209_ReceiveData()
{
	//GPIO_ResetBits(GPIOA, GPIO_Pin_4);//CS_on
  /* Check the parameters */
  assert_param(IS_SPI_ALL_PERIPH(SPI3));

  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	/* Write in the DR register the data to be sent */
	SPI_I2S_SendData(SPI2, 0);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	//GPIO_SetBits(GPIOA, GPIO_Pin_4);//CS_off
  /* Return the data in the DR register */
  return SPI2->DR;
}

uint16_t ReadReg_ADIS16209(uint8_t Addr)
{
uint16_t tmp;
GPIO_ResetBits(GPIOB, GPIO_Pin_12);//CS_on
    for (int i = 0; i < 400; ++i); //for usb-spi imitation
		ADIS16209_SendData(Addr);
		for (int i = 0; i < 800; ++i); //for usb-spi imitation
		ADIS16209_ReceiveData();
	for (int i = 0; i < 400; ++i);
		GPIO_SetBits(GPIOB, GPIO_Pin_12);//CS_off

		for (int i = 0; i < 202560; ++i); //400  //for usb-spi imitation


	GPIO_ResetBits(GPIOB, GPIO_Pin_12);//CS_on
	for (int i = 0; i < 400; ++i); //for usb-spi imitation
		tmp=ADIS16209_ReceiveData()<< 8;
		for (int i = 0; i < 800; ++i); //for usb-spi imitation
		tmp|=ADIS16209_ReceiveData();
	for (int i = 0; i < 400; ++i); //for usb-spi imitation
		GPIO_SetBits(GPIOB, GPIO_Pin_12);//CS_off


		for (int i = 0; i < 102560; ++i); //400 //for usb-spi imitation
	return tmp;
}



