import { common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import pvz from 'libmain.so';

const DOMAIN = 0x0000;

@Entry
@Component
struct Index {
  aboutToAppear(): void {
    const context = this.getUIContext().getHostContext() as common.UIAbilityContext;
    pvz.setTerminateHandler(() => {
      context.terminateSelf().catch((e: Error) => {
        hilog.error(DOMAIN, 'PvZPortable', 'terminateSelf failed: %{public}s', e.message);
      });
    });
  }

  onBackPress(): boolean {
    pvz.sendBackKey();
    return true;
  }

  build() {
    Stack() {
      // id and libraryname must stay as in SDL's openharmony-project: SDL3 attaches to this XComponent.
      XComponent({ id: 'SDL_XComponent', type: XComponentType.SURFACE, libraryname: 'SDL3' })
        .id('SDL_XComponent')
        .width('100%')
        .height('100%')
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.Black)
  }
}
