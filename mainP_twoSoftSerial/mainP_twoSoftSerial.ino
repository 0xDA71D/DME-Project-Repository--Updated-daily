#include <SoftwareSerial.h>                         //WARNING!!! I WILL CRASH IF USING STRING CLASS!!!!! http://forum.arduino.cc/index.php?topic=115954 http://forum.arduino.cc/index.php/topic,116242.0.html
#include <math.h>                                   //TODO Rewrite W/O String or dynamic allocation. This commit is BEFORE (read: Needs Work) change!! NEVER use Free() statement as that is reportedly
//Mathematical Defines vv                           //Buggy too. This will rectify all problems!! (hopefully)
#define pi 3.14159265358979
#define dlat (pi*(Lat2-Lat1)/180)
#define dlon (pi*(Lon2-Lon1)/180)
#define earthRadius 6371000 //meters
//Mathematical Defines ^^
#define txPin 8      //tx pin in GPS connection
#define rxPin 9      //rx pin in GPS connection

SoftwareSerial gps = SoftwareSerial(rxPin, txPin);
SoftwareSerial xbee = SoftwareSerial(2,3 );
//Xbee vars ---------------------
String ATString;

//Xbee vars----------------------
//GPS  vars ---------------------
byte byteGPS = 0;  
int i = 0;
int state = 0;
char dataGPG[100]="";
char *pch;
char *GGA[15];
int sat = 0;
uint32_t myHAddress;
uint32_t myLAddress;
float myLatVal = 0, myLonVal = 0;
//GPS  vars ---------------------
char* GPSPacketByteValue;
//TYPEDEFS ---------------------- 

typedef struct GPSPacket{  //defines a structure 
  float Latitude;
  float Longitude;
  uint32_t magicNumber;
  uint32_t sourceHAddress; //ATSH /
  uint32_t sourceLAddress; //ATSL
}GPSPacket;
//TYPEDEFS ----------------------

//Create objects ----------------

//Create objects ----------------

//flags and timers vv
#define GPS_TIME 60000000
#define SEND_OVER_TIME 20000000
boolean GPSFlag; //true if GPS received.
boolean mustSendFlag;
int32_t GPSTicker = 0;
int32_t sendOverTicker = 20000000;
//flags and timers ^^

void setup(){
  Serial.println("Note to human: Hardware Settings: GPS 8/9 (RX/TX); Xbee 2/3 (RX/TX) | Firmware Settings: Xbee Baud rate 19200"); 
  Serial.begin(9600);
  Serial.flush();
  gps.begin(9600); //setup for GPS Serial Port  
  gps.flush();
  xbee.begin(19200);
  xbee.flush();
  xbee.listen();
  //xbee.begin(9600);
  //xbee.end();
  pinMode(13, OUTPUT); //setup zigbee signal
  digitalWrite(13, LOW);     // Turn off the led until a zigbee signal
  pinMode(11, OUTPUT);
  digitalWrite(11, LOW); //basestation led
  pinMode(12, OUTPUT);
  digitalWrite(12, LOW); //Permanently like this
  //init vvvv
  enterAT(3);
  sendAT("ATDH0\r\n",3);
  sendAT("ATDL0\r\n",3);
  sendAT("ATCN\r\n", 3);
  while(!getSerialNumber()); //keep getting until it's set; constant value for the rest of the code. 
}
void loop(){
  unsigned long nowMicro = micros();
  ////////////////////////////////
  if(GPSTicker < 0 ){ 
    gps.listen();
    getGPS(5000);
    //Serial.println("Theoretically, get GPS");
    GPSTicker = GPS_TIME;
    xbee.listen(); 
    
  }
  ////////////////////////////////
  if(sendOverTicker < 0){
   sendGPSPacket();
   sendOverTicker = SEND_OVER_TIME; 
  }
  Serial.println (sendOverTicker);
  ///////////////////////////////
  if(xbee.available()){process();}
  ///////////////////////////////
  
  uint32_t elapsedMicro = (uint32_t)(4294967295*((int)(micros()<nowMicro)) - (int32_t)nowMicro) + (int32_t)(micros());
  GPSTicker = GPSTicker - elapsedMicro;
  sendOverTicker = sendOverTicker - elapsedMicro;
}
//Main loop ^^

