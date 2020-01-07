/*
 * 彩色目标跟踪Arduino程序
 * 2018/04/06
 * -------------------------
 * 功能：
 * 云台跟踪彩色目标
 * 实现：
 * 树莓派或者电脑作为上位机使用OpenCv识别彩色目标，获取位置数据并通过USB串口发送至下位机
 * Arduino作为下位机通过位置数据控制云台舵机
 * --------------------------
 * 摄像头屏幕方向：
 * 100-----------X-----------560
 * |
 * |
 * Y
 * |
 * |
 * |
 * 440
 */
#include <Arduino.h>
#include <Wire.h>
#include <MultiLCD.h>
#include <Servo.h>

//PWM
#define X_SERVO_ANGLE_MIN 650
#define X_SERVO_ANGLE_MAX 2000
#define Y_SERVO_ANGLE_MIN 1100
#define Y_SERVO_ANGLE_MAX 2000

LCD_SSD1306 lcd;
Servo myServo[2];

//定义屏幕边缘最大最小值
int Xmin = 120;
int Xmax = 560;
int Ymin = 100;
int Ymax = 440;

//定义X和Y方向上物体移动一个单位舵机需要转动的角度
float x_move_angle = (X_SERVO_ANGLE_MAX - X_SERVO_ANGLE_MIN) / (Xmax - Xmin);
float y_move_angle = (Y_SERVO_ANGLE_MAX - Y_SERVO_ANGLE_MIN) / (Ymax - Ymin);

struct Point{   //x,y点
  int x = Xmax / 2;
  int y = Ymax / 2;  
}point;

int servo_port[2] = {4, 7};
float value_init[2] = {1330, 1600};

void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.clear();
  ServoReset();
//  Serial.println(x_move_angle);
//  Serial.println(y_move_angle);
}

void loop() {
  Display(&point);    //12864OLED屏显示串口所接收的位置数据
  GetCommand();
}

/*串口部分，对接收到的位置数据进行处理*/
void GetCommand(){
  String s,str;
  
  while(Serial.available() > 0){
    s = Serial.readStringUntil('\n');
    s.trim();
    
    for(int i=0;i<s.length();i++){
      if(s[i] == '\0') continue;
      str+=s[i];
    }

    DataProcessing(&point,str);
  }
}

void DataProcessing(struct Point * pos, String s){
  int x, y, xPos, yPos, spacePosition;
  float dx, dy;
  float move_angle[2];
  float * pAngle = move_angle;
  
  xPos = pos->x;yPos = pos->y;

  spacePosition = s.indexOf(',');
  x = s.substring(0, spacePosition).toInt();
  s = s.substring(spacePosition+1, s.length());
  y = s.toInt();

  //物体超出屏幕边缘矫正舵机
  if(x < Xmin) {
    x = Xmin;
    Adjust(-x_move_angle,0);
  }
  if(x > Xmax) {
    x = Xmax;
    Adjust(x_move_angle,0);
  }
  if(y < Ymin) {
    y = Ymin;
    Adjust(0,-y_move_angle);
  }
  if(y > Ymax) {
    y = Ymax;
    Adjust(0,y_move_angle);
  }
  
  if(x == xPos && y == yPos) return;

  dx = x - xPos;dy = y - yPos;
  
  //(正面看)当前位置小于先前位置，摄像头右转，否则摄像头左转
  if(abs(dx) > 1){
    *pAngle = dx < 0 ? -x_move_angle : x_move_angle;  
  }
  //(正面看)当前位置小于先前位置，摄像头向下降，否则摄像头上抬 
  if(abs(dy) > 1){
    *(pAngle+1) = dy < 0 ? -y_move_angle : y_move_angle;
  }
  
  ServoMove(value_init, pAngle);

  pos->x = x;
  pos->y = y;
}

//超出边缘矫正
void Adjust(int pos_x, int pos_y){
  
  if(value_init[0] > X_SERVO_ANGLE_MAX || value_init[0] < X_SERVO_ANGLE_MIN) return;
      else value_init[0]+=pos_x;
  if(value_init[1] > Y_SERVO_ANGLE_MAX || value_init[1] < Y_SERVO_ANGLE_MIN) return;
      else value_init[1]+=pos_y;  

  ServoGo(0, value_init[0]);
  ServoGo(1, value_init[1]);
}

/*舵机部分*/
void ServoReset(){
  for(int i=0;i<2;i++) ServoGo(i, value_init[i]);
}

void ServoStart(int which){
  if(!myServo[which].attached())myServo[which].attach(servo_port[which]);
  pinMode(servo_port[which], OUTPUT);  
}

void ServoStop(int which){
  myServo[which].detach();
  pinMode(servo_port[which], LOW);  
}

void ServoGo(int which, int where){
  ServoStart(which);
  myServo[which].writeMicroseconds(where);
}

void ServoMove(float * servo_value, float * pArray){
  //舵机临界值判断
  if(*(servo_value) > X_SERVO_ANGLE_MAX) {
    *(servo_value) = X_SERVO_ANGLE_MAX;
    return;
  }
  else if(*(servo_value) < X_SERVO_ANGLE_MIN) {
    *(servo_value) = X_SERVO_ANGLE_MIN;
    return;
  }
  else *(servo_value)+=*pArray;

  if(*(servo_value+1) > Y_SERVO_ANGLE_MAX) {
    *(servo_value+1) = Y_SERVO_ANGLE_MAX;
    return;
  }
  else if(*(servo_value+1) < Y_SERVO_ANGLE_MIN){
    *(servo_value+1) = Y_SERVO_ANGLE_MIN;
    return;
  }
  else *(servo_value+1) += *(pArray+1); 

  //舵机转动控制
  ServoGo(0, *(servo_value));
  ServoGo(1, *(servo_value+1));
}

/*显示部分，显示串口接收到的物体在屏幕中的位置,pos_x, pos_y*/
void Display(struct Point * pos){
  lcd.setCursor(5, 0);
  lcd.print("(x , y)");
  lcd.setCursor(10, 3);
  lcd.printLong(pos->x, FONT_SIZE_LARGE);
  lcd.setCursor(70, 3);
  lcd.printLong(pos->y, FONT_SIZE_LARGE);
}
