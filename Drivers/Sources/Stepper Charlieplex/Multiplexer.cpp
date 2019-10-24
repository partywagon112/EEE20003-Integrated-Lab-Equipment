#include <Arduino.h>

void stepperLogic(aOn, bOn, cOn, dOn, reverseButton, outPin1, outPin2, outPin3, outPin4) {
	static const int pinConfig[12][4] = {
		{ OUTPUT,OUTPUT,INPUT,INPUT },  //Diode 1
	{ OUTPUT,OUTPUT,INPUT,INPUT },  //Diode 2
	{ INPUT,OUTPUT,OUTPUT,INPUT },  //Diode 3
	{ INPUT,OUTPUT,OUTPUT,INPUT },  //Diode 4
	{ INPUT,INPUT,OUTPUT,OUTPUT },  //Diode 5
	{ INPUT,INPUT,OUTPUT,OUTPUT },  //Diode 6
	{ OUTPUT,INPUT,OUTPUT,INPUT },  //Diode 7
	{ OUTPUT,INPUT,OUTPUT,INPUT },  //Diode 8
	{ INPUT,OUTPUT,INPUT,OUTPUT },  //Diode 9
	{ INPUT,OUTPUT,INPUT,OUTPUT },  //Diode 10
	{ OUTPUT,INPUT,INPUT,OUTPUT },  //Diode 11
	{ OUTPUT,INPUT,INPUT,OUTPUT }   //Diode 12
	};

	static const int pinOutState[12][4] = {
		{ LOW,HIGH,LOW,LOW },  //Diode 1
	{ HIGH,LOW,LOW,LOW },  //Diode 2
	{ LOW,LOW,HIGH,LOW },  //Diode 3
	{ LOW,HIGH,LOW,LOW },  //Diode 4
	{ LOW,LOW,LOW,HIGH },  //Diode 5
	{ LOW,LOW,HIGH,LOW },  //Diode 6
	{ LOW,LOW,HIGH,LOW },  //Diode 7
	{ HIGH,LOW,LOW,HIGH }, //Diode 8
	{ LOW,LOW,LOW,HIGH },  //Diode 9
	{ LOW,HIGH,LOW,LOW },  //Diode 10
	{ LOW,LOW,LOW,HIGH },  //Diode 11
	{ HIGH,LOW,LOW,LOW }   //Diode 12
	};


	void turnOn(int state, int out1, int out2, int out3, int out4) {

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



	static int count = 0; //Which LED is turned on
	static int stepPrev = 0; // what the previous inputs were
	static int aOn, bOn,cOn,dOn; //current individual inputs

	if (reverseButton) {
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


