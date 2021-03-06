#ifndef UNDISTORTER_H
#define UNDISTORTER_H

#include <Eigen/Eigen>

#include <ros/ros.h>
#include <iostream>

#include <cv.h>
#include <highgui.h>
#include "opencv2/imgproc/imgproc.hpp"

class Undistorter {
 public:
  Undistorter(const cv::Size& resolution,
              const Eigen::Matrix<double, 3, 4>& P_in,
              const Eigen::Matrix<double, 3, 4>& P_out, const bool using_radtan,
              const std::vector<double>& D);

  void undistortImage(const cv::Mat& image, cv::Mat* undistored_image);

 private:
  void distortPixel(const Eigen::Matrix<double, 3, 4>& P_in,
                    const Eigen::Matrix<double, 3, 4>& P_out,
                    const bool using_radtan, const std::vector<double>& D,
                    const Eigen::Vector2d& pixel_location,
                    Eigen::Vector2d* distorted_pixel_location);

  cv::Mat map_x_;
  cv::Mat map_y_;
};

#endif