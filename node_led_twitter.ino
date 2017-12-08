#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <TextFinder.h>
#include <QueueList.h>

#include <LedControl.h> 
#include <SPI.h>

uint8_t led = 13;

//Setup WiFI
const char* ssid = "<SSID>";
const char* password = "";

//To check if we have internet
const char* iCheckHost = "google.com";


//Default IP, if displayed on webpage means we didn't get an IP
String publicIP = "0.0.0.0"; 
WiFiClient client;
ESP8266WebServer server(80);

//Twitter Stuff and LED
String TwitterHashtag = "makerfaire"; //change this to your own twitter hashtag, or follow arduino ;-)
String hashTag = "#makerfaire"; //For removing from tweet
String hashTag2 = "#makerfaire"; //With capital
char tweet[140];
char serverName[] = "<your own server runnnig twitter PHP script>";  // twitter URL
int timeOut = 20; //Timeout in seconds when connecting to the internet

//Default message
char msg[140] = "Loading...";
int msgsize =  strlen(msg);
char inString[144]="";
int inCount=0;
int binLED[192];

//For tracking position of LED Matrix scrolling of text
// The character set courtesy of cosmicvoid.  It is rowwise
int curcharix = 0;
int curcharbit = 0;
int curcharixsave = 0;
int curcharbitsave = 0;
int curcharixsave2 = 0;
int curcharbitsave2 = 0;
char curchar;
int i,j,k;

unsigned long delayTwitter = 300000; //Time in milliseconds to connect to Twitter
unsigned long delayTime = millis();
unsigned long displayLastTime = millis();

const unsigned int tweetHistoryTime = 20000; //Time in hours 10000 being 1 hour Note: Doesn't account for going back a day, will show too many tweets

int tweetCount = 0; //Keep track of numer of tweets received during last connection
int displayCycleTime = 10000; //Time betweek LED messages refresh, recalculated each Twitter connection
const int minDisplayTime = 6000; //Min time a tweet will be displayed on LED, if rearched, not all tweets may be displayed, timing could be adjusted.

bool firstBoot = true; 

//LED Matrix Config
const int devCount = 8;
bool scrollFlag = true;
int scrollSpeed = 50;
int brightness = 15;
bool ledShutdown = false;
const int NUMSAMPLES = 10;

//Queue for Twitter Messages
QueueList <String> tweetMsg;



