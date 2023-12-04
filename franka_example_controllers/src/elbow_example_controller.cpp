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

#include <franka_example_controllers/elbow_example_controller.hpp>
#include <franka_msgs/srv/set_full_collision_behavior.hpp>

#include <cassert>
#include <cmath>
#include <exception>
#include <string>

using namespace std::chrono_literals;

namespace franka_example_controllers {

controller_interface::InterfaceConfiguration
ElbowExampleController::command_interface_configuration() const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  config.names = franka_cartesian_velocity_->get_command_interface_names();

  return config;
}

controller_interface::InterfaceConfiguration ElbowExampleController::state_interface_configuration()
    const {
  controller_interface::InterfaceConfiguration config;
  config.type = controller_interface::interface_configuration_type::INDIVIDUAL;
  config.names = franka_cartesian_velocity_->get_state_interface_names();

  return config;
}

controller_interface::return_type ElbowExampleController::update(
    const rclcpp::Time& /*time*/,
    const rclcpp::Duration& /*period*/) {
  if (initialization_flag_) {
    initial_elbow_configuration_ = franka_cartesian_velocity_->getInitialElbowConfiguration();
    initialization_flag_ = false;
  }
  elapsed_time_ = elapsed_time_ + traj_frequency_;

  double angle = M_PI / 15.0 * (1.0 - std::cos(M_PI / 5.0 * elapsed_time_));

  std::array<double, 6> cartesian_velocity_command = {{0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
  std::array<double, 2> elbow_command = {
      {initial_elbow_configuration_[0] + angle, initial_elbow_configuration_[1]}};

  if (franka_cartesian_velocity_->setCommand(cartesian_velocity_command, elbow_command)) {
    return controller_interface::return_type::OK;
  } else {
    RCLCPP_FATAL(get_node()->get_logger(),
                 "Set command failed. Did you activate the elbow command interface?");
    return controller_interface::return_type::ERROR;
  }
}

CallbackReturn ElbowExampleController::on_init() {
  return CallbackReturn::SUCCESS;
}

CallbackReturn ElbowExampleController::on_configure(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  franka_cartesian_velocity_ =
      std::make_unique<franka_semantic_components::FrankaCartesianVelocityInterface>(
          franka_semantic_components::FrankaCartesianVelocityInterface(k_elbow_activated_));

  auto client = get_node()->create_client<franka_msgs::srv::SetFullCollisionBehavior>(
      "service_server/set_full_collision_behavior");
  auto request = default_robot_behavior_.getDefaultCollisionBehaviorRequest();

  auto future_result = client->async_send_request(request);
  future_result.wait_for(1000ms);

  auto success = future_result.get();
  if (!success) {
    RCLCPP_FATAL(get_node()->get_logger(), "Failed to set default collision behavior.");
    return CallbackReturn::ERROR;
  } else {
    RCLCPP_INFO(get_node()->get_logger(), "Default collision behavior set.");
  }

  return CallbackReturn::SUCCESS;
}

CallbackReturn ElbowExampleController::on_activate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  franka_cartesian_velocity_->assign_loaned_command_interfaces(command_interfaces_);
  franka_cartesian_velocity_->assign_loaned_state_interfaces(state_interfaces_);

  initialization_flag_ = true;
  elapsed_time_ = 0.0;
  return CallbackReturn::SUCCESS;
}

controller_interface::CallbackReturn ElbowExampleController::on_deactivate(
    const rclcpp_lifecycle::State& /*previous_state*/) {
  franka_cartesian_velocity_->release_interfaces();
  return CallbackReturn::SUCCESS;
}

}  // namespace franka_example_controllers
#include "pluginlib/class_list_macros.hpp"
// NOLINTNEXTLINE
PLUGINLIB_EXPORT_CLASS(franka_example_controllers::ElbowExampleController,
                       controller_interface::ControllerInterface)