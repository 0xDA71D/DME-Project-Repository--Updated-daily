#include <SoftwareSerial.h>
#include <math.h>
//Mathematical Defines vv
#define pi 3.14159265358979
#define dlat (pi*(Lat2-Lat1)/180)
#define dlon (pi*(Lon2-Lon1)/180)
#define earthRadius 6371000 //meters
//Mathematical Defines ^^
#define txPin 8      //tx pin in GPS connection
#define rxPin 9      //rx pin in GPS connection

SoftwareSerial gps = SoftwareSerial(rxPin, txPin);
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
  uint32_t sourceHAddress; //ATSH
  uint32_t sourceLAddress; //ATSL
}GPSPacket;
//TYPEDEFS ----------------------

//Create objects ----------------

GPSPacket newPacket;
GPSPacket* receivedPacket;

//Create objects ----------------

//flags and timers vv
#define GPS_TIME 60000000
#define DEBUG_TIME 20000000
boolean GPSFlag; //true if GPS received.
uint32_t GPSTicker = 0;
uint32_t debugTicker = 20000000;
//flags and timers ^^

void setup(){
  Serial.begin(9600);
  Serial.flush(); 
  gps.begin(9600); //setup for GPS Serial Port  
  gps.flush();
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
  long nowMicro = micros();


}
//Main loop ^^

//Functions to be called to do stuff vv
boolean getGPS(int timeOutTime){
  float timeMillis = millis();
  memset(dataGPG, 0, sizeof(dataGPG));    // Remove previous readings
  byteGPS = 0;                            // Remove data
  byteGPS = gps.read();                   // Read the byte that is in the GPS serial port
  delay(1000);
  while(byteGPS != '$') //find the desired string to parse
  {
    byteGPS = gps.read();
    if((int) (millis()-timeMillis) >= timeOutTime){
      return false;
    }
  }
  i=1;
  dataGPG[0] = '$';
  while(byteGPS != '*' )
  {
    byteGPS = gps.read();
    dataGPG[i]=byteGPS; 
    i++; 
  }
  dataGPG[i]= '\0';
  i=0;
  memset(GGA, 0, sizeof(GGA));          // Remove previous readings
  pch = strtok (dataGPG,",");
  if (strcmp(pch,"$GPRMC")==0)
  {  
    while (pch != NULL)
    {
      pch = strtok (NULL, ",");
      GGA[i]=pch;    
      i++;
    }
  }else{
    return false;
  }
  float rawLat = parseDegree(GGA[2]);
  float rawLon = parseDegree(GGA[4]);
  if(rawLat == 0 || rawLat == 0 || *GGA[1] == 'V' ){
  return false;
  }
  if(*GGA[3] == 'S'){
    rawLat = -1 * rawLat;
  }
    if(*GGA[5] == 'W'){
    rawLon = -1 * rawLon;
  }
  //Reality Check
  if(((myLatVal * myLonVal)!=0) && distance (rawLat, rawLon, myLatVal, myLonVal) > 5000 /* if moved 3 miles a minute*/){return false;}
  myLatVal = rawLat;
  myLonVal = rawLon;
  return true;
}
boolean getSerialNumber(){
   enterAT(3);
   myHAddress = (uint32_t) strtol(sendAT("ATSH\r\n",3).c_str(), NULL, 16);
   Serial.println(myHAddress);                                          
   myLAddress = (uint32_t) strtol(sendAT("ATSL\r\n",3).c_str(), NULL, 16);
   Serial.println(myLAddress);    
   //confirmation step
   if((uint32_t) strtol(sendAT("ATSH\r\n",3).c_str(), NULL, 16) !=myHAddress || (uint32_t) strtol(sendAT("ATSL\r\n",3).c_str(), NULL, 16) != myLAddress){
     return false;
   }
   Serial.print("ATCN\r\n");
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
  String finalString = "";
  float startTime = millis();
  while (!Serial.available()) {
    if(millis()-startTime > timeoutTime){
      return "";
    }
  }
  /*while (Serial.available()){
    finalString = finalString + (char)Serial.read();
  }*/
      float firstStreamTime;

  do{
    finalString = finalString + (char) Serial.read();
    firstStreamTime = millis();
    while ( ((millis() - firstStreamTime )< 5 )& ! Serial.available()); // changed stream time from 100 to 5; normally does not take that long. 1ms nominal.
  }while((millis()-firstStreamTime)<5);
 // delay(1000);
  //Serial.println(finalString);
  return finalString;
}
boolean enterAT(int maxIterations){
  do{
    maxIterations--;
    delay(1100);
    Serial.print("+++");
    delay(1000);
    String gotString = readXbee(2000);
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
  Serial.print(whatToSend);
  do{
    maxIterations--;
    String gotString = readXbee(2000);
    if(strlen(gotString.c_str())>0){
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
  enterAT(3);
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
  digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  Serial.println( *whatToSend);
  analogWrite(11, 8);
  enterAT(3);
  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug(float* whatToSend){
  analogWrite(11, 8);
  enterAT(3);
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  Serial.println( *whatToSend);
  analogWrite(11, 8);  enterAT(3);
  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug(String* whatToSend, uint32_t Haddress, uint32_t Laddress){
  analogWrite(11, 8);
  enterAT(3);
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  Serial.print( *whatToSend);Serial.print(" | "); Serial.print(Haddress);Serial.print(" | "); Serial.println(Laddress);
  analogWrite(11, 8);  enterAT(3);
  sendAT("ATDH" + ATDH + "\n" , 3);
  sendAT("ATDL" + ATDL + "\n" , 3);
  sendAT("ATDH\r\n", 3);
  sendAT("ATDL\r\n", 3);
  sendAT("ATCN\r\n",3);
  digitalWrite(11, LOW);
}
void debug(float whatToSend){
  analogWrite(11, 8);
  enterAT(3);
  String ATDL =  sendAT ("ATDL\r\n", 3);
  String ATDH =  sendAT("ATDH\r\n",3);
  sendAT("ATDH0\r\n", 3);
  sendAT("ATDLDEB6\r\n",3);
    digitalWrite(11, HIGH);

  sendAT("ATCN\r\n",3);
  Serial.println(whatToSend);
  analogWrite(11, 8);  enterAT(3);
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
