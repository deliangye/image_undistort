#include <image_undistort/undistorter.h>

Undistorter::Undistorter(const cv::Size& resolution,
                         const Eigen::Matrix<double, 3, 4>& P_in,
                         const Eigen::Matrix<double, 3, 4>& P_out,
                         const bool using_radtan,
                         const std::vector<double>& D) {
  // Initialize maps
  cv::Mat map_x_float(resolution, CV_32FC1);
  cv::Mat map_y_float(resolution, CV_32FC1);

  ROS_ERROR_STREAM(" " << P_in);
  ROS_ERROR_STREAM(" " << P_out);

  // Compute the remap maps
  for (size_t v = 0; v < resolution.height; ++v) {
    for (size_t u = 0; u < resolution.width; ++u) {
      Eigen::Vector2d pixel_location(u, v);
      Eigen::Vector2d distorted_pixel_location;
      distortPixel(P_in, P_out, using_radtan, D, pixel_location,
                   &distorted_pixel_location);

      // Insert in map
      map_x_float.at<float>(v, u) =
          static_cast<float>(distorted_pixel_location.x());
      map_y_float.at<float>(v, u) =
          static_cast<float>(distorted_pixel_location.y());
    };
  }

  // convert to fixed point maps for increased speed
  cv::convertMaps(map_x_float, map_y_float, map_x_, map_y_, CV_16SC2);
}

void Undistorter::undistortImage(const cv::Mat& image,
                                 cv::Mat* undistorted_image) {
  cv::remap(image, *undistorted_image, map_x_, map_y_, cv::INTER_LINEAR,
            cv::BORDER_CONSTANT);
}

void Undistorter::distortPixel(const Eigen::Matrix<double, 3, 4>& P_in,
                               const Eigen::Matrix<double, 3, 4>& P_out,
                               const bool using_radtan,
                               const std::vector<double>& D,
                               const Eigen::Vector2d& pixel_location,
                               Eigen::Vector2d* distorted_pixel_location) {
  // Transform image coordinates to be size and focus independent
  Eigen::Vector2d norm_pixel_location =
      P_out.topLeftCorner<2, 2>().inverse() *
      (pixel_location - P_out.block<2, 1>(0, 2) - P_out.block<2, 1>(0, 3));

  const double& x = norm_pixel_location.x();
  const double& y = norm_pixel_location.y();

  Eigen::Vector4d norm_distorted_pixel_location(0, 0, 1, 1);
  double& xd = norm_distorted_pixel_location.x();
  double& yd = norm_distorted_pixel_location.y();

  if (using_radtan) {
    // Split out parameters for easier reading
    const double& k1 = D[0];
    const double& k2 = D[1];
    const double& k3 = D[4];
    const double& p1 = D[2];
    const double& p2 = D[3];

    // Undistort
    const double r2 = x * x + y * y;
    const double r4 = r2 * r2;
    const double r6 = r4 * r2;
    const double kr = (1.0 + k1 * r2 + k2 * r4 + k3 * r6);
    xd = x * kr + 2.0 * p1 * x * y + p2 * (r2 + 2.0 * x * x);
    yd = y * kr + 2.0 * p2 * x * y + p1 * (r2 + 2.0 * y * y);

  } else {
    // Split out distortion parameters for easier reading
    const double& k1 = D[0];
    const double& k2 = D[1];
    const double& k3 = D[2];
    const double& k4 = D[3];

    // Undistort
    const double r = std::sqrt(x * x + y * y);
    if (r < 1e-10) {
      *distorted_pixel_location = pixel_location;
      return;
    }
    const double theta = atan(r);
    const double theta2 = theta * theta;
    const double theta4 = theta2 * theta2;
    const double theta6 = theta2 * theta4;
    const double theta8 = theta4 * theta4;
    const double thetad =
        theta * (1 + k1 * theta2 + k2 * theta4 + k3 * theta6 + k4 * theta8);

    const double scaling = (r > 1e-8) ? thetad / r : 1.0;
    xd = x * scaling;
    yd = y * scaling;
  }

  *distorted_pixel_location =
      P_in.topLeftCorner<2, 4>() * norm_distorted_pixel_location;
};