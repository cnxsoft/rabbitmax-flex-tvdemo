/* Useless TV demo - Turn on/off with button or temperature sensor */
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <wiringPi.h>
#include <lcd.h>

#include "BMP180.h"

/* LCD Display defines */
#define LCDROWS    2
#define LCDCOLS   16
#define LCDBITS    4
#define LCDRS      7
#define LCDSTRB   29
#define LCDDATA0   2
#define LCDDATA1   3 
#define LCDDATA2  12 
#define LCDDATA3  13

/* GPIO number definitions for WiringPi library */
#define ButtonPin 14
#define BuzzerPin 22

void beep() {
	int i;
	for (i=0; i<250; i++) { /* Beep for around one second */

		digitalWrite(BuzzerPin, LOW);
		delay(2); 
		digitalWrite(BuzzerPin, HIGH);
		delay(2);
	}
}



int main()
{
	static int lcdHandle = 0;
	if (-1 == wiringPiSetup()) {
		printf("setup wiringPi failed!\n");
		return 1;
	}
	/* Set GPIO pins direction */ 
	pinMode(ButtonPin, INPUT);
	pullUpDnControl(ButtonPin, PUD_UP);
	pinMode(BuzzerPin, OUTPUT);

	/* Configure LCD display, assume TV is OFF initially */
	
	lcdHandle = lcdInit(LCDROWS, LCDCOLS, LCDBITS, LCDRS, LCDSTRB, LCDDATA0, LCDDATA1, LCDDATA2, LCDDATA3, 0, 0, 0, 0);
	lcdClear(lcdHandle);
	lcdPosition(lcdHandle, 0, 0);
	lcdPuts(lcdHandle, "TV OFF");

	/* Initialize BMP180 Sensor */
	int fd = wiringPiI2CSetup(BMP180_I2CADDR);
	if (0 > fd) {
		fprintf(stderr, "ERROR: Unable to access RabbitMax temperature sensor: %s\n", strerror (errno));
		return 1;
	}
	if (0 > begin(fd)) {
		fprintf(stderr, "ERROR: RabbitMax temperature sensor not found\n");
		return 1;
	}

	/* Check whether button has been released or temperature over 30C, and change TV status */
	while(1)
	{
		static int OldButtonStatus = HIGH; /* Previous value of GPIO Pin */
		static int TVStatus = LOW; /* OFF */
		static double OldBMP180Temp = 0; /* Store temperature value */
		double BMP180Temp = 0; /* Current temperature value */
		int ButtonStatus = digitalRead(ButtonPin); /* Current value of GPIO pin */

		/* Get temperature from BMP180 I2C sensor */
		getTemperature(fd, &BMP180Temp);
		if ((OldBMP180Temp < 30) && (BMP180Temp >= 30)) {
			/* If temp get over 30 C, simulate button released and set TV status to OFF (to turn it ON) */
			printf("Temperature rose over 30 C...\n");
			ButtonStatus = HIGH;
			OldButtonStatus = LOW; 
			TVStatus = LOW;
		}
		if ((OldBMP180Temp >= 30) && (BMP180Temp < 30)) {
			/* Simulate button released and set TV status to ON (to turn it OFF) */
			printf("Temperature dropped below 30 C...\n");
			ButtonStatus = HIGH;
			OldButtonStatus = LOW; 
			TVStatus = HIGH;
		}
		OldBMP180Temp = BMP180Temp;
			
		if ( (HIGH == ButtonStatus) && (LOW == OldButtonStatus) )
		{
			if (LOW == TVStatus) { /* TV OFF, turn it on and beep for 1 second */
				printf("Turn TV On!\n");

				/* Send power IR code */
				system("sudo irsend SEND_ONCE /home/pi/lircd.conf KEY_POWER");
			
				lcdClear(lcdHandle);
				lcdPosition(lcdHandle, 0, 0);
				lcdPuts(lcdHandle, "TV ON");
				beep(); /* Buzzer on */
				TVStatus = HIGH;
			} else { /* TV ON, turn it off */
				printf("Turn TV Off!\n");

				/* Send Power IR code */

				system("sudo irsend SEND_ONCE /home/pi/lircd.conf KEY_POWER");
				lcdClear(lcdHandle);
				lcdPosition(lcdHandle, 0, 0);
				lcdPuts(lcdHandle, "TV OFF");
				lcdPosition (lcdHandle, 0, 1);
				lcdPuts(lcdHandle, "");
				TVStatus = LOW;
			}
		}
		/* Update Temperature value */
		lcdPosition (lcdHandle, 0, 1);
		lcdPrintf(lcdHandle, "Temp.: \t%0.1f C", OldBMP180Temp);

		OldButtonStatus = ButtonStatus;
		delay(500);
	}
	return 0;
}
