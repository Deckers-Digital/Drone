//libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <BlynkSimpleEsp32.h>
#include <TaskScheduler.h>
#include <TaskSchedulerDeclarations.h>
#include <TaskSchedulerSleepMethods.h>
#include <Wire.h>

Scheduler userScheduler; // to control your personal task

//Drone number
String DroneId = "Master Drone";
int DroneIdNumber = 0;


//ultrasonic sensor
#define TriggerPin 15
#define EchoPin 4

long duration;
int distance;

int StoppingDistance = 15;


// Line tracking
#define RightSensorPin 12
#define LeftSensorPin 14
#define midSensorPin 13
#define RightMotorOut 27
#define LeftMotorOut 33
#define RightMotorOut2 26
#define LeftMotorOut2 25

//RGB Led pins
#define LED1R //Not connected
#define LED1G 22
#define LED1B //Not conencted

#define LED2R 1.0
#define LED2G 1.1
#define LED2B 1.2

#define LED3R 32
#define LED3G 16
#define LED3B 17

#define LED4R 21
#define LED4G 18
#define LED4B 5

#define LED5R 1.5
#define LED5G 1.6
#define LED5B 1.7

#define LED6R 0.2
#define LED6G 0.1
#define LED6B 0.0


//Set line color (White or Black)
String LineColor = "Black";
int LineColorInt;

//horizontal line
int WaitTime = 20; //seconds
int FullWaitTime;

//Settings
String DroneMode = "Line";
int DroneModeInt = 0;

//bools
bool LineLost = false;
bool PathObstructed = false;

//Timers
unsigned long StartMillis;
unsigned long CurrentMillis;
const unsigned long LostTime = 5;
unsigned long FullLostTime;


unsigned long PauseStartMillis;
unsigned long PauseCurrentMillis;
const unsigned long PauseTime = 5;
unsigned long FullPauseTime;


//axis
int y;
int x;


//Switches

int MainSwitch = 0;
int SkipButton = 0;

//Amount of stops
int AmountOfStops = 3;
int LineNumber = 0;
int CourseNumber = 0;


#define BLYNK_PRINT Serial

WidgetTerminal terminal(V10);

//declarations for voids
void sendMessage();

//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND * 2 , TASK_FOREVER, &sendMessage);

WiFiClient DroneClient;

//==============================================================================================

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "vrOk8oNfsJ1EoJSt2uuO5iVf68YzDtZt";

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "LAPTOP-Vince";
const char* pass = "2#n0193F";
const char* mqttServer = "192.168.137.38";
const int mqttPort = 1883;
const char* mqttUser = "vincent";
const char* mqttPassword = "raspberry";
const char* clientID = "Drone_Client"; // MQTT client ID*/

PubSubClient client(DroneClient);

//==============================================================================================

void setup() {

  Serial.begin(115200);

  //Sensor Inputs
  pinMode(RightSensorPin, INPUT_PULLUP);
  pinMode(LeftSensorPin, INPUT_PULLUP);

  //Motor Outputs
  pinMode(RightMotorOut, OUTPUT);
  pinMode(LeftMotorOut, OUTPUT);
  pinMode(RightMotorOut2, OUTPUT);
  pinMode(LeftMotorOut2, OUTPUT);

  //ultrasonic sensor I/O
  pinMode(TriggerPin, OUTPUT);
  pinMode(EchoPin, INPUT);

  //Led Outputs
  pinMode(LED1R, OUTPUT);
  pinMode(LED1G, OUTPUT);
  pinMode(LED1B, OUTPUT);
  pinMode(LED2R, OUTPUT);
  pinMode(LED2G, OUTPUT);
  pinMode(LED2B, OUTPUT);

  //Pull all the output pins on low

  digitalWrite (RightMotorOut, LOW);
  digitalWrite(LeftMotorOut, LOW);
  digitalWrite (RightMotorOut2, LOW);
  digitalWrite(LeftMotorOut2, LOW);

  digitalWrite(LED1R, LOW);
  digitalWrite(LED1G, LOW);
  digitalWrite(LED1B, LOW);
  digitalWrite(LED2R, LOW);
  digitalWrite(LED2G, LOW);
  digitalWrite(LED2B, LOW);

  FullLostTime = LostTime * 1000;

  FullPauseTime = PauseTime * 1000;

  StartMillis = millis();

  x = 512;
  y = 512;

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  client.setServer(mqttServer, mqttPort);

  //set up wifi connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  Blynk.begin(auth, ssid, pass, "server.wyns.it", 8081);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  digitalWrite(LED1R, HIGH);
  digitalWrite(LED2R, HIGH);
  digitalWrite(LED1G, LOW);
  digitalWrite(LED2G, LOW);
  digitalWrite(LED1B, LOW);
  digitalWrite(LED2B, LOW);
  delay(1000);
  digitalWrite(LED1R, LOW);
  digitalWrite(LED2R, LOW);
  digitalWrite(LED1G, HIGH);
  digitalWrite(LED2G, HIGH);
  digitalWrite(LED1B, LOW);
  digitalWrite(LED2B, LOW);
  delay(1000);
  digitalWrite(LED1R, LOW);
  digitalWrite(LED2R, LOW);
  digitalWrite(LED1G, LOW);
  digitalWrite(LED2G, LOW);
  digitalWrite(LED1B, HIGH);
  digitalWrite(LED2B, HIGH);
  delay(1000);
  digitalWrite(LED1R, LOW);
  digitalWrite(LED2R, LOW);
  digitalWrite(LED1G, LOW);
  digitalWrite(LED2G, LOW);
  digitalWrite(LED1B, LOW);
  digitalWrite(LED2B, LOW);

  terminal.clear();
  terminal.println("Drone: " + DroneId + " With id: " + DroneIdNumber + " Is ready for action!");

}

