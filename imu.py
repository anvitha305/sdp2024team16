import serial
import time

arduino = serial.Serial(port='COM3', baudrate=9600, timeout=.1)

def read_imu():
	orientation = arduino.readline()
	angvel = arduino.readline()
	linaccel = arduino.readline()
	mag = arduino.readline()
	acc = arduino.readline()
	return orientation, angvel, linaccel, mag, acc
while True: 
	for i in read_imu():
		print(i)