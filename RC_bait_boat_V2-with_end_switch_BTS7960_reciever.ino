#include <SPI.h>
#include<nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <Servo.h>
                                                                                                                                                                                                             
#define driveMotorLeftLPWM 5
#define driveMotorLeftRPWM 3
#define driveMotorLeftREN 2
#define driveMotorLeftLEN 4

#define driveMotorRightRPWM 9
#define driveMotorRightLPWM 6
#define driveMotorRightREN A5
#define driveMotorRightLEN A3

#define gearMotorIn3 A1
#define gearMotorIn4 A0
#define gearMotorEnB 7
#define microSwitchUp A2
#define microSwitchDown A4

RF24 radio(8,10);

const byte address[6] = "00001";
unsigned long lastReceiveTime = 0;
unsigned long currentTime =0;
unsigned long currentTimeMicro=0;

struct Data_Package{
  byte leftJoyX;
  byte leftJoyY;                              
  byte toggleSwitch;
  };

Data_Package data;

int leftJoyXValue;
int leftJoyYValue;
int toggleSwitchValue;
int motorSpeedDriveMotorLeft=0;
int motorSpeedDriveMotorRight=0;
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


pinMode(driveMotorLeftLPWM, OUTPUT);
pinMode(driveMotorLeftRPWM, OUTPUT);
pinMode(driveMotorLeftREN, OUTPUT);
pinMode(driveMotorLeftLEN, OUTPUT);

pinMode(gearMotorIn3, OUTPUT);
pinMode(gearMotorIn4, OUTPUT);
pinMode(gearMotorEnB, OUTPUT);
pinMode(microSwitchUp, INPUT_PULLUP);
pinMode(microSwitchDown, INPUT_PULLUP);

digitalWrite(driveMotorLeftREN, HIGH);
digitalWrite(driveMotorLeftLEN, HIGH);
digitalWrite(driveMotorRightREN, HIGH);
digitalWrite(driveMotorRightLEN, HIGH);
digitalWrite(gearMotorIn3, LOW);
digitalWrite(gearMotorIn4, LOW);
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
   motorSpeedDriveMotorLeft = map(leftJoyXValue, 110, 0, 0, 255);
   motorSpeedDriveMotorRight = map(leftJoyXValue, 110, 0, 0, 255);
   analogWrite(driveMotorLeftLPWM, motorSpeedDriveMotorLeft);
   analogWrite(driveMotorLeftRPWM, 0); 
   analogWrite(driveMotorRightLPWM, motorSpeedDriveMotorRight);
   analogWrite(driveMotorRightRPWM, 0); 
  }

else if (leftJoyXValue>140){
   motorSpeedDriveMotorLeft = map(leftJoyXValue, 140, 255, 0, 255);
   motorSpeedDriveMotorRight = map(leftJoyXValue, 140, 255, 0, 255);
   analogWrite(driveMotorLeftLPWM, 0);
   analogWrite(driveMotorLeftRPWM, motorSpeedDriveMotorLeft);
   analogWrite(driveMotorRightLPWM, 0);
   analogWrite(driveMotorRightRPWM, motorSpeedDriveMotorRight);
  }

else {
  motorSpeedDriveMotorLeft=0;
  motorSpeedDriveMotorRight=0;
  analogWrite(driveMotorLeftLPWM, 0);
  analogWrite(driveMotorLeftRPWM, 0);
  analogWrite(driveMotorRightLPWM, 0);
  analogWrite(driveMotorRightRPWM, 0);

  }


//Steering
if (leftJoyYValue < 110) {
  int yMapped = map(leftJoyYValue, 110,0,0,255);
  motorSpeedDriveMotorLeft=motorSpeedDriveMotorLeft + yMapped;
  motorSpeedDriveMotorRight=motorSpeedDriveMotorRight - yMapped;
  analogWrite(driveMotorLeftLPWM, motorSpeedDriveMotorLeft);
  analogWrite(driveMotorLeftRPWM, 0);
  analogWrite(driveMotorRightLPWM, motorSpeedDriveMotorRight);
  analogWrite(driveMotorRightRPWM, 0);   
  }

  if (leftJoyYValue > 140) {
    
  int yMapped = map(leftJoyYValue, 140,255,0,255);
  motorSpeedDriveMotorLeft=motorSpeedDriveMotorLeft - yMapped;
  motorSpeedDriveMotorRight=motorSpeedDriveMotorRight + yMapped;
  analogWrite(driveMotorLeftLPWM, motorSpeedDriveMotorLeft);
  analogWrite(driveMotorLeftRPWM, 0);
  analogWrite(driveMotorRightLPWM, motorSpeedDriveMotorRight);
  analogWrite(driveMotorRightRPWM, 0); 
  }

/*if (motorSpeedDriveMotorLeft < 70) {
  motorSpeedDriveMotorLeft = 0;
  analogWrite(driveMotorLeftLPWM, 0);
  analogWrite(driveMotorLeftRPWM, 0);    
  }
  
if (motorSpeedDriveMotorRight < 70) {
  motorSpeedDriveMotorRight = 0;
  analogWrite(driveMotorRightLPWM, 0);
  analogWrite(driveMotorRightRPWM, 0);
  }*/
  
 if (toggleSwitchValue==0) { // A billenőkapcsoló alsó állásban
    if (digitalRead(microSwitchUp) == LOW) { // Az alsó végállás kapcsoló aktív
      // Ha aktív az alsó végállás, kikapcsoljuk a motort
      motorSpeedGearMotor = 0;  
      analogWrite(gearMotorEnB, motorSpeedGearMotor);   
    } else { // Az alsó végállás kapcsoló nem aktív, de a billenőkar alsó végállásban, bekapcsoljuk a motort
      motorSpeedGearMotor = 255;  
      digitalWrite(gearMotorIn3, LOW);
      digitalWrite(gearMotorIn4, HIGH);
      digitalWrite(gearMotorEnB, HIGH);   
    }
  } else { // A billenőkapcsoló felső állásban van
    if (digitalRead(microSwitchDown) == LOW) { // A felső végállás kapcsoló aktív
      // Ha aktív a felső végállás, kikapcsoljuk a motort
      motorSpeedGearMotor = 0;  
      analogWrite(gearMotorEnB, motorSpeedGearMotor);   
    } else { // A felső végállás kapcsoló nem aktív, de a billenőkar felső végállásban, bekapcsoljuk a motort
      motorSpeedGearMotor = 255;  
      digitalWrite(gearMotorIn3, HIGH);
      digitalWrite(gearMotorIn4, LOW);
      digitalWrite(gearMotorEnB, HIGH);   
    }
  }
}

void resetData(){
  data.leftJoyX=127;
  data.leftJoyY=127;
  data.toggleSwitch=0;
  }
