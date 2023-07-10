// Copyright (c) 2023 Franka Emika GmbH
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <limits>
#include <string>
#include <vector>

#include "franka/robot_state.h"
#include "franka_msgs/msg/errors.hpp"
#include "franka_msgs/msg/franka_state.hpp"
#include "semantic_components/semantic_component_interface.hpp"

namespace franka_semantic_components {
class FrankaState
    : public semantic_components::SemanticComponentInterface<franka_msgs::msg::FrankaState> {
 public:
  explicit FrankaState(const std::string& name);

  virtual ~FrankaState() = default;

  const franka::RobotState& getRobotState() const noexcept { return *robot_state_ptr_; }

  /**
   * Constructs and return a FrankaState message from the current values.
   * \return FrankaState message from values;
   */
  bool get_values_as_message(franka_msgs::msg::FrankaState& message);

 protected:
  franka::RobotState* robot_state_ptr_;
};

}  // namespace franka_semantic_components
