/*
 * moni_iic_slave.h
 *
 *  Created on: Jun 20, 2023
 *      Author: 17868
 */

#ifndef INC_MONI_IIC_SLAVE_H_
#define INC_MONI_IIC_SLAVE_H_

//#define i2c_addr 0x64
//
//#define S_SDA_IN()  {GPIOD->MODER&=~(3<<(5*2));GPIOD->MODER|=0<<5*2;} //SDA 设置为输入模式
//#define S_SDA_OUT  {GPIOD->MODER&=~(3<<(5*2));GPIOD->MODER|=1<<5*2;} //设置为输出模式
//
//#define S_SDA_H() HAL_GPIO_WritePin(GPIOD,S_SDA_Pin,GPIO_PIN_SET)
//#define S_SDA_L() HAL_GPIO_WritePin(GPIOD,S_SDA_Pin,GPIO_PIN_RESET)
//
//#define S_SCL_H() HAL_GPIO_WritePin(GPIOD,S_SCL_Pin,GPIO_PIN_SET)
//#define S_SCL_L() HAL_GPIO_WritePin(GPIOD,S_SCL_Pin,GPIO_PIN_RESET)
//
//#define S_Read_SDA() HAL_GPIO_ReadPin(GPIOD,S_SDA_Pin)
//#define S_Read_SCL() HAL_GPIO_ReadPin(GPIOD,S_SCL_Pin)
#ifndef __SW_SLAVE_I2C_H_
#define __SW_SLAVE_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

#define SW_SLAVE_ADDR		0x64

#define SW_SLAVE_SCL_CLK_EN()	__HAL_RCC_GPIOD_CLK_ENABLE()
#define SW_SLAVE_SDA_CLK_EN()	__HAL_RCC_GPIOD_CLK_ENABLE()

#define SW_SLAVE_SCL_PRT		GPIOD
#define SW_SLAVE_SCL_PIN		GPIO_PIN_6
#define SW_SLAVE_SDA_PRT		GPIOD
#define SW_SLAVE_SDA_PIN		GPIO_PIN_7

#define GPIO_MODE_MSK            0x00000003U

#define I2C_STA_IDLE			0
#define I2C_STA_START			1
#define I2C_STA_DATA			2
#define I2C_STA_ACK			3
#define I2C_STA_NACK			4
#define I2C_STA_STOP			5

#define I2C_READ				1
#define I2C_WRITE			0

#define GPIO_DIR_IN			0
#define GPIO_DIR_OUT			1

#define SET_SCL_DIR(Temp, InOut)				\
	Temp = SW_SLAVE_SCL_PRT->MODER;			\
	Temp &= ~(GPIO_MODER_MODER6);			\
	Temp |= ((InOut & GPIO_MODE_MSK) << (6 * 2U));	\
	SW_SLAVE_SCL_PRT->MODER = temp;

#define SET_SDA_DIR(Temp, InOut)				\
	Temp = SW_SLAVE_SDA_PRT->MODER;			\
	Temp &= ~(GPIO_MODER_MODER7);			\
	Temp |= ((InOut & GPIO_MODE_MSK) << (7 * 2U));	\
	SW_SLAVE_SDA_PRT->MODER = Temp;

#define CLR_SDA_PIN()			(SW_SLAVE_SDA_PRT->BSRR = SW_SLAVE_SDA_PIN << 16)
#define SET_SDA_PIN()			(SW_SLAVE_SDA_PRT->BSRR = SW_SLAVE_SDA_PIN)

typedef struct _SwSlaveI2C_t
{
    	uint8_t State;					// I2C通信状态
	uint8_t Rw;						// I2C读写标志：0-写，1-读
	uint8_t SclFallCnt;				// SCL下降沿计数
	uint8_t Flag;					// I2C状态标志，BIT0：0-地址无效，1-地址匹配
	uint32_t StartMs;				// I2C通信起始时间，单位ms，用于判断通信是否超时
	uint8_t* RxBuf;					// 指向接收缓冲区的指针
	uint8_t* TxBuf;					// 指向发送缓冲区的指针
	uint8_t RxIdx;					// 接收缓冲区数据写入索引，最大值255
	uint8_t TxIdx;					// 发送缓冲区数据读取索引，最大值255
}SwSlaveI2C_t;

extern SwSlaveI2C_t SwSlaveI2C;

void InitSwSlaveI2C(void);
void I2cGpioIsr(void);
void CheckSwSlaveI2cTimeout(void);

#ifdef __cplusplus
}
#endif

#endif /* __SW_TIMER_H_ */


#endif /* INC_MONI_IIC_SLAVE_H_ */
