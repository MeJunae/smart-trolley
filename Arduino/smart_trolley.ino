#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Encoder.h>
#include <WiFiEspAT.h>

WiFiServer server(80);
float Kp = 0.107;  // proportional  
float Ki = 0.00178;   // integral
float Kd = 0.136;  // derivate
String use = "fuzzy";
bool fuzzyWithKi = true;

String pathEasy = "1113111";
String pathMedium = "11131113113113131";
String pathHard = "11131311141141413113131";
String predeterminedPath = "";

char lastPosition = '\0';
long accError = 0;
long cumError = 0;
int iter_count = 0;
//Variable for PID
int previousError = 0; 
long I = 0;
//

//Fuzzy
#include "FuzzySet.h"

//Error fuzzyset
FuzzySet* GE = new FuzzySet(0, 0, 200, 600);
FuzzySet* NE = new FuzzySet(300, 1000, 1000, 1700);
FuzzySet* BE = new FuzzySet(1400, 2000, 2400, 2400);

//DeltaError fuzzyset
FuzzySet* GDE = new FuzzySet(-200, -200, -100, 0);
FuzzySet* NDE = new FuzzySet(-50, 0, 0, 50);
FuzzySet* BDE = new FuzzySet(0, 100, 200, 200);

//SigmaError fuzzyset
FuzzySet* SSE = new FuzzySet(0, 0, 4000, 9000);
FuzzySet* MSE = new FuzzySet(6500, 12000, 12000, 17500);
FuzzySet* BSE = new FuzzySet(15000, 20000, 24000, 24000);

//Kp Fuzzyset
FuzzySet* smallKp = new FuzzySet(1, 1, 5, 8);
FuzzySet* mediumKp = new FuzzySet(5, 8, 14, 16);
FuzzySet* highKp = new FuzzySet(14, 16, 20, 20);
const int Kp_scaler = 100;

//Kd Fuzzyset
FuzzySet* smallKd = new FuzzySet(0, 0, 3, 7);
FuzzySet* mediumKd = new FuzzySet(5, 10, 10, 15);
FuzzySet* highKd = new FuzzySet(13, 17, 20, 20);
const int Kd_scaler = 100;

//Ki Fuzzyset
FuzzySet* smallKi = new FuzzySet(0, 0, 3, 7);
FuzzySet* mediumKi = new FuzzySet(5, 9, 9, 12);
FuzzySet* highKi = new FuzzySet(11, 15, 18, 18);
const int Ki_scaler = 10000;
//

#define SS_PIN 53
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

//This code below is for controlling motor
class SPDMotor {
  public:
  SPDMotor( int encoderA, int encoderB, bool encoderReversed, int motorPWM, int motorDir1, int motorDir2 );
  void speed( int pwm );

  private:
    Encoder *_encoder;
    bool _encoderReversed;
    int _motorPWM, _motorDir1, _motorDir2;

    // Current speed setting.
    int _speed;
};

SPDMotor::SPDMotor( int encoderA, int encoderB, bool encoderReversed, int motorPWM, int motorDir1, int motorDir2 ) {
  _encoder = new Encoder(encoderA, encoderB);
  _encoderReversed = encoderReversed;

  _motorPWM = motorPWM;
  pinMode( _motorPWM, OUTPUT );
  _motorDir1 = motorDir1; 
  pinMode( _motorDir1, OUTPUT );
  _motorDir2 = motorDir2;
  pinMode( _motorDir2, OUTPUT );
}

int maximum_speed = 80;
void SPDMotor::speed( int speedPWM ) {
  _speed = speedPWM;
  if( speedPWM == 0 ) {
    digitalWrite(_motorDir1,LOW);
    digitalWrite(_motorDir2,LOW);
    analogWrite( _motorPWM, 255);
  } else if( speedPWM > 0 ) {
    digitalWrite(_motorDir1, LOW );
    digitalWrite(_motorDir2, HIGH );
    analogWrite( _motorPWM, speedPWM < maximum_speed ? speedPWM : maximum_speed);
  } else if( speedPWM < 0 ) {
    digitalWrite(_motorDir1, HIGH );
    digitalWrite(_motorDir2, LOW );
    analogWrite( _motorPWM, (-speedPWM) < maximum_speed ? (-speedPWM): maximum_speed);
  }
}

