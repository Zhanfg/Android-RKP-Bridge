#include "rkp_bridge/rkp_policy.hpp"

#include <cassert>
#include <iostream>

using rkp_bridge::Decision;
using rkp_bridge::Properties;
using rkp_bridge::SecurityLevel;

int main() {
  assert(rkp_bridge::parse_bool("true") == true);
  assert(rkp_bridge::parse_bool("1") == true);
  assert(rkp_bridge::parse_bool("FALSE") == false);
  assert(!rkp_bridge::parse_bool("").has_value());
  assert(!rkp_bridge::parse_bool("maybe").has_value());

  const auto tee = rkp_bridge::security_level_from_component(
      "android.hardware.security.keymint.IRemotelyProvisionedComponent/default");
  assert(tee == SecurityLevel::Tee);

  const auto sb = rkp_bridge::security_level_from_component(
      "android.hardware.security.keymint.IRemotelyProvisionedComponent/strongbox");
  assert(sb == SecurityLevel::StrongBox);
  assert(!rkp_bridge::security_level_from_component("vendor-special").has_value());

  Properties props;
  props.oem_enable_rkpd = "true";
  props.persistent_enable_rkpd = "false";
  assert(rkp_bridge::effective_rkp_enabled(props) == true);

  auto non_target = rkp_bridge::evaluate_registration(
      false,
      "android.hardware.security.keymint.IRemotelyProvisionedComponent/default",
      props);
  assert(non_target.decision == Decision::Allow);

  props.tee_rkp_only = "true";
  auto rkp_only = rkp_bridge::evaluate_registration(
      true,
      "android.hardware.security.keymint.IRemotelyProvisionedComponent/default",
      props);
  assert(rkp_only.decision == Decision::Allow);
  assert(rkp_only.rkp_only == true);

  props.tee_rkp_only = "false";
  auto hybrid = rkp_bridge::evaluate_registration(
      true,
      "android.hardware.security.keymint.IRemotelyProvisionedComponent/default",
      props);
  assert(hybrid.decision == Decision::DenyForTarget);
  assert(hybrid.rkp_only == false);

  props.strongbox_rkp_only.clear();
  auto strongbox = rkp_bridge::evaluate_registration(
      true,
      "android.hardware.security.keymint.IRemotelyProvisionedComponent/strongbox",
      props);
  assert(strongbox.decision == Decision::DenyForTarget);
  assert(strongbox.level == SecurityLevel::StrongBox);

  auto unknown = rkp_bridge::evaluate_registration(true, "vendor/other", props);
  assert(unknown.decision == Decision::Allow);
  assert(!unknown.level.has_value());

  std::cout << "rkp_policy_test: ok\n";
  return 0;
}
