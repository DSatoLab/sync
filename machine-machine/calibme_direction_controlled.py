#usr/bin/env/python
import RPi.GPIO as GPIO
import time
import numpy as np
import smbus #FOR the I2C bus

LedPin = 12
sampling_rate = 0.01; #sampling rate in seconds
			 #this value indicates how often we will 
			 #evaluate our function in real time

def setup():
	#Set up the I2C bus for the LUX
	# Get I2C bus
	global bus;
	bus = smbus.SMBus(1)
	# TSL2561 address, 0x39(57)
	# Select control register, 0x00(00) with command register, 0x80(128)
	#		0x03(03)	Power ON mode
	bus.write_byte_data(0x39, 0x00 | 0x80, 0x03)
	# TSL2561 address, 0x39(57)
	# Select timing register, 0x01(01) with command register, 0x80(128)
	#		0x02(02)	Nominal integration time = 402ms
	bus.write_byte_data(0x39, 0x01 | 0x80, 0x02)
	time.sleep(0.5);
	#Set up the GPIO modes for PWM
	global p
	global pm
	GPIO.setmode(GPIO.BOARD)	   # Numbers GPIOs by physical location
	GPIO.setup(LedPin, GPIO.OUT)   # Set LedPin's mode is output
	GPIO.output(LedPin, GPIO.LOW)  # Set LedPin to low(0V)

	# for the motor
	GPIO.setup(MotorPin1, GPIO.OUT)   # mode --- output
	GPIO.setup(MotorPin2, GPIO.OUT)
	GPIO.setup(MotorEnable, GPIO.OUT)
	GPIO.output(MotorEnable, GPIO.LOW) # motor stop

#This function calculates the dutyCycle based on the 
#function value
def calcDutyCycle(fval, fmin, fmax):
	dc_0_1 = (fval - fmin)/(fmax-fmin);#this part maps the value of the function
					   #to a value between 0 and 1

	#dc_0_1 = dc_0_1*dc_0_1; #instead of a straight line between 0 and 1
				#we can also try a quadratic line between 0 and 1
				#try changing it to a cubic or a square root and see what feels better
	if dc_0_1 > 1:
		dc_0_1 = 1; 
	if dc_0_1 < 0:
		dc_0_1 = 0; 
	
	return dc_0_1*100;

#Read from the Light sensor
def readExternal():
	# Read data back from 0x0C(12) with command register, 0x80(128), 2 bytes
	# ch0 LSB, ch0 MSB
	data = bus.read_i2c_block_data(0x39, 0x0C | 0x80, 2)
	# Read data back from 0x0E(14) with command register, 0x80(128), 2 bytes
	# ch1 LSB, ch1 MSB
	data1 = bus.read_i2c_block_data(0x39, 0x0E | 0x80, 2)
	# Convert the data
	ch0 = data[1] * 256 + data[0]
	ch1 = data1[1] * 256 + data1[0]
	vis_val = (ch0 - ch1); 
	return vis_val;  
	
	
#Nagumo program
def fitzhughNagumo():
	#time step
	tstep = 0.1;
	#initial values of v and w
	v = 2.0; 
	w = 0.0; 
	I_ext = 0; 
	#parameters
	a = 1; b = 0.0; c = 0.8;
	eps = 10*1.0;
	#time scale used to shift the model
	t_sc = 11; 
	#end of parameters 



	fmin =0; fmax = 2; 

	I_ext_old=0;

	stim=0;
	while(True):
		#calculate how long it takes to do the computation
		#and substract that time from the sleep time 
		#in case the computation takes a long time
		t_start = time.time();
		#calculate the rate of change
		I_ext = readExternal();
		de=I_ext-I_ext_old
		stim=stim+de/30.0-stim/10.0
		I_ext_old = I_ext
		for lp in np.arange(0, 10, 0.5):
			dv = t_sc*(v - v**3/3 - w + stim)
			dw = t_sc*(1.0/eps*(a*v+b-c*w));
			#calculate the value of v based on the rate of change
			v = v+tstep*dv/10.0; 
			w = w+tstep*dw/10.0;

		#now calculate the dc associated with the current v
		dc = calcDutyCycle(v, fmin, fmax);
		t_end = time.time();

		#change the duty cycle
		p.ChangeDutyCycle(dc);

		# NEW: outputting to the motor
		if v > 0:
			GPIO.output(MotorEnable, GPIO.HIGH) # motor driver enable
			GPIO.output(MotorPin1, GPIO.LOW)   # clockwise
			GPIO.output(MotorPin2, GPIO.LOW)
		if v == 0:
			GPIO.output(MotorEnable, GPIO.LOW)  # motor stop
		if v < 0:
			GPIO.output(MotorEnable, GPIO.HIGH) # motor driver enable
			GPIO.output(MotorPin1, GPIO.LOW)    # anticlockwise
			GPIO.output(MotorPin2, GPIO.HIGH)
			 

		print (v, I_ext)
		#print v, w, dc, I_ext, de, stim, (v - v**3/3 - w + 0)
		#print v, stim
		#sleep for sampling time
		time.sleep(tstep-(t_end-t_start)); 
		

		
#Calibration code
def calibrate():
	global s_val
	s_val = readExternal();
	print s_val

def destroy():
	p.stop()
	GPIO.output(LedPin, GPIO.HIGH)	  # turn off all leds

	# for motor
	GPIO.output(MotorEnable, GPIO.LOW) # motor stop
	GPIO.cleanup() # release resource

if __name__ == '__main__':	   # Program start from here
	setup()
	try:
		#loop()
		#fitzhughNagumo();
		#	calibrate();
		#	time.sleep(2); 
			fitzhughNagumo();
	except KeyboardInterrupt:  # When 'Ctrl+C' is pressed, the child program destroy() will be	executed.
		destroy()