//Functions to be called to do stuff vv
void sendGPSPacket(){
  GPSPacket GPS;
  GPS.Latitude = myLatVal;
  GPS.Longitude= myLonVal;
  GPS.magicNumber = 0x7E57;
  GPS.sourceHAddress = myHAddress;
  GPS.sourceLAddress = myLAddress;
  char* whatToSend = (char*) malloc(25);
  whatToSend = (char*)&GPS;
  processString(whatToSend, 20);
  /*debug(whatToSend);
  char* receivedPacket = (char*) malloc(25);
  memcpy(receivedPacket, whatToSend, 25);
  parseWrapper(receivedPacket, 20);
  GPSPacket* newGPS;
  newGPS = (GPSPacket*) receivedPacket;
  debug(newGPS, sizeof(GPSPacket));*/
  Serial.print("Sending to other nodes this data:");Serial.println(whatToSend);Serial.print(myLatVal);Serial.print(",");Serial.print(myLonVal);Serial.print(",");Serial.print(myHAddress);Serial.print(",");Serial.println(myLAddress);
  debug(whatToSend);
  xbee.write(whatToSend);
  digitalWrite(11, HIGH);
  delay(20);
  digitalWrite(11, LOW);


}
boolean process(){
  String got = readXbee(3);
  //debug((GPSPacket*)got.c_str(), strlen(got.c_str()));
//  char* thisString = (char*)got.c_str();
//  parseWrapper(thisString, sizeof(GPSPacket));
//  debug((GPSPacket*)thisString, 0);
//  debug(thisString);
   char* receivedPacket = (char*) malloc(25);
   memcpy(receivedPacket, got.c_str(), 25);
   parseWrapper(receivedPacket, 20);
   GPSPacket* newGPS;
   memcpy(newGPS, receivedPacket, 20);
   Serial.print("Got Packet... Latitude: "); Serial.print(newGPS->Latitude);  Serial.print("| Longitude: "); Serial.print(newGPS->Longitude); Serial.print("| Magic Number: " ); Serial.print(newGPS->magicNumber); Serial.print("| Source Address: ");Serial.print(newGPS->sourceHAddress);Serial.print(".");Serial.print(newGPS->sourceLAddress);Serial.print("| Sizeof: ");Serial.println(); 
   //debug(newGPS, sizeof(GPSPacket));
   
}
boolean getGPS(int timeOutTime){
  float timeMillis = millis();                                        Serial.println("Tag 1");
  memset(dataGPG, 0, sizeof(dataGPG));                                Serial.println("Tag 2");                  
  byteGPS = 0;                                                        Serial.println("Tag 3");
  byteGPS = gps.read();                                               Serial.println("Tag 4");
  delay(1000);                                                        Serial.println("Tag 5");
  while(byteGPS != '$')
  {
    byteGPS = gps.read();                                             Serial.println("Tag 6");
    if((int) (millis()-timeMillis) >= timeOutTime){                   Serial.println("Fail 1");
      return false;                
    }
  }
  i=1;                                                                Serial.println("Tag 7");
  dataGPG[0] = '$';                                                   Serial.println("Tag 8");
  while(byteGPS != '*' )                                     
  {
    byteGPS = gps.read();
    dataGPG[i]=byteGPS; 
    i++; 
  }
  dataGPG[i]= '\0';                                                   Serial.println("Tag 9");
  i=0;                                                                
  memset(GGA, 0, sizeof(GGA));                                        Serial.println("Tag 10");
  pch = strtok (dataGPG,",");                                         Serial.println("Tag 11");
  if (strcmp(pch,"$GPRMC")==0)
  {                                                                   Serial.println("Got GPRMC");
    while (pch != NULL)
    {
      pch = strtok (NULL, ",");
      GGA[i]=pch;    
      i++;
    }
  }else{                                                             Serial.println("Fail 2");
    return false;                                                     
  }
  float rawLat = parseDegree(GGA[2]);                                Serial.println("Tag 12");
  float rawLon = parseDegree(GGA[4]);
  if(rawLat == 0 || rawLat == 0 || *GGA[1] == 'V' ){                 Serial.println("Fail 3");
  return false;                                                      
  }
  if(*GGA[3] == 'S'){
    rawLat = -1 * rawLat;
  }
    if(*GGA[5] == 'W'){
    rawLon = -1 * rawLon;
  }                                                                              
  //Reality Check
  if(((myLatVal * myLonVal)!=0) && distance (rawLat, rawLon, myLatVal, myLonVal) > 5000 ){return false;}
  myLatVal = rawLat;                                                Serial.println("Tag 13");
  myLonVal = rawLon;
  Serial.println(myLatVal);
  return true;
  
}