//Motor/Servo declaration
SPDMotor *motorLF = new SPDMotor(18, 31, true, 12, 34, 35); // <- Encoder reversed to make +position measurement be forward.
SPDMotor *motorRF = new SPDMotor(19, 38, false, 8, 36, 37); // <- NOTE: Motor Dir pins reversed for opposite operation
SPDMotor *motorLR = new SPDMotor( 3, 49, true,  9, 43, 42); // <- Encoder reversed to make +position measurement be forward.
SPDMotor *motorRR = new SPDMotor( 2, 23, false, 5, 27, 26); // <- NOTE: Motor Dir pins reversed for opposite operation

int speed_multiplier = 1;
int base_speed = 50;

//Setting speed for each wheel
void moveForward() {
  motorLF->speed(base_speed * speed_multiplier);
  motorLR->speed(base_speed * speed_multiplier);
  motorRF->speed(base_speed * speed_multiplier - 3);
  motorRR->speed(base_speed * speed_multiplier - 3);
}
void moveBackward() {
  motorLF->speed(-base_speed * speed_multiplier + 5);
  motorLR->speed(-base_speed * speed_multiplier + 5);
  motorRF->speed(-base_speed * speed_multiplier - 1.5);
  motorRR->speed(-base_speed * speed_multiplier - 1.5);
}
void moveSidewaysRight() {
  motorLF->speed(base_speed * speed_multiplier + 1);
  motorLR->speed(-base_speed * speed_multiplier + 4);
  motorRF->speed(-base_speed * speed_multiplier + 1);
  motorRR->speed(base_speed * speed_multiplier + 2);
}
void moveSidewaysLeft() {
  motorLF->speed(-base_speed * speed_multiplier - 2.5);
  motorLR->speed(base_speed * speed_multiplier - 4);
  motorRF->speed(base_speed * speed_multiplier - 2.5);
  motorRR->speed(-base_speed * speed_multiplier + 4);
}
void moveRightForward() {
  motorLF->speed(base_speed * speed_multiplier + 2);
  motorLR->speed(0);
  motorRF->speed(0);
  motorRR->speed(base_speed * speed_multiplier - 5);
}
void moveLeftForward() {
  motorLF->speed(0);
  motorLR->speed(base_speed * speed_multiplier - 1);
  motorRF->speed(base_speed * speed_multiplier);
  motorRR->speed(0);
}
void moveRightBackward() {
  motorLF->speed(0);
  motorLR->speed(-base_speed * speed_multiplier);
  motorRF->speed(-base_speed * speed_multiplier);
  motorRR->speed(0);
}
void moveLeftBackward() {
  motorLF->speed(-base_speed * speed_multiplier - 3);
  motorLR->speed(0);
  motorRF->speed(0);
  motorRR->speed(-base_speed * speed_multiplier + 3);
}

void spinRight()
{
  motorLF->speed(base_speed * speed_multiplier);
  motorLR->speed(base_speed * speed_multiplier);
  motorRF->speed(-base_speed * speed_multiplier);
  motorRR->speed(-base_speed * speed_multiplier);
  
//  MOTORA_FORWARD(Motor_PWM);MOTORB_BACKOFF(Motor_PWM);
//  MOTORC_FORWARD(Motor_PWM);MOTORD_BACKOFF(Motor_PWM);  
 }
void spinLeft()
{
  motorLF->speed(-base_speed * speed_multiplier);
  motorLR->speed(-base_speed * speed_multiplier);
  motorRF->speed(base_speed * speed_multiplier);
  motorRR->speed(base_speed * speed_multiplier);
//  MOTORA_BACKOFF(Motor_PWM);MOTORB_FORWARD(Motor_PWM);
//  MOTORC_BACKOFF(Motor_PWM);MOTORD_FORWARD(Motor_PWM); 
 }
