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

*  This source code is derived from Continuous Curvature (CC) Steer.
*  Copyright (c) 2016, Thierry Fraichard and Institut national de
*  recherche en informatique et en automatique (Inria), licensed under
*  the BSD license, cf. 3rd-party-licenses.txt file in the root
*  directory of this source tree.
**********************************************************************/

#include "steering_functions/hc_cc_state_space/hcpmpm_reeds_shepp_state_space.hpp"

#define HC_REGULAR false
#define CC_REGULAR false

class HCpmpm_Reeds_Shepp_State_Space::HCpmpm_Reeds_Shepp
{
private:
  HCpmpm_Reeds_Shepp_State_Space *parent_;

public:
  explicit HCpmpm_Reeds_Shepp(HCpmpm_Reeds_Shepp_State_Space *parent)
  {
    parent_ = parent;
  }

  double distance;
  double angle;

  // ##### TT ###################################################################
  bool TT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return fabs(distance - 2 * parent_->radius_) < get_epsilon();
  }

  double TT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                 Configuration **q1, Configuration **q2, Configuration **q3)
  {
    double x = (c1.xc + c2.xc) / 2;
    double y = (c1.yc + c2.yc) / 2;
    double angle = atan2(c2.yc - c1.yc, c2.xc - c1.xc);
    double theta;
    if (c1.left)
    {
      if (c1.forward)
      {
        theta = angle + HALF_PI - parent_->mu_;
      }
      else
      {
        theta = angle + HALF_PI + parent_->mu_;
      }
    }
    else
    {
      if (c1.forward)
      {
        theta = angle - HALF_PI + parent_->mu_;
      }
      else
      {
        theta = angle - HALF_PI - parent_->mu_;
      }
    }
    *q2 = new Configuration(x, y, theta, 0);
    if (cstart && cend && q1 && q3)
    {
      *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
      *cend = new HC_CC_Circle(**q2, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
      *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
      *q3 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
      return (*cstart)->hc_turn_length(**q1) + (*cend)->hc_turn_length(**q3);
    }
    return numeric_limits<double>::max();
  }

  // ##### TcT ##################################################################
  bool TcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return fabs(distance - fabs(2 / c1.kappa)) < get_epsilon();
  }

  double TcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                  Configuration **q)
  {
    double distance = center_distance(c1, c2);
    double delta_x = 0.5 * distance;
    double delta_y = 0.0;
    double angle = atan2(c2.yc - c1.yc, c2.xc - c1.xc);
    double x, y, theta;
    if (c1.left)
    {
      if (c1.forward)
      {
        theta = angle + HALF_PI;
        global_frame_change(c1.xc, c1.yc, angle, delta_x, delta_y, &x, &y);
      }
      else
      {
        theta = angle + HALF_PI;
        global_frame_change(c1.xc, c1.yc, angle, delta_x, -delta_y, &x, &y);
      }
    }
    else
    {
      if (c1.forward)
      {
        theta = angle - HALF_PI;
        global_frame_change(c1.xc, c1.yc, angle, delta_x, -delta_y, &x, &y);
      }
      else
      {
        theta = angle - HALF_PI;
        global_frame_change(c1.xc, c1.yc, angle, delta_x, delta_y, &x, &y);
      }
    }
    *q = new Configuration(x, y, theta, c1.kappa);
    if (cstart && cend)
    {
      *cstart = new HC_CC_Circle(c1);
      *cend = new HC_CC_Circle(c2);
      return (*cstart)->rs_turn_length(**q) + (*cend)->rs_turn_length(**q);
    }
    return numeric_limits<double>::max();
  }

  // ##### Reeds-Shepp families: ################################################

  // ##### TcTcT ################################################################
  bool TcTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return distance <= fabs(4 / c1.kappa);
  }

  void TcTcT_tangent_circles(const HC_CC_Circle &c1, const HC_CC_Circle &c2, Configuration **q1, Configuration **q2,
                             Configuration **q3, Configuration **q4)
  {
    double theta = angle;
    double r = fabs(2 / c1.kappa);
    double delta_x = 0.5 * distance;
    double delta_y = sqrt(fabs(pow(r, 2) - pow(delta_x, 2)));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, !c1.forward, c1.regular, parent_->rs_circle_param_);
    global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c1.left, !c1.forward, c1.regular, parent_->rs_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TcT_path(tgt1, c2, nullptr, nullptr, q2);
    TcT_path(c1, tgt2, nullptr, nullptr, q3);
    TcT_path(tgt2, c2, nullptr, nullptr, q4);
  }

  double TcTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, HC_CC_Circle **ci)
  {
    Configuration *qa, *qb, *qc, *qd;
    TcTcT_tangent_circles(c1, c2, &qa, &qb, &qc, &qd);
    HC_CC_Circle *middle1, *middle2;
    middle1 = new HC_CC_Circle(*qa, !c1.left, !c1.forward, true, parent_->rs_circle_param_);
    middle2 = new HC_CC_Circle(*qc, !c1.left, !c1.forward, true, parent_->rs_circle_param_);

    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(c2);

    // select shortest connection
    double length1 = (*cstart)->rs_turn_length(*qa) + middle1->rs_turn_length(*qb) + (*cend)->rs_turn_length(*qb);
    double length2 = (*cstart)->rs_turn_length(*qc) + middle2->rs_turn_length(*qd) + (*cend)->rs_turn_length(*qd);
    if (length1 < length2)
    {
      *q1 = qa;
      *q2 = qb;
      *ci = middle1;
      delete qc;
      delete qd;
      delete middle2;
      return length1;
    }
    else
    {
      *q1 = qc;
      *q2 = qd;
      *ci = middle2;
      delete qa;
      delete qb;
      delete middle1;
      return length2;
    }
    return numeric_limits<double>::max();
  }

  // ##### TcTT #################################################################
  bool TcTT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return (distance <= 2 * parent_->radius_ + 2 / fabs(c1.kappa)) &&
           (distance >= 2 * parent_->radius_ - 2 / fabs(c1.kappa));
  }

  void TcTT_tangent_circles(const HC_CC_Circle &c1, const HC_CC_Circle &c2, Configuration **q1, Configuration **q2,
                            Configuration **q3, Configuration **q4)
  {
    double theta = angle;
    double r1 = 2 / fabs(c1.kappa);
    double r2 = 2 * parent_->radius_;
    double delta_x = (pow(r1, 2) + pow(distance, 2) - pow(r2, 2)) / (2 * distance);
    double delta_y = sqrt(fabs(pow(r1, 2) - pow(delta_x, 2)));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, !c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c1.left, !c1.forward, c1.regular, parent_->hc_cc_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TT_path(tgt1, c2, nullptr, nullptr, nullptr, q2, nullptr);
    TcT_path(c1, tgt2, nullptr, nullptr, q3);
    TT_path(tgt2, c2, nullptr, nullptr, nullptr, q4, nullptr);
  }

  double TcTT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                   Configuration **q1, Configuration **q2, HC_CC_Circle **ci)
  {
    Configuration *qa, *qb, *qc, *qd;
    TcTT_tangent_circles(c1, c2, &qa, &qb, &qc, &qd);
    HC_CC_Circle *end1, *end2, *middle1, *middle2;
    end1 = new HC_CC_Circle(*qb, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    end2 = new HC_CC_Circle(*qd, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    middle1 = new HC_CC_Circle(*qb, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    middle2 = new HC_CC_Circle(*qd, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);

    *cstart = new HC_CC_Circle(c1);
    *q2 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);

    // select shortest connection
    double length1 = (*cstart)->rs_turn_length(*qa) + middle1->hc_turn_length(*qa) + end1->hc_turn_length(**q2);
    double length2 = (*cstart)->rs_turn_length(*qc) + middle2->hc_turn_length(*qc) + end2->hc_turn_length(**q2);
    if (length1 < length2)
    {
      *cend = end1;
      *q1 = qa;
      *ci = middle1;
      delete qb;
      delete qc;
      delete qd;
      delete end2;
      delete middle2;
      return length1;
    }
    else
    {
      *cend = end2;
      *q1 = qc;
      *ci = middle2;
      delete qa;
      delete qb;
      delete qd;
      delete end1;
      delete middle1;
      return length2;
    }
    return numeric_limits<double>::max();
  }

  // ##### TTcT #################################################################
  bool TTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return (distance <= 2 * parent_->radius_ + 2 / fabs(c1.kappa)) &&
           (distance >= 2 * parent_->radius_ - 2 / fabs(c1.kappa));
  }

  void TTcT_tangent_circles(const HC_CC_Circle &c1, const HC_CC_Circle &c2, Configuration **q1, Configuration **q2,
                            Configuration **q3, Configuration **q4)
  {
    double theta = angle;
    double r1 = 2 * parent_->radius_;
    double r2 = 2 / fabs(c1.kappa);
    double delta_x = (pow(r1, 2) + pow(distance, 2) - pow(r2, 2)) / (2 * distance);
    double delta_y = sqrt(fabs(pow(r1, 2) - pow(delta_x, 2)));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c1.left, c1.forward, c1.regular, parent_->hc_cc_circle_param_);

    TT_path(c1, tgt1, nullptr, nullptr, nullptr, q1, nullptr);
    TcT_path(tgt1, c2, nullptr, nullptr, q2);
    TT_path(c1, tgt2, nullptr, nullptr, nullptr, q3, nullptr);
    TcT_path(tgt2, c2, nullptr, nullptr, q4);
  }

  double TTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                   Configuration **q1, Configuration **q2, HC_CC_Circle **ci)
  {
    Configuration *qa, *qb, *qc, *qd;
    TTcT_tangent_circles(c1, c2, &qa, &qb, &qc, &qd);
    HC_CC_Circle *start1, *start2, *middle1, *middle2;
    start1 = new HC_CC_Circle(*qa, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    start2 = new HC_CC_Circle(*qc, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    middle1 = new HC_CC_Circle(*qa, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    middle2 = new HC_CC_Circle(*qc, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);

    *cend = new HC_CC_Circle(c2);
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);

    // select shortest connection
    double length1 = start1->hc_turn_length(**q1) + middle1->hc_turn_length(*qb) + (*cend)->rs_turn_length(*qb);
    double length2 = start2->hc_turn_length(**q1) + middle2->hc_turn_length(*qd) + (*cend)->rs_turn_length(*qd);
    if (length1 < length2)
    {
      *cstart = start1;
      *q2 = qb;
      *ci = middle1;
      delete qa;
      delete qc;
      delete qd;
      delete start2;
      delete middle2;
      return length1;
    }
    else
    {
      *cstart = start2;
      *q2 = qd;
      *ci = middle2;
      delete qa;
      delete qb;
      delete qc;
      delete start1;
      delete middle1;
      return length2;
    }
    return numeric_limits<double>::max();
  }

  // ##### TST ##################################################################
  bool TiST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return distance >= 2 * parent_->radius_;
  }

  bool TeST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return distance >= 2 * parent_->radius_ * parent_->sin_mu_;
  }

  bool TST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TiST_exists(c1, c2) || TeST_exists(c1, c2);
  }

  double TiST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                   Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    double distance = center_distance(c1, c2);
    double angle = atan2(c2.yc - c1.yc, c2.xc - c1.xc);
    double alpha = fabs(asin(2 * parent_->radius_ * parent_->cos_mu_ / distance));
    double delta_x = fabs(parent_->radius_ * parent_->sin_mu_);
    double delta_y = fabs(parent_->radius_ * parent_->cos_mu_);
    double x, y, theta;
    if (c1.left && c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (c1.left && !c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (!c1.left && c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (!c1.left && !c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (cstart && cend && q1 && q4)
    {
      *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
      *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
      *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
      *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
      return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*cend)->hc_turn_length(**q4);
    }
    return numeric_limits<double>::max();
  }

  double TeST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                   Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    double delta_x = fabs(parent_->radius_ * parent_->sin_mu_);
    double delta_y = fabs(parent_->radius_ * parent_->cos_mu_);
    double theta = atan2(c2.yc - c1.yc, c2.xc - c1.xc);
    double x, y;
    if (c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (!c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (!c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }

    if (cstart && cend && q1 && q4)
    {
      *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
      *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
      *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
      *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
      return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*cend)->hc_turn_length(**q4);
    }
    return numeric_limits<double>::max();
  }

  double TST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                  Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    if (TiST_exists(c1, c2))
    {
      return TiST_path(c1, c2, cstart, cend, q1, q2, q3, q4);
    }
    if (TeST_exists(c1, c2))
    {
      return TeST_path(c1, c2, cstart, cend, q1, q2, q3, q4);
    }
    return numeric_limits<double>::max();
  }

  // ##### TSTcT ################################################################
  bool TiSTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return distance >= sqrt(pow(2 * parent_->radius_ * parent_->sin_mu_ + 2 / fabs(c1.kappa), 2) +
                            pow(2 * parent_->radius_ * parent_->cos_mu_, 2));
  }

  bool TeSTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return distance >= 2 * (1 / fabs(c1.kappa) + parent_->radius_ * parent_->sin_mu_);
  }

  bool TSTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TiSTcT_exists(c1, c2) || TeSTcT_exists(c1, c2);
  }

  double TiSTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4, HC_CC_Circle **ci)
  {
    double theta = angle;
    double delta_y = (4 * parent_->radius_ * parent_->cos_mu_) / (fabs(c2.kappa) * distance);
    double delta_x = sqrt(pow(2 / c2.kappa, 2) - pow(delta_y, 2));
    double x, y;

    global_frame_change(c2.xc, c2.yc, theta, -delta_x, +delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c2.left, c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TiST_path(c1, tgt1, nullptr, nullptr, nullptr, q2, q3, nullptr);
    TcT_path(tgt1, c2, nullptr, nullptr, q4);

    *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *cend = new HC_CC_Circle(c2);
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *ci = new HC_CC_Circle(**q3, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);

    return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*ci)->hc_turn_length(**q4) +
           (*cend)->rs_turn_length(**q4);
  }

  double TeSTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4, HC_CC_Circle **ci)
  {
    double theta = angle;
    double delta_x = 2 / fabs(c2.kappa);
    double delta_y = 0;
    double x, y;

    global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c2.left, c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TeST_path(c1, tgt1, nullptr, nullptr, nullptr, q2, q3, nullptr);
    TcT_path(tgt1, c2, nullptr, nullptr, q4);

    *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *cend = new HC_CC_Circle(c2);
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *ci = new HC_CC_Circle(**q3, c1.left, c1.forward, true, parent_->hc_cc_circle_param_);

    return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*ci)->hc_turn_length(**q4) +
           (*cend)->rs_turn_length(**q4);
  }

  double TSTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4, HC_CC_Circle **ci)
  {
    if (TiSTcT_exists(c1, c2))
    {
      return TiSTcT_path(c1, c2, cstart, cend, q1, q2, q3, q4, ci);
    }
    if (TeSTcT_exists(c1, c2))
    {
      return TeSTcT_path(c1, c2, cstart, cend, q1, q2, q3, q4, ci);
    }
    return numeric_limits<double>::max();
  }

  // ##### TcTST ################################################################
  bool TcTiST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return (distance >= sqrt(pow(2 * parent_->radius_ * parent_->sin_mu_ + 2 / fabs(c1.kappa), 2) +
                             pow(2 * parent_->radius_ * parent_->cos_mu_, 2)));
  }

  bool TcTeST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return (distance >= 2 * (1 / fabs(c1.kappa) + parent_->radius_ * parent_->sin_mu_));
  }

  bool TcTST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TcTiST_exists(c1, c2) || TcTeST_exists(c1, c2);
  }

  double TcTiST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4, HC_CC_Circle **ci)
  {
    double theta = angle;
    double delta_y = (4 * c2.radius) / (fabs(c2.kappa) * distance);
    double delta_x = sqrt(pow(2 / c2.kappa, 2) - pow(delta_y, 2));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c2.left, !c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TiST_path(tgt1, c2, nullptr, nullptr, nullptr, q2, q3, nullptr);

    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
    *ci = new HC_CC_Circle(**q2, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);

    return (*cstart)->rs_turn_length(**q1) + (*ci)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) +
           (*cend)->hc_turn_length(**q4);
  }

  double TcTeST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4, HC_CC_Circle **ci)
  {
    double theta = angle;
    double delta_x = 2 / fabs(c2.kappa);
    double delta_y = 0;
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, c2.left, !c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TeST_path(tgt1, c2, nullptr, nullptr, nullptr, q2, q3, nullptr);

    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
    *ci = new HC_CC_Circle(**q2, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);

    return (*cstart)->rs_turn_length(**q1) + (*ci)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) +
           (*cend)->hc_turn_length(**q4);
  }

  double TcTST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4, HC_CC_Circle **ci)
  {
    if (TcTiST_exists(c1, c2))
    {
      return TcTiST_path(c1, c2, cstart, cend, q1, q2, q3, q4, ci);
    }
    if (TcTeST_exists(c1, c2))
    {
      return TcTeST_path(c1, c2, cstart, cend, q1, q2, q3, q4, ci);
    }
    return numeric_limits<double>::max();
  }

  // ##### TcTSTcT ##############################################################
  bool TcTiSTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return (distance >= sqrt(pow(2 * parent_->radius_, 2) + 16 * parent_->radius_ * parent_->sin_mu_ / fabs(c1.kappa) +
                             pow(4 / c1.kappa, 2)));
  }

  bool TcTeSTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return (distance >= 4 / fabs(c1.kappa) + 2 * parent_->radius_ * parent_->sin_mu_);
  }

  bool TcTSTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TcTiSTcT_exists(c1, c2) || TcTeSTcT_exists(c1, c2);
  }

  double TcTiSTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                       Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4,
                       HC_CC_Circle **ci1, HC_CC_Circle **ci2)
  {
    double theta = angle;
    double delta_y = (4 * parent_->radius_ * parent_->cos_mu_) / (distance * fabs(c1.kappa));
    double delta_x = sqrt(pow(2 / c1.kappa, 2) - pow(delta_y, 2));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, !c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c2.left, c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TiST_path(tgt1, tgt2, nullptr, nullptr, nullptr, q2, q3, nullptr);
    TcT_path(tgt2, c2, nullptr, nullptr, q4);

    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(c2);
    *ci1 = new HC_CC_Circle(**q2, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    *ci2 = new HC_CC_Circle(**q3, !c2.left, c2.forward, true, parent_->hc_cc_circle_param_);

    return (*cstart)->rs_turn_length(**q1) + (*ci1)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) +
           (*ci2)->hc_turn_length(**q4) + (*cend)->rs_turn_length(**q4);
  }

  double TcTeSTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                       Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4,
                       HC_CC_Circle **ci1, HC_CC_Circle **ci2)
  {
    double theta = angle;
    double delta_x = 2 / fabs(c1.kappa);
    double delta_y = 0;
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, !c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c2.left, c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TeST_path(tgt1, tgt2, nullptr, nullptr, nullptr, q2, q3, nullptr);
    TcT_path(tgt2, c2, nullptr, nullptr, q4);

    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(c2);
    *ci1 = new HC_CC_Circle(**q2, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    *ci2 = new HC_CC_Circle(**q3, !c2.left, c2.forward, true, parent_->hc_cc_circle_param_);

    return (*cstart)->rs_turn_length(**q1) + (*ci1)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) +
           (*ci2)->hc_turn_length(**q4) + (*cend)->rs_turn_length(**q4);
  }

  double TcTSTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                      Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4,
                      HC_CC_Circle **ci1, HC_CC_Circle **ci2)
  {
    if (TcTiSTcT_exists(c1, c2))
    {
      return TcTiSTcT_path(c1, c2, cstart, cend, q1, q2, q3, q4, ci1, ci2);
    }
    if (TcTeSTcT_exists(c1, c2))
    {
      return TcTeSTcT_path(c1, c2, cstart, cend, q1, q2, q3, q4, ci1, ci2);
    }
    return numeric_limits<double>::max();
  }

  // ##### TTcTT ###############################################################
  bool TTcTT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return (distance <= 4 * parent_->radius_ + 2 / fabs(c1.kappa));
  }

  void TTcTT_tangent_circles(const HC_CC_Circle &c1, const HC_CC_Circle &c2, Configuration **q1, Configuration **q2,
                             Configuration **q3, Configuration **q4, Configuration **q5, Configuration **q6)
  {
    double theta = angle;
    double r1, r2, delta_x, delta_y, x, y;
    r1 = 2 / fabs(c1.kappa);
    r2 = 2 * parent_->radius_;
    if (distance < 4 * parent_->radius_ - 2 / fabs(c1.kappa))
    {
      delta_x = (distance + r1) / 2;
      delta_y = sqrt(fabs((pow(r2, 2) - pow((distance + r1) / 2, 2))));
    }
    else
    {
      delta_x = (distance - r1) / 2;
      delta_y = sqrt(fabs((pow(r2, 2) - pow((distance - r1) / 2, 2))));
    }

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c2.left, !c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt3(x, y, !c1.left, c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt4(x, y, !c2.left, !c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TT_path(c1, tgt1, nullptr, nullptr, nullptr, q1, nullptr);
    TcT_path(tgt1, tgt2, nullptr, nullptr, q2);
    TT_path(tgt2, c2, nullptr, nullptr, nullptr, q3, nullptr);

    TT_path(c1, tgt3, nullptr, nullptr, nullptr, q4, nullptr);
    TcT_path(tgt3, tgt4, nullptr, nullptr, q5);
    TT_path(tgt4, c2, nullptr, nullptr, nullptr, q6, nullptr);
  }

  double TTcTT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, HC_CC_Circle **ci1, HC_CC_Circle **ci2)
  {
    Configuration *qa, *qb, *qc, *qd, *qe, *qf;
    TTcTT_tangent_circles(c1, c2, &qa, &qb, &qc, &qd, &qe, &qf);
    HC_CC_Circle *start1, *start2, *end1, *end2, *middle1, *middle2, *middle3, *middle4;
    start1 = new HC_CC_Circle(*qa, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    middle1 = new HC_CC_Circle(*qa, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    middle2 = new HC_CC_Circle(*qc, !c2.left, c2.forward, true, parent_->hc_cc_circle_param_);
    end1 = new HC_CC_Circle(*qc, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    start2 = new HC_CC_Circle(*qd, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    middle3 = new HC_CC_Circle(*qd, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    middle4 = new HC_CC_Circle(*qf, !c2.left, c2.forward, true, parent_->hc_cc_circle_param_);
    end2 = new HC_CC_Circle(*qf, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);

    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *q3 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);

    // select shortest connection
    double length1 = start1->hc_turn_length(**q1) + middle1->hc_turn_length(*qb) + middle2->hc_turn_length(*qb) +
                     end1->hc_turn_length(**q3);
    double length2 = start2->hc_turn_length(**q1) + middle3->hc_turn_length(*qe) + middle4->hc_turn_length(*qe) +
                     end2->hc_turn_length(**q3);
    if (length1 < length2)
    {
      *cstart = start1;
      *cend = end1;
      *ci1 = middle1;
      *ci2 = middle2;
      *q2 = qb;
      delete qa;
      delete qc;
      delete qd;
      delete qe;
      delete qf;
      delete start2, delete end2, delete middle3;
      delete middle4;
      return length1;
    }
    else
    {
      *cstart = start2;
      *cend = end2;
      *ci1 = middle3;
      *ci2 = middle4;
      *q2 = qe;
      delete qa;
      delete qb;
      delete qc;
      delete qd;
      delete qf;
      delete start1;
      delete end1;
      delete middle1;
      delete middle2;
      return length2;
    }
    return numeric_limits<double>::max();
  }

  // ##### TcTTcT ###############################################################
  bool TcTTcT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return (distance <= 4 / fabs(c1.kappa) + 2 * parent_->radius_) &&
           (distance >= 4 / fabs(c1.kappa) - 2 * parent_->radius_);
  }

  void TcTTcT_tangent_circles(const HC_CC_Circle &c1, const HC_CC_Circle &c2, Configuration **q1, Configuration **q2,
                              Configuration **q3, Configuration **q4, Configuration **q5, Configuration **q6)
  {
    double theta = angle;
    double r1 = 2 / fabs(c1.kappa);
    double r2 = parent_->radius_;
    double delta_x = (pow(r1, 2) + pow(distance / 2, 2) - pow(r2, 2)) / distance;
    double delta_y = sqrt(fabs(pow(r1, 2) - pow(delta_x, 2)));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, !c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c2.left, c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt3(x, y, !c1.left, !c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt4(x, y, !c2.left, c2.forward, c2.regular, parent_->hc_cc_circle_param_);

    TcT_path(c1, tgt1, nullptr, nullptr, q1);
    TT_path(tgt1, tgt2, nullptr, nullptr, nullptr, q2, nullptr);
    TcT_path(tgt2, c2, nullptr, nullptr, q3);

    TcT_path(c1, tgt3, nullptr, nullptr, q4);
    TT_path(tgt3, tgt4, nullptr, nullptr, nullptr, q5, nullptr);
    TcT_path(tgt4, c2, nullptr, nullptr, q6);
  }

  double TcTTcT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2, HC_CC_Circle **ci1, HC_CC_Circle **ci2)
  {
    Configuration *qa, *qb, *qc, *qd, *qe, *qf;
    TcTTcT_tangent_circles(c1, c2, &qa, &qb, &qc, &qd, &qe, &qf);
    HC_CC_Circle *middle1, *middle2, *middle3, *middle4;
    middle1 = new HC_CC_Circle(*qb, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    middle2 = new HC_CC_Circle(*qb, c1.left, !c1.forward, true, parent_->hc_cc_circle_param_);
    middle3 = new HC_CC_Circle(*qe, !c1.left, c1.forward, true, parent_->hc_cc_circle_param_);
    middle4 = new HC_CC_Circle(*qe, c1.left, !c1.forward, true, parent_->hc_cc_circle_param_);

    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(c2);

    // select shortest connection
    double length1 = (*cstart)->rs_turn_length(*qa) + middle1->hc_turn_length(*qa) + middle2->hc_turn_length(*qc) +
                     (*cend)->rs_turn_length(*qc);
    double length2 = (*cstart)->rs_turn_length(*qd) + middle3->hc_turn_length(*qd) + middle4->hc_turn_length(*qf) +
                     (*cend)->rs_turn_length(*qf);
    if (length1 < length2)
    {
      *q1 = qa;
      *q2 = qc;
      *ci1 = middle1;
      *ci2 = middle2;
      delete qb;
      delete qd;
      delete qe;
      delete qf;
      delete middle3;
      delete middle4;
      return length1;
    }
    else
    {
      *q1 = qd;
      *q2 = qf;
      *ci1 = middle3;
      *ci2 = middle4;
      delete qa;
      delete qb;
      delete qc;
      delete qe;
      delete middle1;
      delete middle2;
      return length2;
    }
    return numeric_limits<double>::max();
  }

  // ############################################################################

  // ##### TTT ##################################################################
  bool TTT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return distance <= 4 * parent_->radius_;
  }

  void TTT_tangent_circles(const HC_CC_Circle &c1, const HC_CC_Circle &c2, Configuration **q1, Configuration **q2,
                           Configuration **q3, Configuration **q4)
  {
    double theta = angle;
    double r = 2 * parent_->radius_;
    double delta_x = 0.5 * distance;
    double delta_y = sqrt(fabs(pow(delta_x, 2) - pow(r, 2)));
    double x, y;

    global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
    HC_CC_Circle tgt1(x, y, !c1.left, c1.forward, c1.regular, parent_->hc_cc_circle_param_);
    global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
    HC_CC_Circle tgt2(x, y, !c1.left, c1.forward, c1.regular, parent_->hc_cc_circle_param_);

    TT_path(c1, tgt1, nullptr, nullptr, nullptr, q1, nullptr);
    TT_path(tgt1, c2, nullptr, nullptr, nullptr, q2, nullptr);
    TT_path(c1, tgt2, nullptr, nullptr, nullptr, q3, nullptr);
    TT_path(tgt2, c2, nullptr, nullptr, nullptr, q4, nullptr);
  }

  double TTT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                  Configuration **q1, Configuration **q2, Configuration **q3, HC_CC_Circle **ci)
  {
    Configuration *qa, *qb, *qc, *qd;
    TTT_tangent_circles(c1, c2, &qa, &qb, &qc, &qd);
    HC_CC_Circle *start1, *start2, *end1, *end2, *middle1, *middle2;
    start1 = new HC_CC_Circle(*qa, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    middle1 = new HC_CC_Circle(*qa, !c1.left, c1.forward, CC_REGULAR, parent_->hc_cc_circle_param_);
    end1 = new HC_CC_Circle(*qb, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    start2 = new HC_CC_Circle(*qc, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    middle2 = new HC_CC_Circle(*qc, !c1.left, c1.forward, CC_REGULAR, parent_->hc_cc_circle_param_);
    end2 = new HC_CC_Circle(*qd, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);

    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *q3 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);

    // select shortest connection
    double length1 = start1->hc_turn_length(**q1) + middle1->cc_turn_length(*qb) + end1->hc_turn_length(**q3);
    double length2 = start2->hc_turn_length(**q1) + middle2->cc_turn_length(*qd) + end2->hc_turn_length(**q3);
    if (length1 < length2)
    {
      *cstart = start1;
      *ci = middle1;
      *cend = end1;
      *q2 = qb;
      delete qa;
      delete qc;
      delete qd;
      delete start2;
      delete middle2;
      delete end2;
      return length1;
    }
    else
    {
      *cstart = start2;
      *ci = middle2;
      *cend = end2;
      *q2 = qd;
      delete qa;
      delete qb;
      delete qc;
      delete start1;
      delete middle1;
      delete end1;
      return length2;
    }
    return numeric_limits<double>::max();
  }

  // ##### TcST ################################################################
  bool TciST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return distance >= 2 * parent_->radius_ * parent_->cos_mu_;
  }

  bool TceST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return distance >= get_epsilon();
  }

  bool TcST_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TciST_exists(c1, c2) || TceST_exists(c1, c2);
  }

  double TciST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    double alpha = fabs(asin(2 * parent_->radius_ * parent_->cos_mu_ / distance));
    double delta_x = fabs(parent_->radius_ * parent_->sin_mu_);
    double delta_y = fabs(parent_->radius_ * parent_->cos_mu_);
    double x, y, theta;
    if (c1.left && c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (c1.left && !c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (!c1.left && c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (!c1.left && !c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
    *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*cend)->hc_turn_length(**q4);
  }

  double TceST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    double theta = angle;
    double delta_x = fabs(parent_->radius_ * parent_->sin_mu_);
    double delta_y = fabs(parent_->radius_ * parent_->cos_mu_);
    double x, y;
    if (c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (!c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (!c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, -delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
    *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*cend)->hc_turn_length(**q4);
  }

  double TcST_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                   Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    if (TciST_exists(c1, c2))
    {
      return TciST_path(c1, c2, cstart, cend, q1, q2, q3, q4);
    }
    if (TceST_exists(c1, c2))
    {
      return TceST_path(c1, c2, cstart, cend, q1, q2, q3, q4);
    }
    return numeric_limits<double>::max();
  }

  // ##### TScT #################################################################
  bool TiScT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return distance >= 2 * parent_->radius_ * parent_->cos_mu_;
  }

  bool TeScT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward != c2.forward)
    {
      return false;
    }
    return distance >= get_epsilon();
  }

  bool TScT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TiScT_exists(c1, c2) || TeScT_exists(c1, c2);
  }

  double TiScT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    double alpha = fabs(asin(2 * parent_->radius_ * parent_->cos_mu_ / distance));
    double delta_x = fabs(parent_->radius_ * parent_->sin_mu_);
    double delta_y = fabs(parent_->radius_ * parent_->cos_mu_);
    double x, y, theta;
    if (c1.left && c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (c1.left && !c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (!c1.left && c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (!c1.left && !c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
    *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*cend)->hc_turn_length(**q4);
  }

  double TeScT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    double theta = angle;
    double delta_x = fabs(parent_->radius_ * parent_->sin_mu_);
    double delta_y = fabs(parent_->radius_ * parent_->cos_mu_);
    double x, y;
    if (c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    if (!c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta, 0);
    }
    if (!c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, 0);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q3 = new Configuration(x, y, theta + PI, 0);
    }
    *q1 = new Configuration(c1.start.x, c1.start.y, c1.start.theta, c1.kappa);
    *q4 = new Configuration(c2.start.x, c2.start.y, c2.start.theta, c2.kappa);
    *cstart = new HC_CC_Circle(**q2, c1.left, !c1.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    *cend = new HC_CC_Circle(**q3, c2.left, !c2.forward, HC_REGULAR, parent_->hc_cc_circle_param_);
    return (*cstart)->hc_turn_length(**q1) + configuration_distance(**q2, **q3) + (*cend)->hc_turn_length(**q4);
  }

  double TScT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                   Configuration **q1, Configuration **q2, Configuration **q3, Configuration **q4)
  {
    if (TiScT_exists(c1, c2))
    {
      return TiScT_path(c1, c2, cstart, cend, q1, q2, q3, q4);
    }
    if (TeScT_exists(c1, c2))
    {
      return TeScT_path(c1, c2, cstart, cend, q1, q2, q3, q4);
    }
    return numeric_limits<double>::max();
  }

  // ##### TcScT ################################################################
  bool TciScT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left == c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return distance >= fabs(2 / c1.kappa);
  }

  bool TceScT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    if (c1.left != c2.left)
    {
      return false;
    }
    if (c1.forward == c2.forward)
    {
      return false;
    }
    return distance >= get_epsilon();
  }

  bool TcScT_exists(const HC_CC_Circle &c1, const HC_CC_Circle &c2)
  {
    return TciScT_exists(c1, c2) || TceScT_exists(c1, c2);
  }

  double TciScT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2)
  {
    double alpha = fabs(asin(2 / (c1.kappa * distance)));
    double delta_x = 0.0;
    double delta_y = fabs(1 / c1.kappa);
    double x, y, theta;
    if (c1.left && c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta + PI, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, c2.kappa);
    }
    if (c1.left && !c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, c2.kappa);
    }
    if (!c1.left && c1.forward)
    {
      theta = angle + alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta + PI, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, c2.kappa);
    }
    if (!c1.left && !c1.forward)
    {
      theta = angle - alpha;
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, c2.kappa);
    }
    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(c2);
    return (*cstart)->rs_turn_length(**q1) + configuration_distance(**q1, **q2) + (*cend)->rs_turn_length(**q2);
  }

  double TceScT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                     Configuration **q1, Configuration **q2)
  {
    double theta = angle;
    double delta_x = 0.0;
    double delta_y = fabs(1 / c1.kappa);
    double x, y;
    if (c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta + PI, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, c2.kappa);
    }
    if (c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, c2.kappa);
    }
    if (!c1.left && c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, -delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta + PI, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, -delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta + PI, c2.kappa);
    }
    if (!c1.left && !c1.forward)
    {
      global_frame_change(c1.xc, c1.yc, theta, -delta_x, delta_y, &x, &y);
      *q1 = new Configuration(x, y, theta, c1.kappa);
      global_frame_change(c2.xc, c2.yc, theta, delta_x, delta_y, &x, &y);
      *q2 = new Configuration(x, y, theta, c2.kappa);
    }
    *cstart = new HC_CC_Circle(c1);
    *cend = new HC_CC_Circle(c2);
    return (*cstart)->rs_turn_length(**q1) + configuration_distance(**q1, **q2) + (*cend)->rs_turn_length(**q2);
  }

  double TcScT_path(const HC_CC_Circle &c1, const HC_CC_Circle &c2, HC_CC_Circle **cstart, HC_CC_Circle **cend,
                    Configuration **q1, Configuration **q2)
  {
    if (TciScT_exists(c1, c2))
    {
      return TciScT_path(c1, c2, cstart, cend, q1, q2);
    }
    if (TceScT_exists(c1, c2))
    {
      return TceScT_path(c1, c2, cstart, cend, q1, q2);
    }
    return numeric_limits<double>::max();
  }
};

