#include "stm32f10x.h"
#include "STM32vldiscovery.h"
#include "loopback.h"
#include "delayNS.h"
#include "stdio.h"

#define BUF_SIZE 33
#define DEBUG 

GPIO_InitTypeDef GPIO_InitStructure;

void Delay(__IO uint32_t nCount);
void clk(void);
void sendLAD(uint32_t LAD);
void printBus(uint32_t bus);
char *int2bin(int a, char *buffer, int buf_size);
void sendNum(int num);
void sendAddress(uint32_t tgtAddress);
uint8_t readAddress(uint32_t tgtAddress);
void sendBiosByte(uint8_t byte);

int main(void)
{
	uint32_t tgtAddress = 0x00000000;
	uint16_t biosData;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC , ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	RCC_MCOConfig(RCC_MCO_HSE); // Put on MCO pin the: System clock selected  

	//Initialise Serial
	serialInit();
	
	//LFRAME start high
	GPIOA->ODR |= (1 << 1);
	clk();
	//RESET high
	GPIOA->ODR |= (1 << 0);
	clk();
	
  while (1)
  {
   	biosData = readAddress(tgtAddress);
		sendBiosByte(biosData);
		tgtAddress += 1;
	}
}

uint8_t readAddress(uint32_t tgtAddress){
	uint8_t outputData = 0x0000;
	GPIOA->IDR = 0x00000000;
	
		//LFRAME set low and START on LAD (0)
		sendNum(0);
		GPIOA->ODR &= ~(1 << 1);
		sendLAD(0x0);
		clk();
		
		//LFRAME set High and start of memory read cycle (1)
		sendNum(1);
		GPIOA->ODR |= (1 << 1);
		sendLAD(0x4);   //010Xb
		clk();

    //(2)
		sendNum(2);
    sendLAD(0xF);   //1111b
    clk();

    //(3)
		sendNum(3);
    sendLAD(0xF);   //1111b
    clk();

    //(4)
		sendNum(4);
    sendLAD(0xF);   //1111b
    clk();
		
	  //Send filler nibble and A[17:16]: 11 A[17] A[16]b (5 through 9)
		sendAddress(tgtAddress);
		
		//TAR (10)
		sendNum(10);
    sendLAD(0xF);   //1111b
    clk();
		
		//TAR TriState, float the bus (11)
		sendNum(11);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , DISABLE);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
		clk();
		
		//Apply bitmask and shift to get LAD component and check for SYNC
		if(((GPIOA->IDR & 0xFF) >> 2) == 0x0){
			sendNum(12);
			clk();
			
			//READ GPIOA->IDR (13) across two clock cycles
			sendNum(13);
			outputData = ((GPIOA->IDR & 0xFF) >> 2); //[3:0]
			clk();
			
			//READ GPIOA->IDR (14) 
			sendNum(14);
			outputData |= (((GPIOA->IDR & 0xFF) << 4) >> 2); //[7:4]
			clk();
		}
		
		//CHECK FOR TAR (15)
		if(((GPIOA->IDR  & 0xFF) >> 2) == 0xF){
			sendNum(15);
			clk();
		}
		
		//TAR TriState, float the bus (16)
		sendNum(16);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , DISABLE);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);
		clk();
		
		return outputData;
}

void sendAddress(uint32_t tgtAddress){
		uint16_t byteToWrite;
		int i;
		int numInstr = 6;
		
		//Send filler nibble and A[17:16]: 11 A[17] A[16]b (5)
		sendNum(5);
		byteToWrite |= (1 << 3);
		byteToWrite |= (1 << 2);
		byteToWrite |= ((tgtAddress >> 17) << 1);
		byteToWrite |= ((tgtAddress >> 16) << 0);
		sendLAD(byteToWrite);
		clk();
	
		//Send Address nibbles
		for(i = 15; i >= 0; i-=4){
			sendNum(numInstr);
			byteToWrite = 0x0000;
			byteToWrite |= ((tgtAddress >> i) << 3);
			byteToWrite |= ((tgtAddress >> (i-1)) << 2);
			byteToWrite |= ((tgtAddress >> (i-2)) << 1);
			byteToWrite |= ((tgtAddress >> (i-3)) << 0);
			sendLAD(byteToWrite);  
			clk();
			numInstr++;
		}
}

void printBus(uint32_t bus){
	char buf[sizeof(bus) * 8] = {0};
	int2bin(bus, buf, (sizeof(bus) * 8)-1);
	printf("%s\n\r", buf);
}
void sendBiosByte(uint8_t byte){
	char buf[sizeof(byte) * 8] = {0};
	int2bin(byte, buf, sizeof(byte) * 8);
	printf("%s\n\r", buf);
}
void sendNum(int num){
	#ifdef DEBUG 
		printf("\n%d\r", num);
	#endif
}
void sendLAD(uint32_t LAD){
		#ifdef DEBUG
			usart_snd_str("\nLAD:	    ");
			printBus(LAD);
		
			usart_snd_str("Bus Before: ");
			printBus(GPIOA->ODR);
		#endif
	
		GPIOA->ODR &= ~(1 << 5) &  ~(1 << 4) & ~(1 << 3) & ~(1 << 2); //Clear LAD
    GPIOA->ODR |= (LAD << 2); //Set LAD

		#ifdef DEBUG
			usart_snd_str("Bus After:  ");
			printBus(GPIOA->ODR);
		#endif
}

void clk(){
	//delay_ns(41); 24Mhz clock
	Delay(3000000);
}

void Delay(__IO uint32_t nCount){
  for(; nCount != 0; nCount--);
}

char *int2bin(int a, char *buffer, int buf_size) {
		int i;
    buffer += (buf_size - 1);
	
    for (i = 31; i >= 0; i--) {
        *buffer-- = (a & 1) + '0';

        a >>= 1;
    }

    return buffer;
}


