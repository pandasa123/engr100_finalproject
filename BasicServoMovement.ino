#include "pt.h" // ProtoThread Library
#include "clock.h" // Library for the CLOCK_SECOND constant
#include "timer.h" // Library for the timer

static struct pt pt1, pt2;
int i = 0;

#include <Servo.h>
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>
#include <RBL_nRF8001.h>

String send_String;
int ti_send = millis();
int tf_send = ti_send;

Servo yawServo; // yaw / bottom servo 1 / 3
Servo rollServo; // roll / top servo 2 /4
int yawAngle; // bottom servo
int rollAngle; // top servo
const int temperaturePin = 0;
int PC1_Pin = 0; // Photocell 1
int PC2_Pin = 1; // Photocell 2
int PC3_Pin = 2; // Photocell 3
int PC4_Pin = 3; // Photocell 4
int Temp_Pin = 4; // Temperature Sensor

int maxYawReading;
int maxRollReading;

int PC1_Reading = analogRead(PC1_Pin);
int PC2_Reading = analogRead(PC2_Pin);
int PC3_Reading = analogRead(PC3_Pin);
int PC4_Reading = analogRead(PC4_Pin);

float voltage;
int tempC;
double irradiance;
double average;

struct locationStruct {
  int yawAngle; // bottom servo
  int rollAngle; // top servo
};

locationStruct location;

void setup() {
  char* name = "Periwinkle";
  ble_set_name(name);
  ble_begin();
  Serial.begin(9600);
  yawServo.attach(5);
  rollServo.attach(3);
  PT_INIT(&pt1);
  PT_INIT(&pt2);
}

static int protothread1(struct pt *pt) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1){
    timestamp = millis();
    PT_WAIT_UNTIL(pt, millis() - timestamp > 100);
      // I'm assuming that Photocell 1 is opposite to Photocell 3. And Photocell 2 is opposite to Photocell 4
    locationStruct location;

    // Finding the yaw angle
    if((PC1_Reading < PC3_Reading) && (abs(PC1_Reading - PC3_Reading) > 10)){
      while((PC3_Reading > maxYawReading) && (abs(PC1_Reading - PC3_Reading) > 40)){
        maxYawReading = PC3_Reading;
        int angle = yawServo.read();
        if(angle > 21){
          yawServo.write(angle - 1);
        }
        delay(15);
      }
      location.yawAngle = yawServo.read();
    }
    else if((PC1_Reading > PC3_Reading) && (abs(PC1_Reading - PC3_Reading) > 10)){
      while(PC1_Reading > maxYawReading && (abs(PC1_Reading - PC3_Reading) > 40)){
        maxYawReading = PC1_Reading;
        int angle = yawServo.read();
        if(angle < 159){
          yawServo.write(angle + 1);
        }
        delay(15);
      }
      location.yawAngle = yawServo.read();
    }
    else{
      location.yawAngle = yawServo.read();
    }
  
    // Finding the roll angle
    if((PC2_Reading < PC4_Reading) && (abs(PC2_Reading - PC4_Reading) > 10)){
      while(PC2_Reading > maxRollReading && (abs(PC2_Reading - PC4_Reading) > 40)){
        maxRollReading = PC2_Reading;
        int angle = rollServo.read();
        rollServo.write(angle - 1);
        delay(15);
      }
      location.rollAngle = rollServo.read();
    }
    else if((PC2_Reading > PC4_Reading) && (abs(PC2_Reading - PC4_Reading) > 10)){
      while(PC4_Reading > maxRollReading && (abs(PC2_Reading - PC4_Reading) > 40)){
        maxRollReading = PC4_Reading;
        int angle = rollServo.read();
        rollServo.write(angle + 1);
        delay(15);
      }
      location.rollAngle = rollServo.read();
    }
    else{
      location.rollAngle = rollServo.read();
    }
      
    PT_WAIT_UNTIL(pt, millis() - timestamp > 100);
    
    voltage = (analogRead(temperaturePin)/205);
    tempC = ((100*voltage*0.001 - 50));
    
    timestamp = millis();
    average = (analogRead(PC1_Pin) + analogRead(PC2_Pin) + analogRead(PC3_Pin) + analogRead(PC4_Pin))/4;
    irradiance = (0.124*average + 9.697);
    
    send_String = "Roll: " + String(location.rollAngle) + " deg\nYaw: " + String(location.yawAngle) + " deg\nTemp: " + String(tempC) + " deg C\nIrradiance " + String(irradiance) + " W/m^2";
    send_data(); 
    PT_WAIT_UNTIL(pt, millis() - timestamp > 1000);
  }
  PT_END(pt);
}

static int protothread2(struct pt *pt) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while(1){
    timestamp = millis();
    PT_WAIT_UNTIL(pt, millis() - timestamp > 100);
    ble_do_events();
  }
  PT_END(pt);
}

void loop() {
  protothread1(&pt1);
  protothread2(&pt2);
}

void send_data(){
  tf_send = millis();
  if(tf_send - ti_send < 1000){
    return;
  }
  if ( ble_connected() )
  {
    int i = 0;
    while(send_String[i]){
      ble_write(send_String[i]);
      i++;
    }
  }
  ti_send = millis();
  tf_send = ti_send;
}