//LED Font Data
byte Font8x5[104*8] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x02, 0x00,
        0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A, 0x00,
        0x0E, 0x15, 0x05, 0x0E, 0x14, 0x15, 0x0E, 0x00,
        0x13, 0x13, 0x08, 0x04, 0x02, 0x19, 0x19, 0x00,
        0x06, 0x09, 0x05, 0x02, 0x15, 0x09, 0x16, 0x00,
        0x02, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x04, 0x00,
        0x01, 0x02, 0x04, 0x04, 0x04, 0x02, 0x01, 0x00,
        0x00, 0x0A, 0x15, 0x0E, 0x15, 0x0A, 0x00, 0x00,
        0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x01,
        0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x10, 0x10, 0x08, 0x04, 0x02, 0x01, 0x01, 0x00,
        0x0E, 0x11, 0x19, 0x15, 0x13, 0x11, 0x0E, 0x00,
        0x04, 0x06, 0x04, 0x04, 0x04, 0x04, 0x0E, 0x00,
        0x0E, 0x11, 0x10, 0x0C, 0x02, 0x01, 0x1F, 0x00,
        0x0E, 0x11, 0x10, 0x0C, 0x10, 0x11, 0x0E, 0x00,
        0x08, 0x0C, 0x0A, 0x09, 0x1F, 0x08, 0x08, 0x00,
        0x1F, 0x01, 0x01, 0x0E, 0x10, 0x11, 0x0E, 0x00,
        0x0C, 0x02, 0x01, 0x0F, 0x11, 0x11, 0x0E, 0x00,
        0x1F, 0x10, 0x08, 0x04, 0x02, 0x02, 0x02, 0x00,
        0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E, 0x00,
        0x0E, 0x11, 0x11, 0x1E, 0x10, 0x08, 0x06, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x02, 0x01,
        0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08, 0x00,
        0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00,
        0x01, 0x02, 0x04, 0x08, 0x04, 0x02, 0x01, 0x00,
        0x0E, 0x11, 0x10, 0x08, 0x04, 0x00, 0x04, 0x00,
        0x0E, 0x11, 0x1D, 0x15, 0x0D, 0x01, 0x1E, 0x00,
        0x04, 0x0A, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x00,
        0x0F, 0x11, 0x11, 0x0F, 0x11, 0x11, 0x0F, 0x00,
        0x0E, 0x11, 0x01, 0x01, 0x01, 0x11, 0x0E, 0x00,
        0x07, 0x09, 0x11, 0x11, 0x11, 0x09, 0x07, 0x00,
        0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x1F, 0x00,
        0x1F, 0x01, 0x01, 0x0F, 0x01, 0x01, 0x01, 0x00,
        0x0E, 0x11, 0x01, 0x0D, 0x11, 0x19, 0x16, 0x00,
        0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11, 0x00,
        0x07, 0x02, 0x02, 0x02, 0x02, 0x02, 0x07, 0x00,
        0x1C, 0x08, 0x08, 0x08, 0x08, 0x09, 0x06, 0x00,
        0x11, 0x09, 0x05, 0x03, 0x05, 0x09, 0x11, 0x00,
        0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0F, 0x00,
        0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11, 0x00,
        0x11, 0x13, 0x13, 0x15, 0x19, 0x19, 0x11, 0x00,
        0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
        0x0F, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00,
        0x0E, 0x11, 0x11, 0x11, 0x15, 0x09, 0x16, 0x00,
        0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00,
        0x0E, 0x11, 0x01, 0x0E, 0x10, 0x11, 0x0E, 0x00,
        0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00,
        0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00,
        0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04, 0x00,
        0x41, 0x41, 0x41, 0x49, 0x2A, 0x2A, 0x14, 0x00,
        0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 0x00,
        0x11, 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x00,
        0x1F, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1F, 0x00,
        0x07, 0x01, 0x01, 0x01, 0x01, 0x01, 0x07, 0x00,
        0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00,
        0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x00,
        0x00, 0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00,
        0x01, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x06, 0x08, 0x0E, 0x09, 0x0E, 0x00,
        0x01, 0x01, 0x0D, 0x13, 0x11, 0x13, 0x0D, 0x00,
        0x00, 0x00, 0x06, 0x09, 0x01, 0x09, 0x06, 0x00,
        0x10, 0x10, 0x16, 0x19, 0x11, 0x19, 0x16, 0x00,
        0x00, 0x00, 0x06, 0x09, 0x07, 0x01, 0x0E, 0x00,
        0x04, 0x0A, 0x02, 0x07, 0x02, 0x02, 0x02, 0x00,
        0x00, 0x00, 0x06, 0x09, 0x09, 0x06, 0x08, 0x07,
        0x01, 0x01, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,
        0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00,
        0x04, 0x00, 0x06, 0x04, 0x04, 0x04, 0x04, 0x03,
        0x01, 0x01, 0x09, 0x05, 0x03, 0x05, 0x09, 0x00,
        0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
        0x00, 0x00, 0x15, 0x2B, 0x29, 0x29, 0x29, 0x00,
        0x00, 0x00, 0x0D, 0x13, 0x11, 0x11, 0x11, 0x00,
        0x00, 0x00, 0x06, 0x09, 0x09, 0x09, 0x06, 0x00,
        0x00, 0x00, 0x0D, 0x13, 0x13, 0x0D, 0x01, 0x01,
        0x00, 0x00, 0x16, 0x19, 0x19, 0x16, 0x10, 0x10,
        0x00, 0x00, 0x0D, 0x13, 0x01, 0x01, 0x01, 0x00,
        0x00, 0x00, 0x0E, 0x01, 0x06, 0x08, 0x07, 0x00,
        0x00, 0x02, 0x07, 0x02, 0x02, 0x02, 0x04, 0x00,
        0x00, 0x00, 0x11, 0x11, 0x11, 0x19, 0x16, 0x00,
        0x00, 0x00, 0x11, 0x11, 0x11, 0x0A, 0x04, 0x00,
        0x00, 0x00, 0x11, 0x11, 0x15, 0x15, 0x0A, 0x00,
        0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00,
        0x00, 0x00, 0x09, 0x09, 0x09, 0x0E, 0x08, 0x06,
        0x00, 0x00, 0x0F, 0x08, 0x06, 0x01, 0x0F, 0x00,
        0x04, 0x02, 0x02, 0x01, 0x02, 0x02, 0x04, 0x00,
        0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00,
        0x01, 0x02, 0x02, 0x04, 0x02, 0x02, 0x01, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x1C, 0x2A, 0x49, 0x49, 0x41, 0x22, 0x1C, 0x00,
        0x1C, 0x22, 0x51, 0x49, 0x41, 0x22, 0x1C, 0x00,
        0x1C, 0x22, 0x41, 0x79, 0x41, 0x22, 0x1C, 0x00,
        0x1C, 0x22, 0x41, 0x49, 0x51, 0x22, 0x1C, 0x00,
        0x1C, 0x22, 0x41, 0x49, 0x49, 0x2A, 0x1C, 0x00,
        0x1C, 0x22, 0x41, 0x49, 0x45, 0x22, 0x1C, 0x00,
        0x1C, 0x22, 0x41, 0x4F, 0x41, 0x22, 0x1C, 0x00,
        0x1C, 0x22, 0x45, 0x49, 0x41, 0x22, 0x1C, 0x00,
};