void stopMoving() {
  motorLF->speed(0);
  motorLR->speed(0);
  motorRF->speed(0);
  motorRR->speed(0);
}

String receivedDirection;
int index;
int hasDirection;

// rotate

//Mux control pins
int s0 = 41;
int s1 = 40;
int s2 = 33;
int s3 = 32;

//Mux in "SIG" pin
int SIG_pin = 0;

int isRotate = 0;
int controlPin[] = {s0, s1, s2, s3};

int muxChannel[16][4]={
  {0,0,0,0}, //channel 0
  {0,1,0,0}, //channel 2
  {1,0,0,0}, //channel 1
  {1,1,0,0}, //channel 3
  {0,0,1,0}, //channel 4
  {0,1,1,0}, //channel 5
  {1,0,1,0}, //channel 6
  {1,1,1,0}, //channel 7
  {0,0,0,1}, //channel 8
  {0,1,0,1}, //channel 9
  {1,0,0,1}, //channel 10
  {1,1,0,1}, //channel 11
  {0,0,1,1}, //channel 12
  {1,1,1,1},  //channel 15
  {0,1,1,1}, //channel 14
  {1,0,1,1} //channel 13
};

void setupRotate(){
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  
  pinMode(SIG_pin, OUTPUT);// set SIG pin as output
  digitalWrite(SIG_pin, LOW); // set SIG sends what should be the output
}

const int black = 600;
bool sensorOnBlack(int val){
  return val >= black;
}

bool rotating = false;
void rotateRobot(){
  if(isRotate == 1){
    spinLeft();
  }
  if(isRotate == 2){
    spinRight();
  }

  int rotateSensors[16];
  for(int i = 0; i < 14; i++){
    for(int j = 0; j < 4; j++){
      digitalWrite(controlPin[j], muxChannel[i][j]);
    }
    rotateSensors[i] = analogRead(SIG_pin);
  }

  if(!rotating){

    bool ro = true;
    for(int i = 0; i < 2; i++){
      int val;
      if(isRotate == 1) val = rotateSensors[7 + i];
      if(isRotate == 2) val = rotateSensors[6 - i];
      ro = ro && !sensorOnBlack(val);
    }

    rotating = ro;
  }
  
  if(rotating){
    bool flag = 1;
    bool flag2 = 1;
    for(int i = 0; i < 14; i++){
      int val = rotateSensors[i];
//      if(i >= 6 && i <= 7) flag = flag && sensorOnBlack(val);
      if(isRotate == 2 && i >= 5 && i <= 6) flag = flag && sensorOnBlack(val);
      if(isRotate == 1 && i >= 7 && i <= 8) flag = flag && sensorOnBlack(val);
      flag2 = flag2 && !sensorOnBlack(val);
    }

    if(flag2){
      stopMoving();
    }

    if(flag){
      isRotate = 0;
      index++;
      rotating = false;
      previousError = I = 0;
      stopMoving();
    }
  }
}
//rotate

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

int prevRFID = 0;

void setupWiFi(){
  Serial3.begin(115200);
  WiFi.init(Serial3);
  
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed! Please reset!");
    while (true);
  }

  WiFi.begin("TrojanCovid19", "Muffin2010");
  Serial.println("Waiting for connection to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print('.');
  }
  Serial.println();

  server.begin();

  IPAddress ip = WiFi.localIP();
  Serial.println();
  Serial.println("Connected to WiFi network.");
  Serial.print("To access the server, enter \"http://");
  Serial.print(ip);
  Serial.println("/\" in web browser.");
}

