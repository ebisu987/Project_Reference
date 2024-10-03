#include <mega328p.h>
#include <delay.h>
 int i=0; 
// chương trình ngắt khi có tín hiệu PIR lên 5V 
interrupt [EXT_INT1] void ext_int1_isr(void)
{
        if(PIND.2==1) // nếu khóa cửa mở thì thực hiện chương trình
        { 
            if(i<1) /
            {
                PORTD.5=1;
                PORTB.5=1;
// nếu chân D4 có 5v tức relay motor đã kích thì tăng biến đếm lên 1
                if (PIND.4==1) i+=1; 
                delay_ms(500);
            }
        else {}
        } 
        else {}
}


void main(void)
{
// Declare your local variables here

// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif



// Khai báo các chân nhập xuất của vi điều khiển
DDRD=0b11100000;
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (1<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);




// thiết lập chế độ ngắt ngoài
// External Interrupt(s) initialization
// INT0: on
// INT1: On
// INT1 Mode: Low level
// Interrupt on any change on pins PCINT0-7: Off
// Interrupt on any change on pins PCINT8-14: Off
// Interrupt on any change on pins PCINT16-23: Off
EICRA=(1<<ISC11) | (1<<ISC10) | (1<<ISC01) | (1<<ISC00);
EIMSK=(1<<INT1) | (1<<INT0);
EIFR=(1<<INTF1) | (1<<INTF0);



// Global enable interrupts
#asm("sei")



while (1)
      {
      
       
      PORTD.5=0;
      PORTB.5=0;
      
      if(i==1) // nếu biến i=1 thì motor đã hạ kính xong  nên bật đèn và còi delay 1 giây
      {
      PORTD.6=1;
      delay_ms(1000);
      PORTD.6=0;
      delay_ms(1000);
      }
// nếu khóa cửa đóng lại thì biến i=0 mạch quay về trạng thái ban đầu 
      if(PIND.2==0) i=0; 
      delay_ms(1000);
      }
}
