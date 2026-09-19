# OnePlus 13 / PJZ110 notes

Observed on Android 16 / API 36 during the 2026-09 compatibility investigation.

## RKP topology

The device exposes independent RKP components for both hardware security levels:

```text
android.hardware.security.keymint.IRemotelyProvisionedComponent/default
android.hardware.security.keymint.IRemotelyProvisionedComponent/strongbox
```

The device also exposes an OEM/effective state property:

```text
remote_provisioning.enable_rkpd=true
```

The following properties may be unset even though RKP is present and enabled:

```text
remote_provisioning.tee.rkp_only
remote_provisioning.strongbox.rkp_only
persist.device_config.remote_key_provisioning_native.enable_rkpd
```

Therefore **absence of those three properties must not be interpreted as absence of RKP**.

## KeyMint topology

The tested device exposes a Qualcomm TEE KeyMint path and an NXP StrongBox path. Compatibility code
must keep the `default` and `strongbox` paths distinct.

A regression detector observed a request classified as StrongBox while the returned attestation and
KeyMint security levels were TEE. That is treated as a routing/level-preservation bug in the
integration layer, not as evidence that the device lacks StrongBox.

## RKP testing rules

Two different test modes must not be conflated:

### Native RKP test

The test application is outside any TEESimulator/OMK interception scope. Expected path:

```text
app -> keystore2 -> native TEE/StrongBox -> native RKP
```

Use this mode to determine whether the device's original RKP path works.

### Attestation-layer test

The test application is explicitly targeted by the integrating attestation layer. On a hybrid,
non-RKP-only level that layer may intentionally deny only that caller's RKP registration so Android
does not append an unrelated remote-provisioned attestation chain.

In that mode, "RKP not observed" can be an expected policy result rather than an RKP failure.

## Regression targets

- TEE request remains TEE.
- StrongBox request remains StrongBox end-to-end.
- Unknown/vendor component names fail open.
- Native RKP remains globally enabled.
- Non-target apps remain on the native path.
- Per-target denial is never applied when the selected level is RKP-only.
- No global RKP property is rewritten by the policy layer.
