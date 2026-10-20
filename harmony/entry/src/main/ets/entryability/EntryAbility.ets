import { abilityAccessCtrl, AbilityConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { i18n } from '@kit.LocalizationKit';
import { inputMethod } from '@kit.IMEKit';
import pointer from '@ohos.multimodalInput.pointer';
import sdl from 'libSDL3.so';

const DOMAIN = 0x0000;

// Taken from SDL's openharmony-project: SDL3 monkey-patches this ability from native code
// (onWindowStageCreate loads pages/Index, onForeground/onBackground become SDL app events),
// so it has to stay this small.
export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    AppStorage.setOrCreate('manageData', want.parameters?.manageData === 'true');
    AppStorage.setOrCreate('gameStarted', false);
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

  onNewWant(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    // SDL supports one game instance. A launcher shortcut may select the initial page, but must not
    // replace the XComponent of a game that is already running.
    const gameStarted = AppStorage.get<boolean>('gameStarted') ?? false;
    if (!gameStarted) {
      AppStorage.setOrCreate('manageData', want.parameters?.manageData === 'true');
    }
  }
}