// ############################################################################

HCpmpm_Reeds_Shepp_State_Space::HCpmpm_Reeds_Shepp_State_Space(double kappa, double sigma, double discretization)
  : HC_CC_State_Space(kappa, sigma, discretization)
  , hcpmpm_reeds_shepp_{ unique_ptr<HCpmpm_Reeds_Shepp>(new HCpmpm_Reeds_Shepp(this)) }
{
  rs_circle_param_.set_param(kappa_, numeric_limits<double>::max(), 1 / kappa_, 0.0, 0.0, 1.0, 0.0);
  radius_ = hc_cc_circle_param_.radius;
  mu_ = hc_cc_circle_param_.mu;
  sin_mu_ = hc_cc_circle_param_.sin_mu;
  cos_mu_ = hc_cc_circle_param_.cos_mu;
}

HCpmpm_Reeds_Shepp_State_Space::~HCpmpm_Reeds_Shepp_State_Space() = default;

HC_CC_RS_Path *HCpmpm_Reeds_Shepp_State_Space::hcpmpm_circles_rs_path(const HC_CC_Circle &c1,
                                                                      const HC_CC_Circle &c2) const
{
  // table containing the lengths of the paths, the intermediate configurations and circles
  double length[nb_hc_cc_rs_paths];
  double_array_init(length, nb_hc_cc_rs_paths, numeric_limits<double>::max());
  Configuration *qi1[nb_hc_cc_rs_paths];
  pointer_array_init((void **)qi1, nb_hc_cc_rs_paths);
  Configuration *qi2[nb_hc_cc_rs_paths];
  pointer_array_init((void **)qi2, nb_hc_cc_rs_paths);
  Configuration *qi3[nb_hc_cc_rs_paths];
  pointer_array_init((void **)qi3, nb_hc_cc_rs_paths);
  Configuration *qi4[nb_hc_cc_rs_paths];
  pointer_array_init((void **)qi4, nb_hc_cc_rs_paths);
  HC_CC_Circle *cstart[nb_hc_cc_rs_paths];
  pointer_array_init((void **)cstart, nb_hc_cc_rs_paths);
  HC_CC_Circle *ci1[nb_hc_cc_rs_paths];
  pointer_array_init((void **)ci1, nb_hc_cc_rs_paths);
  HC_CC_Circle *ci2[nb_hc_cc_rs_paths];
  pointer_array_init((void **)ci2, nb_hc_cc_rs_paths);
  HC_CC_Circle *cend[nb_hc_cc_rs_paths];
  pointer_array_init((void **)cend, nb_hc_cc_rs_paths);

  // precomputations
  hcpmpm_reeds_shepp_->distance = center_distance(c1, c2);
  hcpmpm_reeds_shepp_->angle = atan2(c2.yc - c1.yc, c2.xc - c1.xc);

  // case Empty
  if (configuration_equal(c1.start, c2.start))
  {
    length[EMPTY] = 0;
    goto label_end;
  }
  // case T
  if (configuration_on_hc_cc_circle(c1, c2.start))
  {
    cstart[T] = new HC_CC_Circle(c1.start, c1.left, c1.forward, false, rs_circle_param_);
    length[T] = cstart[T]->rs_turn_length(c2.start);
    goto label_end;
  }
  // case TT
  if (hcpmpm_reeds_shepp_->TT_exists(c1, c2))
  {
    length[TT] = hcpmpm_reeds_shepp_->TT_path(c1, c2, &cstart[TT], &cend[TT], &qi1[TT], &qi2[TT], &qi3[TT]);
  }
  // case TcT
  if (hcpmpm_reeds_shepp_->TcT_exists(c1, c2))
  {
    length[TcT] = hcpmpm_reeds_shepp_->TcT_path(c1, c2, &cstart[TcT], &cend[TcT], &qi1[TcT]);
  }
  // ##### Reeds-Shepp families: ############################################
  // case TcTcT
  if (hcpmpm_reeds_shepp_->TcTcT_exists(c1, c2))
  {
    length[TcTcT] =
        hcpmpm_reeds_shepp_->TcTcT_path(c1, c2, &cstart[TcTcT], &cend[TcTcT], &qi1[TcTcT], &qi2[TcTcT], &ci1[TcTcT]);
  }
  // case TcTT
  if (hcpmpm_reeds_shepp_->TcTT_exists(c1, c2))
  {
    length[TcTT] =
        hcpmpm_reeds_shepp_->TcTT_path(c1, c2, &cstart[TcTT], &cend[TcTT], &qi1[TcTT], &qi2[TcTT], &ci1[TcTT]);
  }
  // case TTcT
  if (hcpmpm_reeds_shepp_->TTcT_exists(c1, c2))
  {
    length[TTcT] =
        hcpmpm_reeds_shepp_->TTcT_path(c1, c2, &cstart[TTcT], &cend[TTcT], &qi1[TTcT], &qi2[TTcT], &ci1[TTcT]);
  }
  // case TST
  if (hcpmpm_reeds_shepp_->TST_exists(c1, c2))
  {
    length[TST] =
        hcpmpm_reeds_shepp_->TST_path(c1, c2, &cstart[TST], &cend[TST], &qi1[TST], &qi2[TST], &qi3[TST], &qi4[TST]);
  }
  // case TSTcT
  if (hcpmpm_reeds_shepp_->TSTcT_exists(c1, c2))
  {
    length[TSTcT] = hcpmpm_reeds_shepp_->TSTcT_path(c1, c2, &cstart[TSTcT], &cend[TSTcT], &qi1[TSTcT], &qi2[TSTcT],
                                                    &qi3[TSTcT], &qi4[TSTcT], &ci1[TSTcT]);
  }
  // case TcTST
  if (hcpmpm_reeds_shepp_->TcTST_exists(c1, c2))
  {
    length[TcTST] = hcpmpm_reeds_shepp_->TcTST_path(c1, c2, &cstart[TcTST], &cend[TcTST], &qi1[TcTST], &qi2[TcTST],
                                                    &qi3[TcTST], &qi4[TcTST], &ci1[TcTST]);
  }
  // case TcTSTcT
  if (hcpmpm_reeds_shepp_->TcTSTcT_exists(c1, c2))
  {
    length[TcTSTcT] =
        hcpmpm_reeds_shepp_->TcTSTcT_path(c1, c2, &cstart[TcTSTcT], &cend[TcTSTcT], &qi1[TcTSTcT], &qi2[TcTSTcT],
                                          &qi3[TcTSTcT], &qi4[TcTSTcT], &ci1[TcTSTcT], &ci2[TcTSTcT]);
  }
  // case TTcTT
  if (hcpmpm_reeds_shepp_->TTcTT_exists(c1, c2))
  {
    length[TTcTT] = hcpmpm_reeds_shepp_->TTcTT_path(c1, c2, &cstart[TTcTT], &cend[TTcTT], &qi1[TTcTT], &qi2[TTcTT],
                                                    &qi3[TTcTT], &ci1[TTcTT], &ci2[TTcTT]);
  }
  // case TcTTcT
  if (hcpmpm_reeds_shepp_->TcTTcT_exists(c1, c2))
  {
    length[TcTTcT] = hcpmpm_reeds_shepp_->TcTTcT_path(c1, c2, &cstart[TcTTcT], &cend[TcTTcT], &qi1[TcTTcT],
                                                      &qi2[TcTTcT], &ci1[TcTTcT], &ci2[TcTTcT]);
  }
  //  ############################################################################
  // case TTT
  if (hcpmpm_reeds_shepp_->TTT_exists(c1, c2))
  {
    length[TTT] =
        hcpmpm_reeds_shepp_->TTT_path(c1, c2, &cstart[TTT], &cend[TTT], &qi1[TTT], &qi2[TTT], &qi3[TTT], &ci1[TTT]);
  }
  // case TcST
  if (hcpmpm_reeds_shepp_->TcST_exists(c1, c2))
  {
    length[TcST] = hcpmpm_reeds_shepp_->TcST_path(c1, c2, &cstart[TcST], &cend[TcST], &qi1[TcST], &qi2[TcST],
                                                  &qi3[TcST], &qi4[TcST]);
  }
  // case TScT
  if (hcpmpm_reeds_shepp_->TScT_exists(c1, c2))
  {
    length[TScT] = hcpmpm_reeds_shepp_->TScT_path(c1, c2, &cstart[TScT], &cend[TScT], &qi1[TScT], &qi2[TScT],
                                                  &qi3[TScT], &qi4[TScT]);
  }
  // case TcScT
  if (hcpmpm_reeds_shepp_->TcScT_exists(c1, c2))
  {
    length[TcScT] = hcpmpm_reeds_shepp_->TcScT_path(c1, c2, &cstart[TcScT], &cend[TcScT], &qi1[TcScT], &qi2[TcScT]);
  }
label_end:
  // select shortest path
  hc_cc_rs_path_type best_path = (hc_cc_rs_path_type)array_index_min(length, nb_hc_cc_rs_paths);
  HC_CC_RS_Path *path;
  path = new HC_CC_RS_Path(c1.start, c2.start, best_path, kappa_, sigma_, qi1[best_path], qi2[best_path],
                           qi3[best_path], qi4[best_path], cstart[best_path], cend[best_path], ci1[best_path],
                           ci2[best_path], length[best_path]);

  // clean up
  for (int i = 0; i < nb_hc_cc_rs_paths; i++)
  {
    if (i != best_path)
    {
      delete qi1[i];
      delete qi2[i];
      delete qi3[i];
      delete qi4[i];
      delete cstart[i];
      delete ci1[i];
      delete ci2[i];
      delete cend[i];
    }
  }
  return path;
}

