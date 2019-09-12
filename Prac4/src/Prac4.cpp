/*
 * Prac4.cpp
 *
 * Originall written by Stefan Schröder and Dillion Heald
 *
 * Adapted for EEE3096S 2019 by Keegan Crankshaw
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "Prac4.h"

using namespace std;

long lastInterruptTime = 0; // used for button debounce

volatile bool playing = true;

volatile bool stopped = false;

unsigned char buffer[2][BUFFER_SIZE][2];

int bufferLocation = 0;

int bufferReading = 0;

bool threadReady = false;

// Configure your interrupts here.
// Don't forget to use debouncing.
void play_pause_isr(void) {

	//Write your logic here
	//Debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime > 200) {

		if (playing) {

			printf("Paused\n");

		} else {

			printf("Resuming\n");

		}

		playing = !playing;


	}

	lastInterruptTime = interruptTime;

}

void stop_isr(void) {

	// Write your logic here
	//Debounce
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime > 200) {

		printf("Stopped\n");

		stopped = true;

	}

	lastInterruptTime = interruptTime;

}

/*
 * Setup Function. Called once
 */

bool setup = false;

int setup_gpio(void) {

	if (setup) { // called once only

		return 0;

	}

	//Set up wiring Pi
	wiringPiSetupPhys();

	//setting up the buttons
	//setup mode and pin number
	pinMode(PLAY_BUTTON, INPUT);

	pullUpDnControl(PLAY_BUTTON, PUD_UP);

	pinMode(STOP_BUTTON, INPUT);

	pullUpDnControl(STOP_BUTTON, PUD_UP);

	//adding interrupts
	wiringPiISR(PLAY_BUTTON, INT_EDGE_FALLING, &play_pause_isr);

	wiringPiISR(STOP_BUTTON, INT_EDGE_FALLING, &stop_isr);

	//setting up the SPI interface
	if (wiringPiSPISetup(SPI_CHAN, SPI_SPEED) != -1) {

		printf("SPI interface setup complete.\n");

	} else {

		printf("SPI interface setup failed.\n");

		return -1;

	}

	setup = true;

	printf("GPIO setup complete.\n");

	return 0;

}

/*
 * Thread that handles writing to SPI
 *
 * You must pause writing to SPI if not playing is true (the player is paused)
 * When calling the function to write to SPI, take note of the last argument.
 * You don't need to use the returned value from the wiring pi SPI function
 * You need to use the buffer_location variable to check when you need to switch buffers
 */

PI_THREAD (play_audio) {

	piHiPri(99);

	while(!threadReady) {

		delay(1);

	}

	while(!stopped) {

		while(!playing) {

			delay(50);

		}

		wiringPiSPIDataRW(SPI_CHAN, buffer[bufferReading][bufferLocation], 2);

		bufferLocation++;

		piLock(0);

		if(bufferLocation >= BUFFER_SIZE) {

			bufferLocation = 0;

			bufferReading = (bufferReading+1) % 2;

		}

		piUnlock(0);

	}

	printf("Thread exiting.\n");

	return(0);

}

int main() {

	// Call the setup GPIO function
	if (setup_gpio() == -1) {

		return 0;

	}

	/* Initialize thread with parameters
	 * Set the play thread to have a 99 priority
	 * Read https://docs.oracle.com/cd/E19455-01/806-5257/attrib-16/index.html
	 */

	//Write your logic here
	piThreadCreate (play_audio);

	// Open the file
	char ch;

	FILE *filePointer;

	printf("%s\n", FILENAME);

	filePointer = fopen(FILENAME, "r"); // read mode

	if (filePointer == NULL) {

		perror("Error while opening the file.\n");

		exit(EXIT_FAILURE);

	}

	/*
	 * Read from the file, character by character
	 * You need to perform two operations for each character read from the file
	 * You will require bit shifting
	 *
	 * buffer[bufferWriting][counter][0] needs to be set with the control bits
	 * as well as the first few bits of audio
	 *
	 * buffer[bufferWriting][counter][1] needs to be set with the last audio bits
	 *
	 * Don't forget to check if you have pause set or not when writing to the buffer
	 *
	 */

	int counter = 0;

	int bufferWriting = 0;

	char config = 0 * 0x40 + 1 * 0x20 + 1 * 0x10;

	printf("spi speed %d\n", SPI_SPEED);
	printf("\nPlaying song >>>\n");

	// Have a loop to read from the file
	while ((ch = fgetc(filePointer)) != EOF && !stopped) {

		while (threadReady && (bufferWriting == bufferReading) && (counter == 0) && !stopped) {

			delay(1);

		}

		//Set config bits
		buffer[bufferWriting][counter][0] = ch >> 6;

		buffer[bufferWriting][counter][0] |= config;

		buffer[bufferWriting][counter][1] = ch << 2;

		counter++;

		piLock(0);

		if (counter >= BUFFER_SIZE) {

			if (!threadReady) {

				threadReady = true;

			}

			counter = 0;

			bufferWriting = (bufferWriting + 1) % 2;

		}

		piUnlock(0);

	}

	fclose(filePointer); // Close the file

	printf("Reading done.\n");
	
	printf("Turn off dac.\n");
	unsigned char data[2] = {0,0};
	wiringPiSPIDataRW(SPI_CHAN, data, 2);

	return 0;
}

