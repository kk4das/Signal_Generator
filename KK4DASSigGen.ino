//
// Signal Generator
//
// Author:    Dean Souleles, KK4DAS
// Revision:  1.0,  25-Jan-2020
//
// Description:
// 
// Signal generator outputs a sine wave from DC to ~62 MHz using and Arduino Nano and Analog Devices AD9850 Direct Digital Synthesizer
//
//      Data sheet:
//            https://www.analog.com/media/en/technical-documentation/data-sheets/AD9850.pdf
//
//      Board: WINGONEER DDS Signal Generator Module 0-40MHz AD9850 2 Sine Wave and 2 Square Wave 
//            https://www.amazon.com/gp/product/B082143FD9/
//
//      Requires: DDS Library: Paul Darlinton, M0XPD
//            https://github.com/m0xpd/DDS
//            http://m0xpd.blogspot.com/2014/03/dds-and-duedds-libraries.html
//
// Other components:
//      16x2 I2C LCD display
//            https://www.amazon.com/JANSANE-Arduino-Display-Interface-Raspberry/dp/B07D83DY17
//
//      Rotary Encoder with switch
//            https://www.mouser.com/ProductDetail/652-PEC11R-4020F-S24
//
//      10K Linear Taper Potentiometer
//            https://www.mouser.com/ProductDetail/Bourns/PDB181-K415K-103B?qs=Zq5ylnUbLm7hRcKeZmRxRw%3D%3D
//
//
// Enclosure and permanent mounting:
//      Enclosure - 3x2" plastic enclosure
//            https://www.mpja.com/3in-X-2in-X-11in-Black-Plastic-Box-Enclosure/productinfo/15522+BX/
//
//      Panel mount BNC Connector
//            https://www.mpja.com/BNC-Type-Connector-UG-1094-Female-Single-Hole-Mount/productinfo/20507+RC/ 
// 
//      PCBs = two Adafruit 1/4 sized breadboard PCBs, one for the Arduino and one for the AD-9850 module
//            https://www.adafruit.com/product/589
//
//      Mounting hardware - 4-40 standoffs and spacers, machine screws, washers and nuts
//
//      Optional - 0.1" female headers on the breadboard make it easy to wire the components up in the case
//

#include <DDS.h>                // DDS 9850 Library
#include <LiquidCrystal_I2C.h>  // LCI via I2C bus
#include "Rotary.h"             // Local copy of rotary library

//#define DEBUG   // enable debug messages to serial
#ifdef DEBUG
char debugmsg[40];
#endif


#define CALLSIGN "KK4DAS"
#define STARTUP_FREQUENCY 1000
#define STARTUP_INCREMENT 1000

//////////////////////////////////////////////////////////////////////
// AD9850 Pin Numbers                                               //
//////////////////////////////////////////////////////////////////////
#define W_CLK 8           // Pin 8 - connect to AD9850 module word load clock pin (CLK)
#define FQ_UD 9           // Pin 9 - connect to freq update pin (FQ)
#define DATA 10           // Pin 10 - connect to serial data load pin (DATA)
#define RESET 11          // Pin 11 - connect to reset pin (RST) 

//////////////////////////////////////////////////////////////////////
// Rotary Encoder Pin Numbers                                        //
//////////////////////////////////////////////////////////////////////
#define ENCODER_A    2    // Encoder pin A  D2 (interrupt pin)
#define ENCODER_B    3    // Encoder pin B  D3 (interrupt pin)
#define ENCODER_BTN  A3   // Encoder push buttonh

//////////////////////////////////////////////////////////////////////
// I2C Bus Pins for LCD                                             //
//      SDA          4                                              //
//      SCL          5                                              //
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  Hardware Definitions                            //
//              (Display, AD9850, Rotary Encoder                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////
DDS dds(W_CLK, FQ_UD, DATA, RESET);      // Instantiate the DDS
LiquidCrystal_I2C lcd(0x27,16,2);        // LCD address: 0x27,16 chars by 2 line display
Rotary r = Rotary(ENCODER_A,ENCODER_B);  // sets the pins the rotary encoder.  Must be interrupt pins.