byte lentbl_S[104] =
{
        2, 2, 3, 5, 5, 5, 5, 2, 
        3, 3, 5, 5, 2, 5, 1, 5, 
        5, 4, 5, 5, 5, 5, 5, 5, 
        5, 5, 1, 2, 4, 4, 4, 5, 
        5, 5, 5, 5, 5, 5, 5, 5, 
        5, 3, 5, 5, 4, 5, 5, 5, 
        5, 5, 5, 5, 5, 5, 5, 7, 
        5, 5, 5, 3, 5, 3, 5, 5, 
        2, 4, 5, 4, 5, 4, 4, 4, 
        5, 2, 3, 4, 2, 6, 5, 4, 
        5, 5, 5, 4, 3, 5, 5, 5, 
        5, 4, 4, 3, 2, 3, 0, 0, 
        7, 7, 7, 7, 7, 7, 7, 7
};

//Setup LED Control Library - TODO Update pings for esp8266 
// pin 6 is connected to the MAX7219 pin 1
// pin 5 is connected to the CLK pin 13
// pin 3 is connected to LOAD pin 12

LedControl lc=LedControl(13,14,12,devCount); 

//HTML Strings
String header = "<!DOCTYPE html><html lang='en'><head><meta charset='utf-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport content='width=device-width, initial-scale=1'> \
    <meta name='Twitter LED Matrix Display' content=''> \
    <meta name='nodeMCU' content=''> \
    <title>Twitter LED</title> \
    <link href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/css/bootstrap.min.css' rel='stylesheet'> \
  </head>";

