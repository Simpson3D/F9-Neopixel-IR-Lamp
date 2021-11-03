/*
  Version: 10.5.0 (RELEASE)

  Created by: Ryan M Simpson - Simpson 3D


  Thanks:

  Thanks to the other members of my team (you know who you are) who helped make this a reality)
  I would also like to thank the AWESOME team at Adafruit Industries for helping troubleshoot errors (https://www.adafruit.com)

  YOU WILL NEED:

  > An Arduino Uno or similar c++ based micro-controller
  > An Infared sensor
  > A capacitor - Minimum 100μf and max 1000μf (OPTIONAL)
  > A resistor - 400 OHM (or any resistors totalling up to that value) (OPTIONAL)
  > Wire (ANY)
  > A soldering iron
  > Solder
  > Arduino IDE (hopefully what you're viewing this code in?)
  > RGB Neopixel ring with 8 pixels



  INFO:

  After many late nights, way too much coffee, and alot of hard work, here it is:

  This project has been the most exciting project we have ever worked on,
  and we are so happy to be able to share it, in all of its glory, with the world. In total we have been working on this project for 64 days, and every day was worth it.

  There has been so many delays and setbacks, but, we think it's come out alright in the end.

  The original design used the Kitronik Alarm PCB, but we had multiple issues with getting the two boards to work together,
  we have left the mount piece for this in the design for if someone wishes to try to get the two to work together,
  the original design aslo included a transistor for controlling the Kitronik Alarm PCB on/off, this has also been removed.

  The project is fully 3d printed, with an 8 LED neopixel ring, an Arduino Uno, A piezo buzzer, an infrared reciever (+ remote),
  a push switch, and wire. (Including all the tools necessary to use these). And optionally, a 1000μf capacitor and a 400 OHM resistor (or any resistors totalling up to that value).

  Piezo buzzer - https://kitronik.co.uk/products/piezo-buzzer-with-drive
  Arduino Uno - https://store.arduino.cc/arduino-uno-rev3
  8 LED neopixel ring - https://www.amazon.co.uk/Cool-Components-LED-32mm-Ring/dp/B07NLFP8BN
  IR Remote and Reciever - https://www.amazon.co.uk/Youmile-Infrared-Receiver-Wireless-Control/dp/B07RFB5KCT
  Push Switch - https://kitronik.co.uk/products/micro-switch-with-medium-lever
  Check out the latest version of the code: https://sites.google.com/view/simpson3d/arduino/neopixel-lamp-alarm/code
  (Code may update up to 1-2 times a month)




  Find most recent version here: https://sites.google.com/view/simpson3d/arduino/neopixel-toggle/code
  View a circuit diagram here:https://sites.google.com/view/simpson3d/arduino/neopixel-toggle/wiring-diagram

  Find out more at: https://sites.google.com/view/simpson3d

  © Simpson 3D - 2021
  This work is licensed under a Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.


  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.


*/


/*
    Note by Adafruit
    IMPORTANT: To reduce NeoPixel burnout risk, add 1000 μF capacitor across
    pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
    and minimize distance between Arduino and first pixel.  Avoid connecting
    on a live circuit...if you must, connect GND first.
*/




// Add the libraries necessary for functioning - Libraries add scripts that make coding alot easier.

#include <Adafruit_NeoPixel.h>                        // Found here: https://github.com/adafruit/Adafruit_NeoPixel
#include <IRLibAll.h>                                // Found here: https://github.com/cyborg5/IRLib2
#include <EEPROM.h>                                 // Found here: Included with the Arduino IDE


/**********************************************************************CHANGE THESE VARIABLES**********************************************************************/



int NeoPin = 4 ;                        // Set this as the pin your neopixel ring/strip is connected to
int Buzpin = 2 ;                       // Set this as the pin you buzzer is attached to
int IRPin = 11 ;                      // The pin the IR reciever is attached to
int AlarmInput = 9 ;                 // The pin that the LED will input to
int AlarmOutput = 8 ;               // This pin will provide a constant HIGH, this goes to the COM port on the switch
int numberofpixels = 8 ;           // Set this as the number of pixels in your ring/string
int rotationspeed = 40 ;          // The speed at which the NeoPixel ring completes the animation
int brightness = 100 ;           // Set the brightness of the ring/strip at default (%)
int maxbrightness = 100 ;       // Set this as the maximum brightness of the lamp that you want (%)
int brincrement = 5;           // Use this to adjust the change in brightness per key press (+ & -)
int alarmtime = 10 ;          // This is the amount of cycles the arduino completes before the alarm goes off

/**********************************************************************DO NOT CHANGE THESE VARIABLES**********************************************************************/



