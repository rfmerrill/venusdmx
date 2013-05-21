#include <avr/io.h>


uint16_t current_address;
uint8_t not_dmx;
uint16_t first_address;

int main(void) {
  uint8_t inbyte = 0;
  uint16_t index = 0;

  current_address = 0;
  first_address = 0;
  not_dmx = 0;
  // Our clock speed is 16000000
  // We want a serial baud rate of 250000 so
  // UBRR = 1000000/(250000) - 1 = 3

  UBRR0H = 0;
  UBRR0L = 3;


  PORTB = 0;
  PORTC = 0;


  // Control signals for the bidirectional motors are
  // on port C, the line for the unidirectional one is B1

  DDRB = 0x02;  // B1 is output
  DDRC = 0x3F;  // 8'b00111111: C0-C5 output.

  // Start address is selected by DIP switches connected to
  // D1-D7 and B0, with B0 as the MSB

  PORTB |= 0x01; // 8'b00000001: B0 pulled up.
  PORTD |= 0xFE; // 8'b11111110: All but D0 pulled up.


  UCSR0B = (1<<RXEN0);      // Enable receiver, not transmitter
  UCSR0C = (1<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01); // 8 data bits, one stop bit, no parity

  for (;;) {


    if (UCSR0A & (1<<RXC0)) {
      // We got a byte!

      if (UCSR0A & (1<<FE0)) {
        // Frame error, beginning of DMX transmission?
        inbyte = UDR0; // Still have to do this

        current_address = 0;
        not_dmx = 0;

        PORTB &= ~(0x04);
        // Check our dipswitches. Only do this once per frame!
        first_address = (~PIND & 0xFE) | (((~PINB) & 1) << 8);
      } else {
        inbyte = UDR0;


        if (!not_dmx) {

          if ((current_address == 0) && (inbyte != 0)) {
            not_dmx = 1;
          } else {
            if (current_address == first_address)
              PORTB |= 0x04;
            switch (current_address - first_address) {

              case 4:
                // Unidirectional motor
                if (inbyte > 128) {
                  PORTB |= 0x02;
                } else {
                  PORTB &= ~0x02;
                }
              break;

              case 3:
                if ((inbyte > 15) && (inbyte < 95)) {
                  PORTC &= ~(1<<0);
                  PORTC |= (1<<1);
                } else if (inbyte > 175) {
                  PORTC &= ~(1<<1);
                  PORTC |= (1<<0);
                } else {
                  PORTC &= ~(1<<1);
                  PORTC &= ~(1<<0);
                }
              break;

              case 2:
                if ((inbyte > 15) && (inbyte < 95)) {
                  PORTC &= ~(1<<2);
                  PORTC |= (1<<3);
                } else if (inbyte > 175) {
                  PORTC &= ~(1<<3);
                  PORTC |= (1<<2);
                } else {
                  PORTC &= ~(1<<2);
                  PORTC &= ~(1<<3);
                }
              break;

              case 1:
                if ((inbyte > 15) && (inbyte < 95)) {
                  PORTC &= ~(1<<4);
                  PORTC |= (1<<5);
                } else if (inbyte > 175) {
                  PORTC &= ~(1<<5);
                  PORTC |= (1<<4);
                } else {
                  PORTC &= ~(1<<5);
                  PORTC &= ~(1<<4);
                }
              break;

            }

            current_address++;
          }
        }
      }
    }
  }
}
