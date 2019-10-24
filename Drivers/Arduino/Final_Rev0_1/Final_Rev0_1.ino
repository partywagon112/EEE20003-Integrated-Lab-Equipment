//search for *** to find things that need attention


#include <SPI.h>
#include <Wire.h>
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
Adafruit_MCP23017 mcp2;

//Pin initialisation - This is a doozy :/
//Defines a struct for pins, so we don't need two arrays for pin number and pin state
typedef struct {
    uint8_t pinNum;
    bool pinState;
} pinControl_t;


//NOTE PIN 9 is the clock output
//stepper and clock on the main MCU
pinControl_t micro_inputs[] = {
  {2, LOW}, //clock up
  {3, LOW}, //clockdown
  {4, LOW}, //northped
  {5, LOW}, //eastped
  {6, LOW}, //northcar
  {7, LOW},//eastcar
  {8, LOW},//stepper1
  {10, LOW},//stepper2
  {11, LOW},//stepper3
  {12, LOW}};//stepper4
const byte clockUp = 0, clockDown = 1;
const byte northPedIn = 2, eastPedIn = 3, northCarIn = 4, eastCarIn = 5;
const byte stepperIn1 = 6, stepperIn2 = 7, stepperIn3 = 8, stepperIn4 = 9;

//stepper outputs
pinControl_t micro_outputs[] = {
  {13, LOW},
  {A0, LOW},
  {A1, LOW},
  {A2, LOW}};
const byte stepperOut1 = 0, stepperOut2 = 1, stepperOut3 = 2, stepperOut4 = 3;

//traffic control on mcp1
pinControl_t mcp1_outputs[] = {
  {0, LOW},
  {1, LOW},
  {2, LOW},
  {3, LOW},
  {4, HIGH},
  {5, LOW},
  {6, LOW},
  {7, LOW},
  {8, LOW},
  {9, LOW},
  {10, LOW},
  {11, LOW},
  };
const byte northRed = 0, northAmber = 1, northGreen = 2, northPed = 3, eastRed = 4, eastAmber = 5, eastGreen = 6,  eastPed = 7;
const byte northPedOut = 8, eastPedOut = 9, northCarOut = 10, eastCarOut = 11;

pinControl_t mcp1_inputs[] = {
  {12, LOW},
  {13, LOW},
  {14, LOW},
  {15, LOW}};
const byte northLight1 = 0, northLight2 = 1, eastLight1 = 2, eastLight2 = 3;

//Buttons and outputs on mcp2 (Fun Buttons)
pinControl_t mcp2_outputs[] = {
  {0, HIGH},
  {1, HIGH},
  {2, HIGH},
  {3, HIGH},
  {4, HIGH},
  {5, HIGH},
  {6, HIGH},
  {7, HIGH}};

pinControl_t mcp2_inputs[] = {
  {8, HIGH},
  {9, LOW},
  {10, HIGH},
  {11, HIGH},
  {12, HIGH},
  {13, HIGH},
  {14, HIGH},
  {15, HIGH}};


//debounce setup
Bounce * buttons = new Bounce[sizeof(micro_inputs)/sizeof(micro_inputs[0])];

void setup() {
  Serial.begin(9600);
  
  timerSetup();
  setupOLED();
  pinInit(); //Sets up all pins and debouncer
  Wire.begin();
  Wire.setClock(400000);  
  pinMode(9, OUTPUT);
  clockControl(0);
  stepper(1,0,0,0, true, micro_outputs[stepperOut1].pinNum, micro_outputs[stepperOut2].pinNum, micro_outputs[stepperOut3].pinNum, micro_outputs[stepperOut4].pinNum);

}

void loop() {
  Wire.setClock(400000);
  pollButtons(); //Updates all pin states as required for each input (eg high on pin transition, denoising, etc)
  funButtons();  //Runs Fun buttons
  trafficControl(); //Runs all things traffic lights
  
  if(micro_inputs[clockUp].pinState == true){
    clockControl(1);
    micro_inputs[clockUp].pinState = false;
  }
  if(micro_inputs[clockDown].pinState == true){
    clockControl(0);
    micro_inputs[clockDown].pinState = false;
  }

  


  stepper(micro_inputs[stepperIn1].pinState, micro_inputs[stepperIn2].pinState, micro_inputs[stepperIn3].pinState, micro_inputs[stepperIn4].pinState, true, 
        micro_outputs[stepperOut1].pinNum, micro_outputs[stepperOut2].pinNum, micro_outputs[stepperOut3].pinNum, micro_outputs[stepperOut4].pinNum);

}