boolean getSerialNumber(){
   enterAT(3);
   myHAddress = (uint32_t) strtol(sendAT("ATSH\r\n",3).c_str(), NULL, 16);
   Serial.print("Got H Address "); Serial.println(myHAddress);                                          
   myLAddress = (uint32_t) strtol(sendAT("ATSL\r\n",3).c_str(), NULL, 16);
   Serial.print("Got L Address "); Serial.println(myLAddress);    
   //confirmation step
   if((uint32_t) strtol(sendAT("ATSH\r\n",3).c_str(), NULL, 16) !=myHAddress || (uint32_t) strtol(sendAT("ATSL\r\n",3).c_str(), NULL, 16) != myLAddress){
     return false;
   }
   sendAT("ATCN\r\n", 3);
   digitalWrite(13, HIGH);
   return true;  
}
//Functions to be called from main loop (WARNING: Only functions that are written in the main loop can be put here!!!!) ^^

//utils vv
float parseDegree(char* inputString){
  int firstNum,degree;
  long secondNum;
  float minute;
  firstNum = atoi(strtok (inputString, "."));
  secondNum = atol (strtok (NULL, "."));
  //Serial.print(firstNum);
  //Serial.print (" | ");    For debug use
  //Serial.print(secondNum);
  //Serial.print("\n");
  degree = floor(firstNum/ 100); 
  //Serial.println(degree);
  minute = firstNum - 100*degree;
  //Serial.println(minute);
  minute = (float)(minute+ secondNum*.00001);
  //Serial.println(minute);
  return degree + (minute*0.0166666667);
}
/*void printFloat ( float numToPrint){
  char addChar;
  if(numToPrint < 0){
    addChar = '-';
  }else{
    addChar = NULL;                                                       // Antique
  }
  numToPrint = abs(numToPrint);
  Serial.print(addChar);
  Serial.print((int)floor(numToPrint));
  Serial.print(".");
  int bigDecimal = 10000 * (numToPrint - floor(numToPrint));
  Serial.println (bigDecimal);
}*/
String readXbee(int timeoutTime){   
  xbee.listen();
  String finalString = "";
  float startTime = millis();
  while (!xbee.available()) {
    if(millis()-startTime > timeoutTime){
      return "";
    }
  }
  /*while (Serial.available()){
    finalString = finalString + (char)Serial.read();
  }*/
      float firstStreamTime;

  do{
    finalString = finalString + (char) xbee.read();
    firstStreamTime = millis();
    while ( ((millis() - firstStreamTime )< 5 )& ! xbee.available()); // changed stream time from 100 to 5; normally does not take that long. 1ms nominal.
  }while((millis()-firstStreamTime)<5);
 // delay(1000);
  Serial.println(finalString);
  return finalString;
}

