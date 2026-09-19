# Architecture

## Separation of responsibilities

The bridge is intended to sit between Android's RKP-facing Binder path and consumers such as a
KeyMint compatibility layer.  It does **not** own KeyMint keys or attestation certificates.

```text
Android keystore2 / Remote Provisioning
                |
        Android adapter layer
                |
        +-------v--------+
        | RKP policy core |
        +-------+--------+
                |
     +----------+-----------+
     |                      |
TEESimulator adapter   OhMyKeymint adapter
(optional)             (optional)
```

The policy core stays platform-independent. Android-specific code should be split into adapters for:

1. property snapshots;
2. `IRemotelyProvisionedComponent` discovery;
3. `IRemoteProvisioning.getRegistration` request decoding;
4. target/caller policy supplied by the integrating project.

## Policy

RKP remains globally enabled. The bridge never changes global RKP properties as part of a routing
decision.

For a targeted caller:

- identify the requested component by its instance name (`default` or `strongbox`);
- read the matching per-level `rkp_only` state;
- if that level is RKP-only, allow the registration;
- otherwise an integrating attestation layer may deny the registration for that target only.

For a non-target caller or an unknown component, fail open to Android's normal RKP path.

## Property precedence

The core accepts both:

- `persist.device_config.remote_key_provisioning_native.enable_rkpd`;
- `remote_provisioning.enable_rkpd`.

When both exist, the OEM/effective property is treated as the current status signal. Neither
`enable_rkpd` value overrides the per-level `rkp_only` decision.

## Future Android adapter

The next layer should expose an immutable snapshot such as:

```cpp
struct AndroidRkpSnapshot {
    Properties properties;
    bool tee_component_present;
    bool strongbox_component_present;
};
```

Component discovery should prefer Binder/service-manager evidence over guessing from properties.