//Function Modules----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//Purely for setup readability, check this to change pin initialisation
void pinInit(){
  //intialise MCU input
  for(int i = 0; i < sizeof(micro_inputs)/sizeof(micro_inputs[0]); i++){
    pinMode(micro_inputs[i].pinNum, INPUT_PULLUP);
  }

  //initialise MCU output
  for(int i = 0; i < sizeof(micro_outputs)/sizeof(micro_outputs[0]); i++){
    pinMode(micro_outputs[i].pinNum, OUTPUT);
    digitalWrite(micro_outputs[i].pinNum, HIGH);
  }

  //initialise debounce on the microcontroller
  for(int i = 0; i < sizeof(micro_inputs)/sizeof(micro_inputs[0]); i++){
    buttons[i].attach(micro_inputs[i].pinNum, INPUT_PULLUP); //Setup debounce instance for declared pins
    buttons[i].interval(10);
  }

 
  mcp1.begin(1);   //initialise address 0 port expander
  for(int i = 0; i < sizeof(mcp1_inputs)/sizeof(mcp1_inputs[0]); i++){
    mcp1.pinMode(mcp1_inputs[i].pinNum, INPUT);
    mcp1.pullUp(mcp1_inputs[i].pinNum, 1);
  }
  

  for(int i = 0; i < sizeof(mcp1_outputs)/sizeof(mcp1_outputs[0]); i++){
    mcp1.pinMode(mcp1_outputs[i].pinNum, OUTPUT);
    mcp1.digitalWrite(mcp1_outputs[i].pinNum, mcp1_outputs[i].pinState );
  }

  mcp2.begin(0);  //initialise address 1 port expander
  for(int i = 0; i < sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0]); i++){
    mcp2.pinMode(mcp2_inputs[i].pinNum, INPUT);
    mcp2.pullUp(mcp2_inputs[i].pinNum, 1);
  }

  for(int i = 0; i < sizeof(mcp2_outputs)/sizeof(mcp2_outputs[0]); i++){
    mcp2.pinMode(mcp2_outputs[i].pinNum, OUTPUT);
    mcp2.digitalWrite(mcp2_outputs[i].pinNum, mcp2_outputs[i].pinState );
  }
  
  
}

//Updates all button states, and debounces as necessary
void pollButtons(){
   //check all inputs on micocontroller and store their state
  for(int i = 0; i < sizeof(micro_inputs)/sizeof(micro_inputs[0]); i++){
    buttons[i].update();

    if(buttons[i].fell()) { //Catch all update, remove later! ***
      micro_inputs[i].pinState = true; 
    }    
  }
  micro_inputs[clockUp].pinState = buttons[clockUp].fell();
  micro_inputs[clockDown].pinState = buttons[clockDown].fell();
  micro_inputs[northPedIn].pinState = !buttons[northPedIn].read();
  micro_inputs[eastPedIn].pinState = !buttons[eastPedIn].read();
  micro_inputs[northCarIn].pinState = !buttons[northCarIn].read();
  micro_inputs[eastCarIn].pinState = !buttons[eastCarIn].read();
  micro_inputs[stepperIn1].pinState = buttons[stepperIn1].read();
  micro_inputs[stepperIn2].pinState = buttons[stepperIn2].read();
  micro_inputs[stepperIn3].pinState = buttons[stepperIn3].read();
  micro_inputs[stepperIn4].pinState = buttons[stepperIn4].read();
  
  //Check all inputs on mcp1 and store state
  for(int i = 0; i < sizeof(mcp1_inputs)/sizeof(mcp1_inputs[0]); i++){
    mcp1_inputs[i].pinState = mcp1.digitalRead(mcp1_inputs[i].pinNum);
  }
  
  //Check all buttons on mcp2 and store state, ACTIVE LOW
  for(int i = 0; i < sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0]); i++){
    mcp2_inputs[i].pinState = !mcp2.digitalRead(mcp2_inputs[i].pinNum);
  }
}


//Maintains the fun buttons - needs special functions added as specified in lab manual (hold down to latch, invert if held down on turn on) ***
void funButtons(){
  static unsigned long lastTime[sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0])];
  static unsigned long currentTime[sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0])];
  static int buttonLatchTime = 2000; //Milliseconds to wait before latching
  static bool lastState[sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0])];
  static bool currentState[sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0])];
  static bool latch[sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0])];

  
  for(int i = 0; i < sizeof(mcp2_inputs)/sizeof(mcp2_inputs[0]); i++){    
    currentTime[i] = millis(); //Check time since button press
    currentState[i] = mcp2_inputs[i].pinState;
    
    if((lastState[i] == currentState[i]) && (currentState[i] == HIGH)){  //If last state was the same, maybe latch
      if((currentTime[i] - lastTime[i]) > buttonLatchTime){   //Set latch if held long enough
        latch[i] = HIGH;
      }
    }else{
      lastTime[i] = currentTime[i]; //Reset latch timer
    }
    
    if((lastState[i] == LOW) && (currentState[i] == HIGH)){ //Clear latch if button is pressed
      latch[i] = LOW;
    }

    if(latch[i] == HIGH){
      mcp2.digitalWrite(mcp2_outputs[i].pinNum, HIGH);
    }else{
      mcp2.digitalWrite(mcp2_outputs[i].pinNum, currentState[i]);
    }
        
    lastState[i] = mcp2_inputs[i].pinState;
  }  
}



