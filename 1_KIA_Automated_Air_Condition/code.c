#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include "mcp_can.h"

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

#define COMPRESSOR 7
#define COOLINGFAN 9
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);    // Set CS pin
LiquidCrystal_I2C lcd(0x27,20,4); //SDA-A20 ; SCL - A21
unsigned char displayRec = 0,adcSun;
volatile unsigned int adcAmb, adcEva, adcPres, adcECT, adcRPM;
volatile int ambient, evaporator, pressure, sun, ect, rpm;
bool acStatus = 0;
unsigned char thoigian;
void setup()
{
  SERIAL.begin(115200);
  pinMode(COMPRESSOR,OUTPUT);
  pinMode(COOLINGFAN,OUTPUT);
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {            // init can bus : baudrate = 500k
    SERIAL.println("CAN BUS Shield init fail");
    SERIAL.println(" Init CAN BUS Shield again");
    delay(100);
  }
  SERIAL.println("CAN BUS Shield init ok!");

  lcd.init();                    
  lcd.backlight();
  
  lcd.setCursor(0,0);
  lcd.print("Amb:");  
  lcd.setCursor(8,0);
  lcd.print("C");
  
  lcd.setCursor(10,0);
  lcd.print("Sun:");
  lcd.setCursor(16,0);
  lcd.print("W/m2");
  
  lcd.setCursor(0,1);
  lcd.print("Cmpress:");

  lcd.setCursor(12,1);
  lcd.print("Eva:");
  lcd.setCursor(19,1);
  lcd.print("C");

  lcd.setCursor(0,2);
  lcd.print("APT:");
  lcd.setCursor(8,2);
  lcd.print("kg/cm2");

  lcd.setCursor(0,3);
  lcd.print("ESS:");
  lcd.setCursor(8,3);
  lcd.print("rpm");  

  lcd.setCursor(12,3);
  lcd.print("ECT:");
  lcd.setCursor(19,3);
  lcd.print("C");

  analogWrite(COOLINGFAN,0);    
}
void loop()
{
    
    //read & convert ambient, evaporator temperature ADC
    adcAmb = analogRead(A0);
    ambient = map(adcAmb,0,1023,50,-50);

    adcEva = analogRead(A1);
    evaporator = map(adcEva,0,1023,10,-10);

    adcPres = analogRead(A2);
    pressure = map(adcPres,0,1023,0,40);

    adcSun = analogRead(A3);
    sun = map(adcSun,0,255,10,70);

    adcECT = analogRead(A4);
    ect = map(adcECT,0,1023,110,27);

    adcRPM = analogRead(A5);
    rpm = map(adcRPM,0,1023,0,2500);

    //read CAN message
    unsigned char len = 0;
    unsigned char buf[8];

    if (CAN_MSGAVAIL == CAN.checkReceive()) {         // check if data coming
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        unsigned long canId = CAN.getCanId();

        SERIAL.println("-----------------------------");
        SERIAL.print("Get data from ID: 0x");
        SERIAL.println(CAN.getCanId());

        for (int i = 0; i < len; i++) { // print the data
            SERIAL.print(buf[i]);
            SERIAL.print("\t");
        }
        SERIAL.println();
    }

    //Check condition to ON/OFF Compressor
    if(thoigian==500)
    {
      if(evaporator >=4 && ambient > 1 && (buf[1] == 41 || buf[1] == 105 || buf[1] == 45 || buf[1] == 109) && (pressure < 31 && pressure >2) 
      && ect < 99 && rpm >= 1000)
      {
        digitalWrite(COMPRESSOR,HIGH); //ON
        acStatus = 1;  
        thoigian = 0;
      }      
    

    else if(evaporator < 3 || ambient < 0 || buf[1] == 32 || buf[1] == 40 || buf[1] == 36 || buf[1] == 44 ||  buf[1] == 104 
            || buf[1] == 108 || pressure >= 31 || pressure <= 2 || ect > 100 || rpm < 1000)
    {
      digitalWrite(COMPRESSOR,LOW);  //OFF
      acStatus = 0;
      thoigian = 0;

    }      
    }


    
    //Check condition to ON/OFF Condenser fan //95 low, 97 high
    if(ect>94 && ect < 96)
    {
      analogWrite(COOLINGFAN,80);
    }
    else if(ect > 96)
    {
      analogWrite(COOLINGFAN,255);      
    }
    else if(ect < 93 )
    {
      analogWrite(COOLINGFAN,0);
    }



    //Debuggg
    /*SERIAL.println("-----------------------------");
    SERIAL.print("acStatus ");
    SERIAL.println(acStatus);
    SERIAL.print(" AMB: ");
    SERIAL.println(ambient);
    SERIAL.print(" EVA: ");
    SERIAL.println(evaporator);*/
    SERIAL.print(" Ect: ");
    SERIAL.println(acStatus);

    //Display LCD every 20 recycle    
    if(displayRec == 200)
    {
        //AC STATUS==============
        if(acStatus == 1)
        {
          lcd.setCursor(8,1);
          lcd.print(" ");          
          lcd.setCursor(9,1);
          lcd.print("ON");
        }
        else if(acStatus == 0)
        {
          lcd.setCursor(8,1);
          lcd.print("OFF");
        }
        //========================
        
        //AMBIENT DISPLAY LCD==================
        if(ambient>=0)
        {
          if(ambient<10)
          {
            lcd.setCursor(4,0);
            lcd.print("  ");
            lcd.setCursor(6,0);  
            lcd.print(ambient);            
          }
          else if(ambient>=10)
          {
            lcd.setCursor(4,0);
            lcd.print(" ");
            lcd.setCursor(5,0);  
            lcd.print(ambient);             
          }                   
        }
        else
        {
          if(ambient>-10)
          {
            lcd.setCursor(4,0);
            lcd.print(" ");
            lcd.setCursor(5,0);
            lcd.print(ambient);             
          } 
          else if(ambient<=-10)
          {
            lcd.setCursor(4,0);
            lcd.print(ambient);                
          } 
        }

        //EVAPORATOR DISPLAY LCD
        if(evaporator>=0)
        {
          if(evaporator<10)
          {
            lcd.setCursor(16,1);
            lcd.print(" ");
            lcd.setCursor(17,1);  
            lcd.print(evaporator);   
            lcd.setCursor(18,1);
            lcd.print(" ");         
          }
          else if(evaporator>=10)
          {
            lcd.setCursor(16,1);  
            lcd.print(evaporator);    
            lcd.setCursor(18,1);
            lcd.print(" ");         
          }                   
        }
        else
        {
          if(evaporator>-10)
          {
            lcd.setCursor(16,1);
            lcd.print(evaporator);
            lcd.setCursor(18,1);
            lcd.print(" ");             
          } 
          else if(evaporator<=-10)
          {
            lcd.setCursor(16,1);
            lcd.print(evaporator);                
          } 
        }

        //APT DISPLAY LCD
        if(pressure>=0 && pressure<10)
        {
           lcd.setCursor(4,2);
           lcd.print("  ");
           lcd.setCursor(6,2);  
           lcd.print(pressure);            
         }
         else if(pressure>=10)
         {
           lcd.setCursor(4,2);
           lcd.print(" ");
           lcd.setCursor(5,2);  
           lcd.print(pressure);             
         }          

        //SUN DISPLAY LCD
        if(sun>=10)
        {
          lcd.setCursor(14,0); 
          lcd.print(sun);             
        }

        //ECT DISPLAY LCD
         if(ect>=10 && ect <100)
         {
           lcd.setCursor(16,3);
           lcd.print(ect);
           lcd.setCursor(18,3);  
           lcd.print(" ");             
         }
         else if(ect>=100)
         {
           lcd.setCursor(16,3);
           lcd.print(ect); 
         }

        //RPM DISPLAY LCD
        if(rpm>=0 && rpm<10)
        {
           lcd.setCursor(4,3);
           lcd.print("  ");
           lcd.setCursor(6,3);  
           lcd.print(rpm);
           lcd.setCursor(7,3);
           lcd.print(" ");            
         }
         else if(rpm>=10 && rpm <100)
         {
           lcd.setCursor(4,3);
           lcd.print(" ");
           lcd.setCursor(5,3);  
           lcd.print(rpm);
           lcd.setCursor(7,3);
           lcd.print(" ");                         
         }
         else if(rpm>=100 && rpm <1000)
         {
           lcd.setCursor(4,3);
           lcd.print(rpm);
           lcd.setCursor(7,3);
           lcd.print(" ");                      
         }
         else if(rpm>=1000)
         {
           lcd.setCursor(4,3);
           lcd.print(rpm);
         }            
        displayRec = 0;      
    }
    //===================================================
    displayRec++;       
    thoigian++; 
    delay(10);
}