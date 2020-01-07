#include <SPI.h>
#include <DHT11.h>
#include <U8glib.h>
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);

#define SENSOR A0

dht11 DHT11;

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR,INPUT);
  digitalWrite(SENSOR,LOW);
}

void loop() {
    u8g.firstPage();
    do{
        play();
   
    } while(u8g.nextPage());  

}

void play(){
    int chk = DHT11.read(SENSOR); 
    
    if (chk == 0) {
        u8g.setFont(u8g_font_6x13);
        u8g.setFontPosTop();
        u8g.setPrintPos(10,15);         //温度
        u8g.print("Temperature: ");  
        u8g.print((float)DHT11.temperature);
        u8g.print("C");
    
        u8g.setPrintPos(20, 36);       //湿度
        u8g.print("Humidity: ");
        u8g.print((float)DHT11.humidity);
        u8g.print("%");
    }
    else{
        Serial.println("Sensor Error");
    }
}


