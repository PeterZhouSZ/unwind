#ifndef COLORS_H
#define COLORS_H

#include <Eigen/Core>

namespace ColorRGB {
  const Eigen::RowVector3d RED = Eigen::RowVector3d(1, 0, 0);
  const Eigen::RowVector3d GREEN = Eigen::RowVector3d(0, 1, 0);
  const Eigen::RowVector3d BLUE = Eigen::RowVector3d(0, 0, 1);
  const Eigen::RowVector3d DARK_GRAY = Eigen::RowVector3d(0.1, 0.1, 0.1);
  const Eigen::RowVector3d GRAY = Eigen::RowVector3d(0.5, 0.5, 0.5);
  const Eigen::RowVector3d STEEL_BLUE = Eigen::RowVector3d(70.0/255.0, 130.0/255.0, 180.0/255.0);
  const Eigen::RowVector3d LIGHT_GREEN = Eigen::RowVector3d(144.0/255.0, 238.0/255.0, 144.0/255.0);
  const Eigen::RowVector3d CRIMSON = Eigen::RowVector3d(220.0/255.0, 20.0/255.0, 60.0/255.0);
  const Eigen::RowVector3d BLACK = Eigen::RowVector3d(0, 0, 0);
  const Eigen::RowVector3d DARK_MAGENTA = Eigen::RowVector3d(139.0/255.0, 0.0/255.0, 139.0/255.0);
}

#endif // COLORS_H
