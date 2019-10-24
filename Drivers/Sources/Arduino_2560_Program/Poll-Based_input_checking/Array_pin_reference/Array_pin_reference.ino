#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_MCP23017.h"
#include <Bounce2.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Decleration for MCP port expanders on I2C (SDA, SCL)
Adafruit_MCP23017 mcp1;



//Pin Definitions
//Inputs
const byte num_buttons = 12;
//const uint8_t buttonInput[num_buttons] = {2, 3, 4, 5, 6, 7, 8, 10, 11, 12, A0, A1}; //Reference array for inputs

//This array and below consts control all pin assignments. Change array numbers to affect pin assignment
const uint8_t buttonInput[num_buttons] = {2, 3, 4, 5, 6, 7, 8, 12, 10, 11, A0, A1}; /////////////////////////////   TEST ARRAY ADDRESSING MCP
bool buttonState[num_buttons]; //Button state storage
//Reference definitions to array - Use these in combination with buttonInput to reference pins
const byte clockUp = 0, clockDown = 1;
const byte pedNorth = 2, pedEast = 3, carNorth = 4, carEast = 5;  //Input control from intersection
const byte northLight1 = 6, northLight2 = 7, eastLight1 = 8, eastLight2 = 9; //Input control from CPLD. Note that these are assigned to MCP pins!
const byte stepCW = 10, stepCCW = 11;




//Outputs
const byte num_micro_outputs = 2;
const byte microOutputs[num_micro_outputs] = {9, A3};
//Use these to reference to output array - microOutputs[...]
const byte CLOCKOUT = 0;
const byte debug = 1;


//MCP pin definitions
const byte num_mcp1_outputs = 8;
const byte mcp1Outputs[num_mcp1_outputs] = {0, 1, 2, 3, 4, 5, 6, 7};
//Reference definition to array
const byte northRed = 0, northAmber = 1, northGreen = 2;
const byte eastRed = 3, eastAmber = 4, eastGreen = 5;
const byte northWalk = 6, eastWalk = 7;


//debounce setup
Bounce * buttons = new Bounce[num_buttons];

void setup() {
  //Set up debug monitoring
  Serial.begin(9600);
  
  //initialise I/O
  for(int i = 0; i < num_buttons; i++){
    pinMode(buttonInput[i], INPUT_PULLUP);
  }

  for(int i = 0; i < num_micro_outputs; i++){
    pinMode(microOutputs[i], OUTPUT);
  }
  mcp1.begin(0);  //Initialises address 0 expander
  for(int k = 0; k <= 7; k++){
    mcp1.pinMode(k, OUTPUT);
  }
  for(int k = 8; k <= 15; k++){
    mcp1.pinMode(k, INPUT);
    mcp1.pullUp(k, 1);
  }
  pinMode(9, OUTPUT);

  
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  //Debouncer tool setup
  for(int i = 0; i < num_buttons; i++){
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

  for(int k = 0; k <= 7; k++){
    mcp1.digitalWrite(k, HIGH);
  }
  mcp1.pinMode(0, OUTPUT);
  mcp1.digitalWrite(0, HIGH);
}



// the loop function runs over and over again forever
void loop() {
  

  for(int i = 0; i < num_buttons; i++){
    buttons[i].update();
    if(buttons[i].fell()) {
      buttonState[i] = true;
    }
  }

  buttonState[northLight1] = !mcp1.digitalRead(buttonInput[northLight1]);
  buttonState[northLight2] = !mcp1.digitalRead(buttonInput[northLight2]);
  buttonState[eastLight1] = !mcp1.digitalRead(buttonInput[eastLight1]);
  buttonState[eastLight2] = !mcp1.digitalRead(buttonInput[eastLight2]);

  if(buttonState[clockUp] == true){
    clockControl(1);
    buttonState[clockUp] = false;
  }

  if(buttonState[clockDown] == true){
    clockControl(0);
    buttonState[clockDown] = false;
  }


  trafficControl(buttonState[northLight1], buttonState[northLight2], buttonState[eastLight1], buttonState[eastLight2]);




  


}




//--------------------------------------------------------------------------------------------------------------------
//Traffic Control Logic for CPLD inputs
void trafficControl(bool north1, bool north2, bool east1, bool east2){
  //set all off so we start from a blank slate
  mcp1.digitalWrite(northGreen, LOW);
  mcp1.digitalWrite(northAmber, LOW);
  mcp1.digitalWrite(northRed, LOW);
  mcp1.digitalWrite(northWalk, LOW);
  //decide which lights to turn on according to specification in lab guide
  if(north1 == true){
    mcp1.digitalWrite(northGreen, HIGH);
    if(north2 == true){
      mcp1.digitalWrite(northWalk, HIGH);
    }
  }else{
    if(north2 == true){
      mcp1.digitalWrite(northAmber, HIGH);
    }else{
      mcp1.digitalWrite(northRed, HIGH);
    }    
  }  

  mcp1.digitalWrite(eastGreen, LOW);
  mcp1.digitalWrite(eastAmber, LOW);
  mcp1.digitalWrite(eastRed, LOW);
  mcp1.digitalWrite(eastWalk, LOW);
  if(east1 == true){
    mcp1.digitalWrite(eastGreen, HIGH);
    if(east2 == true){
      mcp1.digitalWrite(eastWalk, HIGH);
    }
  }else{
    if(east2 == true){
      mcp1.digitalWrite(eastAmber, HIGH);
    }else{
      mcp1.digitalWrite(eastRed, HIGH);
    }    
  }
}




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
