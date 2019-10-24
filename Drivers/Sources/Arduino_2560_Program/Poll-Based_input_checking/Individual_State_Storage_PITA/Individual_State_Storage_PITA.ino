#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_MCP23017.h"
#include <Bounce2.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Decleration for MCP port expanders on I2C (SDA, SCL)
Adafruit_MCP23017 mcp1;



//Pin Definitions
//Clock
const byte CLOCKOUT = 9;

//Inputs
const byte clockUp = 2, clockDown = 3;
const byte pedNorth = 4, pedEast = 5, carNorth = 6, carEast = 7;  //Input control from intersection
const byte northLight1 = 8, northLight2 = 10, eastLight1 = 11, eastLight2 = 12; //Input control from CPLD
const byte stepCW = A0, stepCCW = A1;

//Input state storage
bool isClockUpOn = false, isClockDownOn = false;
bool isPedNorthOn = false, isPedEastOn = false, isCarNorthOn = false, isCarEastOn = false;
bool isNorthLight1On = false, isNorthLight2On = false; isEastLight1On = false, isEastLight2On = false;
bool isStepCWOn = false, isStepCCWOn = false;

//Outputs
const byte debug = A3;

//MCP pin definitions
const byte northRed = 0, northYellow = 1, northGreen = 2;
const byte eastRed = 3, eastYellow = 4, eastGreen = 5;


//Variables
int stateClockUp = 0, lastStateClockUp = 0;
int stateClockDown = 0, lastStateClockDown = 0;

//debouncer definitions
const byte numButtons = 12;
const uint8_t buttonInput[numButtons] = {2, 3, 4, 5, 6, 7, 8, 10, 11, 12, A0, A1};

bool buttonState[numButtons];
Bounce * buttons = new Bounce[numButtons];

void setup() {
  //Set up debug monitoring
  Serial.begin(9600);
  
  //pinMode declerations
  pinMode(clockUp, INPUT_PULLUP);
  pinMode(clockDown, INPUT_PULLUP);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  //Set up timer on CLOCKOUT (OC1A) 
  pinMode (CLOCKOUT, OUTPUT);

  //Generic GPIO Output declerations
  pinMode (debug, OUTPUT);

  //Generic GPIO Input declerations

  //Debouncer tool setup
  for(int i = 0; i < numButtons; i++){
    buttons[i].attach(buttonInput[i], INPUT_PULLUP); //Setup debounce instance for declared pins
    buttons[i].interval(10);
  }
   
  //Set up Timer 1  
  TCCR1A = bit (COM1A0); // toggle OC1A on compare match
  //Prescaler registers are
  TCCR1B = (1 << CS12)| (1 << WGM12); //CTC (clear on compare is true), Prescaler initialised to x256
  //number of clocks to count for each toggle 
  //0 results in toggling every clock, 1 every 2 clocks etc
  OCR1A = 0; //Output every clock 


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }  
  setupOLED();
  resetOLED();
  clockControl(1);
  clockControl(0);

  //Port Expander Setup
  mcp1.begin(0);  //Initialises address 0 expander
  for(int k = 0; k <= 7; k++){
    mcp1.pinMode(k, OUTPUT);
  }
  for(int k = 0; k <= 7; k++){
    mcp1.digitalWrite(k, HIGH);
  }
  mcp1.pinMode(0, OUTPUT);
  mcp1.digitalWrite(0, HIGH);
}


int i = 0;
// the loop function runs over and over again forever
void loop() {

  for(int i = 0; i < numButtons; i++){
    buttons[i].update();
    if(buttons[i].fell()) {
      buttonState[i] = true;
    }
  }

if(buttonState[0] == true){
  clockControl(1);
  buttonState[0] = false;
}

if(buttonState[1] == true){
  clockControl(0);
  buttonState[1] = false;
}

if(buttonState[2] == true){
  
}

if(buttonState[3] == true){
  
}

if(buttonState[4] == true){
  
}

if(buttonState[5] == true){
  
}

if(buttonState[6] == true){
  
}

if(buttonState[7] == true){
  
}





  /*
  stateClockUp = digitalRead(clockUp);
  stateClockDown = digitalRead(clockDown);

  digitalWrite(debug, stateClockDown);

  


  if(stateClockUp != lastStateClockUp){
    if(stateClockUp == 0){
      clockControl(1);
    }
  }
  
  if(stateClockDown != lastStateClockDown){
    if(stateClockDown == 0){
      clockControl(0);
    }
  }

  lastStateClockUp = stateClockUp;
  lastStateClockDown = stateClockDown;
  */
}




