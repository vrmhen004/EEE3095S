/*
 * BinClock.c
 * Jarrod Olivier
 * Modified for EEE3095S/3096S by Keegan Crankshaw
 * August 2019
 *
 * VRMHEN004 DPLKYL002
 * Date
*/

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include <signal.h> // For catching interrupts
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance

int HH, MM, SS;

void initGPIO(void) {
    /*
     * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
     * You can also use "gpio readall" in the command line to get the pins
     * Note: wiringPi does not use GPIO or board pin numbers (unless specifically set to that mode)
     */
    printf("Setting up\n");
    wiringPiSetupPhys(); //This is the default mode. If you want to change pinouts, be aware

    RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC

    //Set up the LEDS
    for (int i; i < sizeof(LEDS) / sizeof(LEDS[0]); i++) {
        pinMode(LEDS[i], OUTPUT);
    }

    //Set Up the Seconds LED for PWM
    //Write your logic here
    pinMode(SECS, PWM_OUTPUT);
    //softPwmCreate(SECS, 0, 100);

    printf("LEDS done\n");

    //Set up the Buttons
    for (int j; j < sizeof(BTNS) / sizeof(BTNS[0]); j++) {
        pinMode(BTNS[j], INPUT);
        pullUpDnControl(BTNS[j], PUD_UP);
    }

    //Attach interrupts to Buttons
    //Write your logic here
    wiringPiISR(BTNS[1], INT_EDGE_FALLING, &hourInc);
    wiringPiISR(BTNS[0], INT_EDGE_FALLING, &minInc);

    printf("BTNS done\n");
    printf("Setup done\n");
}


int convertFromRTCBCDtoInt(int bcd) {
    int firstDigit = bcd & 0b00001111;
    int secondDigit = (bcd & 0b01110000) >> 4;
    return secondDigit * 10 + firstDigit;
}

int convertFromRTCBCDHourstoInt(int bcd) {
    int firstDigit = bcd & 0b00001111;
    int secondDigit = (bcd & 0b00010000) >> 4;
    return secondDigit * 10 + firstDigit;
}

/*
 * The main function
 * This function is called, and calls all relevant functions we've written
 */
int main(void) {
    initGPIO();
    signal(SIGNAL, cleanup); //catch interrupts

    //Enable oscilation
    wiringPiI2CWriteReg8(RTC, SEC, 0b10000000);

    //Set random time (3:04PM)
    //Set hours to 12h clock and PM and random time
    wiringPiI2CWriteReg8(RTC, HOUR, 0b01100000); //03

    //You can comment this file out later
    //wiringPiI2CWriteReg8(RTC, HOUR, 0x13+TIMEZONE);
    //wiringPiI2CWriteReg8(RTC, MIN, 0x4);
    //wiringPiI2CWriteReg8(RTC, SEC, 0x00);*/
    toggleTime();

    // Repeat this until we shut down
    for (;;) {
        //Fetch the time from the RTC
        //Write your logic here
        hours = convertFromRTCBCDHourstoInt(wiringPiI2CReadReg8(RTC, HOUR));
        mins = convertFromRTCBCDtoInt(wiringPiI2CReadReg8(RTC, MIN));
        secs = convertFromRTCBCDtoInt(wiringPiI2CReadReg8(RTC, SEC));

        //Function calls to toggle LEDs
        //Write your logic here
        lightHours(hours);
        lightMins(mins);
        secPWM(secs);

        /*For testing sec
        secPWM(100);
        delay(1000);
        for(int i = 0; i<61; i++){
            delay(100);
            secPWM(i);
        }
        //For testing hours
        for(int i = 0; i<13; i++){
            delay(1000);
            lightHours(i);
        }
        //For testing mins
        for(int i = 0; i<61; i++){
            delay(100);
            lightMins(i);
        }*/

        // Print out the time we have stored on our RTC
        printf("The current time is: %d:%d:%d\n", hours, mins, secs);

        //using a delay to make our program "less CPU hungry"
        delay(1000); //milliseconds
    }
    return 0;
}

/*
 * Change the hour format to 12 hours
 */
int hFormat(int hours) {
    /*formats to 12h*/
    if (hours >= 24) {
        hours = 0;
    } else if (hours > 12) {
        hours -= 12;
    }
    return (int) hours;
}

/*
 * Turns on corresponding LED's for hours
 */
void lightHours(int units) {
    // Write your logic to light up the hour LEDs here
    for (int i = 3; i > -1; i--) {
        digitalWrite(LEDS[i], units % 2);
        units /= 2;
    }
}

/*
 * Turn on the Minute LEDs
 */
