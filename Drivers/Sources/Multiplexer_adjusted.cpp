#include <Arduino.h>

int inputPins[] = {2,3,4,5}; //Digital pins being used
int outputPins [] = {6,7,8,9};
const int outPinLength = 4; //allows for easy increase of pins size.Need to only change pins here and add cases
const int inputPinLength = 4;



static const int pinConfig[12][4] = {
			{OUTPUT,OUTPUT,INPUT,INPUT},  //Diode 1
			{OUTPUT,OUTPUT,INPUT,INPUT},  //Diode 2
			{INPUT,OUTPUT,OUTPUT,INPUT},  //Diode 3
			{INPUT,OUTPUT,OUTPUT,INPUT},  //Diode 4
			{INPUT,INPUT,OUTPUT,OUTPUT},  //Diode 5
			{INPUT,INPUT,OUTPUT,OUTPUT},  //Diode 6
			{OUTPUT,INPUT,OUTPUT,INPUT},  //Diode 7
			{OUTPUT,INPUT,OUTPUT,INPUT},  //Diode 8
			{INPUT,OUTPUT,INPUT,OUTPUT},  //Diode 9
			{INPUT,OUTPUT,INPUT,OUTPUT},  //Diode 10
			{OUTPUT,INPUT,INPUT,OUTPUT},  //Diode 11
			{OUTPUT,INPUT,INPUT,OUTPUT}   //Diode 12
	};

	static const int pinOutState[12][4] = {
				{LOW,HIGH,LOW,LOW},  //1
				{HIGH,LOW,LOW,LOW},  //2
				{LOW,LOW,HIGH,LOW},  //3
				{LOW,HIGH,LOW,LOW},  //4
				{LOW,LOW,LOW,HIGH},  //5
				{LOW,LOW,HIGH,LOW},  //6
				{LOW,LOW,HIGH,LOW},  //7
				{HIGH,LOW,LOW,HIGH}, //8
				{LOW,LOW,LOW,HIGH},  //9
				{LOW,HIGH,LOW,LOW},  //10
				{LOW,LOW,LOW,HIGH},  //11
				{HIGH,LOW,LOW,LOW}   //12
		};


void setup() {
	for (int i = 0; i < inputPinLength;i++){
	pinMode(inputPins[i],INPUT_PULLUP);
}

}

void turnOn(int state){
	
	  pinMode(outputPins[0], pinConfig[state-1][0] );
	  pinMode(outputPins[1], pinConfig[state-1][1]);
	  pinMode( outputPins[2], pinConfig[state-1][2] );
	  pinMode( outputPins[3], pinConfig[state-1][3] );
	  digitalWrite( outputPins[0], pinOutState[state-1][0] );
	  digitalWrite( outputPins[1], pinOutState[state-1][1] );
	  digitalWrite( outputPins[2], pinOutState[state-1][2] );
	  digitalWrite( outputPins[3], pinOutState[state-1][3] );

	}

void loop() {
	int aOn = !digitalRead(inputPins[0]); //Active low (using pull up) so inverted
	int bOn = !digitalRead(inputPins[1]);
	int cOn = !digitalRead(inputPins[2]);
	int dOn = !digitalRead(inputPins[3]);

	int count = aOn +2*bOn + 4*cOn + 8*dOn;

	if((count > 12)){
		count = 1;
		turnOn(count);
	}

	else{
			turnOn(count);
}
}


