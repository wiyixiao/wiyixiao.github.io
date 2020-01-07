#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <stdio.h>      /*标准输入输出定义*/
#include <stdlib.h>     /*标准函数库定义*/
#include <unistd.h>     /*Unix 标准函数定义*/
#include <sys/types.h>  
#include <sys/stat.h>   
#include <fcntl.h>      /*文件控制定义*/
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <errno.h>      /*错误号定义*/
#include <string.h>
#include <iostream>
#include <string>
#include <ctype.h>

#define COM_DEVICE "/dev/ttyUSB0"

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;
bool selectObject = false;//用来判断是否选中，当鼠标左键按下时为true，左键松开时为false
int trackObject = 0;
bool showHist = true;
Point origin;//选中的起点
Rect selection;//选中的区域
//int vmin = 10, vmax = 256, smin = 30;//图像掩膜需要的边界常数
int vmin = 130, vmax = 256, smin = 150;//图像掩膜需要的边界常数

/*
 * 串口控制部分
 */
static int openSerialDevice()
{
    int fd;

    fd = open(COM_DEVICE, O_RDWR);

    return fd;
}

static void closeSerialDevice(int fd)
{
    close(fd);
}

/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param  speed  类型 int  串口速度
*@return  void
*/
static int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,};
static int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,};
static void set_speed(int fd, int speed){
    int   i; 
    int   status; 
    struct termios   Opt;

    tcgetattr(fd, &Opt); 
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
        if  (speed == name_arr[i]) {     
            tcflush(fd, TCIOFLUSH);     
            cfsetispeed(&Opt, speed_arr[i]);  
            cfsetospeed(&Opt, speed_arr[i]);   
            status = tcsetattr(fd, TCSANOW, &Opt);  
            if  (status != 0) {        
                perror("tcsetattr fd1");  
                return;     
            }    
            tcflush(fd,TCIOFLUSH);   
        }  
    }
}

/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  char  效验类型 取值为N,E,O,,S
*/
static int set_Parity(int fd,int databits,int stopbits,char parity)
{ 
    struct termios options; 
    if  ( tcgetattr( fd,&options)  !=  0) { 
        perror("SetupSerial 1");     
        return -1;  
    }
    options.c_cflag &= ~CSIZE; 
    switch (databits) /*设置数据位数*/
    {   
    case 7:     
        options.c_cflag |= CS7; 
        break;
    case 8:     
        options.c_cflag |= CS8;
        break;   
    default:    
        fprintf(stderr,"Unsupported data size\n"); 
        return -2;  
    }

    switch (parity) 
    {   
    case 'n':
    case 'N':    
        options.c_cflag &= ~PARENB;   /* Clear parity enable */
        options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
        break;  
    case 'o':   
    case 'O':     
        options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
        options.c_iflag |= INPCK;             /* Disnable parity checking */ 
        break;  
    case 'e':  
    case 'E':   
        options.c_cflag |= PARENB;     /* Enable parity */    
        options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
        options.c_iflag |= INPCK;       /* Disnable parity checking */
        break;
    case 'S': 
    case 's':  /*as no parity*/   
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;break;  
    default:   
        fprintf(stderr,"Unsupported parity\n");    
        return -3;  
    }  
    /* 设置停止位*/  
    switch (stopbits)
    {   
    case 1:    
        options.c_cflag &= ~CSTOPB;  
        break;  
    case 2:    
        options.c_cflag |= CSTOPB;  
       break;
    default:    
         fprintf(stderr,"Unsupported stop bits\n");  
         return -4; 
    } 
    options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
//    options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|INPCK);
    tcflush(fd,TCIFLUSH);
    options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0){ 
        perror("SetupSerial 3");
        return -5;  
    }

    return 0;  
}

int serial_data_read(int fd, char *buf, int len)
{
    int nRead;
    
    nRead = read(fd, buf, len);
    return nRead;
}