int count_value = 0 ;                 // Used to identify the stage/colour the arduino is on
int onoff = 1 ;                      // Used for the arduino to know wether the lamp should be on or off
int pulse = 0 ;                     // This is used to prevent infinite loops or repeats
int toggle = 0 ;                   // Used to idenify wether the Arduino should go back to the colour it was on before it was turned off
int toggle2 = 0 ;                 // Used to identify wether the alarm should be turned on or not
int pulse2 = 0 ;                 // This is used to prevent infinite loops or repeats
int alarmlight = 0 ;            // Used to identify wether the yellow light should be on or not to indicate alarm on or off
int alarmpin ;                 // Used to check the 'AlarmInput' pin for a volatge input
int alarmcount = 0 ;          // Used to count the amount of alarm warning cycles the Arduino has completed
int countdown = 0 ;          // Used to identify if the alarm should be in "WARNING" mode
int alarmprev = 0 ;         // Used to identify if the alarm is going off or not
int beeponoff = 1 ;        // This keeps track of wether the "beep" should occur on button presses
int buzonoff = 0;         // This is used to identify if the user wants the beep noise on, or off
int ResNum;              // This keeps track of the number of resets the system has had
int FIRSTUPLOAD = 0;    // This is used to indicate if this is the first upload of this code (if it is, please leave this as 0, if not, set it to 1)

/**********************************************************************END OF VARIABLES**********************************************************************/

/*
   Initiate NeoPixel ring with the values that are given above

   Parameter 1 = number of pixels in strip
   Parameter 2 = Arduino pin number (most are valid)
   Parameter 3 = pixel type flags, add together as needed:
   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

*/
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numberofpixels, NeoPin, NEO_GRB + NEO_KHZ800);



IRrecv myReceiver(IRPin);    // Initiate the IR reciever on the pin given above
IRdecode myDecoder;         // Decoder object




void setup() {

  FIRSTUPLOAD = EEPROM.read(75);      // Set 'FIRSTUPLOAD' to the value of byte 75 (should be 255 if unchanged)

  if (FIRSTUPLOAD == 255) {         // If 'FIRSTUPLOAD' is 255 (Default), then
    count_value = 0 ;                 // Used to identify the stage/colour the arduino is on
    onoff = 1 ;                      // Used for the arduino to know wether the lamp should be on or off
    pulse = 0 ;                     // This is used to prevent infinite loops or repeats
    toggle = 0 ;                   // Used to idenify wether the Arduino should go back to the colour it was on before it was turned off
    toggle2 = 0 ;                 // Used to identify wether the alarm should be turned on or not
    pulse2 = 0 ;                 // This is used to prevent infinite loops or repeats
    alarmlight = 0 ;            // Used to identify wether the yellow light should be on or not to indicate alarm on or off
    alarmpin ;                 // Used to check the 'AlarmInput' pin for a volatge input
    alarmcount = 0 ;          // Used to count the amount of alarm warning cycles the Arduino has completed
    countdown = 0 ;          // Used to identify if the alarm should be in "WARNING" mode
    alarmprev = 0 ;         // Used to identify if the alarm is going off or not
    beeponoff = 1 ;        // This keeps track of wether the "beep" should occur on button presses
    buzonoff = 0;         // This is used to identify if the user wants the beep noise on, or off
    ResNum = 0;          // This keeps track of the number of resets the system has had

    EEPROM.write(0, count_value);               // Write the value of 'Count_value' to byte 0 of EEPROM
    EEPROM.write(1, brightness);               // Write the value of 'brightness' to byte 1 of EEPROM
    EEPROM.write(2, onoff);                   // Write the value of 'onoff' to byte 2 of EEPROM
    EEPROM.write(10, toggle);                // Write the value of 'toggle' to byte 10 of EEPROM
    EEPROM.write(30, alarmprev);            // Write the value of 'alarmprev' to byte 30 of EEPROM
    EEPROM.write(31, toggle2);             // Write the value of 'toggle2' to byte 31 of EEPROM
    EEPROM.write(32, alarmlight);         // Write the value of 'alarmlight' to byte 32 of EEPROM
    EEPROM.write(33, countdown);         // Write the value of 'countdown' to byte 33 of EEPROM
    EEPROM.write(34, alarmcount);       // Write the value of 'alarmcount' to byte 34 of EEPROM
    EEPROM.write(35, ResNum);          // Write the value of 'ResNum' to byte 35 of EEPROM
    EEPROM.write(75, 0);              // Write 0 to byte 75 of EEPROM
    FIRSTUPLOAD = 0;
  }

  strip.begin();                    // Initialise the Neopixel ring
  strip.show();                    // Initialize all pixels to 'off'
  myReceiver.enableIRIn();        // Start the receiver

  pinMode(AlarmInput , INPUT);              // Set the pin 'AlarmPin' (declared above) as an input
  pinMode(AlarmInput, INPUT_PULLUP);       // Connect the pin 'AlarmInput' to the internal pullup resistor
  pinMode(LED_BUILTIN, OUTPUT);           // Engage the built in LED
  pinMode(AlarmOutput, OUTPUT);          // Set the pin 'AlarmOutput' (declared above) as an output (COM on switch)
  pinMode(Buzpin, OUTPUT);              // Set the pin 'Buzpin' (declared above) as an output (buzzer)
  digitalWrite(LED_BUILTIN, LOW);      // Set the built in LED to off
  digitalWrite(Buzpin, LOW);          // Set the Buzzer to off
  digitalWrite(AlarmOutput, LOW);    //Set the pin 'AlarmOutput' to off

  count_value = EEPROM.read(0);           // Set 'count_value' to the value of byte 0 in EEPROM
  brightness = EEPROM.read(1);           // Set 'brightness' to the value of byte 1 in EEPROM
  onoff = EEPROM.read(2);               // Set 'onoff' to the value of byte 2 in EEPROM
  toggle = EEPROM.read(10);            // Set 'toggle' to the value of byte 10 in EEPROM
  alarmprev = EEPROM.read(3);         // Set 'alarmprev' to the value of byte 3 in EEPROM
  toggle2 = EEPROM.read(31);         // Set 'toggle2' to the value of byte 31 in EEPROM
  beeponoff = EEPROM.read(32);      // Set 'beeponoff' to the value of byte 32 in EEPROM
  countdown = EEPROM.read(33);     // Set 'countdown' to the value of byte 33 in EEPROM
  alarmcount = EEPROM.read(34);   // Set 'alarmcount' to the value of byte 34 in EEPROM
  ResNum = EEPROM.read(35);

  ResNum++;                                 // Add 1 to ResNum
  EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM

  if (ResNum == 2) {                       // If 'ResNum' is 2 (second reset), then....
    count_value = 0 ;                 // Used to identify the stage/colour the arduino is on
    onoff = 1 ;                      // Used for the arduino to know wether the lamp should be on or off
    pulse = 0 ;                     // This is used to prevent infinite loops or repeats
    toggle = 0 ;                   // Used to idenify wether the Arduino should go back to the colour it was on before it was turned off
    toggle2 = 0 ;                 // Used to identify wether the alarm should be turned on or not
    pulse2 = 0 ;                 // This is used to prevent infinite loops or repeats
    alarmlight = 0 ;            // Used to identify wether the yellow light should be on or not to indicate alarm on or off
    alarmpin ;                 // Used to check the 'AlarmInput' pin for a volatge input
    alarmcount = 0 ;          // Used to count the amount of alarm warning cycles the Arduino has completed
    countdown = 0 ;          // Used to identify if the alarm should be in "WARNING" mode
    alarmprev = 0 ;         // Used to identify if the alarm is going off or not
    beeponoff = 1 ;        // This keeps track of wether the "beep" should occur on button presses
    buzonoff = 0;         // This is used to identify if the user wants the beep noise on, or off

    EEPROM.write(0, count_value);               // Write the value of 'Count_value' to byte 0 of EEPROM
    EEPROM.write(1, brightness);               // Write the value of 'brightness' to byte 1 of EEPROM
    EEPROM.write(2, onoff);                   // Write the value of 'onoff' to byte 2 of EEPROM
    EEPROM.write(10, toggle);                // Write the value of 'toggle' to byte 10 of EEPROM
    EEPROM.write(30, alarmprev);            // Write the value of 'alarmprev' to byte 30 of EEPROM
    EEPROM.write(31, toggle2);             // Write the value of 'toggle2' to byte 31 of EEPROM
    EEPROM.write(32, alarmlight);         // Write the value of 'alarmlight' to byte 32 of EEPROM
    EEPROM.write(33, countdown);         // Write the value of 'countdown' to byte 33 of EEPROM
    EEPROM.write(34, alarmcount);       // Write the value of 'alarmcount' to byte 34 of EEPROM
    ResNum = 0;                               // Set 'ResNum' to 0
    EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
  }
  else {
    if (onoff == 0) {       // If 'onoff' is 0 then....
      toggle = 1;          // Set toggle to 1
    }

    strip.setBrightness(brightness);    // Set the ring to its brightness


    colourReturn();                   // Call for the function "colourReturn" to be completed
  }
}





