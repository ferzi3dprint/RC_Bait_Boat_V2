#include <SPI.h>
#include<nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <Servo.h>
                                                                                                                                                                                                             
#define driveMotorsLeftEnA 7
#define driveMotorsLeftIn1 6
#define driveMotorsLeftIn2 5
#define driveMotorsRightEnB 2
#define driveMotorsRightIn3 4
#define driveMotorsRightIn4 3
#define gearMotorIn3 A1
#define gearMotorIn4 A0
#define gearMotorEnB 9
#define microSwitchUp A2
#define microSwitchDown A4

RF24 radio(8,10);

const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime =0;
unsigned long currentTimeMicro=0;

Servo servo1;

struct Data_Package{
  byte leftJoyX;
  byte leftJoyY;                              
  byte toggleSwitch;
  };

Data_Package data;

int leftJoyXValue;
int leftJoyYValue;
int toggleSwitchValue;
int motorSpeedDriveMotorsLeft=0;
int motorSpeedDriveMotorsRight=0;
int motorSpeedGearMotor=0;
int microSwitchDownVal=0;
int microSwitchUpVal=0;
int readyToBait=0;
int gearMotorDir=0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
radio.begin();
printf_begin();
radio.openReadingPipe(0,address);
radio.setPALevel(RF24_PA_MAX);
radio.startListening();
radio.printDetails();
radio.enableDynamicPayloads();
radio.setDataRate(RF24_1MBPS);
radio.setAutoAck(false);

pinMode(driveMotorsLeftEnA, OUTPUT);
pinMode(driveMotorsLeftIn1, OUTPUT);
pinMode(driveMotorsLeftIn2, OUTPUT);
pinMode(driveMotorsRightEnB, OUTPUT);
pinMode(driveMotorsRightIn3, OUTPUT);
pinMode(driveMotorsRightIn4, OUTPUT);
pinMode(gearMotorIn3, OUTPUT);
pinMode(gearMotorIn4, OUTPUT);
pinMode(gearMotorEnB, OUTPUT);
pinMode(microSwitchUp, INPUT_PULLUP);
pinMode(microSwitchDown, INPUT_PULLUP);
digitalWrite(gearMotorIn3, LOW);
digitalWrite(gearMotorIn4, LOW);
digitalWrite(driveMotorsLeftIn1, LOW);
digitalWrite(driveMotorsLeftIn2, LOW);
digitalWrite(driveMotorsRightIn3, LOW);
digitalWrite(driveMotorsRightIn4, LOW);
analogWrite(driveMotorsLeftEnA, 0);
analogWrite(driveMotorsRightEnB, 0);
analogWrite(gearMotorEnB, 0);  


}

