#include <Arduino.h>

int Pin1 = 12;
int Pin2 = 11;
int Pin3 = 10;
int Pin4 = 9;

void setup() {
pinMode(Pin1,OUTPUT);
pinMode(Pin2,OUTPUT);
pinMode(Pin3,OUTPUT);
pinMode(Pin4,OUTPUT);
}

void loop() {

	for (int i = 0; i < 4; i++){

		switch (i){

		case (0):                 //1
		digitalWrite(Pin1,HIGH);
		digitalWrite(Pin2,LOW);
		digitalWrite(Pin3,LOW);
		digitalWrite(Pin4,LOW);
		delay(100);
		break;

		case (1):                 //4
		digitalWrite(Pin1,LOW);
		digitalWrite(Pin2,HIGH);
		digitalWrite(Pin3,LOW);
		digitalWrite(Pin4,LOW);
		delay(100);
		break;

		case (2):              //2
		digitalWrite(Pin1,LOW);
		digitalWrite(Pin2,HIGH);
		digitalWrite(Pin3,LOW);
		digitalWrite(Pin4,LOW);
		delay(100);
		break;

		case (3):            //8
		digitalWrite(Pin1,LOW);
		digitalWrite(Pin2,LOW);
		digitalWrite(Pin3,LOW);
		digitalWrite(Pin4,HIGH);
		delay(100);
		break;



		}


	}

}
