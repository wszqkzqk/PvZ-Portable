import { common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { promptAction } from '@kit.ArkUI';
import { GameData } from '../common/GameData';
import { BUNDLE_NAME, GAME_ABILITY_NAME } from '../common/AbilityNames';

const DOMAIN = 0x0000;

@Entry
@Component
struct DataManager {
  @State hasResources: boolean = false;
  @State hasSaveData: boolean = false;
  @State working: boolean = false;
  private gameData?: GameData;

  aboutToAppear(): void {
    const context = this.getUIContext().getHostContext() as common.UIAbilityContext;
    this.gameData = new GameData(context);
    this.refresh();
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

  private async startGame(): Promise<void> {
    const context = this.getUIContext().getHostContext() as common.UIAbilityContext;
    try {
      await context.startAbility({ bundleName: BUNDLE_NAME, moduleName: 'entry', abilityName: GAME_ABILITY_NAME });
      await context.terminateSelf();
    } catch (e) {
      const message = (e as Error).message;
      hilog.error(DOMAIN, 'PvZPortable', 'Failed to start game: %{public}s', message);
      this.getUIContext().getPromptAction().showToast({ message: $r('app.string.launch_failed') });
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
        Row({ space: 12 }) {
          Button($r('app.string.btn_pick_zip'))
            .enabled(!this.working)
            .onClick(() => {
              this.run(() => this.gameData!.importResourcesZip(), $r('app.string.import_success'),
                $r('app.string.import_failed'));
            })
          Button($r('app.string.btn_pick_dir'))
            .enabled(!this.working)
            .onClick(() => {
              this.run(() => this.gameData!.importResourcesFolder(), $r('app.string.import_success'),
                $r('app.string.import_failed'));
            })
        }

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
          Button($r('app.string.btn_import_save_dir'))
            .enabled(!this.working)
            .onClick(() => {
              this.run(() => this.gameData!.importSaveFolder(), $r('app.string.import_success'),
                $r('app.string.import_failed'));
            })
        }

        Button($r('app.string.btn_launch_game'))
          .enabled(!this.working && this.hasResources)
          .margin({ top: 24 })
          .onClick(() => {
            this.startGame();
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
      this.DataManager()
    }
    .width('100%')
    .height('100%')
    .backgroundColor(Color.White)
  }
}