void setup(void) {
  setupRotate();
  setupFuzzy();
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  receivedDirection = "";
  hasDirection = 0;
  if(!predeterminedPath.equalsIgnoreCase("")){  
    receivedDirection = predeterminedPath;
    hasDirection = 1;
  }
//  use = "basic";
  iter_count = 0;
  index = 0;
  accError = 0;
  prevRFID = 0;
  cumError = 0;
//  Kp = Ki = Kd = 0;

  Serial.begin(115200);
  while (!Serial);

  setupWiFi();
  Serial.println("Setup Completed");
  Serial.println(Ki);
  Serial.println(String(Ki,5));
  Serial.println(0.0001);
  Serial.println(Ki*100000000);
}

void loop(void) {
//  delay(150);
//  receiveDirection();

  receiveDirectionFromWiFi();
  followDirection();
//  PID_Control();

//    spinLeft();
  
//  for(int i = 0; i < 14; i++){
//    for(int j = 0; j < 4; j++){
//      digitalWrite(controlPin[j], muxChannel[i][j]);
//    }
//    Serial.print(analogRead(SIG_pin));
//    Serial.print("|");
//    Serial.print(i);
//    Serial.print(" ");
//  }
//  Serial.println("");
}

void receiveDirectionFromWiFi(){
  WiFiClient client = server.available();
  
  if (client) {
    IPAddress ip = client.remoteIP();
    Serial.print("new client with ip: ");
    Serial.println(ip);
    String url = "";
    String path = "";
    String body = "";
    String clientStr = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n";

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        line.trim();
        Serial.println(line);
        
        if(hasDirection != 1){
            url = getValue(getValue(line, ' ', 1), '?', 1);
            path = getValue(getValue(url, '&', 0), '=', 1);
          if(line.startsWith("GET /basic?")){
            use = "basic";
            Kp = getValue(getValue(url, '&', 1), '=', 1).toFloat();
            body = "RECEIVED PATH: " + path + " | Proportionality: " + String(Kp,5);
          }else if(line.startsWith("GET /pid?")){
            use = "pid";
            Kp = getValue(getValue(url, '&', 1), '=', 1).toFloat();
            Ki = getValue(getValue(url, '&', 2), '=', 1).toFloat();
            Kd = getValue(getValue(url, '&', 3), '=', 1).toFloat();
            body = "RECEIVED PATH: " + path + " | KP KI KD: " + String(Kp,5) + " " + String(Ki,5) + " " + String(Kd,5);
          }else if(line.startsWith("GET /fuzzy?")){
            use = "fuzzy";
            body = "RECEIVED PATH: " + path + " | Use Fuzzy";
          }
          receivedDirection = path;
          hasDirection = 1;
          Serial.println("BODY: " + body);
        }
        
        if (line.length() == 0) {
          client.println(clientStr);
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println(body);
          client.println("</html>");
          client.flush();
          break;
        }
      }
    }

    client.stop();
    Serial.println("Client disconnected and connection closed");
  }
}

void receiveDirection(){
  if(!Serial2.available() || hasDirection) return;
  receivedDirection = "";
  
  while(Serial2.available()){
    receivedDirection += ((byte)Serial2.read());
    delay(25);
  }
  Serial.print("Received Direction: ");
  Serial.println(receivedDirection);

  hasDirection = 1;
}

void resetAll(){  
  receivedDirection = "";
  hasDirection = 0;
  index = 0;
  accError = 0;
  cumError = 0;
  stopMoving();
  prevRFID = 0;
  iter_count = 0;
  Kp = 0.107;
  Ki = 0.00178;
  Kd = 0.136;
}

