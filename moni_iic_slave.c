/*
 * moni_iic_slave.c
 *
 *  Created on: Jun 20, 2023
 *      Author: 17868
 */


#include "moni_iic_slave.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"

uint8_t RxDataBuf[256];
uint8_t TxDataBuf[256];
SwSlaveI2C_t SwSlaveI2C =
{
	I2C_STA_IDLE,		// State
	I2C_WRITE,			// Rw
	0, 					// SclFallCnt
	0,					// Flag
	0,					// StartMs
	RxDataBuf,			// RxBuf
	TxDataBuf,			// TxBuf
	0,					// RxIdx
	0					// TxIdx
};

void InitSwSlaveI2C(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  /* Enable I2C GPIO clock */
  SW_SLAVE_SCL_CLK_EN();
  SW_SLAVE_SDA_CLK_EN();

  /* Configure SCL GPIO pin */
  GPIO_InitStructure.Pin       = SW_SLAVE_SCL_PIN;
  GPIO_InitStructure.Mode      = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStructure.Pull      = GPIO_PULLUP;
  GPIO_InitStructure.Speed     = GPIO_SPEED_FAST;
  HAL_GPIO_Init(SW_SLAVE_SCL_PRT, &GPIO_InitStructure);

  /* Configure SDA GPIO pin */
  GPIO_InitStructure.Pin       = SW_SLAVE_SDA_PIN;
  HAL_GPIO_Init(SW_SLAVE_SDA_PRT, &GPIO_InitStructure);

  /* Configure SCL GPIO pin as input interruption with pull up */
  GPIO_InitStructure.Pin       = SW_SLAVE_SCL_PIN;
  GPIO_InitStructure.Mode      = GPIO_MODE_IT_RISING_FALLING;
  HAL_GPIO_Init(SW_SLAVE_SCL_PRT, &GPIO_InitStructure);

  /* Configure SDA GPIO pin as input interruption with pull up */
  GPIO_InitStructure.Pin       = SW_SLAVE_SDA_PIN;
  HAL_GPIO_Init(SW_SLAVE_SDA_PRT, &GPIO_InitStructure);

  /* Enable and set EXTI Line9_5 Interrupt to the highest priority */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

//void EXTI9_5_IRQHandler(void)
//{
//
//}

void I2cGpioIsr(void)
{
	uint32_t temp;

	// 处理SCL的上下沿中断
	if(__HAL_GPIO_EXTI_GET_IT(SW_SLAVE_SCL_PIN) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(SW_SLAVE_SCL_PIN);
		// 更新通信起始时间
		SwSlaveI2C.StartMs = HAL_GetTick();
		// SCL的下降沿事件处理，此时需要更新要传输的数据

		if(HAL_GPIO_ReadPin(SW_SLAVE_SCL_PRT,SW_SLAVE_SCL_PIN) == (uint32_t)GPIO_PIN_RESET)
		{
		//	printf("State %d\r\n",SwSlaveI2C.State);
			switch(SwSlaveI2C.State)
			{
				case I2C_STA_START:		// 起始信号的下降沿，初始化相关参数并转到接收比特数据状态
					SwSlaveI2C.SclFallCnt = 0;
					SwSlaveI2C.RxIdx = 0;
					SwSlaveI2C.TxIdx = 0;
					SwSlaveI2C.Flag = 0;	// 默认地址不匹配
					SwSlaveI2C.RxBuf[SwSlaveI2C.RxIdx] = 0;
					SwSlaveI2C.Rw = I2C_WRITE;	// 第1字节为设备地址，一定是写入
					SwSlaveI2C.State = I2C_STA_DATA;
					break;
				case I2C_STA_DATA:
					SwSlaveI2C.SclFallCnt++;
			//		printf("Rw %d\r\n",SwSlaveI2C.Rw);
					if(8 > SwSlaveI2C.SclFallCnt)
					{
						// 如果是主机读取数据，则在SCL低电平时更新比特数据
						if(SwSlaveI2C.Rw == I2C_READ)
						{
							if(SwSlaveI2C.TxBuf[SwSlaveI2C.TxIdx] & (1 << (7 - SwSlaveI2C.SclFallCnt)))
							{
								SET_SDA_PIN();
							}
							else
							{
								CLR_SDA_PIN();
							}
						}
					}
					else if(8 == SwSlaveI2C.SclFallCnt)
					{
						if(SwSlaveI2C.Rw == I2C_WRITE)
						{
							// 从第一个地址字节中获取读写标志位，并判断地址是否匹配
							if(SwSlaveI2C.RxIdx == 0)
							{
			//					printf("RxBuf[0] %d\r\n",SwSlaveI2C.RxBuf[0]);
								if((SwSlaveI2C.RxBuf[0] & 0xFE) == SW_SLAVE_ADDR)
								{
									SwSlaveI2C.Flag = 1;	// 地址匹配
									SwSlaveI2C.Rw = SwSlaveI2C.RxBuf[0] & 0x01;
								}
							}
							if(SwSlaveI2C.Flag)
							{
								// 如果是主机写入数据，且地址匹配，则接收完8比特数据后，需要发送ACK信号进行应答
								SET_SDA_DIR(temp, GPIO_DIR_OUT);
								CLR_SDA_PIN();
							}
						}
						else
						{
							// 如果是主机读取数据，需要将SDA设置成输入以便判断应答标志位状态
							SET_SDA_DIR(temp, GPIO_DIR_IN);
							// 如果是主机读取数据，准备发送下一个字节的数据
							SwSlaveI2C.TxIdx++;
						}
						// 接收或发送完8比特数据后，准备发送或接收应答信号
						SwSlaveI2C.State = I2C_STA_ACK;
					}
					break;
				case I2C_STA_ACK:
			//		printf("ack\r\n");
					SwSlaveI2C.SclFallCnt = 0;
					if(SwSlaveI2C.Rw == I2C_WRITE)
					{
						// 如果是主机写入数据，且ACK发送完毕，则SDA设置成输入，继续接收数据
						SET_SDA_DIR(temp, GPIO_DIR_IN);
						SwSlaveI2C.RxIdx++;
						SwSlaveI2C.RxBuf[SwSlaveI2C.RxIdx] = 0;
					}
					else
					{
						// 如果是主机读取数据，且ACK接收完毕，则SDA设置成输出，继续发送数据
						SET_SDA_DIR(temp, GPIO_DIR_OUT);
						if(SwSlaveI2C.TxBuf[SwSlaveI2C.TxIdx] & 0x80)
						{
							SET_SDA_PIN();
						}
						else
						{
							CLR_SDA_PIN();
						}
					}
					SwSlaveI2C.State = I2C_STA_DATA;
					break;
				case I2C_STA_NACK:		// 如果收到了NACK，则后面将是STOP或者ReSTART信号，需要将SDA设置成输入
		//			printf("nack\r\n");
					SwSlaveI2C.SclFallCnt = 0;
					SET_SDA_DIR(temp, GPIO_DIR_IN);
					break;
			}
		}
		// SCL的上升沿事件处理，此时需要采集数据，而且在数据阶段，SCL高电平时数据必须保持不变
		else
		{
		//	printf("2 State %d\r\n",SwSlaveI2C.State);
			switch(SwSlaveI2C.State)
			{
				case I2C_STA_DATA:	// 数据阶段，如果是主机写入数据，则采集比特数据
					if((I2C_WRITE == SwSlaveI2C.Rw) && (8 > SwSlaveI2C.SclFallCnt))
					{
						if(SW_SLAVE_SDA_PRT->IDR & SW_SLAVE_SDA_PIN)
						{
							SwSlaveI2C.RxBuf[SwSlaveI2C.RxIdx] |= (1 << (7 - SwSlaveI2C.SclFallCnt));
						}
					}
					break;
				case I2C_STA_ACK:	// 应答阶段，如果是主机读取数据，则判断ACK/NACK信号，默认状态是ACK
					if((SwSlaveI2C.Rw == I2C_READ) && (SW_SLAVE_SDA_PRT->IDR & SW_SLAVE_SDA_PIN))
					{
						SwSlaveI2C.State = I2C_STA_NACK;
					}
					break;
			}
		}
	}
	else if(__HAL_GPIO_EXTI_GET_IT(SW_SLAVE_SDA_PIN) != RESET)
	{
		__HAL_GPIO_EXTI_CLEAR_IT(SW_SLAVE_SDA_PIN);
		if((SW_SLAVE_SDA_PRT->IDR & SW_SLAVE_SDA_PIN) == (uint32_t)GPIO_PIN_RESET)
		{
			// SCL为高电平时，SDA从高变低，说明是起始信号
			if(SW_SLAVE_SCL_PRT->IDR & SW_SLAVE_SCL_PIN)
			{
				SwSlaveI2C.State = I2C_STA_START;
			}
		}
		else
		{
			// SCL为高电平时，SDA从低变高，说明是停止信号，一次I2C通信结束，直接将状态设置成空闲
			if(SW_SLAVE_SCL_PRT->IDR & SW_SLAVE_SCL_PIN)
			{
				SwSlaveI2C.State = I2C_STA_IDLE;
			}
		}
	}
}
void CheckSwSlaveI2cTimeout(void)
{
  uint32_t TimeMs, TimeCurMs;

  if(SwSlaveI2C.State != I2C_STA_IDLE)
  {
    TimeCurMs = HAL_GetTick();
    if(TimeCurMs >= SwSlaveI2C.StartMs)
    {
      TimeMs = TimeCurMs - SwSlaveI2C.StartMs;
    }
    else
    {
      TimeMs = ~(SwSlaveI2C.StartMs - TimeCurMs) + 1;
    }
    if(500 <= TimeMs)
    {
      // I2C通信超时的话，重置状态机，并把SDA设置成输入
      SwSlaveI2C.State = I2C_STA_IDLE;
      SET_SDA_DIR(TimeMs, GPIO_DIR_IN);
    }
  }
}
