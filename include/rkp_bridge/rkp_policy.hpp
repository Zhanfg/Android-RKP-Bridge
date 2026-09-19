#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace rkp_bridge {

enum class SecurityLevel {
  Tee,
  StrongBox,
};

enum class Decision {
  Allow,
  DenyForTarget,
};

struct Properties {
  std::string tee_rkp_only;
  std::string strongbox_rkp_only;
  std::string persistent_enable_rkpd;
  std::string oem_enable_rkpd;
};

struct Evaluation {
  Decision decision = Decision::Allow;
  std::optional<SecurityLevel> level;
  std::optional<bool> rkp_only;
  std::optional<bool> rkp_enabled;
  std::string reason;
};

std::optional<bool> parse_bool(std::string_view value);
std::optional<SecurityLevel> security_level_from_component(std::string_view component_name);
std::optional<bool> effective_rkp_enabled(const Properties& properties);

Evaluation evaluate_registration(bool target_uid,
                                 std::string_view component_name,
                                 const Properties& properties);

}  // namespace rkp_bridge