//For Main() readability, can be substituted directly into it - no encapsulation
void trafficControl(){
  mcp1.digitalWrite(mcp1_outputs[northGreen].pinNum, LOW);
  mcp1.digitalWrite(mcp1_outputs[northAmber].pinNum, LOW);
  mcp1.digitalWrite(mcp1_outputs[northRed].pinNum, LOW);
  mcp1.digitalWrite(mcp1_outputs[northPed].pinNum, LOW);
  if(mcp1_inputs[northLight1].pinState == true){
     mcp1.digitalWrite(mcp1_outputs[northGreen].pinNum, HIGH);
     if(mcp1_inputs[northLight2].pinState == true){
       mcp1.digitalWrite(mcp1_outputs[northPed].pinNum, HIGH);
     }
  }else{
    if(mcp1_inputs[northLight2].pinState == true){
      mcp1.digitalWrite(mcp1_outputs[northAmber].pinNum, HIGH);
    }else{
      mcp1.digitalWrite(mcp1_outputs[northRed].pinNum, HIGH);
    }
  }

  mcp1.digitalWrite(mcp1_outputs[eastGreen].pinNum, LOW);
  mcp1.digitalWrite(mcp1_outputs[eastAmber].pinNum, LOW);
  mcp1.digitalWrite(mcp1_outputs[eastRed].pinNum, LOW);
  mcp1.digitalWrite(mcp1_outputs[eastPed].pinNum, LOW);
  if(mcp1_inputs[eastLight1].pinState == true){
     mcp1.digitalWrite(mcp1_outputs[eastGreen].pinNum, HIGH);
     if(mcp1_inputs[eastLight2].pinState == true){
       mcp1.digitalWrite(mcp1_outputs[eastPed].pinNum, HIGH);
     }
  }else{
    if(mcp1_inputs[eastLight2].pinState == true){
      mcp1.digitalWrite(mcp1_outputs[eastAmber].pinNum, HIGH);
    }else{
      mcp1.digitalWrite(mcp1_outputs[eastRed].pinNum, HIGH);
    }
  }

  mcp1.digitalWrite(mcp1_outputs[northPedOut].pinNum, micro_inputs[northPedIn].pinState);
  mcp1.digitalWrite(mcp1_outputs[eastPedOut].pinNum, micro_inputs[eastPedIn].pinState);
  mcp1.digitalWrite(mcp1_outputs[northCarOut].pinNum, micro_inputs[northCarIn].pinState);
  mcp1.digitalWrite(mcp1_outputs[eastCarOut].pinNum, micro_inputs[eastCarIn].pinState);
  
}



//Initialises regeisters for timer
void timerSetup(){
    //Set up Timer 1  
  TCCR1A = bit (COM1A0); // toggle OC1A on compare match
  //Prescaler registers are
  TCCR1B = (1 << CS12)| (1 << WGM12); //CTC (clear on compare is true), Prescaler initialised to x256
  //number of clocks to count for each toggle 
  //0 results in toggling every clock, 1 every 2 clocks etc
  OCR1A = 0; //Output every clock 
}


//Controls the clock generator. clockButton determines increment or decrement clock
//1 is increment, anything else is decrement
void clockControl(int clockButton){  static int desiredClock = 0;
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
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }  
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