String navbar = "<body role=document> \
    <nav class='navbar navbar-inverse navbar-fixed-top'> \
      <div class='container'> \
        <div class='navbar-header'> \
          <button type='button' class='navbar-toggle collapsed' data-toggle='collapse' data-target='#navbar' aria-expanded='false' aria-controls='navbar'> \
            <span class='sr-only'>Toggle navigation</span> \
            <span class='icon-bar'></span> \
            <span class='icon-bar'></span> \
            <span class='icon-bar'></span> \ 
          </button> \
          <a class='navbar-brand' href=#>ESP8266 - Twitter LED Control</a> \
        </div> \
        <div id='navbar' class='navbar-collapse collapse'> \
          <ul class='nav navbar-nav'> \
            <li class='active'><a href=#>Home</a></li> \
            <li><a href='#about'>About</a></li> \
            <li><a href='#contact'>Contact</a></li> \
            <li class='dropdown'> \
              <a href='#' class='dropdown-toggle' data-toggle='dropdown' role='button' aria-haspopup='true' aria-expanded=false>Dropdown <span class=caret></span></a> \
              <ul class=dropdown-menu> \
                <li><a href=#>Action</a></li> \
                <li><a href=#>Another action</a></li> \
                <li><a href=#>Something else here</a></li> \
              </ul> \
            </li> \
          </ul> \
        </div><!--/.nav-collapse --> \
      </div> \
    </nav> \
    <style type='text/css'>html, body{ padding-top: 70px }</style> \
    <div class=container role=main>";
//class='form-inline'
String textBox = "<br><br><form role='form' action='/submit' method='POST'> <div class='form-group'> \
  <label for='hashtag'>Twitter Hashtag:</label> \
  <input type='text' class='form-control' id='hashtag' name='hashtag' style='width:200px;'> \ 
  <button type='submit' class='btn btn-default'>Submit</button> \
</div><br>";

String deviceData = "";

String footer = "</div><script src='https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js'></script><script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.6/js/bootstrap.min.js'></script></body></html>";

String redirect = "<META HTTP-EQUIV=REFRESH CONTENT='0; URL=/'>.";


bool checkInternet(){
    if (!client.connect(iCheckHost, 80)) {
    Serial.println("Connection Failed!");
    return false;
  }
  else{
    Serial.println("Connected to Internet!");
  
  }

  /* 
  Serial.print("Requesting URL: ");
  Serial.println(iCheckURL);

  // This will send the request to the server
  client.print(String("GET ") + iCheckURL + " HTTP/1.1\r\n" +
               "Host: " + iCheckHost + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("Respond:");
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

 */
  return true;
}

void getDeviceData(){
  deviceData = "<b>Status:</b> ";
  if(WiFi.status() == WL_CONNECTED){
    deviceData += "Connected";  
  }
  else {
    deviceData += "Not Connected";
  }
  deviceData += "<br><b>MAC:</b> ";
  deviceData += WiFi.macAddress();
  deviceData += "<br><b>Local IP:</b>  ";
  deviceData += WiFi.localIP().toString();
  deviceData += "<br><b>Public IP:</b> ";
  deviceData += publicIP;
  deviceData += "<br><b>Hashtag:</b> ";
  deviceData += TwitterHashtag;

  
}

String getPublicIP(){
  String line;
  Serial.println("Getting Public IP");
    if (!client.connect("api.ipify.org", 80)) {
      return "0.0.0.0";
    }
    else{
      /*client.print(String("GET ") + "/" + " HTTP/1.0\r\n" +
                 "Host: " + "api.ipify.org" +"\r\n" + 
                 "Connection: close\r\n\r\n"); */
      client.println("GET / HTTP/1.0");
      client.println("Host: api.ipify.org");
      client.println();  
      int i=0;
      while((!client.available()) && (i<1000)){
        delay(10);
        i++;
      } // See more at: http://www.esp8266.com/viewtopic.php?f=29&t=3375#sthash.Kw2Etlj7.dpuf            
      while(client.available()){
         if(client.read() == '\r'){
            if(client.read() == '\n'){
              if(client.read() == '\r'){
                if(client.read() == '\n'){
                  line = client.readStringUntil('\r');    
                }
              }
            }
         }
      }
    }
  Serial.println(line);
  return line;
  
}