// Begin loop
void loop() {

  ircheck ();                            // Call for the function "ircheck" to be completed

  unknowncoderecv();                   // Call for the function "unknowncoderecv" to be completed

  alarmcheck();                      // Call for the function "alarmcheck" to be completed

  alarmcountdown();                 // Call for the function "alarmcountdown" to be completed

  myReceiver.enableIRIn();         //Restart the receiver

  if (buzonoff == 0) {               // If buzonoff is 0, then....
    digitalWrite(Buzpin, LOW);      // Set the Buzzer to off
    buzonoff = 1;                  // Set the 'buzonoff' to 1
  }

  if (alarmprev == 1) {         // If 'alarmprev' is 1 then....
    alarmflash();              // Call for the function 'alarmflash' to be completed
  }

}


/*******************************************************************************************************************************************************************/

void alarmcountdown() {                                   // Create function
  if (countdown == 1 && alarmprev == 0) {                // If 'countdown' is 1 and 'alarmprev' is 0 then....
    alarmcount++ ;                                      // Add 1 to 'alarmcount'
    strip.setBrightness(40);                           // Set the Neopixel ring to 40% brightness
    colourWipe(strip.Color(255, 255, 0)) ;            // Set the colour to yellow
    colourWipe(strip.Color(0, 0, 0)) ;               // Turn the Neopixel ring off
    if (alarmcount == alarmtime) {                  // If 'alarmcount' is equal to 'alarmtime' then....
      countdown = 0;                               // Set 'countdown' to 0
      alarmflash();                               // Call for the 'alarmflash' function to be completed
      alarmcount = 0;                            // Set 'alarmcount' to 0
      alarmprev = 1 ;                           // Set 'Alarmprev' to 1
    }
  }
  EEPROM.write(34, alarmcount);             // Write the value of 'alarmcount' to Byte 34 of EEPROM
  EEPROM.write(33, countdown);             // Write the value of 'countdown' to Byte 33 of EEPROM
  EEPROM.write(30, alarmprev);            // Write the value of 'alarmprev' to Byte 30 of EEPROM
}