int serial_data_send(int fd, const char *buf, int len)
{
    int nWrite;

    nWrite = write(fd, buf, len);
    return nWrite;
}

string to_string(int a)
{
    ostringstream ostr;
    ostr << a;
    string astr = ostr.str();
    return astr;
}

//鼠标事件响应函数，这个函数从按下左键时开始响应直到左键释放
static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
		//选择区域的x坐标选起点与当前点的最小值，保证鼠标不管向右下角还是左上角拉动都正确选择
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
		//获得选择区域的宽和高
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);
		//这条语句多余，注释掉不影响结果
    //  selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN://按下鼠标时，捕获点origin
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;//这时switch前面的if语句条件为true，执行该语句
        break;
    case CV_EVENT_LBUTTONUP://松开鼠标时，捕获width和height
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;//重新计算直方图
        break;
    }
}

static void help()//打印控制按键说明
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   ./camshiftdemo [camera number]\n";

    cout << "\n\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tc - stop the tracking\n"
            "\tb - switch to/from backprojection view\n"
            "\th - show/hide object histogram\n"
            "\tp - pause video\n"
            "To initialize tracking, select the object with mouse\n";
}

const char* keys =
{
    "{1|  | 0 | camera number}"
};

int main( int argc, const char** argv )
{
    help();

    VideoCapture cap;
    Rect trackWindow;//要跟踪的窗口
    int hsize = 16;//创建直方图时要用的常量
    float hranges[] = {0,180};
    const float* phranges = hranges;
    CommandLineParser parser(argc, argv, keys);
    int camNum = parser.get<int>("1");//现在camNum = 0
    cap.open(camNum);
    
    //打开串口
    int sfd;
    int ret;
    int i;
    char dataBuf[1024] = {0};

    sfd = openSerialDevice();
    if (sfd < 0){
        printf("open device failed!\n");
        return -1;
    }
    
    set_speed(sfd, 9600);   //串口波特率设置为115200
    
    ret = set_Parity(sfd, 8, 1, 'N');
    if (ret < 0){
        printf("set parity error, ret: %d\n", ret);
        return -2;
    }

    //摄像头画面捕捉不成功则退出程序
    if( !cap.isOpened() )
    {
        help();
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        parser.printParams();//打印出cmd参数信息
        return -1;
    }
    
    //关于显示窗口的一些设置
    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    //设置鼠标事件，把鼠标响应与onMouse函数关联起来
    setMouseCallback( "CamShift Demo", onMouse, 0 );
    //创建三个滑块条，特定条件用滑块条选择不同参数能获得较好的跟踪效果
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

    //创建Mat变量，frame, hsv, hue, mask, hist, histimg, backproj;其中histimg初始化为200*300的零矩阵
    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;

    //主循环
    for(;;)
    {
        if( !paused )
        {
            cap >> frame;//从摄像头输入frame
            
            if( frame.empty() )//为空，跳出主循环
                break;
        }

        frame.copyTo(image);//frame存入image
        
        

        if( !paused )
        {
            cvtColor(image, hsv, CV_BGR2HSV);//将BGR转换成HSV格式，存入hsv中，hsv是3通道

            if( trackObject )//松开鼠标左键时，trackObject为-1，执行核心部分
            {
                int _vmin = vmin, _vmax = vmax;

                //inRange用来检查元素的取值范围是否在另两个矩阵的元素取值之间，返回验证矩阵mask（0-1矩阵）
                //这里用于制作掩膜板，只处理像素值为H:0~180，S:smin~256, V:vmin~vmax之间的部分。mask是要求的，单通道
                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),
                        Scalar(180, 256, MAX(_vmin, _vmax)), mask);

                int ch[] = {0, 0};
                //type包含通道信息，例如CV_8UC3，而深度信息depth不包含通道信息，例如CV_8U.
                hue.create(hsv.size(), hsv.depth());//hue是单通道
                mixChannels(&hsv, 1, &hue, 1, ch, 1);//将H分量拷贝到hue中，其他分量不拷贝。

                if( trackObject < 0 )
                {
                    //roi为选中区域的矩阵，maskroi为0-1矩阵
                    Mat roi(hue, selection), maskroi(mask, selection);
                    //绘制色调直方图hist，仅限于用户选定的目标矩形区域
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
                    normalize(hist, hist, 0, 255, CV_MINMAX);//必须是单通道，hist是单通道。归一化，范围为0-255

                    trackWindow = selection;
                    trackObject = 1;//trackObject置1，接下来就不需要再执行这个if块了
                    
                    histimg = Scalar::all(0);//用于显示直方图
                    //计算每个直方的宽度
                    int binW = histimg.cols / hsize;//hsize为16，共显示16个
                    Mat buf(1, hsize, CV_8UC3);//

                    for( int i = 0; i < hsize; i++ )
                        //直方图每一项的颜色是根据项数变化的
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);
                    cvtColor(buf, buf, CV_HSV2BGR);
                    //量化等级一共有16个等级，故循环16次，画16个直方块
                    for( int i = 0; i < hsize; i++ )
                    {
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);//获取直方图每一项的高
                        //画直方图。opencv中左上角为坐标原点
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }
                //根据直方图hist计算整幅图像的反向投影图backproj,backproj与hue相同大小
                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                //计算两个矩阵backproj、mask的每个元素的按位与，返回backproj
                backproj &= mask;
                //调用最核心的camshift函数
                //TermCriteria是算法完成的条件
                
                RotatedRect trackBox;
                int  Iteration = meanShift(backproj, trackWindow,
                                    TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                if(Iteration != 0 && trackWindow.x >= 0 && trackWindow.y >= 0){
                    trackBox = CamShift(backproj, trackWindow,
                                    TermCriteria( CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 10, 1 ));
                        
                }
                else
                {   
                    Point cp(trackWindow.x + (trackWindow.width / 2), trackWindow.y + (trackWindow.height / 2));
                    circle(image, cp, 100, Scalar(0, 0, 255), 2);
                }
                    
                int xpos = trackWindow.x + (trackWindow.width / 2);
                int ypos = trackWindow.y + (trackWindow.height / 2);
                
                serial_data_send(sfd, to_string(xpos).c_str(),sizeof(to_string(xpos).c_str()));
                serial_data_send(sfd, ",",sizeof(","));
                serial_data_send(sfd, to_string(ypos).c_str(),sizeof(to_string(ypos).c_str()));
                serial_data_send(sfd, "\n",sizeof("\n"));
                
                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);
                }

                if( backprojMode )//转换显示方式，将backproj显示出来
                    cvtColor( backproj, image, CV_GRAY2BGR );
                //画出椭圆，第二个参数是一个矩形，画该矩形的内接圆
                
                if(trackBox.size.width > 0 && trackBox.size.height > 0)
                    ellipse( image, trackBox, Scalar(0,0,255), 3, CV_AA );
            

            }
        }
        else if( trackObject < 0 )
            paused = false;

        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);
        }

        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );

        //每轮都要等待用户的按键控制
        char c = (char)waitKey(10);
        if( c == 27 )//"Esc"键，直接退出
            break;
        switch(c)
        {
        case 'b'://转换显示方式
            backprojMode = !backprojMode;
            break;
        case 'c'://停止追踪
            trackObject = 0;
            histimg = Scalar::all(0);
            break;
        case 'h'://隐藏或显示直方图
            showHist = !showHist;
            if( !showHist )
                destroyWindow( "Histogram" );
            else
                namedWindow( "Histogram", 1 );
            break;
        case 'p'://暂停
            paused = !paused;//frame停止从摄像头获取图像，只显示旧的图像
            break;
        default:
            ;
        }
    }

    return 0;
}
