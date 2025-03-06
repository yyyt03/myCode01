#include "bh1750.h"
#include "sys.h"

//写入
void Single_Write_BH1750(unsigned char REG_Address)
{
  IIC_Start();                  
  IIC_Send_Byte(BHAddWrite);   
  IIC_Send_Byte(REG_Address);    
  IIC_Stop();                   
}
//发送数据
void bh_data_send(u8 command)
{
    do{
    IIC_Start();                      
    IIC_Send_Byte(BHAddWrite);       
    }while(IIC_Wait_Ack());          
    IIC_Send_Byte(command);         
    IIC_Wait_Ack();                  
    IIC_Stop();                     
}
//接受数据
u16 bh_data_read(void)
{
	u16 buf;
	u8 a;
	u8 b;
	IIC_Start();                       
	IIC_Send_Byte(BHAddRead);         
	IIC_Wait_Ack();                  
	b=IIC_Read_Byte(1);
	a=IIC_Read_Byte(0);
	buf=b*256+a;   
	IIC_Stop();                      
	return buf; 
}
//初始化
void Init_BH1750(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // 修改为 PB10 和 PB11
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;  
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct); 

  Single_Write_BH1750(0x01);  // 发送电源开启命令
  delayms(180);              // 延时等待初始化完成
}




