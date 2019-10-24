#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>

//Pin constants, to make it easier to read the program
const byte CLOCKOUT = 9; //All others

const byte buttonUp = 2;


//Global Variables
volatile boolean changeClock = false;

//Connect to LCD via I2C, default address 0x27 (A0-A2 not jumpered)
LiquidCrystal_I2C lcd(0x27,16,2); //Change to (0x27,16,2) for 16x2 LCD.

// the setup function runs once when you press reset or power the board
void setup() {
  //Set up debug monitoring
  Serial.begin(9600);
  
  //pinMode declerations
  pinMode(buttonUp, INPUT_PULLUP);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  //Set up timer on CLOCKOUT (OC1A) 
  pinMode (CLOCKOUT, OUTPUT);
   
  //Set up Timer 1
  
  TCCR1A = bit (COM1A0); // toggle OC1A on compare match
  //Prescaler registers are
  TCCR1B = (1 << CS12)| (1 << WGM12); //CTC (clear on compare is true), Prescaler initialised to x256
  //number of clocks to count for each toggle 
  //0 results in toggling every clock, 1 every 2 clocks etc
  OCR1A = 0; //Output every clock 

  //Initiate the LCD
  lcd.init();
  lcd.backlight();

  //Set up ISR for button inputs
  attachInterrupt (digitalPinToInterrupt(buttonUp), changeClockISR, FALLING);
}
//Check if a button was pressed, then hand off to loop()
void changeClockISR(){
  if(digitalRead(buttonUp) == LOW){
    changeClock = true;    
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

  Serial.print("In the clock Control");
  lcd.clear();
  switch(desiredClock){
    //1Hz
    case 0:
      TCCR1B = (1 << CS12)| (1 << WGM12);
      OCR1A  = 31249;
      lcd.setCursor(0,0); //Set the cursor on the first column and first row.
      lcd.print("1Hz");
    break;
    //2Hz
    case 1:
      OCR1A  = 15624;
      lcd.setCursor(0,0);
      lcd.print("2Hz");
    break;
    //5Hz
    case 2:
      OCR1A  = 6249;
      lcd.setCursor(0,0);
      lcd.print("5Hz");
    break;
    //10Hz
    case 3:
      OCR1A  = 3124;
      lcd.setCursor(0,0);
      lcd.print("10Hz");
    break;
    //20Hz
    case 4:
      OCR1A  = 1561;
      lcd.setCursor(0,0);
      lcd.print("20Hz");
    break;
    //50Hz
    case 5:
      OCR1A  = 624;
      lcd.setCursor(0,0);
      lcd.print("50Hz");
    break;
    //100Hz
    case 6:
      TCCR1B = 0; //Clear prescale register, since we can move from here to a different scale
      TCCR1B = (1 << CS12)| (1 << WGM12);
      OCR1A  = 311;
      lcd.setCursor(0,0);
      lcd.print("100Hz");
    break;
    //200Hz, change prescaler to 1x (CS10 set)
    case 7:
      TCCR1B = 0; //Clear prescale register, since we can move here from a different scale
      TCCR1B = (1 << CS10)| (1 << WGM12);
      OCR1A  = 39999;
      lcd.setCursor(0,0);
      lcd.print("200Hz");
    break;
    //500Hz
    case 8:
      OCR1A  = 15999;
      lcd.setCursor(0,0);
      lcd.print("500Hz");
    break;
    //1kHz
    case 9:
      OCR1A  = 7999;
      lcd.setCursor(0,0);
      lcd.print("1kHz");
    break;
    //2kHz
    case 10:
      OCR1A  = 3999;
      lcd.setCursor(0,0);
      lcd.print("2kHz");
    break;
    //5kHz
    case 11:
      OCR1A  = 1599;
      lcd.setCursor(0,0);
      lcd.print("5kHz");
    break;
    //10kHz
    case 12:
      OCR1A  = 799;
      lcd.setCursor(0,0);
      lcd.print("10kHz");
    break;
    //20kHz
    case 13:
      OCR1A  = 399;
      lcd.setCursor(0,0);
      lcd.print("20kHz");
    break;
    //50kHz
    case 14:
      OCR1A  = 159;
      lcd.setCursor(0,0);
      lcd.print("50kHz");
    break;
    //100kHz
    case 15:
      OCR1A  = 79;
      lcd.setCursor(0,0);
      lcd.print("100kHz");
    break;
    //200kHz
    case 16:
      OCR1A  = 39;
      lcd.setCursor(0,0);
      lcd.print("200kHz");
    break;
    //500kHz
    case 17:
      TCCR1B = 0; //Clear prescale register, since we can move here from a different scale
      TCCR1B = (1 << CS10)| (1 << WGM12);
      OCR1A  = 15;
      lcd.setCursor(0,0);
      lcd.print("500kHz");
    break;
    
  }

  changeClock = false;  

}

// the loop function runs over and over again forever
void loop() {



  //check if we need to change the clock
  if(changeClock){
    if(digitalRead(buttonUp)== LOW){
      clockControl(1);
    }
  }
}