//////////////////////////////////////////////////////////////////////
//                                                                  //
//              Rotary Encoder Variables                            //
//   Number of clockwise and counterclockwise ticks                 //
//   Delta between successive measurements                          //
//                                                                  // 
//////////////////////////////////////////////////////////////////////

volatile int encoder_count = 0; // count of encoder clicks +1 for CW, -1 for CCW  (volatile since used in ISR)
int prev_encoder_count = 0;     // used to measure changes over time
int encoder_delta = 0;          // differrnce between successive checks of encoder count
                                // when multiplied by tuning increment tells what the frequency change on 
                                // on the active VFO will be

//////////////////////////////////////////////////////////////////////
//                                                                  //
//              Button Control Variables                            //
//   Encoder button control (sets the increment)                    //
//                                                                  // 
//////////////////////////////////////////////////////////////////////
int EncButtonState = 0;
int lastEncButtonState = 0;


//////////////////////////////////////////////////////////////////////
//                                                                  //
//              Current Frequency and Increment                     //
//                                                                  // 
//////////////////////////////////////////////////////////////////////
int_fast32_t freq = STARTUP_FREQUENCY;      //  Startup frequency
uint32_t increment = STARTUP_INCREMENT;     //  Tuning startup VFO tuning increment in HZ.




//////////////////////////////////////////////////////////////////////
//                                                                  //
//                    Display Routines                              //
//                                                                  //
// Display Layout 26x2                                              //
//              111111                                              //
//    0123456789012345                                              //
// 0:  MM.KKK.HHH                                                   //          
// 1:     NNN XHz                                                   //
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// displayFrequency                                                 //
//   Display frequency                                              //
//   " MM.KKK.HHH Mhz "                                             //
//////////////////////////////////////////////////////////////////////
void displayFrequency(int_fast32_t fq) {
  byte ones,tens,hundreds,thousands,tenthousands,hundredthousands,millions ;  //Placeholders
 
  millions = (fq/1000000);
  hundredthousands = ((fq/100000)%10);
  tenthousands = ((fq/10000)%10);
  thousands = ((fq/1000)%10);
  hundreds = ((fq/100)%10);
  tens = ((fq/10)%10);
  ones = ((fq/1)%10);
  
  lcd.setCursor(0,0);
  lcd.print("                ");
  if (millions > 9){
    lcd.setCursor(1,0);
  } else {
    lcd.setCursor(2,0);
  }
    lcd.print(millions);
    lcd.print(".");
    lcd.print(hundredthousands);
    lcd.print(tenthousands);
    lcd.print(thousands);
    lcd.print(".");
    lcd.print(hundreds);
    lcd.print(tens);
    lcd.print(ones);
    lcd.print(" Mhz  ");
};

void displayIncrement(long increment) {
  String hertz = "        "; // tune step display

  #ifdef DEBUG
    sprintf(debugmsg, "Increment: %ld",increment);
    Serial.println(debugmsg);
  #endif
  switch (increment) {
    case 10:
      hertz = "   10 Hz";
      break; 
    case 50:
      hertz = "   50 Hz";
      break; 
    case 100:
      hertz = "  100 Hz";
      break; 
    case 500:
      hertz = "  500 Hz";
      break; 
    case 1000:
      hertz = "   1K Hz";
      break; 
    case 2500:
      hertz = " 2.5K Hz";
      break; 
    case 5000:
      hertz = "   5K Hz";
      break; 
    case 10000:
      hertz = "  10K Hz";
      break; 
    case 100000:
      hertz = " 100K Hz";
      break; 
    case 1000000:
      hertz = "   1M Hz";
      break; 
  }
      
  lcd.setCursor(4,1);
  lcd.print(hertz); 

}


//////////////////////////////////////////////////////////////////////
//                                                                  //
//                  User Input Handling                             //
//                                                                  //
//////////////////////////////////////////////////////////////////////


void CheckEncoder() {
  int current_count = encoder_count;  // grab the current encoder_count
  long encoder_delta = 0;

  if (current_count != prev_encoder_count) {  // if there is any change in the encoder coount
    
    #ifdef DEBUG
      sprintf(debugmsg, "Freq: %ld", freq);
      Serial.println(debugmsg);
    #endif

    //
    //  Calculate the delta (how many click positive or negaitve)
    //
    encoder_delta = current_count - prev_encoder_count;
    
    //
    //  Calculate and display the new frequency
    //
    freq = freq + (encoder_delta * increment);
    dds.setFrequency(freq);
    displayFrequency(freq);
    
    #ifdef DEBUG
      sprintf(debugmsg, "current_count: %d, New Freq: %ld", current_count, freq);
      Serial.println(debugmsg);
    #endif

    prev_encoder_count = current_count;  // save the current_count for next time around
    
  }
}