/*******************************************************************************************************************************************************************/

// Create the function for beep
void beep() {                              // Create function
  if (beeponoff == 1) {                   // If 'beeponoff' is equal to 1 then....
    digitalWrite(Buzpin, LOW);           // Set the Buzzer to off
    digitalWrite(Buzpin, HIGH);         // Set the Buzzer to on
    delay(10);                         // wait 0.01 seconds
    digitalWrite(Buzpin, LOW);        // Set the Buzzer to off
    buzonoff = 0;
  }
}


/*******************************************************************************************************************************************************************/

// Create the function for unknowncoderecv
void unknowncoderecv() {                        // Create function
  if (myDecoder.value == UNKNOWN) {            // If the code recieved is unknown (not listed in the 'ircheck' function) then....
    myReceiver.enableIRIn();                  // reset the sensor
  }
  if (myDecoder.value == 0x11) {             // If the recieved code is in the 0x11 format (not what we want) then....
    myReceiver.enableIRIn();                // reset the sensor
  }
}




/*******************************************************************************************************************************************************************/

// Create the function for alarmcheck
void alarmcheck() {                                                  // Create function
  alarmpin = digitalRead(AlarmInput);                               // Set 'alarmpin' to the state of 'AlarmInput' ( HIGH or LOW )
  if (alarmpin == HIGH && alarmlight == 1 && toggle2 == 1) {       // If 'alarmpin' is HIGH and 'alarmlight' is 1 (light to indicate if the alarm is on, is on) and 'toggle2' is 1 (alarm is on) then....
    countdown = 1 ;                                               // Set 'countdown' to 1
  }
  EEPROM.write(33, countdown);                                  // Write the value of 'countdown' to Byte 33 of EEPROM
}


/*******************************************************************************************************************************************************************/





// Create the function for colourwipe
void colourWipe(uint32_t c) {                                 // Create function
  for (uint16_t i = 0; i < strip.numPixels(); i++) {         // As long as "i" equals 0 and "i" is less than the number of pixels in the ring, add one to "i" then..
    strip.setPixelColor(i, c);                              // Set the colour of the first pixel to the values given in the function IE:colourWipe(strip.Color(This value, This value, And this one))
    strip.show();                                          // Show this pixel
    delay(rotationspeed);                                 // Wait for the value of 'rotationspeed' (declared above)
  }
}



/*******************************************************************************************************************************************************************/



// Create the function for colourwipeSlow
void colourWipeSlow(uint32_t c) {                             // Create the function
  for (uint16_t i = 0; i < strip.numPixels(); i++) {         // As long as "i" equals 0 and "i" is less than the number of pixels in the ring, add one to "i" then..
    strip.setPixelColor(i, c);                              // Set the colour of the first pixel to the values given in the function IE:colourWipe(strip.Color(This value, This value, And this one))
    strip.show();                                          // Show this pixel
    delay(60);                                            // Wait 60 milliseconds
  }
}


/*******************************************************************************************************************************************************************/




// Create the function for colourwipeQuick
void colourWipeQuick(uint32_t c) {                            // Create the function
  for (uint16_t i = 0; i < strip.numPixels(); i++) {         // As long as "i" equals 0 and "i" is less than the number of pixels in the ring, add one to "i" then..
    strip.setPixelColor(i, c);                              // Set the colour of the first pixel to the values given in the function IE:colourWipe(strip.Color(This value, This value, And this one))
    strip.show();                                          // Show this pixel
    delay(20);                                            // Wait 20 milliseconds
  }
}




/*******************************************************************************************************************************************************************/



// Create the function for ColourReturn
void colourReturn() {                                                   // Create the function

  if (toggle == 1 && count_value == 1 && alarmlight == 0) {           // If toggle is 1, count_value is 1 and alarmlight is 0 (return to colour, colour 1 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(255, 255, 255));                         // Set the ring to white
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 2 && alarmlight == 0) {           // If toggle is 1, count_value is 2 and alarmlight is 0 (return to colour, colour 2 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(0, 0, 255));                             // Set the ring to Blue
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 3 && alarmlight == 0) {           // If toggle is 1, count_value is 3 and alarmlight is 0 (return to colour, colour 3 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(0, 255, 0));                             // Set the ring to Green
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 4 && alarmlight == 0) {           // If toggle is 1, count_value is 4 and alarmlight is 0 (return to colour, colour 4 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(255, 0, 0)) ;                            // Set the ring to Red
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 5 && alarmlight == 0) {           // If toggle is 1, count_value is 5 and alarmlight is 0 (return to colour, colour 5 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(255, 0, 255)) ;                          // Set the ring to Purple
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 6 && alarmlight == 0) {           // If toggle is 1, count_value is 6 and alarmlight is 0 (return to colour, colour 6 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(0, 255, 255)) ;                          // Set the ring to Turquoise
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 7 && alarmlight == 0) {           // If toggle is 1, count_value is 7 and alarmlight is 0 (return to colour, colour 7 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(255, 255, 0)) ;                          // Set the ring to Yellow
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 8 && alarmlight == 0) {           // If toggle is 1, count_value is 8 and alarmlight is 0 (return to colour, colour 8 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(255, 128, 0)) ;                          // Set the ring to Orange
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 9 && alarmlight == 0) {           // If toggle is 1, count_value is 9 and alarmlight is 0 (return to colour, colour 9 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(143, 0, 255)) ;                          // Set the ring to Violet
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }

  if (toggle == 1 && count_value == 10 && alarmlight == 0) {          // If toggle is 0, count_value is 10 and alarmlight is 0 (return to colour, colour 10 and alarm is off) then....
    strip.setBrightness(brightness);                                 // Set the ring to its brightness
    colourWipe(strip.Color(255, 127, 127)) ;                        // Set the ring to Pink
    toggle = 0;                                                    // Set toggle to 0 (don't return to previous colour)
    onoff = 0;                                                    // Set onoff to 0 (lamp is on)
    EEPROM.write(10, toggle);                                    // Write the value of 'toggle' to Byte 10 of EEPROM
    EEPROM.write(2, onoff);                                     // Write the value of 'onoff' to Byte 2 of EEPROM
  }
}