void handleSubmit(){
  if (server.args() > 0 ) {
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      if (server.argName(i) == "hashtag") {
         TwitterHashtag = server.arg(i);
         Serial.println(TwitterHashtag);
      }
   }
   getDeviceData();
   server.send(200, "text/html", header+navbar+deviceData+textBox+footer);
   //server.send(200, "text/html", redirect);
   
} //- See more at: http://www.esp8266.com/viewtopic.php?f=8&t=4345#sthash.W5X9rS5Y.dpuf
  
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", header+navbar+deviceData+textBox+footer);
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.softAP("ESP","<soft ap password>");
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/submit", handleSubmit);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  
  publicIP = getPublicIP();
  
    getDeviceData();
  
  if(!checkInternet()){
      Serial.println("Not Connected!");
  }

  //Init Queue and LED Display
  tweetMsg.setPrinter (Serial);

  //we have already set the number of devices when we created the LedControl
  //int devices=lc.getDeviceCount();
  //we have to init all devices in a loop
  for(int address=0;address<devCount;address++) {
    /*The MAX72XX is in power-saving mode on startup*/
    lc.shutdown(address,false);
    /* Set the brightness to a medium values */
    lc.setIntensity(address,brightness);
    /* and clear the display */
    lc.clearDisplay(address);
  }
    
}

void clearDisplay()
{
  
   for(int address=0;address<devCount;address++)
      {
         lc.clearDisplay(address);
      } 
  
}


void getTwitter(){
  
  if ((millis() > (delayTime + delayTwitter)) || (firstBoot)){
    
    int tweetCount = 0;
    
    //digitalWrite(ledPin, HIGH);
    
   bool firstLoop = true;
   long firstTime = 0;
   long currTime = 0;
   
  
   //SerialUSB.println("connecting to server...");
   
   if (client.connect(serverName, 80)) {
   //if (client.connect()) {
    TextFinder  finder( client,  timeOut);
    //SerialUSB.println("making HTTP request...");
    // make HTTP GET request to twitter:
    client.print("GET /twitter/twitter.php?q=%23");
    client.print(TwitterHashtag);
    client.print("&count=20"); //Make sure we get enough tweets returned since we are filtering out RT
    client.println(" HTTP/1.1");
    client.println("HOST: <server with Twitter PHP script>");
    client.println();
    //SerialUSB.println("sended HTTP request...");
    while (client.connected()) {
      if (client.available()) {
        Serial.println("looking for tweet...");
        while (finder.find("<published>")){
          
          if (firstLoop) {
           
            finder.find("T");
            firstTime = finder.getValue();
            //SerialUSB.println(firstTime);
            firstTime = firstTime + 1000000L; //corrects for midnight to allow comparsion of tweets without negitives
            //SerialUSB.println(firstTime);
            firstLoop = false;
            
          }
          else {
            
            finder.find("T");
            currTime = finder.getValue();
            currTime = currTime + 1000000L;
            //SerialUSB.print(currTime);   
            //SerialUSB.print(" : ");
            
            //SerialUSB.println(firstTime - tweetHistoryTime);
            
            //Will stop when history time is reached or max possible tweets that can be displayed in the min display time per tweet
            if ((delayTwitter / minDisplayTime) <= tweetCount) break; //stops loading tweets if we are at max tweets that can be displayed in the alotted display time, this stops uC from running out or memory over time
            if (currTime < (firstTime - tweetHistoryTime)) break;  //With the above line this isn't really needed you can just adjust display time
            
          
          }
                   
          
          if((finder.getString("<title>","</title>",tweet,140)!=0)){
            //if ((tweet[0] == 'R') & (tweet[1] == 'T')){/*SerialUSB.println("RT")*/;}else{
            String tweetClean(tweet);
            //tweetClean.replace(hashTag, " * "); //Remove search hashtag before being displayed
            //tweetClean.replace(hashTag2, " * "); //With capital
            
            int URL = tweetClean.indexOf("http");

            if(URL > -1) {
            
                //SerialUSB.println(URL);
            
                int endURL = tweetClean.indexOf(' ', URL + 1);

                //if -1 URL is at end of tweet otherwise it is in the middle
                if (endURL = -1) {
                  tweetClean.replace(tweetClean.substring(URL, tweetClean.length()), " ");
                }
                else {
                  //SerialUSB.println(endURL);
                  tweetClean.replace(tweetClean.substring(URL, endURL), " ");
                } 
            }
             
            Serial.println(tweetClean);
            tweetCount++;            
            tweetMsg.push(tweetClean); //If there is too many tweets and min display is set uC will run out of memory eventually
            //}
          }
        }
        break;
      }
    }
    delay(1);
    //client.stop(); //causes long hangs and sometimes stops
    firstLoop = true;
  }
  
   //SerialUSB.println("delay...");
   Serial.print(tweetMsg.count());
   Serial.print(" : ");
   Serial.println(tweetCount);
   displayCycleTime = delayTwitter / tweetCount; //Calculation to spread display of tweets evenly over the time between connections
   if (displayCycleTime < minDisplayTime) displayCycleTime = minDisplayTime; //If there are too many tweets for messages to be display set display time to min display time.
   Serial.println(displayCycleTime);  
 
   delayTime = millis();
   firstBoot = false;
   firstLoop = true;

 }
 
 
  if (millis() > (displayLastTime + displayCycleTime)){

    if (!tweetMsg.isEmpty()) {
      
      String currTweet = tweetMsg.pop();
    
      for (int i = 0; i < currTweet.length(); i++){
          msg[i]=currTweet[i];
      }
   
      msg[currTweet.length()]='\0';
   
      msgsize =  currTweet.length();
      
    }
    
    displayLastTime = millis();
    
    
 }
  //client.server(8080);
  //digitalWrite(ledPin, LOW);
}
  

