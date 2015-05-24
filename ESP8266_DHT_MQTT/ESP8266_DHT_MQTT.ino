#include <PubSubClient.h>
#include <ESP8266WiFi.h>

const char* ssid = "OpenWrt_NAT_500GP.101";
const char* password = "activegateway";
const byte DATA_PIN = 2;

byte server[] = {198, 41, 30, 241};
byte data[5];

char buf_temp[20];
char buf_hud[20];
String Str;
char chararray[20];
// Update these with values suitable for your network.

void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
}

void setup() {
  int wifiWaiting = 0;
  pinMode( DATA_PIN, INPUT );
  digitalWrite( DATA_PIN, HIGH ); // enable internal pull-up
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  int waitSecs = 15;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    Serial.print(wifiWaiting);
    wifiWaiting++;

    if (wifiWaiting > 15 * waitSecs) {
      abort();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (client.connect("arduinoClient")) {
    //if (client.connect("mqtt_arduino","sqgjiqvd","LZ3JY63V9XlW"))
    //client.publish("outTopic","hello world");
    client.subscribe("cmmc/chuan/in");
    Serial.println("Authen Passed");
  }
  else {
    Serial.println("AUTHEN FAILED");
  }
}

void loop()
{ client.loop();
  ///////////////// DHT Operation /////////////
  int count = 0;
  byte i = 0, j = 0;
  byte new_state, state = HIGH;

  for (byte x = 0; x < 5; x++) {
    data[x] = 0; // clear data buffer
  }
  pinMode( DATA_PIN, OUTPUT ); // change direction to output
  digitalWrite( DATA_PIN, LOW ); // output low (send the start bit)
  delayMicroseconds( 1000 );
  digitalWrite( DATA_PIN, HIGH ); // output high
  delayMicroseconds( 40 );
  pinMode( DATA_PIN, INPUT ); // change direction to input
  digitalWrite( DATA_PIN, HIGH ); // enable internal pull-up

  // AM2302 will send a response signal of 40-bit data that
  // represent the relative humidity and temperature information to MCU.
  unsigned long t1, t0 = micros();
  while (1) {
    new_state = digitalRead( DATA_PIN );
    if ( state != new_state ) {
      t1 = micros();
      if ( (state == HIGH) && (i > 2) ) {
        byte b = ( (t1 - t0) > 40 ) ?  1 : 0;
        data[j / 8] <<= 1;
        data[j / 8] |= b;
        j++;
      }
      i++;
      state = new_state;
      t0 = t1;
      count = 0;
    } else {
      count++;
      if ( count > 1000 ) // timeout
        break;
    }
  }

  byte check_sum = 0x00;
  for (byte x = 0; x < 4; x++) {
    check_sum += data[x];
  }
  if ( check_sum != data[4] ) {
    Serial.println( "CHECKSUM error" );
  } else {
    Serial.print( ((data[0] << 8) | data[1]) / 10.0 );
    Serial.print( "%RH, " );
    Serial.print( ((data[2] << 8) | data[3]) / 10.0 );
    Serial.println( " C" );
  }
  float temp = ((data[2] << 8) | data[3]) / 10.0;
  float humi = ((data[0] << 8) | data[1]) / 10.0;
  String a = String(temp);
  
  char *nat = const_cast<char*>(a.c_str());
  Serial.println(nat);

  /////////////// Pubsub //////////////////////
  if (client.connected()) {

    client.publish("cmmc/chuan/out",nat);
    Serial.println("Printed");
  }
  else
  {
    Serial.println("Connect have a problem");
    client.connect("arduinoClient");
    //client.subscribe("cmmc/chuan/in");
  }
  delay(1000);
}


