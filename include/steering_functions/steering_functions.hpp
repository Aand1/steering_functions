/*********************************************************************
*  Copyright (c) 2017 - for information on the respective copyright
*  owner see the NOTICE file and/or the repository
*
*      https://github.com/hbanzhaf/steering_functions.git
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
*  implied. See the License for the specific language governing
*  permissions and limitations under the License.
***********************************************************************/

#ifndef STEERING_FUNCTIONS_HPP
#define STEERING_FUNCTIONS_HPP

namespace steer
{
/** \brief Description of a kinematic car's state */
struct State
{
  /** \brief Position in x of the robot */
  double x;

  /** \brief Position in y of the robot */
  double y;

  /** \brief Orientation of the robot */
  double theta;

  /** \brief Curvature at position (x,y) */
  double kappa;

  /** \Driving direction {-1,0,1} */
  double d;
};

/** \brief Description of a path segment with its corresponding control inputs */
struct Control
{
  /** \brief Signed arc length of a segment */
  double delta_s;

  /** \brief Curvature at the beginning of a segment */
  double kappa;

  /** \brief Sharpness (derivative of curvature with respect to arc length) of a segment */
  double sigma;
};
}

#endif
