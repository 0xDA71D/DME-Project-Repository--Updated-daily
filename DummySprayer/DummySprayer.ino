typedef struct GPSPacket{  //defines a structure 
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
  GPS.Latitude = 12.3456;
GPS.Longitude=65.4321;
GPS.magicNumber = 123123;
GPS.sourceHAddress = 1234;
GPS.sourceLAddress = 5678;
  //memcpy(finalPacket, &GPS, sizeof(GPSPacket));
  finalPacket = (char*)&GPS;
  Serial.print(finalPacket);
  /*for(int i = 0; i <strlen(finalPacket); i++){
    Serial.println((uint8_t)finalPacket[i]); 
    
    
    
  }*/
  
  
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
 