void loop() {

if (currentTime - lastReceiveTime > 1000){
  resetData();
  }

if (radio.available()){
  radio.read(&data, sizeof(Data_Package));
  lastReceiveTime = millis();
  }
  
leftJoyXValue = data.leftJoyX;
leftJoyYValue = data.leftJoyY;
toggleSwitchValue = data.toggleSwitch;

Serial.println(toggleSwitchValue);

//Drive
if (leftJoyXValue < 110){
   digitalWrite(driveMotorsLeftIn1, HIGH);
   digitalWrite(driveMotorsLeftIn2, LOW);
   digitalWrite(driveMotorsRightIn3, HIGH);
   digitalWrite(driveMotorsRightIn4, LOW);
   motorSpeedDriveMotorsLeft = map(leftJoyXValue, 110, 0, 0, 255);
   motorSpeedDriveMotorsRight = map(leftJoyXValue, 110, 0, 0, 255);
  }

else if (leftJoyXValue>140){
   digitalWrite(driveMotorsLeftIn1, LOW);
   digitalWrite(driveMotorsLeftIn2, HIGH);
   digitalWrite(driveMotorsRightIn3, LOW);
   digitalWrite(driveMotorsRightIn4, HIGH);
   motorSpeedDriveMotorsLeft = map(leftJoyXValue, 140, 255, 0, 255);
   motorSpeedDriveMotorsRight = map(leftJoyXValue, 140, 255, 0, 255);
  }

else {
  motorSpeedDriveMotorsLeft=0;
  motorSpeedDriveMotorsRight=0;
  }

//Steering
if (leftJoyYValue < 110) {
  int yMapped = map(leftJoyYValue, 110,0,0,255);
  motorSpeedDriveMotorsLeft=motorSpeedDriveMotorsLeft - yMapped;
  motorSpeedDriveMotorsRight=motorSpeedDriveMotorsRight + yMapped;
  
  if(motorSpeedDriveMotorsLeft<0){
    motorSpeedDriveMotorsLeft=0;
    }
  if(motorSpeedDriveMotorsRight>255){
    motorSpeedDriveMotorsRight=255;
    }
  }

  if (leftJoyYValue > 140) {
  int yMapped = map(leftJoyYValue, 140,255,0,255);
  motorSpeedDriveMotorsLeft=motorSpeedDriveMotorsLeft + yMapped;
  motorSpeedDriveMotorsRight=motorSpeedDriveMotorsRight - yMapped;
  
  if(motorSpeedDriveMotorsLeft>255){
    motorSpeedDriveMotorsLeft=255;
    }
  if(motorSpeedDriveMotorsRight<0){
    motorSpeedDriveMotorsRight=0;
    }
  }

if (motorSpeedDriveMotorsLeft < 70) {
  motorSpeedDriveMotorsLeft = 0;
  }
if (motorSpeedDriveMotorsRight < 70) {
  motorSpeedDriveMotorsRight = 0;
  }
  
analogWrite(driveMotorsLeftEnA, motorSpeedDriveMotorsLeft);
analogWrite(driveMotorsRightEnB, motorSpeedDriveMotorsRight);

/*if (toggleSwitchValue==0) { // A billenőkapcsoló alsó állásban
    motorSpeedGearMotor = 50;  
    digitalWrite(gearMotorIn3, LOW);
    digitalWrite(gearMotorIn4, HIGH);
    analogWrite(gearMotorEnB, motorSpeedGearMotor);   
  }
  
if (toggleSwitchValue==1) { // A billenőkapcsoló felső állásban van
    motorSpeedGearMotor = 50;  
    digitalWrite(gearMotorIn3, HIGH);
    digitalWrite(gearMotorIn4, LOW);
    analogWrite(gearMotorEnB, motorSpeedGearMotor);   
  }*/

  
 if (toggleSwitchValue==0) { // A billenőkapcsoló alsó állásban
    if (digitalRead(microSwitchUp) == LOW) { // Az alsó végállás kapcsoló aktív
      // Ha aktív az alsó végállás, kikapcsoljuk a motort
      motorSpeedGearMotor = 0;  
      analogWrite(gearMotorEnB, motorSpeedGearMotor);   
    } else { // Az alsó végállás kapcsoló nem aktív, de a billenőkar alsó végállásban, bekapcsoljuk a motort
      motorSpeedGearMotor = 100;  
      digitalWrite(gearMotorIn3, LOW);
      digitalWrite(gearMotorIn4, HIGH);
      analogWrite(gearMotorEnB, motorSpeedGearMotor);   
    }
  } else { // A billenőkapcsoló felső állásban van
    if (digitalRead(microSwitchDown) == LOW) { // A felső végállás kapcsoló aktív
      // Ha aktív a felső végállás, kikapcsoljuk a motort
      motorSpeedGearMotor = 0;  
      analogWrite(gearMotorEnB, motorSpeedGearMotor);   
    } else { // A felső végállás kapcsoló nem aktív, de a billenőkar felső végállásban, bekapcsoljuk a motort
      motorSpeedGearMotor = 100;  
      digitalWrite(gearMotorIn3, HIGH);
      digitalWrite(gearMotorIn4, LOW);
      analogWrite(gearMotorEnB, motorSpeedGearMotor);   
    }
  }
}

void resetData(){
  data.leftJoyX=127;
  data.leftJoyY=127;
  data.toggleSwitch=0;
  }