void followDirection(){
  if(!hasDirection) return;

  char moveDirection = convertRFIDValueToDirection();

//  Serial.println(moveDirection);
//  Serial.print("AE: ");
//  Serial.println(accError);
//  Serial.print("CE: ");
//  Serial.println(cumError);
//  Serial.println(isRotate);
 
  if(isRotate != 0){
    rotateRobot();
    return;
  }
  
  if(moveDirection == '1'){
    PID_Control();
  }
  if(moveDirection == '2'){
  }
  if(moveDirection == '3'){
//    lineFollowHorizontal(true);
    isRotate = 2;
  }
  if(moveDirection == '4'){
//    lineFollowHorizontal(false);
    isRotate = 1;
  }
  
//  bool flag = emergencyFlag(moveDirection - '0' - 1);
//  Serial.print(moveDirection - '0' - 1);
//  Serial.println(flag);
//  if(moveDirection == '5' || flag){
//    stopMoving();
//  }
  
  if(index >= receivedDirection.length()){
    Serial.println("Destination");
    Serial.println(cumError / iter_count);
    resetAll();
  }
}

char convertRFIDValueToDirection(){
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return receivedDirection[index];
  int value = rfid.uid.uidByte[1];
  if(prevRFID == value) return receivedDirection[index];
  prevRFID = value;
  char currentLocation = generateLocationFromRFID(value);
  lastPosition = currentLocation;
  
//  Serial.println(currentLocation);
//  Serial.println();

  // Halt PICC
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  if(value) index++;
  return receivedDirection[index];
}

char generateLocationFromRFID(int value){
  switch(value){
    case 113: return 'A';
    case 123: return 'B';
    case 141: return 'C';
    case 164: return 'D';
    case 124: return 'E';
    case 137: return 'F';
    case 159: return 'G';
    case 154: return 'H';
    case 111: return 'I';
    case 122: return 'J';
    case 126: return 'K';
    case 172: return 'L';
    case 180: return 'M';
    case 196: return 'N';
    case 192: return 'O';
    case 188: return 'P';
  }

  return '\0';
}

//Fuzzy Logic
#include "FuzzyRule.h"
#include "FuzzyComposition.h"
#include "Fuzzy.h"
#include "FuzzyRuleConsequent.h"
#include "FuzzyOutput.h"
#include "FuzzyInput.h"
#include "FuzzyIO.h"
#include "FuzzyRuleAntecedent.h"

Fuzzy* fuzzy = new Fuzzy();
Fuzzy* fuzzy2 = new Fuzzy();

void fuzzyRuleBase();
void setupFuzzy(){
  //inputs
  FuzzyInput* error = new FuzzyInput(1);
  error->addFuzzySet(BE);
  error->addFuzzySet(NE);
  error->addFuzzySet(GE);
  fuzzy->addFuzzyInput(error);
  fuzzy2->addFuzzyInput(error);
  
  FuzzyInput* deltaError = new FuzzyInput(2);
  deltaError->addFuzzySet(BDE);
  deltaError->addFuzzySet(NDE);
  deltaError->addFuzzySet(GDE);
  fuzzy->addFuzzyInput(deltaError);

  FuzzyInput* sigmaError = new FuzzyInput(2);
  sigmaError->addFuzzySet(SSE);
  sigmaError->addFuzzySet(MSE);
  sigmaError->addFuzzySet(BSE);
  fuzzy2->addFuzzyInput(sigmaError);
  
  //outputs
  FuzzyOutput* fuzzyKd = new FuzzyOutput(1);
  fuzzyKd->addFuzzySet(smallKd);
  fuzzyKd->addFuzzySet(mediumKd);
  fuzzyKd->addFuzzySet(highKd);
  fuzzy->addFuzzyOutput(fuzzyKd);
  
  FuzzyOutput* fuzzyKi = new FuzzyOutput(1);
  fuzzyKi->addFuzzySet(smallKi);
  fuzzyKi->addFuzzySet(mediumKi);
  fuzzyKi->addFuzzySet(highKi);
  fuzzy2->addFuzzyOutput(fuzzyKi);
  
  fuzzyRuleBase();
}

