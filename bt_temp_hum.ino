#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "Redmi"; // SSID / nome da rede WI-FI que deseja se conectar
const char* password = "12345678@"; // Senha da rede WI-FI que deseja se conectar

//const char* ssid = "CLARO_2G54F29D"; // SSID / nome da rede WI-FI que deseja se conectar
//const char* password = "6654F29D"; // Senha da rede WI-FI que deseja se conectar

//#define WIFI_AP "lika_0"
//#define WIFI_PASSWORD "RdD7InPv9"

#define TOKEN "WszAncqlnLgJ3ZwYRlPL"

// DHT
#define DHTPIN D2
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22 // DHT22

// GPIO where the DS18B20 is connected to
const int oneWireBus = 13;   

//char thingsboardServer[] = "192.168.1.132";
//char thingsboardServer[] = "150.161.248.143";
//char thingsboardServer[] = "150.161.248.211";
char thingsboardServer[] = "demo.thingsboard.io";


// Initialize the Ethernet client object
//WiFiEspClient espClient;
WiFiClient espClient;

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

PubSubClient client(espClient);

SoftwareSerial soft(2, 3); // RX, TX

int status = WL_IDLE_STATUS;
unsigned long lastSend;

void InitWiFi()
{
  Serial.println("Connecting to AP ...");
  // attempt to connect to WiFi network
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    //WiFi.begin(ssid, password);
    delay(500);
  }
  Serial.println("Connected to AP");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("NodeMCU", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }
}

void setup() {
  // initialize serial for debugging
  Serial.begin(115200);
  dht.begin();
  sensors.begin();
  InitWiFi();
  client.setServer( thingsboardServer, 1883 );
  //lastSend = 0;
}

void loop() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(ssid, password);
      Serial.println(status);
      delay(500);
    }
    Serial.println("Connected to AP");
  }

  if ( !client.connected() ) {
    reconnect();
  }

  if ( millis() - lastSend > 10000 ) { // Update and send only after 1 seconds
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }

  client.loop();
}

void getAndSendTemperatureAndHumidityData()
{
  Serial.println("Collecting temperature data.");
  Serial.println("Collecting temperatureDS data.");
  Serial.println("Collecting humidity data.");

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  sensors.requestTemperatures(); 
  float tDS = sensors.getTempCByIndex(0);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(tDS)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" % ");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" ºC ");
  Serial.print("Temperature DS: ");
  Serial.print(tDS);
  Serial.println(" ºC ");

  String temperature = String(t);
  String humidity = String(h);
  String temperatureDS = String(tDS);


  // Just debug messages
  Serial.print( "Sending temperature, temperatureDS and humidity: [" );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( temperatureDS ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]   -> " );

  // Prepare a JSON payload string
  String payload = "{";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"temperatureDS\":"; payload += temperatureDS; payload += ",";
  payload += "\"humidity\":"; payload += humidity;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );
}

