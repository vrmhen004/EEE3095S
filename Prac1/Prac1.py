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

prevState = False

# Logic that you write
def main():
	print("write")
	GPIO.setmode(GPIO.BOARD)
	GPIO.setwarnings(False)
	channel = 7
	GPIO.setup(channel, GPIO.OUT)
	if prevState:
		GPIO.output(channel, GPIO.HIGH)
		prevState = True
	else:
		GPIO.output(channel, GPIO.LOW)
		prevState = False
	print("done")

# Only run the functions if 
if __name__ == "__main__":
	# Make sure the GPIO is stopped correctly
	try:
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
