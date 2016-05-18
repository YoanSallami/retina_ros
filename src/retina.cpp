#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/contrib/retina.hpp>
#include <string>

using namespace std;
using namespace cv;

class RetinaNodelet
{
private:
  ros::NodeHandle node_;
  image_transport::ImageTransport * img_transport_;
  Retina * retina_;
  image_transport::Subscriber image_sub_;
  image_transport::Publisher parvo_pub_;
  image_transport::Publisher magno_pub_;
public:
  /**
   * Default constructor
   */
  RetinaNodelet(ros::NodeHandle& node)
  {
    node_ = node;
    img_transport_ = new image_transport::ImageTransport(node_);
    image_sub_ = img_transport_->subscribe("/camera/image_raw", 1, &RetinaNodelet::imageCallback, this);
    parvo_pub_ = img_transport_->advertise("/retina/parvo", 1);
    magno_pub_ = img_transport_->advertise("/retina/magno", 1);
    retina_ = new Retina(Size(640,480));
    retina_->write("RetinaDefaultParameters.xml");
    //myRetina->setup("/RetinaSpecificParameters.xml");
  }
  /** 
   * Default destructor
   */
  ~RetinaNodelet()
  {

  }

  void imageCallback(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImage parvo;
    cv_bridge::CvImage magno;
    
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("[cv_bridge] Exception was caught : %s", e.what());
      return;
    }
    
    retina_->run(cv_ptr->image);

    retina_->getParvo(parvo.image);
    parvo.header=msg->header;
    parvo.encoding="bgr8";

    retina_->getMagno(magno.image);
    magno.header=msg->header;
    magno.encoding="mono8";

    parvo_pub_.publish(parvo.toImageMsg());
    magno_pub_.publish(magno.toImageMsg());
  }


};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "retina");
  ros::NodeHandle n;
  RetinaNodelet * r = new RetinaNodelet(n);
  while(ros::ok())
  {
    ros::spinOnce();
  }
}