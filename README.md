# sdp2024team16 - Evaluation of the Inertial Measurement Units (IMUs) we Wanted to Use

Our team has run an evaluation on all the available IMU models at both makerspaces to determine which ones would be the best for our purposes.

## BMI160 Shuttleboard
The Bosch BMI160 shuttleboard is a PCB that combines a 6 degrees of freedom (6DoF) IMU with a BMM150 magnetometer. It is a breakout board that lets us interface with all three types of sensors (accelerometer, gyroscope, and magnetometer) easily. 

### Advantages
- Readily available at both M5 and UMass All-Campus Makerspace.
- Both the BMI160 and BMM150 are low power and low noise.
### Diadvantages
- Lack of driver support for shuttleboard with Arduino, which doesn't facilitate prototyping as well as other IMUs.
- Form factor of the shuttleboard is bulkier than 9DoF accelerometers we are also considering.

## BNO055 IMU
The BNO055 is the IMU that was recommended by Shira on Piazza for our purposes.
