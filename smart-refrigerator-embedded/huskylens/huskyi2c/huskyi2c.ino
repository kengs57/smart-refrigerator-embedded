#include "HUSKYLENS.h"
#include "SoftwareSerial.h"
#include <Wire.h>                     // i2C 통신을 위한 라이브러리

HUSKYLENS huskylens;
//HUSKYLENS green line >> SDA; blue line >> SCL
void printResult(HUSKYLENSResult result);

int aatime = 0;
int bbtime = 0;
int cctime = 0;
int ddtime = 0;
int eetime = 0;
long curr_time;
long prev_time =0;

int aaa=0, bbb=0, ccc=0, ddd=0, eee=0;


void setup() {
    Serial.begin(115200);
    Wire.begin();
    while (!huskylens.begin(Wire))
    {
        //Serial.println(F("Begin failed!"));
        //Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>I2C)"));
        //Serial.println(F("2.Please recheck the connection."));
        delay(100);
    }
}

void loop() {
    /* 
    if (!huskylens.request()) Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    else if(!huskylens.isLearned()) Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    else if(!huskylens.available()) Serial.println(F("No block or arrow appears on the screen!"));
    */
    if (!huskylens.request()) ;
    else if(!huskylens.isLearned());
    else if(!huskylens.available());
    else
    {
        //Serial.println(F("###########"));
        while (huskylens.available())
        {
            HUSKYLENSResult result = huskylens.read();
            printResult(result);
            //0.5초마다 인식한 1~5번의 값을 누적 저장
            if(result.ID == 1){
              aatime++;
            }
            else if(result.ID == 2){
              bbtime++;
            }
            else if(result.ID == 3){
              cctime++;
            }
            else if(result.ID == 4){
              ddtime++;
            }
            else if(result.ID == 5){
              eetime++;
            }
        }    
    }
    delay(500);
    //누적된 값을 시리얼프린트
   // Serial.println(String()+F("a=")+aatime+F(", b=")+bbtime+(", c=")+cctime+(", d=")+ddtime+(", e=")+eetime);


    //(현재시간-가동시간)이 10초가 넘으면 실행
    curr_time = millis();
    if (curr_time - prev_time > 10000) {
    prev_time = curr_time;

    //10초동안 6번이상 인식되면 1을저장 아니면 0을저장

    if(aatime>6){
      aaa=1;
    }
    else{
      aaa=0;
    }
    if(bbtime>6){
      bbb=1;
    }
    else{
      bbb=0;
    }
    if(cctime>6){
      ccc=1;
    }
    else{
      ccc=0;
    }
    if(ddtime>6){
      ddd=1;
    }
    else{
      ddd=0;
    }
    if(eetime>6){
      eee=1;
    }
    else{
      eee=0;
    }
    // 저장한 값을 시리얼통신으로 출력함
    String data = "";
        data = String(data + aaa ); 
        data = String(data + "|"); 
        data = String(data + bbb); 
        data = String(data + "|");
        data = String(data + ccc ); 
        data = String(data + "|"); 
        data = String(data + ddd); 
        data = String(data + "|"); 
        data = String(data + eee);
        Serial.println(data);

    
    // 10초동안 누적된 인식한값 초기화
    aatime=0;
    bbtime=0;
    cctime=0;
    ddtime=0;
    eetime=0;
    // 1~5번이 30개 이상이면 있다고 간주하고 시리얼 통신으로 정보(1~5번존재 유무)를보냄
    }
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
      //Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
       // Serial.println(String()+F("ID=")+result.ID);
    }
    else{
        //Serial.println("Object unknown!");
    }
}

void sumResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
       
    }
    else{
       // Serial.println("Object unknown!");
    }
}
