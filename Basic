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
struct locationStruct {
  int yawAngle; // bottom servo
  int rollAngle; // top servo
};
void setup() {
  char* name = "Periwinkle";
  ble_set_name(name);
  ble_begin();
  Serial.begin(9600);
  yawServo.attach(5);
  rollServo.attach(3);
}
void loop() {
  locationStruct sunLocation = trackSun();
  ble_do_events();
  int tempC = tempReadC();
  double irradiance = calc_irradiance();
  send_String = "Roll: " + String(sunLocation.rollAngle) + " deg\nYaw: " + String(sunLocation.yawAngle) + " deg\nTemp: " + String(tempC) + " deg C\nIrradiance " + String(irradiance) + " W/m^2";
  send_data();
  Serial.println(analogRead(PC1_Pin));
  Serial.println(analogRead(PC2_Pin));
  Serial.println(analogRead(PC3_Pin));
  Serial.println(analogRead(PC4_Pin));
  Serial.println(yawServo.read());
  Serial.println(rollServo.read());
  ble_do_events();

  Serial.print("\n");
  // delay(60000);
}
float getVoltage(int pin) {
  ble_do_events();
  return (analogRead(pin)/205);
}
double tempReadC() {
  ble_do_events();
  float voltage, degreesC, degreesF;
  voltage = getVoltage(temperaturePin);
  Serial.print("\n");
  return ((100*voltage*0.001 - 50));
  // return ((voltage - 0.5) * 100);
}
locationStruct trackSun() {
  ble_do_events();
  // I'm assuming that Photocell 1 is opposite to Photocell 3. And Photocell 2 is opposite to Photocell 4
  locationStruct location;

  // Finding the yaw angle
  int maxYawReading = -1;
  int PC1_Reading = analogRead(PC1_Pin);
  int PC3_Reading = analogRead(PC3_Pin);
  ble_do_events();
  if((PC1_Reading < PC3_Reading) && (abs(PC1_Reading - PC3_Reading) > 10)){
    while((PC3_Reading > maxYawReading) && (abs(PC1_Reading - PC3_Reading) > 40)){
      maxYawReading = PC3_Reading;
      ble_do_events();
      int angle = yawServo.read();
      if(angle > 21){
        ble_do_events();
        yawServo.write(angle - 1);
      }
      delay(15);
    }
    location.yawAngle = yawServo.read();
  }
  else if((PC1_Reading > PC3_Reading) && (abs(PC1_Reading - PC3_Reading) > 10)){
    while(PC1_Reading > maxYawReading && (abs(PC1_Reading - PC3_Reading) > 40)){
      ble_do_events();
      maxYawReading = PC1_Reading;
      int angle = yawServo.read();
      if(angle < 159){
        ble_do_events();
        yawServo.write(angle + 1);
      }
      delay(15);
    }
    ble_do_events();
    location.yawAngle = yawServo.read();
  }
  else{
    ble_do_events();
    location.yawAngle = yawServo.read();
  }

  // Finding the roll angle
  int maxRollReading = -1;
  int PC2_Reading = analogRead(PC2_Pin);
  int PC4_Reading = analogRead(PC4_Pin);
  if((PC2_Reading < PC4_Reading) && (abs(PC2_Reading - PC4_Reading) > 10)){
    while(PC2_Reading > maxRollReading && (abs(PC2_Reading - PC4_Reading) > 40)){
      ble_do_events();
      maxRollReading = PC2_Reading;
      int angle = rollServo.read();
      rollServo.write(angle - 1);
      delay(15);
    }
    location.rollAngle = rollServo.read();
  }
  else if((PC2_Reading > PC4_Reading) && (abs(PC2_Reading - PC4_Reading) > 10)){
    while(PC4_Reading > maxRollReading && (abs(PC2_Reading - PC4_Reading) > 40)){
      ble_do_events();
      maxRollReading = PC4_Reading;
      int angle = rollServo.read();
      rollServo.write(angle + 1);
      delay(15);
    }
    location.rollAngle = rollServo.read();
  }
  else{
    ble_do_events();
    location.rollAngle = rollServo.read();
  }
  return location;
}

double calc_irradiance(){
  double average = (analogRead(PC1_Pin) + analogRead(PC2_Pin) + analogRead(PC3_Pin) + analogRead(PC4_Pin))/4;
  return (0.124*average + 9.697);
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
