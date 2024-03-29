#include <Wire.h>
#include <Servo.h>

Servo leftMotor;
Servo rightMotor;

int16_t Acc_rawX, Acc_rawY, Acc_rawZ,Gyr_rawX, Gyr_rawY, Gyr_rawZ; 

float accAngle;
float gyroAngle;
float totalAngle;

float error, previousError;

// PID constants 
double kp = 5.00;
double ki = 0.01;
double kd = 0.50;

float pid_p = 0.00;
float pid_i = 0.00;
float pid_d = 0.00;

float pid_total;

int left_speed, right_speed;

float elapsedTime, currTime, prevTime;
float rad_to_deg = 180/3.141592654;

float desiredAngle = 12.00;

void setup() {
  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  Serial.begin(9600);
  leftMotor.attach(12);
  rightMotor.attach(13);
}

void loop() {
  prevTime = currTime;
  currTime = millis();
  elapsedTime = (currTime - prevTime) / 1000; 
  Wire.beginTransmission(0x68);
  Wire.write(0x3B); //Ask for the 0x3B register- correspond to AcX
  Wire.endTransmission(false);
  Wire.requestFrom(0x68,6,true); 
    
  Acc_rawX=Wire.read()<<8|Wire.read(); //each value needs two registres
  Acc_rawY=Wire.read()<<8|Wire.read();
  Acc_rawZ=Wire.read()<<8|Wire.read();

  accAngle = atan((Acc_rawX/16384.0)/sqrt(pow((Acc_rawY/16384.0),2) + pow((Acc_rawZ/16384.0),2)))*rad_to_deg;
    
  Wire.beginTransmission(0x68);
  Wire.write(0x43); //Gyro data first adress
  Wire.endTransmission(false);
  Wire.requestFrom(0x68,4,true); //Just 4 registers
 
  Gyr_rawX=Wire.read()<<8|Wire.read();
  gyroAngle = Gyr_rawX/131.0; 
  totalAngle = 0.98 *(totalAngle + gyroAngle*elapsedTime) + 0.02*accAngle;
  Serial.println(totalAngle);
  error = totalAngle - desiredAngle;
  //Serial.println(error);
  
  pid_p = kp*error;
  pid_i = pid_i + (ki*error);
  pid_d = kd*((error - previousError)/elapsedTime);

  pid_total = pid_p + pid_i + pid_d;
  //Serial.println(pid_total);

  left_speed = constrain(90 - pid_total, 0, 180);
  right_speed = constrain(90 + pid_total, 0, 180);

  leftMotor.write(left_speed);
  rightMotor.write(right_speed);
  previousError = error;
}