void loop(void){
  server.handleClient();
  curcharixsave2 = curcharix;
    curcharbitsave2 = curcharbit;
    

    for (i=devCount-1;i>=0;i--) // Loop through our 8 displays
    {
      for (j=0;j<8;j++) // Set up rows on current  display
      {      
        byte outputbyte = 0;
      
        curchar = msg[curcharix];
      
        curcharixsave = curcharix;
        curcharbitsave = curcharbit;
      
        for (k=7;k>=0;k--) // Copy over data for 8 columns to current row and send it to current display
        {
          // This byte is the bitmap of the current character for the current row
          byte currentcharbits = Font8x5[((curchar-32)*8)+j];
      
          if (currentcharbits & (1<<curcharbit))
            outputbyte |= (1<<k);
      
          // advance the current character bit of current character
      
          curcharbit ++;
      
          if (curcharbit > lentbl_S[curchar-32]) // we are past the end of this character, so advance.
          {
            curcharbit = 0;
            curcharix += 1;          
            if (curcharix+1 > msgsize) curcharix=0;
            curchar = msg[curcharix];
          }
        }
      
        lc.setRow(i, j, outputbyte);

        if (j != 7) // if this is not the last row, roll back advancement, if it is, leave the counters advanced.
        {
          curcharix = curcharixsave;
          curcharbit = curcharbitsave;
        }
      }
    }
  
    curcharix = curcharixsave2;
    curcharbit = curcharbitsave2;
    curchar = msg[curcharix];
 
  
    // advance the current character bit of current character
      
    curcharbit ++;
      
    if (curcharbit > lentbl_S[curchar-32]) // we are past the end of this character, so advance.
    {
      curcharbit = 0;
      curcharix += 1;
      if (curcharix+1 > msgsize) curcharix=0;
      curchar = msg[curcharix];
    }
      
    delay(scrollSpeed);
 
  getTwitter();
}