// Create the function for alarmon
void alarmon () {                                                 // Create function
  alarmpin = digitalRead(AlarmInput);                            // Set 'alarmpin' to the value of the pin 'AlarmInput'
  if (alarmpin == LOW) {                                        // If 'alarmpin' is LOW then....
    alarmprev = 0 ;                                            // Set 'alarmprev' to 0
    pulse2 = 1;                                               // Set 'Pulse 2' to 1 (to prevent loops or repeats)
    toggle2 = 1;                                             // Set 'toggle2' to 1 (this tells the Arduino that the alarm is on)
    strip.setBrightness(40);                                // Set the Neopixel ring to 40% brightness
    colourWipe(strip.Color(255, 255, 0)) ;                 // Set the neopixel ring to yellow
    alarmlight = 1;                                       // Set 'alarmlight' to 1 (Set the Alarm indication light to on)
  }
  EEPROM.write(30, alarmprev);                          // Write the value of 'alarmprev' to Byte 30 of EEPROM
  EEPROM.write(31, toggle2);                           // Write the value of 'toggle2' to Byte 31 of EEPROM

}







// Create the function for alarmoff
void alarmoff () {                                        // Create function
  alarmprev = 0 ;                                        // Set 'alarmprev' to 0
  pulse2 = 1;                                           // Set 'Pulse 2' to 1 (to prevent loops or repeats)
  toggle2 = 0;                                         // Set 'toggle2' to 0 (this tells the Arduino that the alarm is off)
  strip.setBrightness(40);                            // Set the Neopixel ring to 40% brightness
  alarmlight = 0;                                    // Set 'alarmlight' to 0 (Set the Alarm indication light to off)
  countdown = 0;                                    // Set 'countdown' to 0
  alarmcount = 0;                                  // Set 'alarmcount' to 0

  if (onoff == 0) {                                 // If the alarm was previously on (onoff = 0) then...
    toggle = 1;                                    // Set toggle to 1 (returning to previous colour)
    colourReturn();                               // Call for the function 'colourReturn' to return the ring to it's prior colour
  }
  else {                                         // If the alarm was not previously on then...
    onoff = 1;                                  // Set onoff to 1 (set the lamp as off)
    colourWipe(strip.Color(0, 0, 0)) ;         // Set the Neopixel ring to off
    strip.show();                             // Restart the Neopixels
  }
  EEPROM.write(30, alarmprev);                 // Write the value of 'alarmprev' to Byte 30 of EEPROM
  EEPROM.write(31, toggle2);                  // Write the value of 'toggle2' to Byte 31 of EEPROM
  EEPROM.write(33, countdown);               // Write the value of 'countdown' to Byte 33 of EEPROM
  EEPROM.write(34, alarmcount);             // Write the value of 'alarmcount' to Byte 34 of EEPROM
  EEPROM.write(10, toggle);                // Write the value of 'toggle' to Byte 10 of EEPROM
  EEPROM.write(2, onoff);                 // Write the value of 'onoff' to Byte 2 of EEPROM
}






// Create the function for alarmflash
void alarmflash () {                                        // Create function
  digitalWrite(Buzpin, LOW) ;                              // Set the Buzzer to off
  digitalWrite(Buzpin, HIGH) ;                            // Set the Buzzer to on
  ircheck ();                                            // Call for the function "ircheck" to be completed
  colourWipeQuick(strip.Color(0, 0, 0)) ;               // Set the Neopixel ring to off
  digitalWrite(Buzpin, LOW) ;                          // Set the Buzzer to off
  ircheck ();                                         // Call for the function "ircheck" to be completed
  digitalWrite(Buzpin, HIGH) ;                       // Set the Buzzer to on
  colourWipe(strip.Color(255, 0, 0)) ;              // Set the Neopixel ring to red
  digitalWrite(Buzpin, LOW) ;                      // Set the Buzzer to off
  ircheck ();                                     // Call for the function "ircheck" to be completed
  digitalWrite(Buzpin, HIGH) ;                   // Set the Buzzer to on
  colourWipeQuick(strip.Color(0, 0, 0)) ;       // Set the Neopixel ring to off
  digitalWrite(Buzpin, LOW) ;                  // Set the Buzzer to off
  ircheck ();                                 // Call for the function "ircheck" to be completed
  buzonoff = 0;
}






