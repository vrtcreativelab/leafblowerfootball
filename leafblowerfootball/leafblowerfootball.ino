#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x01 };
IPAddress ip(192,168,1,101);
IPAddress myDns(8, 8, 8, 8);

// initialize the library instance:
EthernetClient client;

const int requestInterval = 2000; 

char serverName[] = "slurper.speeltuin.sambal.be";
//char serverName[] = "52.30.164.191"; //games.sambal.be
//char serverName[] = "52.51.99.236"; // speeltuin.sambal.be

boolean requested;                   
long lastAttemptTime = 0;          

String currentLine = "";     
String statusText = "";              
boolean readingStatus = false;    

const int pinStatusLed = 13;
const int pinOnOffSwitch = 2;
const int pinOnOffLed = 3;
const int pinA = 7;
const int pinB = 8;

int valueA = 0;
int valueB = 0;

void setup() {
  pinMode(pinStatusLed, OUTPUT);
  pinMode(pinOnOffLed, OUTPUT);
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinOnOffSwitch,INPUT_PULLUP);
  
  // reserve space for the strings:
  currentLine.reserve(256);
  statusText.reserve(150);

  // initialize serial:
  Serial.begin(9600);
  
  // attempt a DHCP connection:
  if (!Ethernet.begin(mac)) {
    // if DHCP fails, start with a hard-coded address:
    Ethernet.begin(mac, ip, myDns);
  }

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());

  delay(500);
  
  // connect to server:
  connectToServer();
}

void loop()
{
  if (client.connected()) {
    if (client.available()) {
      // read incoming bytes:
      char inChar = client.read();

      // add incoming byte to end of line:
      currentLine += inChar; 

      // if you get a newline, clear the line:
      if (inChar == '\n') {
        currentLine = "";
      }
      
      // if the current line ends with <text>, it will
      // be followed by the tweet:
      if ( currentLine.endsWith("<")) {
        // tweet is beginning. Clear the tweet string:
        readingStatus = true; 
        statusText = "";
      }
      
      // if you're currently reading the bytes of a tweet,
      // add them to the tweet String:
      if (readingStatus) {
        if (inChar != '>') {
          statusText += inChar;
        } 
        else {
          // if you got a "<" character,
          // you've reached the end of the tweet:
          readingStatus = false;
          Serial.println(statusText);
          parseValues(statusText);
          
          // close the connection to the server:
          client.stop(); 
        }
      }
    }   
  } else if (millis() - lastAttemptTime > requestInterval) {
    connectToServer();
  }

  if (digitalRead(pinOnOffSwitch)) {
    digitalWrite(pinOnOffLed, HIGH);
    digitalWrite(pinA, valueA);
    digitalWrite(pinB, valueB);
  } else {
    digitalWrite(pinOnOffLed, LOW);
    digitalWrite(pinA, LOW);
    digitalWrite(pinB, LOW);
  }
}

void connectToServer() {
  // attempt to connect, and wait a millisecond:
  Serial.println("connecting to server...");
  if (client.connect(serverName, 80)) {
    //Serial.println("making HTTP request...");
  // make HTTP GET request:
    client.println("GET /api/bladblazer/10 HTTP/1.1");
    client.println("HOST: slurper.speeltuin.sambal.be");
    client.println();
  }
  // note the time of this connect attempt:
  lastAttemptTime = millis();
}   

void parseValues(String data) {
  data.replace("<", "");
  String temp = getValue(data, ':', 1);
  valueA = temp.toInt();
  temp = getValue(data, ':', 3);
  valueB = temp.toInt();
}

String getValue(String data, char separator, int index) {
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