void fuzzyRuleBase(){
  FuzzyRuleAntecedent* GE_GDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* GE_NDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* GE_BDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* NE_GDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* NE_NDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* NE_BDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* BE_GDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* BE_NDE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* BE_BDE = new FuzzyRuleAntecedent();
  
  FuzzyRuleAntecedent* GE_SSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* GE_MSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* GE_BSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* NE_SSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* NE_MSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* NE_BSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* BE_SSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* BE_MSE = new FuzzyRuleAntecedent();
  FuzzyRuleAntecedent* BE_BSE = new FuzzyRuleAntecedent();

  
  GE_GDE->joinWithAND(GE, GDE);
  GE_NDE->joinWithAND(GE, NDE);
  GE_BDE->joinWithAND(GE, BDE);
  NE_GDE->joinWithAND(NE, GDE);
  NE_NDE->joinWithAND(NE, NDE);
  NE_BDE->joinWithAND(NE, BDE);
  BE_GDE->joinWithAND(BE, GDE);
  BE_NDE->joinWithAND(BE, NDE);
  BE_BDE->joinWithAND(BE, BDE);

  GE_SSE->joinWithAND(GE, SSE);
  GE_MSE->joinWithAND(GE, MSE);
  GE_BSE->joinWithAND(GE, BSE);
  NE_SSE->joinWithAND(NE, SSE);
  NE_MSE->joinWithAND(NE, MSE);
  NE_BSE->joinWithAND(NE, BSE);
  BE_SSE->joinWithAND(BE, SSE);
  BE_MSE->joinWithAND(BE, MSE);
  BE_BSE->joinWithAND(BE, BSE);


  FuzzyRuleConsequent* THEN_smallKd = new FuzzyRuleConsequent();
  FuzzyRuleConsequent* THEN_mediumKd = new FuzzyRuleConsequent();
  FuzzyRuleConsequent* THEN_highKd = new FuzzyRuleConsequent();
  FuzzyRuleConsequent* THEN_smallKi = new FuzzyRuleConsequent();
  FuzzyRuleConsequent* THEN_mediumKi = new FuzzyRuleConsequent();
  FuzzyRuleConsequent* THEN_highKi = new FuzzyRuleConsequent();

  THEN_smallKd->addOutput(smallKd);
  THEN_mediumKd->addOutput(mediumKd);
  THEN_highKd->addOutput(highKd);
  THEN_smallKi->addOutput(smallKi);
  THEN_mediumKi->addOutput(mediumKi);
  THEN_highKi->addOutput(highKi);
  

  int ruleCount = 9;
  FuzzyRuleAntecedent* ra[ruleCount] = {
    GE_GDE,
    GE_NDE,
    GE_BDE,
    NE_GDE,
    NE_NDE,
    NE_BDE,
    BE_GDE,
    BE_NDE,
    BE_BDE,
  };

  FuzzyRuleAntecedent* ra1[ruleCount] = {
    GE_SSE,
    GE_MSE,
    GE_BSE,
    NE_SSE,
    NE_MSE,
    NE_BSE,
    BE_SSE,
    BE_MSE,
    BE_BSE,
  };
  
  FuzzyRuleConsequent* rc[ruleCount] = {
    THEN_smallKd,
    THEN_smallKd,
    THEN_mediumKd,
    THEN_smallKd,
    THEN_mediumKd,
    THEN_mediumKd,
    THEN_smallKd,
    THEN_mediumKd,
    THEN_highKd,
  };
  FuzzyRuleConsequent* rc1[ruleCount] = {
    THEN_mediumKi,
  THEN_smallKi,
  THEN_smallKi,
  THEN_mediumKi,
  THEN_mediumKi,
  THEN_highKi,
  THEN_mediumKi,
  THEN_mediumKi,
  THEN_highKi,
  };
  
  for(int i = 0; i < ruleCount; i++){
    FuzzyRule* newRule = new FuzzyRule(i + 1, ra[i], rc[i]);
    fuzzy->addFuzzyRule(newRule);
  }
  for(int i = 0; i < ruleCount; i++){
    FuzzyRule* newRule = new FuzzyRule(i + 1, ra1[i], rc1[i]);
    fuzzy2->addFuzzyRule(newRule);
  }
}
// end Fuzzy Logic

//PID

