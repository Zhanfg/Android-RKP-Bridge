#include "rkp_bridge/rkp_policy.hpp"

#include <algorithm>
#include <cctype>

namespace rkp_bridge {
namespace {

std::string lower_ascii(std::string_view input) {
  std::string out(input);
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return out;
}

std::string_view component_instance(std::string_view name) {
  const auto slash = name.find_last_of('/');
  return slash == std::string_view::npos ? name : name.substr(slash + 1);
}

}  // namespace

std::optional<bool> parse_bool(std::string_view value) {
  if (value.empty()) return std::nullopt;
  const std::string normalized = lower_ascii(value);
  if (normalized == "1" || normalized == "true" || normalized == "y" ||
      normalized == "yes" || normalized == "on") {
    return true;
  }
  if (normalized == "0" || normalized == "false" || normalized == "n" ||
      normalized == "no" || normalized == "off") {
    return false;
  }
  return std::nullopt;
}

std::optional<SecurityLevel> security_level_from_component(std::string_view component_name) {
  const auto instance = component_instance(component_name);
  if (instance == "default") return SecurityLevel::Tee;
  if (instance == "strongbox") return SecurityLevel::StrongBox;
  return std::nullopt;
}

std::optional<bool> effective_rkp_enabled(const Properties& properties) {
  // OEM stacks may publish the effective runtime state separately from the persistent
  // device_config-backed property. Prefer the effective signal when it is present.
  if (auto value = parse_bool(properties.oem_enable_rkpd)) return value;
  return parse_bool(properties.persistent_enable_rkpd);
}

Evaluation evaluate_registration(bool target_uid,
                                 std::string_view component_name,
                                 const Properties& properties) {
  Evaluation out;
  out.level = security_level_from_component(component_name);
  out.rkp_enabled = effective_rkp_enabled(properties);

  if (!target_uid) {
    out.reason = "caller is not targeted";
    return out;
  }

  if (!out.level.has_value()) {
    // Never deny a registration we cannot classify. A malformed or vendor-specific instance should
    // degrade to the platform path rather than turn a compatibility heuristic into a key failure.
    out.reason = "unrecognized RKP component; fail open";
    return out;
  }

  const std::string& raw_rkp_only =
      *out.level == SecurityLevel::StrongBox ? properties.strongbox_rkp_only
                                            : properties.tee_rkp_only;
  out.rkp_only = parse_bool(raw_rkp_only);

  if (out.rkp_only == true) {
    out.reason = "security level is RKP-only; denial would break key generation";
    return out;
  }

  // Missing rkp_only is intentionally treated like false: hybrid devices can fall back to a
  // factory/batch path, which is the condition under which a targeted attestation layer can deny
  // the remote registration without globally disabling RKP.
  out.decision = Decision::DenyForTarget;
  out.reason = out.rkp_only == false
                   ? "targeted caller on a hybrid/non-RKP-only level"
                   : "targeted caller; rkp_only unset, treating level as hybrid";
  return out;
}

}  // namespace rkp_bridge
