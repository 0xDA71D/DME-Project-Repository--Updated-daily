sendtypedef struct GPSPacket{  //defines a structure 
  float Latitude;
  float Longitude;
  uint32_t magicNumber;
  uint32_t sourceHAddress; //ATSH
  uint32_t sourceLAddress; //ATSL
}GPSPacket;
GPSPacket GPS;

char* finalPacket;
void setup(){
  Serial.begin(9600);
}

void loop(){
  GPS.Latitude = 40.836857;
GPS.Longitude=-73.296373;
GPS.magicNumber = 32343;
GPS.sourceHAddress = 123456;
GPS.sourceLAddress = 7890;

char* payload=(char*)malloc(25);  
payload = (char*)&GPS;
processString(payload, sizeof(GPS));
Serial.print(payload);
delay(15000);
}
char * append_strings(const char * old, const char * newString)
{
    // find the size of the string to allocate
    size_t len = strlen(old) + strlen(newString) + 1;

    // allocate a pointer to the new string
    char *out =(char*) malloc(len);

    // concat both strings and return
    sprintf(out, "%s%s", old, newString);

    return out;
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
