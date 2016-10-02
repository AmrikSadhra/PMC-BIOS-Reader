/**
 * USART Loopback sample for STM32 Discovery board
 *
 * 26.07.2011, Stefan Wendler, devnull@kaltpost.de
 *
 * Reads bytes from RX of USART1, echos them surrounded
 * by squere breaces to TX of USART1.
 *
 * E.g. if you type "A", the program will echo "[A]".
 *
 * Each time a byte is received, the build in green LED is flashed,
 * each time the three output bytes are send, the build
 * in blue LED is flashed.
 *
 * USART1 is configured to 38400, 8, N, 1
 * PA9 is the TX pin, PA10 the RX pin.
 *
 */
#include "loopback.h"
#include "stm32f10x.h"


#define LED_BLUE	8	/* PC8 is the blue LED */
#define LED_GREEN 	9	/* PC9 is the green LED */
#define MODE_ON 	0	/* Offset to add to pin for ON */
#define MODE_OFF	16	/* Offset to add to pin for OFF */

void nmi_handler(void);
void hardfault_handler(void);
void serialInit(void);

/**
 * Four vectors - the starting stack pointer value, 
 * code entry point and NMI and Hard-Fault handlers 
 */
unsigned int * myvectors[4]
__attribute__ ((section("vectors")))= {
    (unsigned int *)	0x20002000,
    (unsigned int *) 	serialInit,
    (unsigned int *)	nmi_handler,
    (unsigned int *)	hardfault_handler
};




/**
 * Initialize the board:
 *  - enable clocks
 *  - setup GPIOs
 *  - setup USAR1
 */
void board_init(void) {
	
    // Most of the peripherals are connected to APB2.  Turn on the
    // clocks for the interesting peripherals
    RCC->APB2ENR = 0
                   // Turn on USART1
                   | RCC_APB2ENR_USART1EN
                   // Turn on IO Port A
                   | RCC_APB2ENR_IOPAEN
                   // Turn on IO Port C
                   | RCC_APB2ENR_IOPCEN;


    // Put PA9  (TX) to alternate function output push-pull at 50 MHz
    // Put PA10 (RX) to floating input
    GPIOA->CRH = 0x000004B0;

    // Configure BRR by deviding the bus clock with the baud rate
    USART1->BRR = 8000000/38400;

    // Enable the USART, TX, and RX circuit
    USART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

/**
 * Delay for "cnt" NOPs.
 *
 * @param[in]	cnt	number of NOPs to delay
 */
void delay(int cnt) {
    while (cnt-- > 0) {
        __ASM("nop");
    }
}

/**
 * Receive a single byte from USART1. This call blocks
 * until the USART signals that a byte was received.
 *
 * @return 	the byte received
 */
int usart_rec(void) {
    /* Wait until the data is ready to be received. */
    while ((USART1->SR & USART_SR_RXNE) == 0);

    // read RX data, combine with DR mask (we only accept a max of 9 Bits)
    return USART1->DR & 0x1FF;
}

/**
 * Send a single byte to USART1. This call blocks
 * until the USART has send the byte.
 *
 * @param[in]		data byte to send
 */
void usart_snd(int data) {
    USART1->DR = data;

    // wait for TX
    while ((USART1->SR & USART_SR_TXE) == 0);
}

/**
 * Send string to USART1
 *
 * @param[in]		str to send
 */
void usart_snd_str(char *str) {
    int   i = 0;

    while(str[i] != 0) {
        usart_snd(str[i++]);
    }
}


void nmi_handler(void) {
    return ;
}

void hardfault_handler(void) {
    return ;
}

void serialInit(void) {

    board_init();

    usart_snd_str("\n\rInitialising Serial\n\r");

//    for(;;) {
//        // read blocking form usart
//        int rec = usart_rec();

//        // flash green led to indicate we received data
//        flash_led(LED_GREEN);

//        if(rec == '\r') {
//            usart_snd_str("\n\r");
//        } else {
//            // send data blocking to usart
//            usart_snd('[');
//            usart_snd(rec);
//            usart_snd(']');
//        }

//        // flash blue led to indicate we send data
//        flash_led(LED_BLUE);
//    }
}
