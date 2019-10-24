#include <Arduino.h>
#include <Bounce2.h>


int LEDPins[] = {A1,A2};
int stateClockUp = 12;
int stateClockDown = 11;


Bounce debouncerA = Bounce(); // Instantiate a Bounce object
Bounce debouncerB = Bounce();

void setup() {
	pinMode(LEDPins[0], OUTPUT);
	pinMode(LEDPins[1],OUTPUT);
	pinMode(stateClockDown,INPUT);
	pinMode(stateClockUp,INPUT);

	debouncerA.attach(stateClockUp,INPUT); // Attach the debouncer to a pin with INPUT_PULLUP mode
	debouncerB.attach(stateClockDown,INPUT); // Attach the debouncer to a pin with INPUT_PULLUP mode
	debouncerA.interval(25); // Use a debounce interval of 25 milliseconds
	debouncerB.interval(25); // Use a debounce interval of 25 milliseconds
}

int DebouncerA(){
	 debouncerA.update();
	  // Get the updated value :
	  int value1 = debouncerA.read();
	  return value1;
}

int DebouncerB(){
	  debouncerB.update();

	  // Get the updated value :
	  int value2= debouncerB.read();



return value2;
}

void loop() {
	  stateClockUp = DebouncerA();
	  stateClockDown = DebouncerB();

	 digitalWrite(LEDPins[0],stateClockUp);

	 if (stateClockUp == HIGH){
		 digitalWrite(LEDPins[1],LOW);
	 }
	 else{

	 digitalWrite(LEDPins[1],stateClockDown);

	 }

	}