void ircheck () {
  if (myReceiver.getResults()) {                   // If the reciever has recieved some data then....
    myDecoder.decode();                           // Decode the recieved data
    if (myDecoder.value == UNKNOWN) {            // If the code recieved is unknown (not listed in the 'ircheck' function) then....
      myReceiver.enableIRIn();                  // reset the sensor
    }

    if (myDecoder.value == 0x11) {             // If the recieved code is in the 0x11 format (not what we want) then....
      myReceiver.enableIRIn();                // reset the sensor
    }

    if (myDecoder.protocolNum == NEC) {      // If the code recieved is in the NEC protocol then....
      switch (myDecoder.value) {            // If the recieved equals....


        case 0xFF9867:  // key 0
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 255, 255));        // Set the colour to white
            count_value = 1;                              // Set count_value to 1
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFFA25D:  // key 1
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(0, 0, 255));            // Set the colour to blue
            count_value = 2;                              // Set count_value to 2
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFF629D:  // key 2
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(0, 255, 0));            // Set the colour to green
            count_value = 3;                              // Set count_value to 3
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFFE21D:  // key 3
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 0, 0));            // Set the colour to red
            count_value = 4;                              // Set count_value to 4
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFF22DD:  // key 4
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 0, 255));          // Set the colour to purple
            count_value = 5;                              // Set count_value to 5
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFF02FD:  // key 5
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(0, 255, 255));          // Set the colour to Turquoise
            count_value = 6;                              // Set count_value to 6
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFFC23D:  // key 6
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 255, 0)) ;         // Set the colour to yellow
            count_value = 7;                              // Set count_value to 7
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP


        case 0xFFE01F:  // key 7
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 128, 0));          // Set the colour to orange
            count_value = 8;                              // Set count_value to 8
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP

        case 0xFFA857:  // key 8
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(143, 0, 255));          // Set the colour to violet
            count_value = 9;                              // Set count_value to 9
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP

        case 0xFF906F:  // key 9
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 127, 127));        // Set the colour to pink
            count_value = 10;                             // Set count_value to 10
            onoff = 0;                                   // Set onoff to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
            ResNum = 0;                               // Set 'ResNum' to 0
            EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          }
          break;                                     // STOP




        case 0xFF18E7:  // up
          beep();                                             // Call for the function 'beep' to be completed
          if (onoff == 0 && alarmlight == 0) {               // If 'onoff' is equal to 0 and 'alarmlight' is equal to 0 then....
            brightness = brightness + brincrement;          // Set 'brightness' to 'brightness' + 'brincrement'
            if (brightness < 5) {                          // If 'brightness is less than 5 then....
              brightness = 5;                             // Set 'brightness' to 5
            }
            if (brightness > maxbrightness) {           // If 'brightness' is greater than 'maxbrightness' then....
              brightness = maxbrightness;              // Set 'brightness' to 'maxbrightness'
            }
            strip.setBrightness(brightness);         // Set the brightness of the LED ring to 'brightness'
          }
          EEPROM.write(1, brightness);              // Write the value of 'brightness' to Byte 1 of EEPROM
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                   // STOP


        case 0xFF4AB5:  // down
          beep();                                             // Call for the function 'beep' to be completed
          if (onoff == 0 && alarmlight == 0) {               // If 'onoff' is equal to 0 and 'alarmlight' is equal to 0 then....
            brightness = brightness - brincrement;          // Set 'brightness' to 'brightness' - 'brincrement'
            if (brightness < 5) {                          // If 'brightness is less than 5 then....
              brightness = 5;                             // Set 'brightness' to 5
            }
            if (brightness > maxbrightness) {           // If 'brightness' is greater than 'maxbrightness' then....
              brightness = maxbrightness;              // Set 'brightness' to 'maxbrightness'
            }
            strip.setBrightness(brightness);         // Set the brightness of the LED ring to 'brightness'
          }
          EEPROM.write(1, brightness);              // Write the value of 'brightness' to Byte 1 of EEPROM
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                   // STOP




        case 0xFFB04F:  // #
          beep();                                             // Call for the function 'beep' to be completed
          if (beeponoff == 1 && pulse == 0) {                // If 'beeponoff' is equal to 1 and 'pulse' is equal to 0 then....
            beeponoff = 0;                                  // Set 'beeponoff' to 0
            pulse = 1;                                     // Set 'pulse' to 1
          }
          if (beeponoff == 0 && pulse == 0) {            // If 'beeponoff' is equal to 0 and 'pulse' is equal to 0 then....
            beeponoff = 1;                              // Set 'beeponoff' to 1
            pulse = 1;                                 // Set 'pulse' to 1
          }
          pulse = 0;                                 // Set 'pulse' to 1
          EEPROM.write(32, beeponoff);              // Write the value of 'beeponoff' to Byte 32 of EEPROM
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                   // STOP



        case 0xFF38C7:  // ok
          beep();                                             //Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is 0, then....
            strip.setBrightness(brightness);                // Set the brightness of the LED ring to 'brightness'
            colourWipe(strip.Color(0, 0, 0));              // Set the neopixel ring to off (no colour)
            if (onoff == 1) {                             // If 'onoff' is 1, then....
              strip.setBrightness(brightness);           // Set the brightness of the LED ring to 'brightness'
              onoff = 0;                                // Set 'onoff' to 0
              EEPROM.write(2, onoff);                  // Write the value of 'onoff' to Byte 2 of EEPROM
              if (count_value == 0) {                                 // If 'count_value' is 0, then....
                count_value = 1;                                     // Set 'count_value' to 1
              }
              if (count_value == 1) {                                 // If 'count_value' is 1, then....
                colourWipe(strip.Color(255, 255, 255));              // Set the colour to white
              }
              if (count_value == 2) {                                 // If 'count_value' is 2, then....
                colourWipe(strip.Color(0, 0, 255));                  // Set the colour to blue
              }
              if (count_value == 3) {                                 // If 'count_value' is 3, then....
                colourWipe(strip.Color(0, 255, 0));                  // Set the colour to green
              }
              if (count_value == 4) {                                 // If 'count_value' is 4, then....
                colourWipe(strip.Color(255, 0, 0)) ;                 // Set the colour to red
              }
              if (count_value == 5) {                                 // If 'count_value' is 5, then....
                colourWipe(strip.Color(255, 0, 255)) ;               // Set the colour to purple
              }
              if (count_value == 6) {                                 // If 'count_value' is 6, then....
                colourWipe(strip.Color(0, 255, 255)) ;               // Set the colour to turquoise
              }
              if (count_value == 7) {                                 // If 'count_value' is 7, then....
                colourWipe(strip.Color(255, 255, 0)) ;               // Set the colour to yellow
              }
              if (count_value == 8) {                                 // If 'count_value' is 8, then....
                colourWipe(strip.Color(255, 128, 0)) ;               // Set the colour to orange
              }
              if (count_value == 9) {                                 // If 'count_value' is 9, then....
                colourWipe(strip.Color(143, 0, 255)) ;               // Set the colour to violet
              }
              if (count_value == 10) {                                // If 'count_value' is 10, then....
                colourWipe(strip.Color(255, 127, 127)) ;             // Set the colour to pink
              }
              pulse = 1;                                             // Set 'pulse' to 1
            }
            if (pulse == 0 ) {                                       // If 'pulse' is 0, then....
              onoff = 1;                                            // Set 'onoff' to 1
              EEPROM.write(2, onoff);                              // Write the value of 'onoff' to Byte 2 of EEPROM
            }
            pulse = 0;                                           // Set 'pulse' to 0
          }
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP





        case 0xFF5AA5:  // right
          beep();                                             //Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is 0, then....
            if (onoff == 0) {                               // If 'onoff' is 0, then....
              toggle = 1;                                  // Set 'toggle' to 1
              count_value ++;                             // Add 1 to 'count_value'
              colourReturn();                            // Call for the function 'colourReturn' to be completed
            }
            else {                                     // Otherwise....
              toggle = 1;                             // Set 'toggle' to 1
              colourReturn();                        // Call for the function 'colourReturn' to be completed
            }
          }
          if (toggle == 1) {                       // If 'toggle' is 1, then....
            if (count_value == 0) {               // If 'count_value' is 0, then....
              count_value = 11;                  // Set 'count_value' to 11
              colourReturn();                   // Call for the function 'colourReturn' to be completed
            }

            if (count_value == 11) {         // If 'count_value' is 11, then....
              count_value = 1;              // Set 'count_value' to 1
              colourReturn();              // Call for the function 'colourReturn' to be completed
            }
          }
          EEPROM.write(0, count_value);                // Write the value of 'count_value' to Byte 0 of EEPROM
          EEPROM.write(10, toggle);                   // Write the value of 'toggle' to Byte 10 of EEPROM
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP




        case 0xFF10EF:  // left
          beep();                                                 //Call for the function 'beep' to be completed
          if (alarmlight == 0) {                                 // If 'alarmlight' is 0, then....
            if (onoff == 0) {                                   // If 'onoff' is 0, then....
              toggle = 1;                                      // Set 'toggle' to 1
              if (count_value >= 1) {                         // If 'count_value' is greater than or equal to 1, then....
                count_value --;                              // Subtract one from 'count_value'
              }
              if (count_value == 0) {                      // If 'count_value' is 0, then....
                count_value = 10;                         // Set 'count_value' to 10
              }
              colourReturn();                           // Call for the function 'colourReturn' to be completed
            }
            else {                                    // Otherwise....
              colourReturn();                        // Call for the function 'colourReturn' to be completed
              toggle = 1;
            }
          }
          if (toggle == 1) {                      // If 'toggle' is 1, then....
            if (count_value == 0) {              // If 'count_value' is 0, then....
              count_value = 11;                 // Set 'count_value' to 11
              colourReturn();                  // Call for the function 'colourReturn' to be completed
            }

            if (count_value == 11) {          // If 'count_value' is 11, then....
              count_value = 1;               // Set 'count_value' to 1
              colourReturn();               // Call for the function 'colourReturn' to be completed
            }
          }
          EEPROM.write(0, count_value);                // Write the value of 'count_value' to Byte 0 of EEPROM
          EEPROM.write(10, toggle);                   // Write the value of 'toggle' to Byte 10 of EEPROM
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP


        case 0xFF6897:  // *
          beep();                                             // Call for the function 'beep' to be completed
          if (toggle2 == 0 && pulse2 == 0) {                 // If 'toggle2' is 0, and 'pulse2' is 0, then....
            alarmon ();                                     // Call for the function 'alarmon' to be completed
          }
          if (toggle2 == 1 && pulse2 == 0) {              // If 'toggle2' is 1, and 'pulse2' is 0, then....
            alarmoff ();                                 // Call for the function 'alarmoff' to be completed
          }
          pulse2 = 0;                                 // Set 'pulse2' to 0
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP
      }


      strip.show();                              // Display the latest colour to the RGB ring
      myReceiver.enableIRIn();                  // Restart the receiver


    }









    // THIS IS EXTRA CODE TO USE THE COLOUR BUTTONS ON A SKY Q REMOTE, LEAVE COMMENTED IF NOT NEEDED

    if (myDecoder.protocolNum == RC6) {      // If the code recieved is in the RC6 protocol then....
      switch (myDecoder.value) {            // If the recieved equals....


        case 0xC0081A6D:  // RED
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is 0, then....
            strip.setBrightness(brightness);                // Set the brightness of the LED ring to 'brightness'
            colourWipe(strip.Color(0, 0, 0));              // Set the neopixel ring to off (no colour)
            if (onoff == 1) {                             // If 'onoff' is 1, then....
              strip.setBrightness(brightness);           // Set the brightness of the LED ring to 'brightness'
              onoff = 0;                                // Set 'onoff' to 0
              EEPROM.write(2, onoff);                  // Write the value of 'onoff' to Byte 2 of EEPROM
              if (count_value == 0) {                                 // If 'count_value' is 0, then....
                count_value = 1;                                     // Set 'count_value' to 1
              }
              if (count_value == 1) {                                 // If 'count_value' is 1, then....
                colourWipe(strip.Color(255, 255, 255));              // Set the colour to white
              }
              if (count_value == 2) {                                 // If 'count_value' is 2, then....
                colourWipe(strip.Color(0, 0, 255));                  // Set the colour to blue
              }
              if (count_value == 3) {                                 // If 'count_value' is 3, then....
                colourWipe(strip.Color(0, 255, 0));                  // Set the colour to green
              }
              if (count_value == 4) {                                 // If 'count_value' is 4, then....
                colourWipe(strip.Color(255, 0, 0)) ;                 // Set the colour to red
              }
              if (count_value == 5) {                                 // If 'count_value' is 5, then....
                colourWipe(strip.Color(255, 0, 255)) ;               // Set the colour to purple
              }
              if (count_value == 6) {                                 // If 'count_value' is 6, then....
                colourWipe(strip.Color(0, 255, 255)) ;               // Set the colour to turquoise
              }
              if (count_value == 7) {                                 // If 'count_value' is 7, then....
                colourWipe(strip.Color(255, 255, 0)) ;               // Set the colour to yellow
              }
              if (count_value == 8) {                                 // If 'count_value' is 8, then....
                colourWipe(strip.Color(255, 128, 0)) ;               // Set the colour to orange
              }
              if (count_value == 9) {                                 // If 'count_value' is 9, then....
                colourWipe(strip.Color(143, 0, 255)) ;               // Set the colour to violet
              }
              if (count_value == 10) {                                // If 'count_value' is 10, then....
                colourWipe(strip.Color(255, 127, 127)) ;             // Set the colour to pink
              }
              pulse = 1;                                             // Set 'pulse' to 1
            }
            if (pulse == 0 ) {                                       // If 'pulse' is 0, then....
              onoff = 1;                                            // Set 'onoff' to 1
              EEPROM.write(2, onoff);                              // Write the value of 'onoff' to Byte 2 of EEPROM
            }
            pulse = 0;                                           // Set 'pulse' to 0
          }
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP

        case 0xC0081A6E:  // GREEN
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 255, 255));        // Set the colour to white
            count_value = 1;                              // Set 'count_value' to 1
            onoff = 0;                                   // Set 'onoff' to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
          }
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP



        case 0xC0081A6F:  // YELLOW
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(255, 128, 0));          // Set the colour to orange
            count_value = 8;                              // Set 'count_value' to 8
            onoff = 0;                                   // Set 'onoff' to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
          }
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP


        case 0xC0081A70:  // BLUE
          beep();                                             // Call for the function 'beep' to be completed
          if (alarmlight == 0) {                             // If 'alarmlight' is equal to 0 then....
            strip.setBrightness(brightness);                // Set the ring's brightness to 'brightness'
            colourWipe(strip.Color(0, 255, 255));          // Set the colour to Turquoise
            count_value = 6;                              // Set 'count_value' to 6
            onoff = 0;                                   // Set 'onoff' to 0
            EEPROM.write(0, count_value);               // Write the value of 'count_value' to Byte 0 of EEPROM
            EEPROM.write(2, onoff);                    // Write the value of 'onoff' to Byte 2 of EEPROM
          }
          ResNum = 0;                               // Set 'ResNum' to 0
          EEPROM.write(35, ResNum);                // Write the value of 'ResNum' to Byte 35 of EEPROM
          break;                                     // STOP

      }
    }


  }




  if (myDecoder.value == UNKNOWN) {            // If the code recieved is unknown (not listed in the 'ircheck' function) then....
    myReceiver.enableIRIn();                  // reset the sensor
  }


  if (myDecoder.value == 0x11) {             // If the recieved code is in the 0x11 format (not what we want) then....
    myReceiver.enableIRIn();                // reset the sensor
  }
}



/***********************************************************************************************END OF CODE***********************************************************************************************/