//Stepper Function---------------------------------------------------
void stepper(bool aOn, bool bOn, bool cOn, bool dOn, bool reverseButton, int outPin1, int outPin2, int outPin3, int outPin4) {

  static int count = 0; //Which LED is turned on
  static int stepPrev = 4; // what the previous inputs were
  
  if (!reverseButton) {
    int aTemp, bTemp;
    aTemp = aOn;
    bTemp = bOn;
    aOn = dOn;
    bOn = cOn;
    cOn = bTemp;
    dOn = aTemp;
  }

    int pins = aOn +2*bOn + 4*cOn+8*dOn; //collective current state of inputs

  switch(pins){   // Only a select sequence turns the 'motor'. Checking for correct sequence by comparing with lat state.

  case (1):

    if ((stepPrev == 4) or (stepPrev == 8)){
      switch (stepPrev){
        case(4):
          count--;            //count incremented or decremented to either go up to the next light
          if(count < 1){      // or down to the one below. (determines direction).
            count = 12;
          }
          if (count > 12){
            count = 1;
          }
          turnOn(count, outPin1, outPin2, outPin3, outPin4);
              break;
          case(8):
          count++;
          if(count < 1){   // only 12 lights. Clipping to make sure it continues going round
            count = 12;
          }
          if (count > 12){
            count = 1;
          }
          turnOn(count, outPin1, outPin2, outPin3, outPin4);
              break;
    }
    }
  break;

  case (2):
    if ((stepPrev == 4) or (stepPrev == 8)){
          switch (stepPrev){
            case(4):
              count++;
              if(count < 1){
                count = 12;
              }
              if (count > 12){
                count = 1;
              }
              turnOn(count, outPin1, outPin2, outPin3, outPin4);
                  break;
              case(8):
              count--;
              if(count < 1){
                count = 12;
              }
              if (count > 12){
                count = 1;
              }
              turnOn(count, outPin1, outPin2, outPin3, outPin4);
                  break;
        }
        }
      break;
  case (4):
    if ((stepPrev == 1) or (stepPrev == 2)){
          switch (stepPrev){
            case(1):
              count++;
              if(count < 1){
                count = 12;
              }
              if (count > 12){
                count = 1;
              }
              turnOn(count, outPin1, outPin2, outPin3, outPin4);
                  break;
              case(2):
              count--;
              if(count < 1){
                count = 12;
              }
              if (count > 12){
                count = 1;
              }
              turnOn(count, outPin1, outPin2, outPin3, outPin4);
                  break;
        }
        }
      break;
  case (8):
    if ((stepPrev == 1) or (stepPrev == 2)){
          switch (stepPrev){
            case(1):
              count--;
              if(count < 1){
                count = 12;
              }
              if (count > 12){
                count = 1;
              }
              turnOn(count, outPin1, outPin2, outPin3, outPin4);
                  break;
              case(2):
              count++;
              if(count < 1){
                count = 12;
              }
              if (count > 12){
                count = 1;
              }
              turnOn(count, outPin1, outPin2, outPin3, outPin4);
                  break;
            }
            }
      break;
  default:
    turnOn(count, outPin1, outPin2, outPin3, outPin4);
    break;
  }
  stepPrev = pins; // remember state.
}

//Stepper function helper
  void turnOn(int state, int out1, int out2, int out3, int out4) {
      static const int pinConfig[12][4] = {
  { OUTPUT,OUTPUT,INPUT,INPUT },  //Diode 1
  { INPUT,INPUT,OUTPUT,OUTPUT },  //Diode 2
  { INPUT,INPUT,OUTPUT,OUTPUT },  //Diode 3
  { INPUT,OUTPUT,INPUT,OUTPUT },  //Diode 4
  { INPUT,OUTPUT,INPUT,OUTPUT },  //Diode 5
  { OUTPUT,INPUT,INPUT,OUTPUT },  //Diode 6
  { INPUT,OUTPUT,OUTPUT,INPUT },  //Diode 7 
  { OUTPUT,INPUT,INPUT,OUTPUT },  //Diode 8
  { INPUT,OUTPUT,OUTPUT,INPUT },  //Diode 9  
  { OUTPUT,INPUT,OUTPUT,INPUT },  //Diode 10
  { OUTPUT,INPUT,OUTPUT,INPUT },  //Diode 11
  { OUTPUT,OUTPUT,INPUT,INPUT }   //Diode 12
  };

  static const int pinOutState[12][4] = {
  { LOW,HIGH,LOW,LOW },  //Diode 1
  { LOW,LOW,HIGH,LOW },  //Diode 2
  { LOW,LOW,LOW,HIGH },  //Diode 3
  { LOW,HIGH,LOW,LOW },  //Diode 4
  { LOW,LOW,LOW,HIGH },  //Diode 5
  { HIGH,LOW,LOW,LOW },  //Diode 6
  { LOW,HIGH,LOW,LOW },  //Diode 7  
  { LOW,LOW,LOW,HIGH },  //Diode 8
  { LOW,LOW,HIGH,LOW },  //Diode 9  
  { HIGH,LOW,LOW,LOW }, //Diode 10
  { LOW,LOW,HIGH,LOW },  //Diode 11
  { HIGH,LOW,LOW,LOW }   //Diode 12
  };


    // uses lookup tables to determine settings
    pinMode(out1, pinConfig[state - 1][0]);
    pinMode(out2, pinConfig[state - 1][1]);
    pinMode(out3, pinConfig[state - 1][2]);
    pinMode(out4, pinConfig[state - 1][3]);
    digitalWrite(out1, pinOutState[state - 1][0]);
    digitalWrite(out2, pinOutState[state - 1][1]);
    digitalWrite(out3, pinOutState[state - 1][2]);
    digitalWrite(out4, pinOutState[state - 1][3]);

  }
