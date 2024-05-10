/* ---------------------- INCLUDE ---------------- */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <stdio.h>

/* ---------------------- DEFINE ---------------- */
#define MAX_NAME_LENGTH 20

/* ---------------------- CLASS OBJECTS ---------------- */
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
StaticJsonDocument<200> doc;
SoftwareSerial portOne(D1, D2);

/* ---------------------- GLOBAL VARIABLES ---------------- */
const char *ssid = "POCO F2 Pro";
const char *password = "haslo123";

/*
* EEPROM ADDRESSES:
* 0-20 bytes = each byte for string length of each measurments
* 21 = byte for save count
* 22 = byte for last saved address
*
*/

typedef enum
{
  IDLE,
  MEASURE
}app_state_e;


typedef struct 
{
  const uint16_t MAX_MEMORY_SIZE;             /* number of memory size in bytes, max is 4kb */
  const uint8_t MAX_SAVE_COUNT;
  char name[MAX_NAME_LENGTH];
  int measurment;
}eeprom_s;


typedef struct
{
  app_state_e state;
}app_state_s;


eeprom_s eeprom =
{
  .MAX_MEMORY_SIZE = 600,
  .MAX_SAVE_COUNT = 7
};

app_state_s measure_state;
/* ---------------------- FUNCTION PROTOTYPES ---------------- */
void init_WiFi();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void load_HTML();
void load_CSS();
void load_JS();
void eeprom_load(eeprom_s *eeprom);
void eeprom_clear();
void eeprom_save(eeprom_s *eeprom);
void eeprom_zero();
int get_measurment();
void send_measurment(int value);


/* ---------------------- SETUP FUNCTION ---------------- */
void setup()
{
  Serial.begin(115200);
  delay(50);
  LittleFS.begin();
  delay(50);
  delay(50);
  EEPROM.begin(512); 
  delay(50);

  init_WiFi();
  delay(50);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.on( "/" , load_HTML );
  server.on( "/style.css", load_CSS );
  server.on( "/script.js", load_JS );
  server.begin();
  delay(5);


  //eeprom_zero();
  delay(5);
  Serial.println("Server started");
  portOne.begin(9600);
}

/* ---------------------- LOOP FUNCTION ---------------- */
void loop()
{
  server.handleClient();
  webSocket.loop();
  delay(10);
  if (measure_state.state == MEASURE)
  {
    send_measurment(get_measurment());
  }
}


/* ---------------------- FUNCTION DEFINITIONS ---------------- */
void init_WiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
  Serial.print("IP addresss:\t");
  Serial.println(WiFi.localIP());
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  switch (type)
  {
    case WStype_DISCONNECTED:
      {
        Serial.printf("[%u] Disconnected!\n", num);
      }
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        measure_state.state = IDLE;
        eeprom_load(&eeprom);
      }
      break;

    case WStype_TEXT:
      {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        if (strcmp((char*)payload, "clear") == 0)
        {
          eeprom_clear();
          Serial.println("Clear");
        }
        else if (strcmp((char*)payload, "start") == 0)
        {
          measure_state.state = MEASURE;
          Serial.println("Start");
        }
        else
        {
          DeserializationError error = deserializeJson(doc, (char*)payload);
          if (error) 
          {
            return;
          }
          eeprom_save(&eeprom);
          Serial.println("Save Data");
        }
      }
      break;
  }
}


void load_HTML()
{
  File f = LittleFS.open("/index.html", "r");
  server.streamFile(f, "text/html");
  f.close();
}


void load_CSS()
{
  File f = LittleFS.open("/style.css", "r");
  server.streamFile(f, "text/css");
  f.close();
}


void load_JS()
{
  File f = LittleFS.open("/script.js", "r");
  server.streamFile(f, "application/javascript");
  f.close();
}


/*
*   Every measurment contains:
*   1 byte for String length
*   20* bytes for Name *- depends on #define MAX_NAME_LENGTH
*   4 bytes for Lux value
*     
*/
void eeprom_save(eeprom_s *eeprom) 
{
  uint8_t saves_count = 0;
  uint8_t starting_addr = 0;
  uint8_t name_length = 0;
  const char* name;

  name = doc["name"];
  strcpy(eeprom->name, name);
  eeprom->measurment = doc["val"];

  /* Get current save number and increment */
  EEPROM.get(0, saves_count); 
  delay(10);
  saves_count++;
  EEPROM.put(0, saves_count);
  delay(10);

  /* Get starting address and save lenght of string NAME */
  starting_addr = 1 + (MAX_NAME_LENGTH * (saves_count - 1));
  name_length = (uint8_t)strlen(eeprom->name);
  EEPROM.put(starting_addr, name_length);
  delay(10);  
  starting_addr++;

  /* Save string NAME */
  for (int i = 0; i < MAX_NAME_LENGTH; i++)
  {
    EEPROM.put(starting_addr, eeprom->name[i]);
    delay(10);
    starting_addr++;
  }

  /* Save data */
  EEPROM.put(starting_addr, eeprom->measurment);
  delay(10);
  EEPROM.commit();
  delay(10);
}


void eeprom_load(eeprom_s *eeprom) 
{
  uint8_t saves_count = 0;
  uint8_t name_length = 0;
  uint8_t starting_addr = 0;
  char name[20];
  char measurment_string[5];

  EEPROM.get(0, saves_count); 
  delay(10);
  if (saves_count > 0)
  {
    for(uint8_t i = 0; i < saves_count; i++)
    {
      starting_addr = 1 + (MAX_NAME_LENGTH * i);
      EEPROM.get(starting_addr, name_length);
      delay(10);
      starting_addr++;
      for(uint8_t j = 0; j < name_length; j++)
        {
          EEPROM.get(starting_addr, name[j]);
          delay(10);
          starting_addr++; 
        }
      starting_addr = 1 + (MAX_NAME_LENGTH * i);
      starting_addr += 21;
      EEPROM.get(starting_addr, eeprom->measurment);
      delay(10);
      EEPROM.commit();
      delay(10);

      JsonObject object = doc.to<JsonObject>();
      object["nazwa"] = name;
      sprintf(measurment_string, "%d", eeprom->measurment);
      object["val"] = measurment_string;
      String StringJSONObject = "";
      serializeJson(doc, StringJSONObject);
      webSocket.broadcastTXT(StringJSONObject);
    }
  }
}


void eeprom_clear() 
{
  uint8_t saves_count = 0;
  uint8_t starting_addr = 0;

  EEPROM.get(0, saves_count);
  delay(10);
  if (saves_count > 0)
  {
    starting_addr = 1 + (25 * saves_count);
    starting_addr -= 25;
    saves_count--;
    EEPROM.put(0, saves_count); 
    delay(10);
    EEPROM.commit();
    delay(10);
  }
}


void eeprom_zero()
{
  for(int i = 0; i<600; i++)
  {
    EEPROM.put(i, 0);
    delay(10);
  }
  EEPROM.commit();
}


void send_measurment(int value)
{ 
  String lux;
  lux = String(value);
  webSocket.broadcastTXT(lux);
}


int get_measurment()
{
  static uint8_t data[2];
  float lux_value = 0;

  if (portOne.available() >= 2 )
  {
    data[0] = portOne.read();
    data[1] = portOne.read();
  }
  lux_value = (data[0] << 8) | (data[1]);
  lux_value = (lux_value / 1.2);
  return (int)lux_value;
}
