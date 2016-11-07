#ifndef CAMERA_PARAMETERS_H
#define CAMERA_PARAMETERS_H

// holds basic properties of a camera
class BaseCameraParameters {
 public:
  BaseCameraParameters(const ros::NodeHandle& nh, const std::string& namespace);

  BaseCameraParameters(const sensor_msgs::CameraInfo& camera_info);

  BaseCameraParameters(const cv::Size& resolution,
                       const Eigen::Matrix<double, 4, 4>& T,
                       const Eigen::Matrix<double, 3, 4>& K);

  const cv::Size& resolution();  // get image size

  const Eigen::Matrix<double, 4, 4>& T();  // get transformation matrix
  const Eigen::Matrix<double, 3, 3>& R();  // get rotation matrix
  const Eigen::Matrix<double, 3, 1>& p();  // get position vector

  const Eigen::Matrix<double, 3, 4>& P();  // get projection matrix
  const Eigen::Matrix<double, 3, 3>& K();  // get camera matrix

 private:
  cv::Size resolution_;
  Eigen::Matrix<double, 4, 4> T_;
  Eigen::Matrix<double, 3, 3> K_;
};

// basic camera properties + distortion parameters
class InputCameraParameters : public BaseCameraParameters {
 public:
  InputCameraParameters(const ros::NodeHandle& nh,
                        const std::string& namespace);

  InputCameraParameters(const sensor_msgs::CameraInfo& camera_info);

  InputCameraParameters(const cv::Size& resolution,
                        const Eigen::Matrix<double, 4, 4>& T,
                        const Eigen::Matrix<double, 3, 4>& K,
                        const std::Vector<double>& D,
                        const bool R_loadedradtan_distortion);

  const std::Vector<double>& D();       // get distortion vector
  const bool& usingRadtanDistortion();  // gets if using radtan distortion

 private:
  std::Vector<double> D_;
  bool radtan_distortion_;
};

// basic camera properties + if undistortion needs to be performed
class OutputCameraParameters : public BaseCameraParameters {
 public:
  OutputCameraParameters(const ros::NodeHandle& nh,
                         const std::string& namespace);

  OutputCameraParameters(const sensor_msgs::CameraInfo& camera_info);

  OutputCameraParameters(const cv::Size& resolution,
                         const Eigen::Matrix<double, 4, 4>& T,
                         const Eigen::Matrix<double, 3, 4>& K,
                         const bool undistort);

  const bool& undistort();  // if the camera output will be undistorted

  void generateCameraInfoMessage(sensor_msgs::CameraInfo* camera_info);

 private:
  bool undistort_;
};

// holds the camera parameters of the input camera and virtual output camera
class CameraParametersPair {
 public:
  bool setCameraParameters(const ros::NodeHandle& nh,
                           const std::string& namespace, bool input_camera);

  bool setCameraParameters(const sensor_msgs::CameraInfo& camera_info,
                           bool input_camera);

  bool setInputCameraParameters(const cv::Size& resolution,
                                const Eigen::Matrix<double, 4, 4>& T,
                                const Eigen::Matrix<double, 3, 4>& K,
                                const std::Vector<double>& D,
                                const bool R_loadedradtan_distortion);

  bool setOutputCameraParameters(const cv::Size& resolution,
                                 const Eigen::Matrix<double, 4, 4>& T,
                                 const Eigen::Matrix<double, 3, 4>& K,
                                 const bool undistort);

  void generateOutputCameraInfoMessage(sensor_msgs::CameraInfo* camera_info);

  const Eigen::Matrix<double, 3, 4>& P_input();
  const Eigen::Matrix<double, 3, 4>& P_output();

  bool valid();
  bool valid(const bool input);

 private:
  std::shared_ptr<InputCameraParameters> input_;
  std::shared_ptr<OutputCameraParameters> output_;
};

// holds the camera parameters of the left and right camera and uses them to
// generate virtual output cameras with properties that will produce correctly
// rectified images
class StereoCameraParameters {
 public:
  bool setInputCameraParameters(const ros::NodeHandle& nh,
                                const std::string& namespace, bool left);

  bool setInputCameraParameters(const sensor_msgs::CameraInfo& camera_info,
                                bool left);

  bool setInputCameraParameters(const cv::Size& resolution,
                                const Eigen::Matrix<double, 4, 4>& T,
                                const Eigen::Matrix<double, 3, 4>& P,
                                const std::Vector<double>& D,
                                const bool radtan_distortion, const bool left);

  void generateOutputCameraInfoMessage(const bool left,
                                       sensor_msgs::CameraInfo* camera_info);

  const Eigen::Matrix<double, 3, 4>& P_left_input();
  const Eigen::Matrix<double, 3, 4>& P_left_output();

  const Eigen::Matrix<double, 3, 4>& P_right_input();
  const Eigen::Matrix<double, 3, 4>& P_right_output();

  bool valid();
  bool valid(const bool left, const bool input);

 private:
  CameraParametersPair left_;
  CameraParametersPair right_;
}