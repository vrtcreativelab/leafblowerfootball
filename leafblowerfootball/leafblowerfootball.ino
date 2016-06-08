#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "games.sambal.be";
//char server[] = "52.30.164.191"; //games.sambal.be
//char server[] = "52.51.99.236"; // speeltuin.sambal.be

/*
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(8, 8, 8, 8);
*/

EthernetClient client;

unsigned long lastConnectionTime = 0;
const unsigned long postingInterval = 1L * 1000L;

const int pinStatusLed = 13;
const int pinOnOffSwitch = 2;
const int pinOnOffLed = 3;
const int pinA = 7;
const int pinB = 8;

int valueA = 0;
int valueB = 0;
String currentLine = "";

void setup() {
  pinMode(pinStatusLed, OUTPUT);
  pinMode(pinOnOffLed, OUTPUT);
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinOnOffSwitch,INPUT_PULLUP);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  delay(500);
  
  // start the Ethernet connection:
  Ethernet.begin(mac);
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  if (digitalRead(2)) {
    digitalWrite(pinOnOffLed, HIGH);
    if (client.available()) {
      char inChar = client.read();
      currentLine += inChar;

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      }
    }
  
    if (millis() - lastConnectionTime > postingInterval) {
      digitalWrite(pinStatusLed, HIGH);
      httpRequest();
      digitalWrite(pinStatusLed, LOW);
    }

    digitalWrite(pinA, valueA);
    digitalWrite(pinB, valueB);
  } else {
    digitalWrite(pinOnOffLed, LOW);
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
    client.stop();
    lastConnectionTime = 0;
  }
}

void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the Ethernet shield
  client.stop();
  parseValues(currentLine);
  currentLine = "";

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /test.php HTTP/1.1");
    client.println("Host: games.sambal.be");
    client.println("User-Agent: bladblazertje");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void parseValues(String data)
{
  Serial.println(data);
  String temp = getValue(data, ':', 1);
  //Serial.println(temp);
  valueA = temp.toInt();
  temp = getValue(data, ':', 3);
  valueB = temp.toInt();
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