void lightMins(int units) {
    //Write your logic to light up the minute LEDs here
    for (int i = 5; i > -1; i--) {
        digitalWrite(LEDS[i + 4], units % 2);
        units /= 2;
    }
}

/*
 * PWM on the Seconds LED
 * The LED should have 60 brightness levels
 * The LED should be "off" at 0 seconds, and fully bright at 59 seconds
 */
void secPWM(int units) {
    pwmWrite(SECS, ((((float) units) / 60) * 100));
}

/*
 * hexCompensation
 * This function may not be necessary if you use bit-shifting rather than decimal checking for writing out time values
 */
int hexCompensation(int units) {
    /*Convert HEX or BCD value to DEC where 0x45 == 0d45
      This was created as the lighXXX functions which determine what GPIO pin to set HIGH/LOW
      perform operations which work in base10 and not base16 (incorrect logic)
    */
    int unitsU = units % 0x10;

    if (units >= 0x50) {
        units = 50 + unitsU;
    } else if (units >= 0x40) {
        units = 40 + unitsU;
    } else if (units >= 0x30) {
        units = 30 + unitsU;
    } else if (units >= 0x20) {
        units = 20 + unitsU;
    } else if (units >= 0x10) {
        units = 10 + unitsU;
    }
    return units;
}


/*
 * decCompensation
 * This function "undoes" hexCompensation in order to write the correct base 16 value through I2C
 */
int decCompensation(int units) {
    int unitsU = units % 10;

    if (units >= 50) {
        units = 0x50 + unitsU;
    } else if (units >= 40) {
        units = 0x40 + unitsU;
    } else if (units >= 30) {
        units = 0x30 + unitsU;
    } else if (units >= 20) {
        units = 0x20 + unitsU;
    } else if (units >= 10) {
        units = 0x10 + unitsU;
    }
    return units;
}

/*
 * hourInc
 * Fetch the hour value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 23 hours in a day
 * Software Debouncing should be used
 */
void hourInc(void) {

    //Debounce

    long interruptTime = millis();

    if (interruptTime - lastInterruptTime > 200) {

        printf("Interrupt 1 triggered, %d\n", hours);

        //Fetch RTC Time

        hours = hexCompensation(wiringPiI2CReadReg8(RTC, HOUR));

        //Increase hours by 1, ensuring not to overflow

        hours++;

        hours = hFormat(hours);

        //Write hours back to the RTC

        wiringPiI2CWriteReg8(RTC, HOUR, decCompensation(hours));

    }

    lastInterruptTime = interruptTime;

}

/*
 * minInc
 * Fetch the minute value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 60 minutes in an hour
 * Software Debouncing should be used
 */
void minInc(void) {

    //Debounce

    long interruptTime = millis();

    if (interruptTime - lastInterruptTime > 200) {

        printf("Interrupt 2 triggered, %d\n", mins);

        //Fetch RTC Time

        mins = hexCompensation(wiringPiI2CReadReg8(RTC, MIN));

        //Increase minutes by 1, ensuring not to overflow

        if (mins >= 59) {

            hours++;

            mins = 0;

        } else {

            mins++;

        }
        //Write minutes back to the RTC

        wiringPiI2CWriteReg8(RTC, HOUR, hours);

        wiringPiI2CWriteReg8(RTC, MIN, decCompensation(mins));


    }

    lastInterruptTime = interruptTime;

}


//This interrupt will fetch current time from another script and write it to the clock registers
//This functions will toggle a flag that is checked in main
void toggleTime(void) {
    long interruptTime = millis();

    //if (interruptTime - lastInterruptTime>200){
    HH = getHours();
    MM = getMins();
    SS = getSecs();

    HH = hFormat(HH);
    HH = decCompensation(HH);
    wiringPiI2CWriteReg8(RTC, HOUR, HH);

    MM = decCompensation(MM);
    wiringPiI2CWriteReg8(RTC, MIN, MM);

    SS = decCompensation(SS);
    wiringPiI2CWriteReg8(RTC, SEC, 0b10000000 + SS);

    //}
    lastInterruptTime = interruptTime;
}

void cleanup(int signal) {

    printf("Keyboard interrupt caught - exiting gracefully\n");

    printf("Cleaning up LEDs\n");

    // turn off all LEDs on breadboard

    pwmWrite(SECS, 0);

    pinMode(SECS, INPUT);

    for (int k = 0; k < sizeof(LEDS) / sizeof(LEDS[0]); k++) {

        digitalWrite(LEDS[k], 0);

        pinMode(LEDS[k], INPUT);

    }

    exit(0);

}
