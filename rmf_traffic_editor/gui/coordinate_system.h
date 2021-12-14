/*
 * Copyright (C) 2019-2021 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef COORDINATE_SYSTEM_H
#define COORDINATE_SYSTEM_H

#include <string>
#include <proj.h>

class CoordinateSystem
{
public:
  enum Value
  {
    Undefined,
    ReferenceImage,
    WebMercator,
    CartesianMeters,
    WGS84
  } value;

  CoordinateSystem();
  CoordinateSystem(const CoordinateSystem::Value& _value);
  ~CoordinateSystem();

  std::string to_string();
  static CoordinateSystem from_string(const std::string& s);
  bool is_y_flipped() const;
  double default_scale() const;

  bool has_tiles() const;
  bool is_global() const;

  // equatorial radius of the earth in WGS84 (meters)
  static constexpr double WGS84_A = 6378137.0;

  struct ProjectedPoint
  {
    double x = 0.0;
    double y = 0.0;
  };
  ProjectedPoint to_epsg3857(const double coord_0, const double coord_1);
};

#endif
