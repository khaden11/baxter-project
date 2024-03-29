#include <ros/ros.h>
#include <geometry_msgs/PointStamped.h>
#include <tf/transform_listener.h>
#include <tf/transform_datatypes.h>
#include <manipulation/LookForBowl.h>

#include <string>
#include "std_msgs/String.h"

ros::Publisher pub;

int counter = 0;

std::vector<float> xVals(20,0.0);
std::vector<float> yVals(20,0.0);
std::vector<float> zVals(20,0.0);

// Create a callback method on the receipt of the last bowl average
void
point_cb (const geometry_msgs::PointStamped point_msg)
{
    // Setup variables
    float frameCount = 20.0;
    int frameInt = 20;
    // Populate the vectors with the first 20 frames of average
    // bowl centres
    if (counter < frameInt) {
        xVals[counter] = point_msg.point.x;
        yVals[counter] = point_msg.point.y;
        zVals[counter] = point_msg.point.z;

        counter++;
    }
    // Once 20 frames was reached, increase the size of each
    // vector, creating a cumulative average
    if (counter >= frameInt) {
        xVals.push_back(point_msg.point.x);
        yVals.push_back(point_msg.point.y);
        zVals.push_back(point_msg.point.z);

        float averageX = 0;
        float averageY = 0;
        float averageZ = 0;

        // Each time, calculate the average centre from the vectors
        for(std::vector<float>::iterator it = xVals.begin(); it != xVals.end(); ++it)
          averageX += *it;
        for(std::vector<float>::iterator it = yVals.begin(); it != yVals.end(); ++it)
          averageY += *it;
        for(std::vector<float>::iterator it = zVals.begin(); it != zVals.end(); ++it)
          averageZ += *it;

        averageX /= counter;
        averageY /= counter;
        averageZ /= counter;

        // Create a pointstamped object from the value and publish
        geometry_msgs::PointStamped pt;
        pt.header = point_msg.header;
        pt.header.frame_id = point_msg.header.frame_id;
        pt.header.stamp = ros::Time();
        pt.point.x = averageX;
        pt.point.y = averageY;
        pt.point.z = averageZ;

        pub.publish(pt);

        counter++;
    }
}

// Subscribe function will reset the cumulative averaege counter
// and empty the vector values
bool add(manipulation::LookForBowl::Request &req,
         manipulation::LookForBowl::Response &res)
{
    res.ok = "OK";
    counter = 0;
    std::fill(xVals.begin(), xVals.end(), 0);
    std::fill(yVals.begin(), yVals.end(), 0);
    std::fill(zVals.begin(), zVals.end(), 0);
    return true;
}

// Main function
int
main (int argc, char** argv)
{
    // Initialize ROS
    ros::init (argc, argv, "second_average");
    ros::NodeHandle nh;

    // Subscribe to the table objects PointCloud2 from the kinect
    ros::Subscriber sub = nh.subscribe ("/bowlAverage", 1, point_cb);

    // Publish the cumulative bowl centre
    pub = nh.advertise<geometry_msgs::PointStamped> ("bowlPos", 1);
    // Subscribe to the service resetting the cumulative average
    ros::ServiceServer service = nh.advertiseService("reset_bowl", add);

    // Spin
    ros::spin ();
}
