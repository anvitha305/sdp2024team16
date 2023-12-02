import serial
import time

arduino = serial.Serial(port='COM3', baudrate=9600, timeout=.1)

def read_imu():
	acc = arduino.readline()
	mag = arduino.readline()
	gyro = arduino.readline()
	return orientation, acc, mag, gyro;
while True: 
	for i in read_imu():
		print(i)
