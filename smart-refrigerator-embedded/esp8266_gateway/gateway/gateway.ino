#include <SoftwareSerial.h> //IoT보드-아두이노우노간 통신라이브러리
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h" //온습도센서 DHT22의 라이브러리
//핀설정
#define DHTPIN D3 //온습도센서
#define relaypin1 D8 //릴레이 1
#define relaypin2 D9 //릴레이 2
#define relaypin3 D10 //릴레이 3
#define relaypin4 D11 //릴레이 4

//시리얼통신 핀설정
SoftwareSerial uno(D5,D6);

// 토픽설정
#define TOPIC_TEMP "jb202211/temp"
#define TOPIC_HUMIDITY "jb202211/humi"
#define TOPIC_RELAY "jb202211/relay"
#define TOPIC_FOOD "jb202211/food"

//온도경고 수치
//#define WARN_TEMP 26.0

float humidity, temperature;
long curr_time;
long prev_time =0;

int prev_buzz = 0;
int curr_buzz = 0;

//와이파이 설정
const char* ssid = "";
const char* password = "";

const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
/*
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
*/

// 온습도센서
#define DHTTYPE DHT22 // DHT22 (AM2302) 센서종류 설정
DHT dht(DHTPIN, DHTTYPE);



void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid); 

  //WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
/*
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  String mytopic = topic;

  // Switch on the LED if an 1 was received as first character
  if(mytopic == TOPIC_BUZZER){
    if (payload[0] == '1') {
     //prev_buzz =1; 
      Serial.print("payload : ");
      Serial.println((char)payload[0]);
      digitalWrite(buzzPin, HIGH);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
   } 
    else {
      if (payload[0] == '0') { 
    // prev_buzz =0;
        Serial.print("payload : ");
        Serial.println((char)payload[0]);
    
        digitalWrite(buzzPin, LOW);  // Turn the LED off by making the voltage HIGH
      }
    }
  }
}*/

void callback(char* topic, byte* payload, unsigned int length) {
  
  //Serial.print("Message arrived ["); 
  //Serial.print(topic);
  //Serial.print("] "); 
  
  String response;
  for (int i = 0; i < length; i++) {
    response += (char)payload[i];  // 브로커에서 수신된 메시지에서 릴레이 제어 명령을 꺼냅니다.
  } 
  //Serial.println();
  if(response == "ona")  // 수신된 메시지가 on인 경우 1켜기
  {
    digitalWrite(relaypin1, LOW);
  }
  else if(response == "offa")  // 수신된 메시지가 off인 경우 1끄기
  {
    digitalWrite(relaypin1, HIGH);   
  }
  else if(response == "onb")  // 수신된 메시지가 on인 경우 2켜기
  {
    digitalWrite(relaypin2, LOW);
  }
  else if(response == "offb")  // 수신된 메시지가 off인 경우 2끄기
  {
    digitalWrite(relaypin2, HIGH);   
  }
  else if(response == "onc")  // 수신된 메시지가 on인 경우 3켜기
  {
    digitalWrite(relaypin3, LOW);
  }
  else if(response == "offc")  // 수신된 메시지가 off인 경우 3끄기
  {
    digitalWrite(relaypin3, HIGH);   
  }
  else if(response == "ond")  // 수신된 메시지가 on인 경우 4켜기
  {
    digitalWrite(relaypin4, LOW);
  }
  else if(response == "offd")  // 수신된 메시지가 off인 경우 4끄기
  {
    digitalWrite(relaypin4, HIGH);   
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      
      client.subscribe(TOPIC_RELAY);// mqtt서버의 토픽 구독하기!
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  uno.begin(115200); //IoT보드-아두이노우노간 통신라인!

  setup_wifi();
  dht.begin();//온습도센서
  prev_time =millis();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //릴레이 핀모드설정 및 초기OFF상태 세팅
  pinMode(relaypin1,OUTPUT);
  pinMode(relaypin2,OUTPUT);
  pinMode(relaypin3,OUTPUT);
  pinMode(relaypin4,OUTPUT);
  /*
  digitalWrite(relaypin1, LOW);
  digitalWrite(relaypin2, LOW);
  digitalWrite(relaypin3, LOW);
  digitalWrite(relaypin4, LOW);
  */
  allrelayoff();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  curr_time = millis();
  if (curr_time - prev_time > 2000) {
    prev_time = curr_time;

    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    if (isnan(humidity)|| isnan(temperature)){
     // Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    /*
     //온습도 확인용
    Serial.print(F("Humidity: "));
    Serial.print(humidity);
    Serial.print(F("%  Temperature: "));
    Serial.print(temperature);
    Serial.println(F("°C "));
    */

    snprintf(msg,50,"%.1f",temperature);
    client.publish(TOPIC_TEMP, msg);
    //Serial.print("Publish meddage(temperature) : ");
    //Serial.println(msg);
    snprintf(msg,50,"%.1f",humidity);
    client.publish(TOPIC_HUMIDITY, msg);
    //Serial.print("Publish meddage(humidity) : ");
    //Serial.println(msg);
/*
    if(temperature >= WARN_TEMP){
      curr_buzz=1;
      if(prev_buzz == 0 && curr_buzz == 1){
        prev_buzz=1;
        digitalWrite(buzzPin,HIGH);
        snprintf(msg, 50, "%d", HIGH);
        client.publish(TOPIC_BUZZER, msg);
      }
    }
    else{
      curr_buzz=0;
      if(prev_buzz == 1 && curr_buzz == 0){
        prev_buzz=0;
        digitalWrite(buzzPin,LOW);
        snprintf(msg, 50, "%d", LOW);
        client.publish(TOPIC_BUZZER, msg);
      }
    }*/
    delay(50);
  //시리얼통신 부분
  uno.write('0');
  //우노가 응답할때 까지 기다려야지~
  /*
  while(true){
    if(uno.available()) break;
  }
  */
  if(uno.available()){
    String data = uno.readStringUntil(0x0a);
    Serial.println(data);
    snprintf(msg,50,"%s",data);
    client.publish(TOPIC_FOOD, msg);
  }
    
  }
  
  //시리얼통신 부분
  //uno.write('0');
  //우노가 응답할때 까지 기다려야지~
  /*
  while(true){
    if(uno.available()) break;
  }
  */
  /*
  if(uno.available()){
    String data = uno.readStringUntil(0x0a);
    Serial.println(data);
  }*/
}



void allrelayoff(){
  digitalWrite(relaypin1, HIGH);
  digitalWrite(relaypin2, HIGH);
  digitalWrite(relaypin3, HIGH);
  digitalWrite(relaypin4, HIGH);
}
