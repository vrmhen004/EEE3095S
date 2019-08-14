#ifndef BINCLOCK_H
#define BINCLOCK_H

//Some reading (if you want)
//https://stackoverflow.com/questions/1674032/static-const-vs-define-vs-enum

// Function definitions
int hFormat(int hours);
void lightHours(int units);
void lightMins(int units);
int hexCompensation(int units);
int decCompensation(int units);
void initGPIO(void);
void secPWM(int units);
void hourInc(void);
void minInc(void);
void toggleTime(void);

// define constants
const char RTCAddr = 0x6f;
const char SEC = 0x00; // see register table in datasheet
const char MIN = 0x01;
const char HOUR = 0x02;
const char TIMEZONE = 2; // +02H00 (RSA)

// define pins
// Hours: 16, 18, 22, 36
// Min: 7, 11, 13, 15, 29, 31
// Sec: 12
// BTN: 33, 37
const int LEDS[] = {16, 18, 22, 36, 7, 11, 13, 15, 29, 31}; //H0-H4, M0-M5
const int SECS = 12;
const int BTNS[] = {33, 37}; // B0, B1


#endif
