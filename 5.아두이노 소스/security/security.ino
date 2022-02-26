#include <ArduinoJson.h>
#include <SimpleTimer.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>


#define FIREBASE_HOST "home-keeper-9b4c7.firebaseio.com" 
#define FIREBASE_AUTH "AIzaSyAfFX1_0rqg8rLpvQUcX1tOfMXbhCRi2gY"

#define WIFI_SSID "soft"
#define WIFI_PASSWORD "ss2420052"


SimpleTimer t1, t2, t3;


const char *month_arr[13] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const char *day_arr[7] = {"Sun", "Mon", "Tue", "Wen", "Thu", "Fri", "Sat"};
char buf[40];
char *time_cut=0;
char *time_val[8] = {0};
char DaysOfMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};

int pin1 = 2;   // D4
int pin2 = 14;  // D5
int pin3 = 12;  // D6

void setup() {
    Serial.begin(115200);
  
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
       Serial.print(".");
       delay(500);
    }

    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
  
    Firebase.begin(FIREBASE_HOST);

     
    pinMode(pin1, INPUT_PULLUP);   // D4
    pinMode(pin2, INPUT_PULLUP);   // D5
    pinMode(pin3, INPUT_PULLUP);   // D6


    t1.setInterval(1000,fn1);
    t2.setInterval(1000,fn2);
    t3.setInterval(1000,fn3);  
    
}

void loop() {
    
    t1.run();
    t2.run();
    t3.run();
    
    delay(1000);
}

void sendDatas(String room, String str) {
    // set string value
    String room_name = "root/"+room;
    Firebase.setString(room_name, str);
    
    if (Firebase.failed()) {
        Serial.print("setting /message failed:");
        Serial.println(Firebase.error());  
        return;
    } 
    
    Serial.print(room_name);
    Serial.print("  ");
    Serial.println(str); 

    delay(1000);
}

void sendTime(String open_time, String room) {
    String date_time = "root/"+open_time;

    Firebase.setString(date_time, room);
    
    if (Firebase.failed()) {
        Serial.print("setting /message failed:");
        Serial.println(Firebase.error());  
        return;
    } 
    
    Serial.print(date_time);
    Serial.print("  ");
    Serial.println(room);

     delay(1000);
}

void fn1() {
  int room1 = digitalRead(pin1);

  if(room1 == 1) {      // 0: 닫힘   1:열림
      sendDatas("room1", "open");
      String st = get_date_time();
      sendTime(st,"room1");         
  } else {
      sendDatas("room1", "close");
  }      
}

void fn2() {
  int room2 = digitalRead(pin2);  

    if(room2 == 1) {      // 0: 닫힘   1:열림
        sendDatas("room2", "open");
        String st = get_date_time();
        sendTime(st,"room2");           
    } else {
        sendDatas("room2", "close");
    }  
}

void fn3() {
  int room3 = digitalRead(pin3); 

  if(room3 == 1) {      // 0: 닫힘   1:열림
      sendDatas("room3", "open");
      String st = get_date_time();
      sendTime(st,"room3");           
  } else {
      sendDatas("room3", "close");
  }       
}

String get_date_time() {
  String str_time = getTime();
  str_time.toCharArray(buf, 30);
  
  time_cut = strtok(buf, ", :");
  
  for(int i=0; time_cut; i++){ 
    time_val[i] = time_cut; 
    time_cut = strtok(0, ", :");
  }
 
  int year   = atoi(time_val[3]);
  int month  = month_to_digit((char *)time_val[2]);
  int date   = atoi(time_val[1]);
  int day    = day_to_digit((char *)time_val[0]);
  int hour   = atoi(time_val[4]) + 9;
  int minute = atoi(time_val[5]);
  int second = atoi(time_val[6]);
 
  if(hour>23) {
    hour %=24;
    if(++day>6) day=0;
    if     (!(year%400)) DaysOfMonth[1]=29;  // 윤년/윤달 계산
    else if(!(year%100)) DaysOfMonth[1]=28; 
    else if(!(year%  4)) DaysOfMonth[1]=29;  
    
    if(date<DaysOfMonth[month-1]) date++;
    else {
      date=1;
      if(++month>12) {
        month=1;
        year++;
      }  
    }
  }
  
  //  Serial.println(str_time);
  
  sprintf(buf, "%04d%02d%02d%02d%02d%02d%s", year, month, date,hour, minute, second,day_arr[day]);
  // Serial.print(buf);  
  int i=0;
  String st;
  
  while(buf[i]) {
    st = st+buf[i];
    i++;
  }
  return st;
}


int month_to_digit(char* str) {
  for(int i=0; i<12; i++) {
      if(!strncmp(str, (char *)month_arr[i], 3)) return i+1;
  }
}

int day_to_digit(char* str) {
  for(int i=0; i<7; i++) {
      if(!strncmp(str, (char *)day_arr[i], 3)) return i;
  }
}
String getTime() {
  WiFiClient client;
  while (!!!client.connect("google.com", 80)) {
      Serial.println("connection failed, retrying...");
  }
 
  client.print("HEAD / HTTP/1.1\r\n\r\n");
 
  while(!!!client.available()) {
     yield();
  }
 
  while(client.available()){
    if (client.read() == '\n') {    
      if (client.read() == 'D') {    
        if (client.read() == 'a') {    
          if (client.read() == 't') {    
            if (client.read() == 'e') {    
              if (client.read() == ':') {    
                client.read();
                String theDate = client.readStringUntil('\r');
                client.stop();
                return theDate;
              }
            }
          }
        }
      }
    }
  }
}
