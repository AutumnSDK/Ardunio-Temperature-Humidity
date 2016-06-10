#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <DHT.h> // this lib needs to be added in Arduino IDE folder
//----------------------------------------------------------
// ethernet config
/* //if do not DHCP
IPAddress ip(192, 168, 1, 111);
 uint8_t  mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
 uint8_t  __dns[] = { 8, 8, 8, 8 };
 uint8_t  gateway[] = { 192, 168, 1, 1 };
 uint8_t  subnet[] = { 255, 255, 255, 0 };
*/
uint8_t  mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //  DHCP
EthernetClient client;
uint16_t port = 80;    // port server
char server[] = "cloud.autumnapp.com";    // Check this is the url to your Autumn instance
//----------------------------------------------------------
//data send
#define TIME_PERIOD_SEND_DATA 10000
#define TIME_WAIT_ANSWER_SERVER 2000
//Add your sensor ELEMENT ID in each of these first is Humidity second Temperature
#define SENSOR_1 "--ELEMENT ID--"
#define SENSOR_2 "--ELEMENT ID--"
#define JSON_STR_1 " {\"data\":{\"type\": \"SensorData\",\"id\": null,\"attributes\": {\"value\": \""
#define JSON_STR_2 "\"},\"relationships\":{\"sensor\":{\"data\":{\"type\": \"Sensor\",\"id\": \""
#define JSON_STR_3 "\"}}}}}"
#define ADRR_SENSOR_1  "/api/external/v1/sensors/" SENSOR_1 "/sensordata/"
#define ADRR_SENSOR_2  "/api/external/v1/sensors/" SENSOR_2 "/sensordata/"
//Add your API KEY and ORGANISATION KEY once for each connection
#define API_KEY_SENSOR1 "--API KEY--"
#define EMAIL_HASH_SENSOR1 "--ORGANISATION KEY--"
#define API_KEY_SENSOR2 "--API KEY--"
#define EMAIL_HASH_SENSOR2 "--ORGANISATION KEY--"
        
//----------------------------------------------------------
// DHT22 config
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DHTPIN 3       // pin DHT22
DHT dht(DHTPIN, DHTTYPE);
float DHT22_Humidity = 0;
float DHT22_temperature = 0;
//----------------------------------------------------------
void setup() {
  
  Serial.begin(57600); //debug serial port
  Serial.println(F("arduino start")); //debug message
  
  while (!Serial) {}
  delay(4000); //delay for inicializate sensor DHT22
  }
//----------------------------------------------------------
void loop() {
    sensor_read();
    send_data_sensor();
    delay(TIME_PERIOD_SEND_DATA);
}
//----------------------------------------------------------
void send_data_sensor(){
   
//start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.print(F("Failed to configure Ethernet using DHCP" "\r\n" ));
    // Ethernet.begin(mac, ip, __dns, gateway, subnet); non DHCP
    Ethernet.begin(mac); //DHCP
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.print(F("connecting..." "\r\n" ));
//Sensor1
  uint16_t byte_string_json = ( (sizeof(JSON_STR_1)-1) + (sizeof(JSON_STR_2)-1) + (sizeof(JSON_STR_3)-1) + (sizeof(SENSOR_1)-1) + String(DHT22_temperature).length() );
  if (client.connect(server, port)) {
    Serial.println(F("connected"));
    
    client.print(F("POST " ADRR_SENSOR_1 " HTTP/1.1""\r\n")); //adress sensor 
    client.print(F("Host: "));
    client.print(server);    
    client.print(F("\r\n"));
   // client.print(F("Authorization: Basic " USR_PASS "\r\n"));    // old type   
    client.print(F("ORGANIZATION-ID: " EMAIL_HASH_SENSOR1 "\r\n"));
    client.print(F("ORGANIZATION-API-KEY: " API_KEY_SENSOR1 "\r\n"));
    client.print(F("User-Agent: Arduino_DHT22" "\r\n" ));
    client.print(F("Accept: */*" "\r\n" ));  
    client.print(F("Content-Type: application/vnd.api+json" "\r\n" )); 
    client.print(F("Content-Length: ")); 
    client.print(byte_string_json, DEC);    
    client.print(F("\r\n" "\r\n"));
    client.print(F(JSON_STR_1)); 
    client.print(DHT22_temperature);  // value sensor
    client.print(F(JSON_STR_2));
    client.print(F(SENSOR_1));    //name sensor  
    client.println(F(JSON_STR_3));  
    client.print(F("\r\n"));
//Sensor2
    byte_string_json = ( (sizeof(JSON_STR_1)-1) + (sizeof(JSON_STR_2)-1) + (sizeof(JSON_STR_3)-1) + (sizeof(SENSOR_2)-1) + String(DHT22_Humidity).length() );
    client.print(F("POST " ADRR_SENSOR_2 " HTTP/1.1""\r\n")); //adress sensor 
    client.print(F("Host: "));
    client.print(server);    
    client.print(F("\r\n"));
   // client.print(F("Authorization: Basic " USR_PASS "\r\n"));       // old type  
    client.print(F("ORGANIZATION-ID: " EMAIL_HASH_SENSOR2 "\r\n"));
    client.print(F("ORGANIZATION-API-KEY: " API_KEY_SENSOR2 "\r\n"));     
    client.print(F("User-Agent: Arduino_DHT22" "\r\n" ));
    client.print(F("Accept: */*" "\r\n" ));  
    client.print(F("Content-Type: application/vnd.api+json" "\r\n" )); 
    client.print(F("Content-Length: ")); 
    client.print(byte_string_json, DEC);    
    client.print(F("\r\n" "\r\n"));
    client.print(F(JSON_STR_1)); 
    client.print(DHT22_Humidity);  // value sensor
    client.print(F(JSON_STR_2));
    client.print(F(SENSOR_2));  //name sensor  
    client.println(F(JSON_STR_3));  
    client.print(F("\r\n"));
                     
  } 
  else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }  
  uint32_t lastTime =millis();
  lastTime += TIME_WAIT_ANSWER_SERVER;
  while(lastTime > (millis()) ){
    if(client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
  Serial.println(F("disconnecting."));
  client.stop();
}
//----------------------------------------------------------------
void sensor_read(){
    DHT22_Humidity = dht.readHumidity();
    DHT22_temperature = dht.readTemperature();
    if (isnan(DHT22_Humidity) || isnan(DHT22_temperature)) {
       Serial.println(F("Failed to read from DHT"));
    }
    else {
       Serial.println();
       Serial.print(F("H=: ")); 
       Serial.print(DHT22_Humidity);
       Serial.print(F(" %\t"));
       Serial.print(F("Temp.=: ")); 
       Serial.print(DHT22_temperature);
       Serial.println(F(" *C"));
       Serial.println();
    }
}