//===========================================================================================================

BLYNK_WRITE(V0) {
  MainSwitch = param.asInt();
}

BLYNK_WRITE(V5) {

  switch (param.asInt())
  {
    case 1: { // Item 1
        Serial.println("Set to black line");
        LineColor = "Black";
        break;
      }
    case 2: { // Item 2
        Serial.println("Set to white line");
        LineColor = "White";
        break;
      }
  }
}

BLYNK_WRITE(V1) {

  switch (param.asInt())
  {
    case 1: { // Item 1
        Serial.println("Set Drone mode to Line");
        DroneMode = "Line";
        break;
      }
    case 2: { // Item 2
        Serial.println("Set Drone mode to Manual");
        DroneMode = "Manual";
        break;
      }
  }
}

BLYNK_WRITE(V20) {
  x = param[0].asInt();
  y = param[1].asInt();

  // Do something with x and y
  Serial.print("X = ");
  Serial.print(x);
  Serial.print("; Y = ");
  Serial.println(y);
}


void loop() {

  Blynk.run();

  FullWaitTime = WaitTime * 1000;

  if (LineColor == "White") {
    LineColorInt = 1;
  } else if (LineColor == "Black") {
    LineColorInt = 0;
  } else {
    while (LineColor == NULL) {
      Serial.println("Please set line color");
    }
  }

  if (MainSwitch == 1) {


    if (DroneMode == "Line") {


      if (digitalRead(midSensorPin) == LineColorInt) {
        CurrentMillis = millis();

        if (CurrentMillis - StartMillis >= FullLostTime) {
          terminal.println("Drone: " + DroneId + " With id: " + DroneIdNumber + " LOST HIS LINE!!!");
          while (digitalRead(midSensorPin) != LineColorInt) {
            Serial.println("LINE LOST!!! LINE LOST!!! LINE LOST!!!");
            digitalWrite(LeftMotorOut, LOW);
            digitalWrite(RightMotorOut, LOW);
            digitalWrite(LED1R, HIGH);
            digitalWrite(LED1G, LOW);
            digitalWrite(LED1B, LOW);
            digitalWrite(LED2R, HIGH);
            digitalWrite(LED2G, LOW);
            digitalWrite(LED2B, LOW);
          }
          StartMillis = CurrentMillis;
        }
      }


      if (distance <= StoppingDistance) {
        digitalWrite(LeftMotorOut, LOW);
        digitalWrite(RightMotorOut, LOW);
        StartMillis = CurrentMillis;
        digitalWrite(LED1R, HIGH);
        digitalWrite(LED1G, HIGH);
        digitalWrite(LED1B, LOW);
        digitalWrite(LED2R, HIGH);
        digitalWrite(LED2G, HIGH);
        digitalWrite(LED2B, LOW);
        Serial.println(" Stop");
      }


      UltrasonicSensor();

      LineTraking();

      SerialPrints();


    } else if (DroneMode == "Manual") {

      if (y >= 1000) {
        digitalWrite(LeftMotorOut, HIGH);
        digitalWrite(RightMotorOut, HIGH);
        digitalWrite(LeftMotorOut2, LOW);
        digitalWrite(RightMotorOut2, LOW);

      }
      if (y <= 200) {
        digitalWrite(LeftMotorOut2, HIGH);
        digitalWrite(RightMotorOut2, HIGH);
        digitalWrite(LeftMotorOut, LOW);
        digitalWrite(RightMotorOut, LOW);

      }
      if (x >= 1000) {
        digitalWrite(LeftMotorOut, HIGH);
        digitalWrite(RightMotorOut, LOW);
        digitalWrite(LeftMotorOut2, LOW);


      }
      if (x <= 400) {
        digitalWrite(LeftMotorOut, LOW);
        digitalWrite(RightMotorOut, HIGH);
        digitalWrite(RightMotorOut2, LOW);

      }
      if (x == 512 && y == 512) {

        digitalWrite(LeftMotorOut, LOW);
        digitalWrite(RightMotorOut, LOW);
        digitalWrite(LeftMotorOut2, LOW);
        digitalWrite(RightMotorOut2, LOW);

      }




    } else {

      Serial.println("Please select in wich mode you want the drone to work");

    }



  } else {

    digitalWrite(LED1R, HIGH);
    digitalWrite(LED1G, LOW);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, HIGH);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
    digitalWrite(LeftMotorOut, LOW);
    digitalWrite(RightMotorOut, LOW);
    delay(1000);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, LOW);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
    digitalWrite(LeftMotorOut, LOW);
    digitalWrite(RightMotorOut, LOW);
    delay(1000);
    StartMillis = CurrentMillis;

  }
}