//--------------------------------------------------------------------------------------------------------------------

//Controls the clock generator
//clockButton determines increment or decrement clock
//1 is increment, anything else is decrement
void clockControl(int clockButton){
  static int desiredClock = 0;
  if(clockButton == 1){
    desiredClock++;
    if(desiredClock > 17){
      desiredClock = 0;
    }
  }else{
    desiredClock--;
    if(desiredClock < 0){
      desiredClock = 17;
    }
  }

  Serial.write("test");

  switch(desiredClock){
    //1Hz
    case 0:
      TCCR1B = (1 << CS12)| (1 << WGM12);
      OCR1A  = 31249;
      resetOLED(); //Set the cursor on the first column and first row.
      display.println("1Hz");
    break;
    //2Hz
    case 1:
      OCR1A  = 15624;
      resetOLED();
      display.println("2Hz");
    break;
    //5Hz
    case 2:
      OCR1A  = 6249;
      resetOLED();
      display.println("5Hz");
    break;
    //10Hz
    case 3:
      OCR1A  = 3124;
      resetOLED();
      display.println("10Hz");
    break;
    //20Hz
    case 4:
      OCR1A  = 1561;
      resetOLED();
      display.println("20Hz");
    break;
    //50Hz
    case 5:
      OCR1A  = 624;
      resetOLED();
      display.println("50Hz");
    break;
    //100Hz
    case 6:
      TCCR1B = 0; //Clear prescale register, since we can move from here to a different scale
      TCCR1B = (1 << CS12)| (1 << WGM12);
      OCR1A  = 311;
      resetOLED();
      display.println("100Hz");
    break;
    //200Hz, change prescaler to 1x (CS10 set)
    case 7:
      TCCR1B = 0; //Clear prescale register, since we can move here from a different scale
      TCCR1B = (1 << CS10)| (1 << WGM12);
      OCR1A  = 39999;
      resetOLED();
      display.println("200Hz");
    break;
    //500Hz
    case 8:
      OCR1A  = 15999;
      resetOLED();
      display.println("500Hz");
    break;
    //1kHz
    case 9:
      OCR1A  = 7999;
      resetOLED();
      display.println("1kHz");
    break;
    //2kHz
    case 10:
      OCR1A  = 3999;
      resetOLED();
      display.println("2kHz");
    break;
    //5kHz
    case 11:
      OCR1A  = 1599;
      resetOLED();
      display.println("5kHz");
    break;
    //10kHz
    case 12:
      OCR1A  = 799;
      resetOLED();
      display.println("10kHz");
    break;
    //20kHz
    case 13:
      OCR1A  = 399;
      resetOLED();
      display.println("20kHz");
    break;
    //50kHz
    case 14:
      OCR1A  = 159;
      resetOLED();
      display.println("50kHz");
    break;
    //100kHz
    case 15:
      OCR1A  = 79;
      resetOLED();
      display.println("100kHz");
    break;
    //200kHz
    case 16:
      OCR1A  = 39;
      resetOLED();
      display.println("200kHz");
    break;
    //500kHz
    case 17:
      TCCR1B = 0; //Clear prescale register, since we can move here from a different scale
      TCCR1B = (1 << CS10)| (1 << WGM12);
      OCR1A  = 15;
      resetOLED();
      display.println("500kHz");
    break;    
  }
  display.display();
}


void setupOLED(){
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.cp437(true);  
}

void resetOLED(){
  display.clearDisplay();
  display.setCursor(0,0);

}
