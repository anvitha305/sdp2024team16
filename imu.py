import serial
import time

arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1)

def read_imu():
	orientation = arduino.readline()
	angvel = arduino.readline()
	linaccel = arduino.readline()
	mag = arduino.readline()
	acc = arduino.readline()
	return orientation, angvel, linaccel, mag, acc
#while True: 
def get_data():
	results = dict()
	for i in read_imu():
		if i is not bytes(): 
			try:
				res = i.strip().decode()
				crds = list(map(lambda x: float(x.strip()[3:]), res[5:].split('|')))
				print(crds)
				if len(crds)!= 3:
					raise ValueError()
				results[res[0:3]] = crds
				print(results)
			except ValueError:
				pass
	if len(results)==5:
		yield results