void LineTraking () {


  if (digitalRead(LeftSensorPin) != LineColorInt && digitalRead(RightSensorPin) != LineColorInt ) {


    if (LineNumber >= AmountOfStops) {
      LineNumber = 0;
      CourseNumber++;
      String msg1 = "Drone" + DroneId + "With id" + DroneIdNumber +  "reached end station, returning to start";
      String msg2 = "Drone" + DroneId + "With id" + DroneIdNumber + "Has done the Course " + CourseNumber + " Times";
      client.publish("Drone/Path/Lines", msg1.c_str());
      client.publish("Drone/Path/Course", msg2.c_str());
      terminal.println("Drone" + DroneId + "With id" + DroneIdNumber + "Has done the Course " + CourseNumber + " Times");
    } else {
      LineNumber++;
      String LineMsg = "Drone" + DroneId + "With id" + DroneIdNumber + "At Line " + LineNumber;
      client.publish("Drone/Path/Lines", LineMsg.c_str());
      terminal.println("Drone" + DroneId + "With id" + DroneIdNumber + "At Line " + LineNumber);
    }

    Serial.println(" Pause");
    digitalWrite(LeftMotorOut, LOW);
    digitalWrite(RightMotorOut, LOW);

    delay (5000);
    Serial.println ("Continue");
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, HIGH);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);
    delay(200);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, LOW);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
    delay(200);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, HIGH);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);
    delay(200);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, LOW);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
    delay(200);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, HIGH);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);
    delay(200);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, LOW);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);
    delay(200);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, HIGH);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);
    Serial.println(" Clear the line");
    //digitalWrite(LeftMotorOut, HIGH);
    //digitalWrite(RightMotorOut, HIGH);
    delay (200);

  }

  else if (digitalRead(LeftSensorPin) == LineColorInt && digitalRead(RightSensorPin) == LineColorInt && distance >= StoppingDistance) {

    digitalWrite(LeftMotorOut, HIGH);
    digitalWrite(RightMotorOut, HIGH);
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, HIGH);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);
    Serial.println(" Drive straight");

  } else if (digitalRead(RightSensorPin) == LineColorInt && distance >= StoppingDistance) {
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, HIGH);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, LOW);
    digitalWrite(LED2B, LOW);

    digitalWrite(RightMotorOut, LOW);
    digitalWrite(LeftMotorOut, HIGH);
    StartMillis = CurrentMillis;
    Serial.println(" Drive Right");

  } else if (digitalRead(LeftSensorPin) == LineColorInt && distance >= StoppingDistance) {
    digitalWrite(LED1R, LOW);
    digitalWrite(LED1G, LOW);
    digitalWrite(LED1B, LOW);
    digitalWrite(LED2R, LOW);
    digitalWrite(LED2G, HIGH);
    digitalWrite(LED2B, LOW);

    digitalWrite(LeftMotorOut, LOW);
    digitalWrite(RightMotorOut, HIGH);
    StartMillis = CurrentMillis;
    Serial.println(" Drive left");
  }

}

void UltrasonicSensor() {

  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);

  duration = pulseIn(EchoPin, HIGH);

  distance = duration * 0.034 / 2;

}

void SerialPrints() {

  Serial.print("Left Line: ");
  Serial.print(digitalRead (LeftSensorPin));
  Serial.print(" Middle Line: ");
  Serial.print(digitalRead (midSensorPin));
  Serial.print(" Right Line: ");
  Serial.print(digitalRead (RightSensorPin));

  Serial.print(" Set line Color: ");
  Serial.print(LineColor);
  Serial.print(" Line Color: ");
  Serial.print(LineColorInt);
  Serial.print(" Distance: ");
  Serial.print(distance);
  Serial.println("cm ");

}

void sendMessage () {

}
