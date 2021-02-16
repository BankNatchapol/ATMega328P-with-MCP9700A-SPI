#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

void USART_Init(unsigned int ubrr) { 
    /* Set baud rate */
    UBRR0 = ubrr;
    /* Enable receiver and transmitter */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
    /* Set frame format: 8data*/ 
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 
}

void USART_Transmit( unsigned char data ) { 
    /* Wait for empty transmit buffer */ 
    while ( !( UCSR0A & (1 << UDRE0)) );
    /* Put data into buffer, sends the data */ 
    UDR0 = data; 
}

void SPI_Init()
{
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    PORTB &= ~(1 << PB2);
    DDRB |= (1 << DDB5);      // SCK DDR->OUT
	DDRB |= (1 << DDB3);    // MOSI DDR->OUT
	DDRB |= (1 << DDB2);    // /SS DDR -> OUT

}

uint16_t SPI_Read()
{
    uint8_t temp;
    uint8_t temp2;
    
    /*Chip select low*/
    PORTB &= ~(1 << PB2);
            
    /*put dummy byte in SPDR*/
    SPDR = 0xF;
    
    /*wait for SPIF high*/
    while (!(SPSR & (1 << SPIF)));
        
    /*copy SPDR out*/
    temp = (SPDR >> 1);
    /*put dummy byte in SPDR*/
    SPDR = 0xF;
    
    /*wait for SPIF high*/
    while (!(SPSR & (1 << SPIF)));
    
    /*copy SPDR out*/
    temp2 = SPDR & 0b00011111;
    temp2 = (temp2 << 7);
    
    /*Chip select high*/
    PORTB |= (1 << PB2);
            
    /*return*/
    return temp2 + temp;
}

int main(void) {
    int i = 0;
    unsigned int sensor_data;
    unsigned int temperature;
    unsigned char temperature_char[5];
    
    USART_Init(51);
    SPI_Init();
    
    while (1) {
        /*read data from sensor*/
        sensor_data = SPI_Read();
        
        /*Convert 12-bit data to temperature*/   
        temperature = ((sensor_data/4095.0)*5 - 0.5) * 100;
       
         /*Convert to String*/
        sprintf(temperature_char, "%d ",temperature);
                
        /*Transmit via USART*/
        i = 0;
        while(temperature_char[i]!=NULL){
            USART_Transmit(temperature_char[i]);
            i++;
        }
        _delay_ms(1000);
    }
}
