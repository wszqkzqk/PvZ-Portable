import { abilityAccessCtrl, AbilityConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { i18n } from '@kit.LocalizationKit';
import { inputMethod } from '@kit.IMEKit';
import pointer from '@ohos.multimodalInput.pointer';
import sdl from 'libSDL3.so';
import { GameData } from '../common/GameData';
import { BUNDLE_NAME, DATA_ABILITY_NAME } from '../common/AbilityNames';

const DOMAIN = 0x0000;

// SDL3 monkey-patches this ability from native code, so keep ArkTS lifecycle code here minimal.
export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    if (!new GameData(this.context).hasResources()) {
      this.context.startAbility({
        bundleName: BUNDLE_NAME,
        moduleName: 'entry',
        abilityName: DATA_ABILITY_NAME
      }).then(() => this.context.terminateSelf()).catch((e: Error) => {
        hilog.error(DOMAIN, 'PvZPortable', 'Failed to open data manager: %{public}s', e.message);
      });
      return;
    }

    hilog.info(DOMAIN, 'PvZPortable', '%{public}s', 'Handing control from ArkTS to native SDL.');
    try {
      // The argument list must match the SDL3 revision selected in harmony/build-deps.sh;
      // the native side aborts on a mismatch.
      sdl.provideArkTSObjects(this, abilityAccessCtrl.createAtManager(), i18n.System,
        pointer.setPointerVisibleSync, inputMethod.getController());
    } catch (e) {
      hilog.error(DOMAIN, 'PvZPortable', '%{public}s', 'SDL rejected provideArkTSObjects; the app will not start.');
    }
  }
}