HC_CC_RS_Path *HCpmpm_Reeds_Shepp_State_Space::hcpmpm_reeds_shepp(const State &state1, const State &state2) const
{
  // compute the 4 circles at the intial and final configuration
  Configuration start(state1.x, state1.y, state1.theta, state1.kappa);
  Configuration end(state2.x, state2.y, state2.theta, state2.kappa);

  HC_CC_Circle *start_circle[4];
  HC_CC_Circle *end_circle[4];
  start_circle[0] = new HC_CC_Circle(start, true, true, true, rs_circle_param_);
  start_circle[1] = new HC_CC_Circle(start, false, true, true, rs_circle_param_);
  start_circle[2] = new HC_CC_Circle(start, true, false, true, rs_circle_param_);
  start_circle[3] = new HC_CC_Circle(start, false, false, true, rs_circle_param_);
  end_circle[0] = new HC_CC_Circle(end, true, true, true, rs_circle_param_);
  end_circle[1] = new HC_CC_Circle(end, false, true, true, rs_circle_param_);
  end_circle[2] = new HC_CC_Circle(end, true, false, true, rs_circle_param_);
  end_circle[3] = new HC_CC_Circle(end, false, false, true, rs_circle_param_);

  // compute the shortest path for the 16 combinations (4 circles at the beginning and 4 at the end)
  HC_CC_RS_Path *path[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

  double lg[] = { numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max(),
                  numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max(),
                  numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max(),
                  numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max(),
                  numeric_limits<double>::max(), numeric_limits<double>::max(), numeric_limits<double>::max(),
                  numeric_limits<double>::max() };

  // skip circle at the beginning for curvature continuity
  for (int i = 0; i < 4; i++)
  {
    if (i == 0 && state1.kappa < 0)
      continue;
    else if (i == 1 && state1.kappa > 0)
      continue;
    else if (i == 2 && state1.kappa < 0)
      continue;
    else if (i == 3 && state1.kappa > 0)
      continue;
    // skip circle at the end for curvature continuity
    for (int j = 0; j < 4; j++)
    {
      if (j == 0 && state2.kappa < 0)
        continue;
      else if (j == 1 && state2.kappa > 0)
        continue;
      else if (j == 2 && state2.kappa < 0)
        continue;
      else if (j == 3 && state2.kappa > 0)
        continue;
      path[4 * i + j] = hcpmpm_circles_rs_path(*start_circle[i], *end_circle[j]);
      if (path[4 * i + j])
      {
        lg[4 * i + j] = path[4 * i + j]->length;
      }
    }
  }

  // select shortest path
  int best_path = array_index_min(lg, 16);

  //  // display calculations
  //  cout << "HCpmpm_Reeds_Shepp_State_Space" << endl;
  //  for (int i = 0; i < 16; i++)
  //  {
  //    cout << i << ": ";
  //    if (path[i])
  //    {
  //      path[i]->print(true);
  //      cout << endl;
  //    }
  //  }
  //  cout << "shortest path: " << (int)best_path << endl;
  //  path[best_path]->print(true);

  // clean up
  for (int i = 0; i < 4; i++)
  {
    delete start_circle[i];
    delete end_circle[i];
  }
  for (int i = 0; i < 16; i++)
  {
    if (i != best_path)
    {
      delete path[i];
    }
  }
  return path[best_path];
}

double HCpmpm_Reeds_Shepp_State_Space::get_distance(const State &state1, const State &state2) const
{
  HC_CC_RS_Path *p = this->hcpmpm_reeds_shepp(state1, state2);
  double length = p->length;
  delete p;
  return length;
}

vector<Control> HCpmpm_Reeds_Shepp_State_Space::get_controls(const State &state1, const State &state2) const
{
  vector<Control> hc_rs_controls;
  hc_rs_controls.reserve(5);
  HC_CC_RS_Path *p = this->hcpmpm_reeds_shepp(state1, state2);
  switch (p->type)
  {
    case EMPTY:
      empty_controls(hc_rs_controls);
      break;
    case T:
      rs_turn_controls(*(p->cstart), p->end, true, hc_rs_controls);
      break;
    case TT:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi3), true, hc_rs_controls);
      break;
    case TcT:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi1), false, hc_rs_controls);
      break;
    // ##### Reeds-Shepp families: ############################################
    case TcTcT:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      rs_turn_controls(*(p->ci1), *(p->qi2), true, hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi2), false, hc_rs_controls);
      break;
    case TcTT:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi1), false, hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi2), true, hc_rs_controls);
      break;
    case TTcT:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi2), true, hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi2), false, hc_rs_controls);
      break;
    case TST:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      straight_controls(*(p->qi2), *(p->qi3), hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi4), true, hc_rs_controls);
      break;
    case TSTcT:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      straight_controls(*(p->qi2), *(p->qi3), hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi4), true, hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi4), false, hc_rs_controls);
      break;
    case TcTST:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi1), false, hc_rs_controls);
      straight_controls(*(p->qi2), *(p->qi3), hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi4), true, hc_rs_controls);
      break;
    case TcTSTcT:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi1), false, hc_rs_controls);
      straight_controls(*(p->qi2), *(p->qi3), hc_rs_controls);
      hc_turn_controls(*(p->ci2), *(p->qi4), true, hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi4), false, hc_rs_controls);
      break;
    case TTcTT:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi2), true, hc_rs_controls);
      hc_turn_controls(*(p->ci2), *(p->qi2), false, hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi3), true, hc_rs_controls);
      break;
    case TcTTcT:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      hc_turn_controls(*(p->ci1), *(p->qi1), false, hc_rs_controls);
      hc_turn_controls(*(p->ci2), *(p->qi2), true, hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi2), false, hc_rs_controls);
      break;
    // ########################################################################
    case TTT:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      cc_turn_controls(*(p->ci1), *(p->qi2), true, hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi3), true, hc_rs_controls);
      break;
    case TcST:
    case TScT:
      hc_turn_controls(*(p->cstart), *(p->qi1), false, hc_rs_controls);
      straight_controls(*(p->qi2), *(p->qi3), hc_rs_controls);
      hc_turn_controls(*(p->cend), *(p->qi4), true, hc_rs_controls);
      break;
    case TcScT:
      rs_turn_controls(*(p->cstart), *(p->qi1), true, hc_rs_controls);
      straight_controls(*(p->qi1), *(p->qi2), hc_rs_controls);
      rs_turn_controls(*(p->cend), *(p->qi2), false, hc_rs_controls);
      break;
    default:
      break;
  }
  delete p;
  return hc_rs_controls;
}
