/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>

// FOR SPI
#define UICC_RESET 5

#define LEN 0
#define CLA 1
#define INS 2
#define P1  3
#define P2  4
#define P3  5
#define DATA_OFFSET 6

#define DUMMY 0

#define CS_LOW
#define CS_HIGH

#define AES_BLOCKSIZE 16
#define INS_ENCRYPTION 0
#define INS_DECRYPTION 1

//#define SERIAL_DEBUG

unsigned char g_apduTx[] = {21, 0x01, INS_DECRYPTION, 0x03, 0x04, 16,
                          0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                          0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89};
unsigned char g_rxData[0x20];

// Update these with values suitable for your network.
const char* ssid = "dlink";
const char* password = "micromax";
const char* mqtt_server = "192.168.0.127";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


int ledState = HIGH;

unsigned char setTemp = 0xFF;
unsigned char prevTemp = 0x00;

unsigned long previousMillis = 0;
const long interval = 1000;

void performUiccReset(void){
  digitalWrite(UICC_RESET, 0);
  delay(100);
  digitalWrite(UICC_RESET, 1);
}

void setup_forSPI() {
  // put your setup code here, to run once:
  pinMode(UICC_RESET, OUTPUT); 

  //spi.setup(1, spi.MASTER, spi.CPOL_HIGH, spi.CPHA_HIGH, 8, 20, spi.FULLDUPLEX);
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setFrequency(10000);
  
  performUiccReset();
}

/**
 * Receives data from Slave.
 * Fist loop waits till slave sends [LEN].
 * Second loop waits till [LEN] bytes are received from slave.
 * The second loop considers that, slave is ready with data, and each SPI transfer will return valid data.
 * @param resp Buffer to hold response.
 * @param len Pointer to return response length.
 */
void rx_spi(unsigned char* resp, unsigned int* len) {
  unsigned int rxBytes = 0;
  
  // receiving 00 from slave is indication for not ready signal.
  do {
    delay(10);
    *len = SPI.transfer(DUMMY);
  } while(*len == DUMMY);

#ifdef SERIAL_DEBUG    
  // Debug code
  Serial.print("-");
  Serial.print(*len);
  Serial.print(" ");
#endif

  // *len has length of data being sent from slave.
  do {
    unsigned char retVal = SPI.transfer(DUMMY);
    resp[rxBytes] = retVal;
    rxBytes++;
  } while(rxBytes != *len);
}

/**
 * This function sends APDU and gets response from slave.
 * @param apdu APDU to be sent to slave in [LEN][APDU] format.
 * @param rxLen pointer which holds response length.
 * @param resp buffer which holds response.
 */
void send_iso_case_4(unsigned char* apdu, unsigned int* rxLen, unsigned char* resp){
  unsigned int i;
  CS_LOW;

  // Loop to transfer [LEN][APDU]
  for(i = 0; i < g_apduTx[0]+1; i++){
    SPI.transfer(g_apduTx[i]);
#ifdef SERIAL_DEBUG    
    Serial.print(g_apduTx[i]);
    Serial.print("+");
#endif
  }

  delay(10);

  // Function to get [LEN][Response]
  rx_spi(g_rxData, rxLen);

#ifdef SERIAL_DEBUG 
  // Debug print
  for(i = 0; i < *rxLen; i++){
    Serial.print(resp[i], HEX);
    Serial.print(" ");
  }
#endif  

  CS_HIGH;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  setup_forSPI();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
  unsigned int rxLen;
  unsigned char currTemp = 0;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  if(memcmp((const void *)topic, (const void *)"Temperature", 11) == 0) {   
    if (AES_BLOCKSIZE == length) {
      for (int i = 0; i < length; i++) {
  #ifdef SERIAL_DEBUG 
        Serial.print((char)payload[i], HEX);
  #endif    
        g_apduTx[DATA_OFFSET + i] = payload[i];
      }
      send_iso_case_4(g_apduTx, &rxLen, g_rxData);
      Serial.print(g_rxData[0] - '0');
      Serial.println(g_rxData[1] - '0');
      currTemp = (g_rxData[0] - '0') << 4;
      currTemp |= (g_rxData[1] - '0');

      if ((setTemp != 0xFF) && (currTemp != prevTemp)) {
        if(currTemp > setTemp) {
          Serial.println("Turn ON the Relay");
        } else {
          Serial.println("Turn OFF the Relay");        
        }
      }
      prevTemp = currTemp;
    } else {
      Serial.println("Incorrect Msg Length!!!");           
    }
  } else if (memcmp((const void *)topic, (const void *)"SetTemp", 7) == 0) {
    Serial.print((char)(payload[0] - '0'), HEX);
    Serial.print((char)(payload[1] - '0'), HEX);
    Serial.println();
    setTemp = (payload[0] - '0') << 4;
    setTemp |= (payload[1] - '0');
    if(prevTemp > setTemp) {
      Serial.println("Turn ON the Relay");
    } else {
      Serial.println("Turn OFF the Relay");        
    }
  } else {
    Serial.println();
  }
}

void toggleInbuiltLed() {
  if (ledState == LOW)
    ledState = HIGH;  // Note that this switches the LED *off*
  else
    ledState = LOW;   // Note that this switches the LED *on*

  digitalWrite(LED_BUILTIN, ledState);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266TempRx")) {
      Serial.println("connected");
      client.subscribe("Temperature");
      client.subscribe("SetTemp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (!client.connected())
    reconnect();

  client.loop();

  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    toggleInbuiltLed();
  }
}
