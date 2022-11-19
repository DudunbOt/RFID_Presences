#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <ESP8266WiFi.h>
#include <WifiUdp.h>
#include <NTPClient.h>
#include <rdm6300.h>
#include <WiFiClient.h>

// Display Pin
#define display_CS  D2 //pin CS on Digital 2
#define display_RST D3 //pin RST on Digital 3
#define display_DC  D4 // pin DC on Digital 4

#define LED D0 //LED Pin D9
#define buzzer D8 // Buzzer Pin D8

#define RDM6300_RX_PIN D1 //RFID Scanner Pin D1
#define READ_LED_PIN D8 //RFID LED Pin D8

Rdm6300 rdm6300;

//Wifi Connect
const char ssid[] = "ssid";
const char pass[] = "pass";
const long utcOffsetInSeconds = 25200;
String city = "Kartasura"; //Displayed city on TFT display

bool alarm = false;
String hour,minute,second;
String string, uid;

String host = "host"; //host for where the web located
const int httpsPort = 443; //https port

const char fingerprint[] PROGMEM = "fingerprint"; //https fingerprint

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

WiFiUDP ntp;
NTPClient timeClient(ntp, "pool.ntp.org", utcOffsetInSeconds); //setting up NTP client for real time tracker

Adafruit_ILI9341 display = Adafruit_ILI9341(display_CS, display_DC, display_RST);

void setup() {
//initialize all the components
  pinMode(buzzer,OUTPUT);
  pinMode(LED,OUTPUT);

  digitalWrite(LED,HIGH);
  
  Serial.begin(115200);

  rdm6300.begin(RDM6300_RX_PIN);

  display.begin();
  display.fillScreen(ILI9341_BLACK); //display background
  display.setTextColor(ILI9341_WHITE); // display text color
  display.setRotation(1); // layout for the display for landscape

//starting wifi client
  WiFi.begin(ssid, pass); 

  display.println("Connecting to :");
  display.println(ssid);

  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
  Serial.println("");
  display.setTextColor(ILI9341_GREEN);
  display.println("Connected!");
  delay(2000);
  display.fillScreen(ILI9341_BLACK);

  timeClient.begin();

  Serial.println("Scan Card");
}

void loop() {

 //Setting up all displayed information for the screen 
  display.setCursor(110,0);
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  display.setTextSize(3);
  display.println("Absensi");
  display.setTextSize(2);
  display.setCursor(93,25);
  display.println("Citra Pustaka");

  timeClient.update();

  int seconds = timeClient.getSeconds();
  int minutes = timeClient.getMinutes();
  int hours   = timeClient.getHours();

  display.setTextSize(5);
  display.setCursor(50,60);
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  if(hours < 10){
    hour = "0"+String(hours);
  }else{
    hour = String(hours);
  }
  if(minutes < 10){
    minute = "0"+String(minutes);
  }else{
    minute = String(minutes);
  }
  if(seconds < 10){
    second = "0"+String(seconds);
  }else{
    second = String(seconds);
  }
  
  display.println(hour + ":" + minute + ":" + second);;

  display.setCursor(110,110);
  display.setTextSize(2);
  display.setTextColor(ILI9341_GREEN, ILI9341_BLACK);
  display.println(city);

  display.setCursor(200,210);
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  display.setTextSize(1);
  display.println("Copyright by DiveOn");

  //Creating custom alarm
  if(timeClient.getHours() == 8){
      alarm = true;
    }
  if(timeClient.getHours() == 11 && timeClient.getMinutes() == 30 && timeClient.getDay() != 6){
      alarm = true;
    }
  if(timeClient.getHours() == 12 && timeClient.getMinutes() == 30 && timeClient.getDay() != 6){
      alarm = true;
    }
  if(timeClient.getHours() == 16){
      alarm = true;
    }
  if(timeClient.getDay() == 5 && timeClient.getHours() == 13 && timeClient.getMinutes() < 1){
      alarm = true;
    }  
  if(timeClient.getDay() == 6 && timeClient.getHours() == 13 && timeClient.getMinutes() < 1){
      alarm = true;
    }
  else{
      alarm = false;
      digitalWrite(buzzer,LOW);
    }
  
  if(alarm == true && timeClient.getSeconds() <= 5){ //if alarm status are activated, turn on buzzer and LED indicator for 5 seconds each
      Serial.println("On");
      digitalWrite(LED,HIGH);
      digitalWrite(buzzer,HIGH);
      delay(1000);
      digitalWrite(buzzer,LOW);
      digitalWrite(LED,LOW);
    }
    
  digitalWrite(LED,HIGH);  
   //Alarm END


   //Scan Card
   
   if(rdm6300.get_new_tag_id()){
      Serial.print("Card detected :"); Serial.println(rdm6300.get_tag_id(), HEX);      digitalWrite(buzzer,HIGH);
      digitalWrite(LED,LOW);
      digitalWrite(buzzer,HIGH);
      delay(200);
      digitalWrite(buzzer,LOW);

      uid = String(rdm6300.get_tag_id(), HEX);
      display.setCursor(0,0);
      display.fillScreen(ILI9341_BLACK);
      display.setTextSize(2);
      display.setCursor(70,10);
      display.println("Kartu Terdeteksi:");
      display.setTextSize(5);
      display.setCursor(80,45);
      display.println(uid);

      WiFiClientSecure Client;
      Client.setFingerprint(fingerprint);
      Client.setTimeout(15000);
      Client.setInsecure();
      delay(100);

      if(!Client.connect(host,httpsPort)){
        Serial.println("Connection Failed");
        return;
        }

      String card = uid;
      String url = "url"; //url of your website
      url += card;

      Serial.print("Request Link: ");
      Serial.println(host + url);

      Client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

      Serial.println("Request Sent");

      display.setCursor(90,100);
      display.setTextSize(2);
      display.setTextColor(ILI9341_GREEN);
      display.println("Sukses Absen!");
      delay(1000);
      digitalWrite(LED,HIGH);
      display.fillScreen(ILI9341_BLACK);
    }
}
