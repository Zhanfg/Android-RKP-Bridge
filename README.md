# Android RKP Bridge

Standalone Android Remote Key Provisioning (RKP) policy and routing layer.

The project starts deliberately small: the core library models RKP component discovery and per-security-level policy without depending on TEESimulator, OhMyKeymint, a keybox, or Android Binder. Android-specific Binder/property adapters can then be added around the tested core.

## Goals

- distinguish TEE (`/default`) and StrongBox (`/strongbox`) RKP components;
- understand both AOSP-style and OEM RKP state properties;
- make per-target allow/deny decisions without globally disabling RKP;
- expose deterministic diagnostics suitable for device-specific compatibility work;
- remain usable by TEESimulator, OhMyKeymint, or another keystore project through a small API.

## Non-goals

- generating or distributing attestation private keys;
- changing global RKP properties behind the user's back;
- implementing KeyMint or replacing Android Keystore;
- coupling the core policy to a specific root framework.

## Initial property model

The first implementation recognizes:

- `remote_provisioning.tee.rkp_only`
- `remote_provisioning.strongbox.rkp_only`
- `persist.device_config.remote_key_provisioning_native.enable_rkpd`
- `remote_provisioning.enable_rkpd` (used by some OEM stacks, including observed OPlus devices)

The OEM/global enable properties are status signals. Per-level `rkp_only` remains the deciding input for whether denying an RKP registration would break key generation.

## License

GPL-3.0-or-later. This keeps the project license-compatible with TEESimulator integrations while allowing the policy core to remain independently maintained.