boolean enterAT(int maxIterations){
  xbee.flush();
  do{
    maxIterations--;
    delay(1100);
    xbee.print("+++");
    delay(1000);
    String gotString = readXbee(3000);
    if(strcmp (gotString.c_str(), "OK\r") == 0/*strlen(gotString.c_str()) > 0*/ ){
      //digitalWrite(13, HIGH);
      return true;
    }
  }
  while(maxIterations > 0 );
  return false;
}
String sendAT(String whatToSend, int maxIterations){
  //Serial.flush();
  xbee.flush();
  xbee.print(whatToSend);
  do{
    maxIterations--;
    String gotString = readXbee(2000);
    if((strlen(gotString.c_str())>0 && !(whatToSend == "ATCN\r\n") )|| (whatToSend == "ATCN\r\n" && strcmp(gotString.c_str(), "OK\r")==0)){
      return gotString;
    }
  }while(maxIterations>0);
  Serial.println("ERROR!! TIMED OUT!"); // This should not display, else there is an error
  return false;  
}
float distance(float Lat1, float Lon1, float Lat2, float Lon2){
  float a = (sin(dlat/2)) * (sin(dlat/2)) + cos(pi*Lat1/180) * cos(pi*Lat2/180) * (sin(dlon/2)) * (sin(dlon/2)) ;
  float c = 2 * atan2( sqrt(a), sqrt(1-a) );
  return (earthRadius * c);
  
} 
void debug(String* whatToSend){
  analogWrite(11, 8);
  if(!enterAT(3)){return;}
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
  digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  xbee.println( *whatToSend);
  analogWrite(11, 8);
  if(!enterAT(3)){return;} 
  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug(float* whatToSend){
  analogWrite(11, 8);
  if(!enterAT(3)){return;} 
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  xbee.println( *whatToSend);
  analogWrite(11, 8);    if(!enterAT(3)){return;} 

  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug(String* whatToSend, uint32_t Haddress, uint32_t Laddress){
  analogWrite(11, 8);
  if(!enterAT(3)){return;} 
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  xbee.print( *whatToSend);xbee.print(" | "); xbee.print(Haddress);xbee.print(" | "); xbee.println(Laddress);
  analogWrite(11, 8);    if(!enterAT(3)){return;} 

  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug(float whatToSend){
  analogWrite(11, 8);
  if(!enterAT(3)){return;} 
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  xbee.println(whatToSend);
  analogWrite(11, 8);    if(!enterAT(3)){return;} 

  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug (struct GPSPacket* GPSIn, int realSize){
    analogWrite(11, 8);
  if(!enterAT(3)){return;} 
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
     xbee.print("Latitude: "); xbee.print(GPSIn->Latitude);  xbee.print("| Longitude: "); xbee.print(GPSIn->Longitude); xbee.print("| Magic Number: " ); xbee.print(GPSIn->magicNumber); xbee.print("| Source Address: ");xbee.print(GPSIn->sourceHAddress);xbee.print(".");xbee.print(GPSIn->sourceLAddress);xbee.print("| Sizeof: ");xbee.println(realSize); 

  
    analogWrite(11, 8);    if(!enterAT(3)){return;} 

  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
  
  
}
void debug(char* whatToSend){
  analogWrite(11, 8);
  if(!enterAT(3)){return;}
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
  digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
for(int i = 0; i <strlen(whatToSend); i++){
    xbee.println((uint8_t)whatToSend[i]); 
    
    
    
  }
  analogWrite(11, 8);
  if(!enterAT(3)){return;} 
  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void decodr(char* inputChar){
   GPSPacket* decodingTemplate;
   decodingTemplate = (GPSPacket*) inputChar; 
   Serial.println(decodingTemplate->magicNumber); 
   Serial.println(decodingTemplate->Latitude);
   Serial.println(decodingTemplate->Longitude);
}
boolean parseWrapper(char* whatString, int sizeToProcess){  //This for receiving side
//  if(whatString[1] != "|"){return false;}
  char tokenValue = whatString[0];
  for(int i = 0; i < sizeToProcess; i++){
  whatString [i] = whatString[i+2]; 
  if(whatString[i] == tokenValue){
    whatString[i] = 0; 
  }
  }   
  whatString[sizeToProcess] = 0;
  
}
char processString(char* whatString, int sizeToProcess){
  uint8_t replacementValue = 36;
  boolean found;
  do{
    found = false;
    for(int i = 0; i < sizeToProcess; i++){
      if((uint8_t)whatString[i] == replacementValue){
        found = true; break;
      }  
    }
    replacementValue--;
  }while(found);
  replacementValue++;
  for(int i = 0; i < sizeToProcess; i++){
    if((uint8_t)whatString[i] == 0){
      whatString[i] = (char)replacementValue;
    }   
  
  }  

  whatString[sizeToProcess] = 0;
  
  for(int i = sizeToProcess-1; i >=0;i--){
    whatString[i+2] = whatString[i];    
  }
  strncpy(whatString + 1, "|", 1);
  memcpy(whatString, &replacementValue, 1);
}

/*Dead code (used to be in loop, now here for reference 
    if((boolean)(myLatVal * myLonVal)){ // if (Latitude != 0.0f && Longitude != 0.0f) 
    newPacket.Latitude = myLatVal;
    newPacket.Longitude = myLonVal;
    newPacket.magicNumber = 0x7E57;
    newPacket.sourceHAddress = myHAddress;
    newPacket.sourceLAddress = myLAddress;
    //Malloc
    //GPSPacketByteValue = (char*) malloc(sizeof(newPacket));
//    memcpy(GPSPacketByteValue, &newPacket, sizeof(newPacket));
    GPSPacketByteValue = (char*) &newPacket;
    Serial.println(GPSPacketByteValue);    
    //decodr(GPSPacketByteValue);
    } else{
      getGPS(30000);       
    }
    if(Serial.available()){ //event void radio received 
      String outputString = readXbee(1000);
    //  receivedPacket = (GPSPacket*) outputString.c_str();
    //  debug(distance(receivedPacket->Latitude, myLatVal, receivedPacket->Longitude, myLonVal));
      debug(&outputString, myHAddress, myLAddress );
    }    
*/
