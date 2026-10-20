import { common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { promptAction } from '@kit.ArkUI';
import pvz from 'libmain.so';
import { GameData } from '../common/GameData';

const DOMAIN = 0x0000;

// SDL3 loads this page from its native onWindowStageCreate. The game starts once the SDL XComponent's
// surface exists, so the XComponent is only created after game resources have been imported.
@Entry
@Component
struct Index {
  @StorageLink('manageData') @Watch('onManageDataChanged') manageData: boolean = false;
  @State gameStarted: boolean = false;
  @State hasResources: boolean = false;
  @State hasSaveData: boolean = false;
  @State working: boolean = false;
  private gameData?: GameData;

  aboutToAppear(): void {
    const context = this.getUIContext().getHostContext() as common.UIAbilityContext;
    this.gameData = new GameData(context);
    pvz.setTerminateHandler(() => {
      context.terminateSelf().catch((e: Error) => {
        hilog.error(DOMAIN, 'PvZPortable', 'terminateSelf failed: %{public}s', e.message);
      });
    });
    this.refresh();
    // Like Android, go straight into the game unless the launcher requested data management.
    this.setGameStarted(this.hasResources && !this.manageData);
  }

  private setGameStarted(started: boolean): void {
    this.gameStarted = started;
    AppStorage.setOrCreate('gameStarted', started);
  }

  private onManageDataChanged(): void {
    // Never replace the XComponent of an active SDL instance. EntryAbility applies new wants only before startup.
    if (!this.gameStarted) {
      this.setGameStarted(this.hasResources && !this.manageData);
    }
  }

  onBackPress(): boolean {
    if (this.gameStarted) {
      pvz.sendBackKey();
      return true;
    }
    return false;
  }

  private refresh(): void {
    this.hasResources = this.gameData?.hasResources() ?? false;
    this.hasSaveData = this.gameData?.hasSaveData() ?? false;
  }

  private async run(task: () => Promise<boolean>, success: Resource, failure: Resource): Promise<void> {
    this.working = true;
    try {
      if (await task()) {
        this.getUIContext().getPromptAction().showToast({ message: success });
      }
    } catch (e) {
      const message = (e as Error).message;
      hilog.error(DOMAIN, 'PvZPortable', 'Data operation failed: %{public}s', message);
      const prefix = this.getUIContext().getHostContext()?.resourceManager.getStringSync(failure.id) ?? '';
      const options: promptAction.ShowToastOptions = { message: prefix + message, duration: 4000 };
      this.getUIContext().getPromptAction().showToast(options);
    } finally {
      this.working = false;
      this.refresh();
    }
  }

  @Builder
  DataManager() {
    Scroll() {
      Column({ space: 12 }) {
        Text(this.working ? $r('app.string.status_working') :
          this.hasResources ? $r('app.string.status_ready') : $r('app.string.status_missing'))
          .fontSize(18)
          .textAlign(TextAlign.Center)

        if (this.working) {
          LoadingProgress().width(48).height(48)
        }

        Text($r('app.string.section_import_resources')).fontSize(20).fontWeight(FontWeight.Bold).margin({ top: 12 })
        Text($r('app.string.import_help')).fontSize(14).textAlign(TextAlign.Center)
        Button($r('app.string.btn_pick_zip'))
          .enabled(!this.working)
          .onClick(() => {
            this.run(() => this.gameData!.importResourcesZip(), $r('app.string.import_success'),
              $r('app.string.import_failed'));
          })

        Text($r('app.string.section_save_data')).fontSize(20).fontWeight(FontWeight.Bold).margin({ top: 12 })
        Row({ space: 12 }) {
          Button($r('app.string.btn_export_save'))
            .enabled(!this.working && this.hasSaveData)
            .onClick(() => {
              this.run(() => this.gameData!.exportSaveZip(), $r('app.string.export_success'),
                $r('app.string.export_failed'));
            })
          Button($r('app.string.btn_import_save_zip'))
            .enabled(!this.working)
            .onClick(() => {
              this.run(() => this.gameData!.importSaveZip(), $r('app.string.import_success'),
                $r('app.string.import_failed'));
            })
        }

        Button($r('app.string.btn_launch_game'))
          .enabled(!this.working && this.hasResources)
          .margin({ top: 24 })
          .onClick(() => {
            this.setGameStarted(true);
          })
      }
      .width('100%')
      .padding(24)
      .alignItems(HorizontalAlign.Center)
    }
    .width('100%')
    .height('100%')
  }

  build() {
    Stack() {
      if (this.gameStarted) {
        // id and libraryname must stay as in SDL's openharmony-project: SDL3 attaches to this XComponent.
        XComponent({ id: 'SDL_XComponent', type: XComponentType.SURFACE, libraryname: 'SDL3' })
          .id('SDL_XComponent')
          .width('100%')
          .height('100%')
      } else {
        this.DataManager()
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor(this.gameStarted ? Color.Black : Color.White)
  }
}
