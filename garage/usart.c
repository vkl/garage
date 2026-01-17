/*
 * usart.c
 *
 * Created: 9/21/2025 1:25:49 PM
 *  Author: vklad
 */ 

#include <avr/io.h>
//#include <avr/interrupt.h>

#include "usart.h"
//#include "lcd.h"


//ISR(USART_RX_vect) {
//	char data = UDR0;
	//USART_Transmit(data);
//	lcd_send_byte(data, Data);
//}

void
USART_Transmit(char data) {
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data;
}

void
USART_SendStr(char *s) {
    while (*s) {
        USART_Transmit(*s);
        s++;
    }
}

void
USART_Init() {
    // Set Baud Rate
    UBRR0H = BAUD_PRESCALER >> 8;
    UBRR0L = BAUD_PRESCALER;
    
    // Set Frame Format
    UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
    
    // Enable Receiver and Transmitter
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);
    
    // Interrupt receive enable
    UCSR0B |= RX_COMPLETE_INTERRUPT;
}