#include "NRF24L01.h"

//NRF24L01 驱动函数 

uint8_t ff = 0xFF;
HAL_StatusTypeDef ERRORERROR;

unsigned char idel_mode_flag = 0;
unsigned char mode_time_counter = 0;	

const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x11,0xa9,0x56,0x82,0x21}; //发射机接收地址
const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x00,0x98,0x45,0x71,0x10}; //小车接收地址
//const uint8_t TX_ADDRESS[TX_ADR_WIDTH]={0x00,0x98,0x45,0x71,0x10}; //发送地址
//const uint8_t RX_ADDRESS[RX_ADR_WIDTH]={0x00,0x98,0x45,0x71,0x10}; //发送地址	

extern uint8_t tx_freq;	//24L01频率初始化为90
extern uint8_t rx_freq;	//24L01频率初始化为90
#ifdef bandwidth_1Mbps
uint8_t bandwidth = 0x06;  //带宽初始化1Mbps
#endif
#ifdef bandwidth_250kbps
uint8_t bandwidth = 0x26;  //带宽初始化为0.25Mbps
#endif

//初始化24L01的IO口
void NRF24L01_RX_Init(void)
{
	Set_NRF24L01_F27_RXEN;
	Clr_NRF24L01_F27_TXEN;
	
	Set_NRF24L01_RX_CE;                                    //初始化时先拉高
  Set_NRF24L01_RX_CSN;                                   //初始化时先拉高

	MX_SPI2_Init();                                     //初始化SPI
	Clr_NRF24L01_RX_CE; 	                                  //使能24L01
	Set_NRF24L01_RX_CSN;                                   //SPI片选取消
}
void NRF24L01_TX_Init(void)
{
	Set_NRF24L01_TX_CE;                                    //初始化时先拉高
  Set_NRF24L01_TX_CSN;                                   //初始化时先拉高

	MX_SPI1_Init();                                     //初始化SPI
	Clr_NRF24L01_TX_CE; 	                                  //使能24L01
	Set_NRF24L01_TX_CSN;                                   //SPI片选取消
}
//上电检测NRF24L01是否在位
//写5个数据然后再读回来进行比较，
//相同时返回值:0，表示在位;否则返回1，表示不在位	
uint8_t NRF24L01_RX_Check(void)
{
	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t rx_buf1[5];
	uint8_t i; 
	NRF24L01_RX_Write_Buf(SPI_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_RX_Read_Buf(TX_ADDR,rx_buf1,5);              //读出写入的地址
	for(i=0;i<5;i++){
		if(rx_buf1[i] != 0XA5)
			break;
	};
	if(i!=5)
		return 1;                               //NRF24L01不在位	
	return 0;		                                //NRF24L01在位
}	

//////NRF2401 DMA CHECK
//////
//////uint8_t NRF24L01_RX_DMA_Check_flag = 0;
//////uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
//////uint8_t rx_buf1[5] = {0, 0, 0, 0, 0};
//////uint8_t rx_temp;
//////uint8_t rx_status;
//////uint8_t NRF24L01_RX_DMA_Check(void)
//////{
//////	NRF24L01_RX_DMA_Check_flag = 1;
////////	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
////////	uint8_t buf1[5];
////////	uint8_t i; 
////////	uint8_t status, temp;
////// 	Clr_NRF24L01_RX_CSN;                                    //使能SPI传输
//////	rx_temp = SPI_WRITE_REG+TX_ADDR;
//////	HAL_SPI_TransmitReceive_DMA(&hspi1, &rx_temp, &rx_status, 1);     //发送寄存器号
//////	return 0;
////////	HAL_SPI_Transmit_DMA(&hspi1, buf, 5);     //写入数据
////////  Set_NRF24L01_RX_CSN;                                    //关闭SPI传输
////////	
////////	Clr_NRF24L01_RX_CSN;                     //使能SPI传输
////////	temp = TX_ADDR;
////////	HAL_SPI_Transmit_DMA(&hspi1, &temp, 1);     //发送寄存器号?
////////	HAL_SPI_Receive_DMA(&hspi1, buf1, 5);     //读出数据
////////  Set_NRF24L01_RX_CSN;                     //关闭SPI传输
////////	
////////	for(i=0;i<5;i++){
////////		if(buf1[i] != 0XA5)
////////			break;
////////	};
////////	if(i!=5)
////////		return 1;                               //NRF24L01不在位	
////////	return 0;		                                //NRF24L01在位
//////}	

//////void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
//////{
//////	if(hspi->Instance == SPI1){
//////		if(NRF24L01_RX_DMA_Check_flag == 1){
//////			NRF24L01_RX_DMA_Check_flag ++;
//////			HAL_SPI_Transmit_DMA(&hspi1, buf, 5);     //写入数据
//////		}
//////	}
//////}
//////void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
//////{
//////	if(hspi->Instance == SPI1){
//////		if(NRF24L01_RX_DMA_Check_flag == 3){
//////			NRF24L01_RX_DMA_Check_flag ++;
//////			HAL_SPI_Receive_DMA(&hspi1, rx_buf1, 5);     //读出数据
//////		}
//////		if(NRF24L01_RX_DMA_Check_flag == 2){
//////			Set_NRF24L01_RX_CSN; 
//////			NRF24L01_RX_DMA_Check_flag ++;
//////			Clr_NRF24L01_RX_CSN;  
//////			rx_temp = TX_ADDR;
//////			HAL_SPI_Transmit_DMA(&hspi1, &rx_temp, 1);     //发送寄存器号?
//////		}
//////	}
//////}

//////void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//////{
//////	uint8_t i = 0;
//////	if(hspi->Instance == SPI1){
//////		if(NRF24L01_RX_DMA_Check_flag == 4){
//////			NRF24L01_RX_DMA_Check_flag = 0;
//////			for(i=0;i<5;i++){
//////				if(rx_buf1[i] != 0XA5)
//////					break;
//////			};
//////			if(i == 5)	
//////				HAL_GPIO_TogglePin(RX_COM_GPIO_Port, RX_COM_Pin);								//NRF24L01在位
//////		}
//////	}
//////}

uint8_t NRF24L01_TX_Check(void)
{
	uint8_t buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t tx_buf1[5];
	uint8_t i; 
	NRF24L01_TX_Write_Buf(SPI_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_TX_Read_Buf(TX_ADDR,tx_buf1,5);              //读出写入的地址
	for(i=0;i<5;i++){
		if(tx_buf1[i] != 0XA5)
			break;
	};
	if(i!=5)
		return 1;                               //NRF24L01不在位	
	return 0;		                                //NRF24L01在位
}	 
//通过SPI写寄存器
uint8_t NRF24L01_RX_Write_Reg(uint8_t regaddr,uint8_t data)
{
	uint8_t status;
  Clr_NRF24L01_RX_CSN;                    //使能SPI传输
	
	//阻塞型
	HAL_SPI_TransmitReceive(&hspi2, &regaddr, &status, 1, NRF24L01_TIME_OUT); //发送寄存器号 
	HAL_SPI_Transmit(&hspi2, &data, 1, NRF24L01_TIME_OUT);           //写入寄存器的值
	
//////	//DMA
//////	HAL_SPI_TransmitReceive_DMA(&hspi1, &regaddr, &status, 1); //发送寄存器号 
//////	HAL_SPI_Transmit_DMA(&hspi1, &data, 1);           //写入寄存器的值
	
  Set_NRF24L01_RX_CSN;                    //禁止SPI传输	   
  return(status);       		         //返回状态值
}
uint8_t NRF24L01_TX_Write_Reg(uint8_t regaddr,uint8_t data)
{
	uint8_t status;
  Clr_NRF24L01_TX_CSN;                    //使能SPI传输
	
	//阻塞型
	HAL_SPI_TransmitReceive(&hspi1, &regaddr, &status, 1, NRF24L01_TIME_OUT); //发送寄存器号 
	HAL_SPI_Transmit(&hspi1, &data, 1, NRF24L01_TIME_OUT);           //写入寄存器的值
	
//////	//DMA
//////	HAL_SPI_TransmitReceive_DMA(&hspi2, &regaddr, &status, 1); //发送寄存器号 
//////	HAL_SPI_Transmit_DMA(&hspi2, &data, 1);           //写入寄存器的值
	
  Set_NRF24L01_TX_CSN;                    //禁止SPI传输	   
  return(status);       		         //返回状态值
}
//读取SPI寄存器值 ，regaddr:要读的寄存器
uint8_t NRF24L01_RX_Read_Reg(uint8_t regaddr)
{
	uint8_t reg_val, status;    
 	Clr_NRF24L01_RX_CSN;                //使能SPI传输
	
	//阻塞型
	HAL_SPI_TransmitReceive(&hspi2, &regaddr, &status, 1, NRF24L01_TIME_OUT);     //发送寄存器号
  HAL_SPI_Receive(&hspi2, &reg_val, 1, NRF24L01_TIME_OUT);		//读取寄存器内容
	
//////	//DMA
//////	HAL_SPI_TransmitReceive_DMA(&hspi1, &regaddr, &status, 1); //发送寄存器号
//////	HAL_SPI_Receive_DMA(&hspi1, &reg_val, 1);           //写入寄存器的值
	
  Set_NRF24L01_RX_CSN;                //禁止SPI传输		    
  return reg_val;                 //返回状态值
}	
uint8_t NRF24L01_TX_Read_Reg(uint8_t regaddr)
{
	uint8_t reg_val, status;    
 	Clr_NRF24L01_TX_CSN;                //使能SPI传输
	
	//阻塞型
	HAL_SPI_TransmitReceive(&hspi1, &regaddr, &status, 1, NRF24L01_TIME_OUT);     //发送寄存器号
  HAL_SPI_Receive(&hspi1, &reg_val, 1, NRF24L01_TIME_OUT);		//读取寄存器内容
	
//////	//DMA
//////	HAL_SPI_TransmitReceive_DMA(&hspi2, &regaddr, &status, 1);     //发送寄存器号
//////  HAL_SPI_Receive_DMA(&hspi2, &reg_val, 1);		//读取寄存器内容
	
  Set_NRF24L01_TX_CSN;                //禁止SPI传输		    
  return reg_val;                 //返回状态值
}	
//在指定位置读出指定长度的数据
//*pBuf:数据指针
//返回值,此次读到的状态寄存器值 
void NRF24L01_RX_Read_Buf(uint8_t regaddr,uint8_t *pBuf,uint8_t datalen)
{      
//	uint8_t status;
	Clr_NRF24L01_RX_CSN;                     //使能SPI传输

	//阻塞型
	HAL_SPI_Transmit(&hspi2, &regaddr, 1, NRF24L01_TIME_OUT);     //发送寄存器号�
	HAL_SPI_Receive(&hspi2, pBuf, datalen, NRF24L01_TIME_OUT);     //读出数据
	
////////	//DMA
////////	HAL_SPI_Transmit_DMA(&hspi1, &regaddr, 1);     //发送寄存器号?
////////	HAL_Delay(1000);
////////	HAL_SPI_Receive_DMA(&hspi1, pBuf, datalen);     //读出数据
////////	HAL_Delay(1000);
	
  Set_NRF24L01_RX_CSN;                     //关闭SPI传输
}
void NRF24L01_TX_Read_Buf(uint8_t regaddr,uint8_t *pBuf,uint8_t datalen)
{      
//	uint8_t status;
	Clr_NRF24L01_TX_CSN;                     //使能SPI传输

	//阻塞型
	HAL_SPI_Transmit(&hspi1, &regaddr, 1, NRF24L01_TIME_OUT);     //发送寄存器号
	HAL_SPI_Receive(&hspi1, pBuf, datalen, NRF24L01_TIME_OUT);     //读出数据
	
//////	//DMA
//////	HAL_SPI_Transmit_DMA(&hspi2, &regaddr, 1);     //发送寄存器号
//////	HAL_SPI_Receive_DMA(&hspi2, pBuf, datalen);     //读出数据
//////	HAL_Delay(0);
	
  Set_NRF24L01_TX_CSN;                     //关闭SPI传输
}
//在指定位置写指定长度的数据
//*pBuf:数据指针
//返回值,此次读到的状态寄存器值
uint8_t NRF24L01_RX_Write_Buf(uint8_t regaddr, uint8_t *pBuf, uint8_t datalen)
{
	uint8_t status;
 	Clr_NRF24L01_RX_CSN;                                    //使能SPI传输

	//阻塞型
	HAL_SPI_TransmitReceive(&hspi2, &regaddr, &status, 1, NRF24L01_TIME_OUT);     //发送寄存器号
	HAL_SPI_Transmit(&hspi2, pBuf, datalen, NRF24L01_TIME_OUT);     //写入数据	

////////	//DMA
////////	HAL_SPI_TransmitReceive_DMA(&hspi1, &regaddr, &status, 1);     //发送寄存器号
////////	HAL_Delay(1000);
////////	HAL_SPI_Transmit_DMA(&hspi1, pBuf, datalen);     //写入数据
////////	HAL_Delay(1000);
	
	
  Set_NRF24L01_RX_CSN;                                    //关闭SPI传输
  return status;                                       //返回读到的状态值
}		
uint8_t NRF24L01_TX_Write_Buf(uint8_t regaddr, uint8_t *pBuf, uint8_t datalen)
{
	uint8_t status;
 	Clr_NRF24L01_TX_CSN;                                    //使能SPI传输

	//阻塞型
	HAL_SPI_TransmitReceive(&hspi1, &regaddr, &status, 1, NRF24L01_TIME_OUT);     //发送寄存器号
	HAL_SPI_Transmit(&hspi1, pBuf, datalen, NRF24L01_TIME_OUT);     //写入数据	
	 
//////	//DMA
//////	HAL_SPI_TransmitReceive_DMA(&hspi2, &regaddr, &status, 1);     //发送寄存器号
//////	HAL_SPI_Transmit_DMA(&hspi2, pBuf, datalen);     //写入数据
//////	HAL_Delay(0);
	
  Set_NRF24L01_TX_CSN;                                    //关闭SPI传输
  return status;                                       //返回读到的状态值
}	
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
uint8_t NRF24L01_TxPacket(uint8_t *txbuf)
{ 
	NRF24L01_TX_Write_Reg(FLUSH_TX,0xff);               //清除TX FIFO寄存器  
	Clr_NRF24L01_TX_CE;
  NRF24L01_TX_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  25个字节
 	Set_NRF24L01_TX_CE;                                     //启动发送	   
	return TX_OK;                                         //发送完成
}

//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:0，接收完成；其他，错误代码
uint8_t NRF24L01_RxPacket(uint8_t *rxbuf)
{	
	uint8_t state;		    							      
	state=NRF24L01_RX_Read_Reg(STATUS);                //读取状态寄存器的值   
	NRF24L01_RX_Write_Reg(SPI_WRITE_REG+STATUS,state); //清除TX_DS或MAX_RT中断标志
	if(state&RX_OK)                                 //接收到数据
	{
		NRF24L01_RX_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_RX_Write_Reg(FLUSH_RX,0xff);          //清除RX FIFO寄存器 
		return 0; 
	}	  	
	return 1;                                      //没收到任何数据
}

//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void RX_Mode(void)
{
	Clr_NRF24L01_RX_CE;	  
	NRF24L01_RX_Write_Reg( SETUP_AW, 0x3 );
	
  //写RX节点地址
  NRF24L01_RX_Write_Buf(SPI_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH);

  //Disable 通道0的自动应答
	NRF24L01_RX_Write_Reg(SPI_WRITE_REG+EN_AA,0x00);    
  //使能通道0的接收地址  	 
	NRF24L01_RX_Write_Reg(SPI_WRITE_REG+EN_RXADDR,0x01);
  //设置RF通信频率		  
  NRF24L01_RX_Write_Reg(SPI_WRITE_REG+RF_CH,rx_freq);	     
  //选择通道0的有效数据宽度 	    
  NRF24L01_RX_Write_Reg(SPI_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);
  //设置TX发射参数,20db增益,0.25Mbps,低噪声增益开启   
	NRF24L01_RX_Write_Reg(SPI_WRITE_REG+RF_SETUP,bandwidth);
  //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,PRIM_RX接收模式 
  NRF24L01_RX_Write_Reg(SPI_WRITE_REG+CONFIG, 0x0f); 
  //CE为高,进入接收模式 
	Set_NRF24L01_RX_CE;      
}			

//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,
//选择RF频道,波特率和LNA HCURR PWR_UP,CRC使能
//当CE变高后,即进入RX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void TX_Mode(void)
{														 
	Clr_NRF24L01_TX_CE;	    
	//Set up Address Width
	NRF24L01_TX_Write_Reg( SETUP_AW, 0x3 );
      
	//写TX节点地址 
  NRF24L01_TX_Write_Buf(SPI_WRITE_REG+TX_ADDR,(uint8_t*)TX_ADDRESS,TX_ADR_WIDTH);    
  //设置TX节点地址,主要为了使能ACK	  
	NRF24L01_TX_Write_Buf(SPI_WRITE_REG+RX_ADDR_P0,(uint8_t*)RX_ADDRESS,RX_ADR_WIDTH); 
	NRF24L01_TX_Write_Reg(SPI_WRITE_REG+RX_PW_P0, TX_PLOAD_WIDTH );
  //Disable 通道0的自动应答
  NRF24L01_TX_Write_Reg(SPI_WRITE_REG+EN_AA,0x00);     
  //Disable All 通道的接收地址
	NRF24L01_TX_Write_Reg(SPI_WRITE_REG+EN_RXADDR,0x00); 
  //设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
  NRF24L01_TX_Write_Reg(SPI_WRITE_REG+SETUP_RETR,0);
	//设置RF通道为24
  NRF24L01_TX_Write_Reg(SPI_WRITE_REG+RF_CH,tx_freq);
	//NRF24L01_Write_Reg(SPI_WRITE_REG+RF_CH,frequency + );
  //设置TX发射参数,20db增益,0.25Mbps,低噪声增益开启   
  NRF24L01_TX_Write_Reg(SPI_WRITE_REG+RF_SETUP,bandwidth);
  //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,PRIM_RX发送模式,开启所有中断
  NRF24L01_TX_Write_Reg(SPI_WRITE_REG+CONFIG, ( 1 << 3 ) | //Enable CRC
                                      ( 1 << 1 )| // PWR_UP
																			( 1 << 2)); // 16bit CRC
  // CE为高,10us后启动发送
	Set_NRF24L01_TX_CE;    
}	


