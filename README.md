# Signal_Generator

Author:    Dean Souleles, KK4DAS
Revision:  1.0,  25-Jan-2020

# Description:
 
 The signal generator outputs a sine wave from DC to ~62 MHz using an Arduino Nano and Analog Devices AD9850 Direct Digital Synthesizer. A 10K linear
 taper potentiomenter is used as a variable attenuator. The display is 16x2 I2C LCD.  The control is a Rotary Digital Encoder with pushbutton. Turn the encoder
 to change frequence.  Push the button to select the tuning increment.
 
 There are various versions of this floatiing around the internet.  This is my implementation.
 
# Build Instructons:
  
     Download the Arduino DDS Library by Paul Darlinton, M0XPD and install using the Arduino IDE library manager.
            https://github.com/m0xpd/DDS
            http://m0xpd.blogspot.com/2014/03/dds-and-duedds-libraries.html
     
     
     Create a folder called KK4DASSigGen and install the the source files there.  The main sketch is KK4DASSigGen.ino.  Open it in the 
     Arduino
     
     A schematic and Fritzing diagram are included for reference.

# Components:
  
      Arduino Nano
      
      Board: WINGONEER DDS Signal Generator Module 0-40MHz AD9850 2 Sine Wave and 2 Square Wave 
          https://www.amazon.com/gp/product/B082143FD9
      Data sheet: https://www.analog.com/media/en/technical-documentation/data-sheets/AD9850.pdf

      16x2 I2C LCD display
            https://www.amazon.com/JANSANE-Arduino-Display-Interface-Raspberry/dp/B07D83DY17

      Rotary Encoder with switch
            https://www.mouser.com/ProductDetail/652-PEC11R-4020F-S24

      10K Linear Taper Potentiometer
            https://www.mouser.com/ProductDetail/Bourns/PDB181-K415K-103B?qs=Zq5ylnUbLm7hRcKeZmRxRw%3D%3D


# Enclosure and permanent mounting:
 
      Enclosure - 3x2" plastic enclosure
            https://www.mpja.com/3in-X-2in-X-11in-Black-Plastic-Box-Enclosure/productinfo/15522+BX/

      Panel mount BNC Connector
            https://www.mpja.com/BNC-Type-Connector-UG-1094-Female-Single-Hole-Mount/productinfo/20507+RC/ 
 
      PCBs = two Adafruit 1/4 sized breadboard PCBs, one for the Arduino and one for the AD-9850 module
            https://www.adafruit.com/product/589

      Mounting hardware - 4-40 standoffs and spacers, machine screws, washers and nuts

      Optional - 0.1" female headers on the breadboard make it easy to wire the components up in the case

# Wiring:

   AD9850  | Arduino
   --------|--------
   W_CLK   | Pin 8 - connect to AD9850 module word load clock pin (CLK)
   FQ_UD   | Pin 9 - connect to freq update pin (FQ)
   DATA    | Pin 10 - connect to serial data load pin (DATA)
   RESET   | Pin 11 - connect to reset pin (RST) 
   VCC     | +5
   Gnd     | Gnd

   Encoder | Arduino
   --------|--------
   A       | 2  -  Encoder pin A  D2 (interrupt pin)
   B       | 3  -  Encoder pin B  D3 (interrupt pin)
   BTN     | A3 -  Encoder push button
   
   LCD/I2C | Arduino
   --------|--------
   SDA     |  4
   SCL     |  5
   VCC     | +5
   Gnd     | Gnd

   Pot | Connects to
   ----|------------
   1   | AD-9850 Sin B
   2   | Wiper goes to center pin of BNC
   3   | Gnd

