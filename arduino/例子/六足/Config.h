/********************************************程序使用Arduino 1.8.2编译************************************************/

/*
 * 此程序包含四个动作组，如有需要可自行添加,需修改处有注释，按注释修改
 * 
 * act_num：定义动作组数量
 * frequency：定义舵机转动频率
 * servo_num：定义舵机数量
 * servo_speed：定义舵机速度，单位毫秒
 * action_delay：定义动作组每行动作执行的动作间隔，单位微秒
 * actChangeDelay：定义不同动作组执行时的切换时间间隔，单位毫秒
 */
#define act_num 6
#define frequency 10
#define servo_num 12
#define servo_speed 800
#define action_delay 1
#define actChangeDelay 200

/*
 * 动作组定义，可自行添加动作组，名称可修改，此处为 4 个动作组
 */
#define action_move 0
#define action_left 1
#define action_right 2
#define action_back 3
#define action_dance 4
#define action_init 5

/*
 * servo_pin：定义舵机引脚，按顺序从小到大定义，引脚号应从 1 开始
 * actPwmNum：定义动作组数组的长度
 */
int servo_pin[servo_num] = {1, 3, 4, 5, 6, 8, 11, 13, 15, 21, 24, 26};
int actPwmNum[act_num] = {};

//中间数组
float value_pre[servo_num] = {};
float value_cur[servo_num] = {};