//*****************************************
void CheckIncrement (){

  EncButtonState = digitalRead(ENCODER_BTN);
  
  if(EncButtonState != lastEncButtonState){ 
    #ifdef DEBUG
      sprintf(debugmsg, "Encoder button state: %d", EncButtonState);
      Serial.println(debugmsg);
    #endif
    if(EncButtonState == LOW) {
      if (increment == 10){increment = 100;}          
      else if (increment == 100){increment = 1000;}
      else if (increment == 1000){increment = 10000;}
      else if (increment == 10000){increment = 100000;}
      else if (increment == 100000){increment = 1000000;}
      else {
        increment = 10; 
     }      
     displayIncrement(increment);
    }
    lastEncButtonState = EncButtonState;
    delay(50); // debounce
    EncButtonState = digitalRead(ENCODER_BTN);
  }
  
  
}
///////////////////////////////////////////////////////////
//      ************* ISR ****************               //
// Interrupt service routine, called on encoder movement //
// Only interested in completed clicks                   //
//  +1 for Clocwwise                                     //
//  -1 for Counter Clockwise                             //
//  Ignore intermediate values                           //
///////////////////////////////////////////////////////////

ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result == DIR_CW) {
    encoder_count++;
  } else if (result == DIR_CCW) {
    encoder_count--;
  }
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//         Setup and Initialization Routines                       //
//                                                                 //
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// setupISR                                                         //
//   Trigger Interrupt Rotary encoder changes                       //
//////////////////////////////////////////////////////////////////////
void setupISR() {
//...........................
  PCICR |= (1 << PCIE2);                    //sets interupt pins D2 D3
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);         
  sei();
//............................
}

//*********************Setup Arduino Pins******************************

void setupPins() {
  pinMode(ENCODER_BTN,INPUT); digitalWrite(ENCODER_BTN,HIGH); // Encoder switch - STEP  
}

//////////////////////////////////////////////////////////////////////
// setupDDS                                                         //
//   Initialize the DDS                                             //
//   Optionally calibrate the oscillator                            //
//     Only needed if testing shows frequency is off                //
//   Set startup frequency                                          //
//////////////////////////////////////////////////////////////////////
void setupDDS() {
  dds.init();
  // dds.trim(125000000);  // Optional oscilattor trim
  dds.setFrequency(freq);
}

//////////////////////////////////////////////////////////////////////
// setupDisplay                                                     //
//   Initialze the display and show the splash screen               //
//////////////////////////////////////////////////////////////////////
void setupDisplay() {
  lcd.init();
//  lcd.backlight();
  lcd.noBacklight();
  lcd.print(CALLSIGN);
  lcd.print(": AD9580");
  lcd.setCursor(0,1);
  lcd.print("Signal Generator");
  delay(3000);
  lcd.clear();
  displayFrequency(freq);
  displayIncrement(increment);
}

//////////////////////////////////////////////////////////////////////
// setup()                                                          //
//   Called once at startup by the OS                               //
//   Initializes AD9580, Rotary Encoder and Display                 //
//////////////////////////////////////////////////////////////////////
void setup() {
  
  #ifdef DEBUG
    Serial.begin(57600);
  #endif
  
  setupISR();      // enable interrupts for digital encoder
  setupPins();     // Initialize Arduino pins
  setupDDS();      // initialize the AD5840 
  setupDisplay();  // setup the display and splash screen
}

//////////////////////////////////////////////////////////////////////
//                         Main Loop                                //
//                                                                  //
// The main loop cycles forever looking for                         //
//    * Frequency changes (rotor turned)                            //
//    * Encoder button for freq step change                         //
//
//////////////////////////////////////////////////////////////////////
void loop(){ 
 CheckEncoder();   // Frequency changes
 CheckIncrement(); // Encoder Button
}
