#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <turtlesim/Pose.h>
#include <cmath>

/*
    題目：烏龜繞場一周
*/

// 欲發布之 前進x 角度z
float linX, angZ;

// 目標位置 現在位置
float destX, destY, nowX, nowY, nowTheta;

// 可容許之誤差
const float tolerance = 0.1;

// 任務編號
int mission = 0;

// 到 目的地 的 距離
float getDistance()
{
    return sqrt( pow(nowX-destX,2) + pow(nowY-destY,2) );
}

// 到 目的地 的 角度
float getDistAngle()
{
    return atan2(destY-nowY, destX-nowX);
}

// 控制方向盤
void setWheel(float &x, float &z, float xConst=1.5, float zConst=12)
{
    x = xConst * getDistance();
    z = zConst * (getDistAngle() - nowTheta);
}

// 到目的地？
bool check_dest()
{
    float dist = getDistance();

    // 與目的地之距離 小於誤差 即表示任務成功
    if(dist < tolerance)
    {
        return true;
    }
    // 未完成任務
    return false;
}

void t1PoseCallback(const turtlesim::PoseConstPtr& msg)
{
    // 更新 x,y,theta
    nowX = msg->x;
    nowY = msg->y;
    nowTheta = msg->theta;
    ROS_INFO("now[x, y, theta] = [%.2f, %.2f, .%2f]", nowX, nowY, nowTheta);
}

// 更新任務點座標(0~11.08 , )
void update_dest()
{
    // 目標位置序列
    switch(mission)
    {
        case 0:
            destX = 10.0;
            destY = 0.5;
            break;
        case 1:
            destX = 10.0;
            destY = 10.0;
            break;
        case 2:
            destX = 0.5;
            destY = 10.0;
            break;
        case 3:
            destX = 0.5;
            destY = 0.5;
            break;
        case 4:
            destX = 10.0;
            destY = 0.5;
            break;
        case 5:
            destX = 6.0;
            destY = 6.0;
            break;
        default:
            ROS_INFO("all mission completed.");
            ros::shutdown();
    }
};

int main(int argc, char **argv)
{
    // init
    ros::init(argc, argv, "mv_turtle1");
    // node
    ros::NodeHandle nh;
    // subscriber
    ros::Subscriber sub = nh.subscribe("/turtle1/pose",10,&t1PoseCallback);
    // publisher
    ros::Publisher pub = nh.advertise<geometry_msgs::Twist>("/turtle1/cmd_vel", 10);
    // rate for sleep
    ros::Rate rate(10);

    // 初始化變數
    destX=0, destY=0, linX=0, angZ=0, mission=0;

    while(ros::ok())
    {
        // 抓任務，會更新：任務目標點
        update_dest();

        // log
        ROS_INFO_STREAM("mission number:" << mission);
        ROS_INFO("new dest:[%f,%f]", destX, destY);

        geometry_msgs::Twist msg;

        // 確認是否抵達目的地
        while(check_dest()==false)
        {
            ros::spinOnce();

            // 控制方向盤
            setWheel(linX, angZ);

            msg.linear.x = linX;
            msg.linear.y = 0;
            msg.linear.z = 0;
            msg.angular.x = 0;
            msg.angular.y = 0;
            msg.angular.z = angZ;

            // 發布任務
            pub.publish(msg);
            rate.sleep();
        }

        // 停車
        msg.linear.x = 0;
        msg.angular.z = 0;

        // 發布停車指令
        pub.publish(msg);

        // log停車
        ROS_INFO_STREAM("arrived, mission:"<<mission<<", mission complete!");

        // 抵達 則切換為下一個任務
        mission++;

        rate.sleep();
    }
}