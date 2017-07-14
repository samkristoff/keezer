#include <OneWire.h>

//Configuration
#define TEMP_PROBE_CHAN 10
#define MAIN_TEMP_PROBE_ADDR 0x28FFD723A01646E0

#define COOL_CHAN 13
#define KEEZER_TEMP_MAX 39
#define KEEZER_TEMP_MIN 37

//Global Variables
//Initialize oneWire on DIO 10 to read from DS18B20 temperature sensors
OneWire ow(TEMP_PROBE_CHAN);
byte mainTempProbeAddr[8] = {0x28, 0xFF, 0xD7, 0x23, 0xA0, 0x16, 0x04, 0x6E};

void setup() {
  Serial.begin(9600);
  
  //Set PowerSwitch Tail DIO pin as output.
  pinMode(COOL_CHAN, OUTPUT);

  //Print all OneWire device addresses
  printOneWireDeviceAddresses();
}

void loop() {  
  float temp = readTempProbe(mainTempProbeAddr);
  //Check to make sure temp is valid (If temp read fails it will return -1000)
  if(temp > -1000)
  {
     updateCoolingMode(temp);    
  } 
  delay(1000);
}

//Read temperature from the DS18B20
float readTempProbe(byte* address)
{  
  //Initialize communication
  ow.reset();
  ow.select(address);
  ow.write(0x44, 1);
  delay(1000); 
  ow.reset(); 
  ow.select(address);    
  ow.write(0xBE);  

  //Read Data
  byte data[12];
  for(int i = 0; i < 9; i++) {
    data[i] = ow.read();   
  }

  if(ow.crc8(data, 8) != data[8])
  {
    Serial.println("Failed CRC");
    return -1000;
  }

  //Convert data to temp
  int16_t raw = (data[1] << 8) | data[0];    
  float fahrenheit = (float)raw * 0.1125 + 32.0;
  Serial.print(fahrenheit);
  Serial.println(" F");
  return fahrenheit;
}

//Update the refridgeration state based on the specified current temperature in F.  This function returns true when cooling is enabled and false otherwise.
bool updateCoolingMode(float tempF)
{
  //Display Freezer Mode
  if (tempF > KEEZER_TEMP_MAX)
  {
    Serial.println("Keezer Cooling");
    digitalWrite(COOL_CHAN, HIGH);
    return true;
  }
  else if (tempF < KEEZER_TEMP_MIN)
  {
    Serial.println("Keezer Idle");
    digitalWrite(COOL_CHAN, LOW);
    return false;
  }
  return false;
}

//Print All OneWire devices addresses on the bus.
void printOneWireDeviceAddresses()
{
  byte addr[8];
  while(ow.search(addr)) 
  {
    Serial.print("Address - 0x");
    for (int i = 0; i < 8; i++) 
    {   
      if(addr[i] > 0xF) {
        Serial.print(addr[i], HEX);  
      }
      else {
        Serial.print("0");  
        Serial.print(addr[i], HEX);  
      }      
    }
  }
  Serial.println();
}
