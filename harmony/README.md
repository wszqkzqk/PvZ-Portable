# HarmonyOS

## Building

1. Install [DevEco Studio](https://developer.huawei.com/consumer/en/download/) or the HarmonyOS command line tools.
2. From the repository root, build native dependencies for each required ABI:
   ```bash
   harmony/build-deps.sh arm64-v8a
   ```
   For the emulator, also pass `x86_64` and add it to `abiFilters` in `entry/build-profile.json5`.
   If the SDK is not found automatically, set `OHOS_NATIVE_HOME` to `<sdk>/default/openharmony/native`.
3. Open this directory in DevEco Studio, generate a signature under **File > Project Structure > Signing Configs**, then build and run.

## Command Line

Set the SDK path, then build an unsigned HAP from this directory:

```bash
export DEVECO_SDK_HOME=<command-line-tools>/sdk
ohpm install --all
hvigorw assembleHap --mode module -p product=default -p buildMode=release
```

The output is written to `entry/build/default/outputs/default/entry-default-unsigned.hap`.
