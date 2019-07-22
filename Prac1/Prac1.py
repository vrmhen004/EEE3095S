#!/usr/bin/python3
"""
Prac1
Hendri Vermeulen
Readjust this Docstring as follows:
Names: Hendri
Student Number: VRMHEN004
Prac: 1
Date: 22/07/19
"""

# import Relevant Librares
import RPi.GPIO as GPIO
import time

binary = [0,0,0]
index = 0

# Logic that you write
def main():
	global binary
	global index	
	
	temp = index
	binary[0] = 0
	binary[1] = 0 
	binary[2] = 0
	count = 2
	while(temp > 0):
		result = temp%2
		temp = int(temp/2)
		if result == 1:
			binary[count] = 1
		else:
			binary[count] = 0
		count = count - 1
	
	
	if binary[0] == 1:
		GPIO.output(7, GPIO.HIGH)
	else:
		GPIO.output(7, GPIO.LOW)
	if binary[1] == 1:
		GPIO.output(11, GPIO.HIGH)
	else:
		GPIO.output(11, GPIO.LOW)
	if binary[2] == 1:
		GPIO.output(13, GPIO.HIGH)
	else:
		GPIO.output(13, GPIO.LOW)
		
	#time.sleep(1);
	
	#index += 1;
	
	#if GPIO.event_detected(15):
	#	index += 1
	#if GPIO.event_detected(15):
	#	index -= 1
		
	if index > 7:
		index = 0
	if index < 0:
		index = 7
		
def my_callback(channel):
	global index
	if channel == 15:
		index += 1
	if channel == 16:
		index -= 1


# Only run the functions if 
if __name__ == "__main__":
	# Make sure the GPIO is stopped correctly
	try:
		GPIO.setmode(GPIO.BOARD)
		GPIO.setwarnings(False)
		
		channel = [7,11,13]
		GPIO.setup(channel, GPIO.OUT)
		GPIO.output(channel, GPIO.LOW)
		GPIO.setup([15,16], GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
		GPIO.add_event_detect(15, GPIO.RISING, callback=my_callback, bouncetime=200)
		GPIO.add_event_detect(16, GPIO.RISING, callback=my_callback, bouncetime=200)
			
		while True:
			main()
	except KeyboardInterrupt:
		print("Exiting gracefully")
		# Turn off your GPIOs here
		GPIO.cleanup()
	except e:
		GPIO.cleanup()
		print("Some other error occurred")
		print(e.message)