// Variables for Error inputs
int leftCount = 4;
int rightCount = 4;
// Variables for PID Control

int PIDvalue = 0; 
int error = 0; 
int P = 0; 
int D = 0;
// Gain constants 

int frontSensor = 0;

int getError(){
  frontSensor = 0;
  
  int leftSensor = 0;
  for(int i = 0; i < leftCount; i++){
    for(int j = 0; j < 4; j++){
      digitalWrite(controlPin[j], muxChannel[6 - i][j]);
    }
    int val = analogRead(SIG_pin);
      val = val > 850 ? 850 : val;
      val = val < 550 ? 550 : val;
    frontSensor += val >= black;

    if(i != 0) val *= (i + 1);
    leftSensor += val;
  }

  int rightSensor = 0;
  for(int i = 0; i < rightCount; i++){
    for(int j = 0; j < 4; j++){
      digitalWrite(controlPin[j], muxChannel[7 + i][j]);
    }
    int val = analogRead(SIG_pin);
      val = val > 850 ? 850 : val;
      val = val < 550 ? 550 : val;
    frontSensor += val >= black;
    
    if(i != 0) val *= (i + 1);
    rightSensor += val;
  }
  int err = rightSensor-leftSensor;
//  Serial.print("ERRRRRRRRRR: ");
//  Serial.println(frontSensor);
  accError = accError + (err < 0? -err : err);
  cumError += err;
  return err;
}

int calcPID(int error, int previousError){
  P = error;
  I = I + error;
  D = error - previousError;

  //fuzzy
    if(use.equalsIgnoreCase("fuzzy")){
      fuzzy->setInput(1, P < 0 ? -P : P);
      fuzzy->setInput(2, P < 0 ? -D : D);
      
      fuzzy->fuzzify();
      
      if(fuzzyWithKi){
        fuzzy2->setInput(1, P < 0 ? -P : P);
        fuzzy2->setInput(2, I < 0 ? -I : I);

        fuzzy2->fuzzify();
      }
    
      Kd = (double)fuzzy->defuzzify(1) / Kd_scaler;
      if(fuzzyWithKi) Ki = (double)fuzzy2->defuzzify(1) / Ki_scaler;
      else Ki = 0;
    }
  //
  int pid = (Kp*P);
  if(!use.equalsIgnoreCase("basic"))  pid += (Ki*I) + (Kd*D);
  
  return pid; 
};

void moveForwardPID(int deltaSpeed) {
  motorLF->speed(base_speed + deltaSpeed);
  motorLR->speed(base_speed + deltaSpeed);
  motorRF->speed(base_speed - deltaSpeed);
  motorRR->speed(base_speed - deltaSpeed);
}

void adjustMoveForwardBasic(int deltaSpeed) {
  motorLF->speed(base_speed + deltaSpeed);
  motorLR->speed(base_speed + deltaSpeed);
  motorRF->speed(base_speed - deltaSpeed);
  motorRR->speed(base_speed - deltaSpeed);
}

void moveForwardBasic(int error){
  if(error > 100) adjustMoveForwardBasic(15);
  else if(error < -100) adjustMoveForwardBasic(-15);
  else moveForward();
}

void PID_Control(){
  int error = getError();
  int deltaSpeed = calcPID(error, previousError);
  Serial.print("DATA,TIMER,");
  Serial.print(error);
//  Serial.print(",");
//  Serial.print(I);
//  Serial.print(",");
//  Serial.print(D);
//  Serial.print(",");
//  Serial.print(Kp * P);
//  Serial.print(",");
//  Serial.print(Ki * I);
//  Serial.print(",");
//  Serial.print(Kd * D);
//  Serial.print(",");
//  Serial.print(cumError);
  Serial.println("");
  cumError += error < 0 ? -error : error;
  iter_count++;
  previousError = error;
  
//  Serial.println(deltaSpeed);
  if(!frontSensor) stopMoving(); else 
  moveForwardPID(deltaSpeed);
  
  delay(25);
}
