#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#define CS PORTB2
#define CS0 DDB2
#define MOSI DDB3
#define CLK DDB5

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
    /* Set CLK, MOSI, CS as output */
    DDRB |= (1<<CLK) | (1<<MOSI) | (1<<CS);
    /* Chip select high*/
    PORTB |= (1<<CS0);
    /* Enable SPI, Master mode, clk/16 */
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

uint16_t SPI_Read()
{
    uint16_t temp;
    uint16_t temp2;
    
    /*Chip select low*/
    PORTB &= ~(1<<CS0);
    /*put dummy byte in SPDR*/
    SPDR = 0xFF;
    /*wait for SPIF high*/
    while (!(SPSR & (1 << SPIF)));
    /*copy SPDR out*/
    temp = SPDR & 0b0000000000111111;
    temp = (temp << 7);
    /*put dummy byte in SPDR*/
    SPDR = 0xFF;
    /*wait for SPIF high*/
    while (!(SPSR & (1 << SPIF)));
    /*copy SPDR out*/
    temp2 = SPDR & 0b0000000011111110;
    temp2 = (temp2 >> 1);
    /*Chip select high*/
    PORTB |= (1 << PB2);
    /*Concatenate data and return*/
    return temp | temp2;
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
