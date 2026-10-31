import { UIAbility } from '@kit.AbilityKit';
import { window } from '@kit.ArkUI';

export default class DataManagerAbility extends UIAbility {
  onWindowStageCreate(windowStage: window.WindowStage): void {
    windowStage.loadContent('pages/DataManager');
  }
}
