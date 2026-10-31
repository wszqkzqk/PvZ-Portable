import { appTasks, OhosPluginId, OhosAppContext, AppJson } from '@ohos/hvigor-ohos-plugin';
import { hvigor, getNode, HvigorNode } from '@ohos/hvigor';
import { execFileSync } from 'child_process';
import * as path from 'path';

function gitVersion(args: string[], fallback: string): string {
  try {
    const output = execFileSync('git', args, {
      cwd: path.resolve(__dirname, '..'),
      encoding: 'utf8'
    });
    return output.trim() || fallback;
  } catch {
    return fallback;
  }
}

hvigor.nodesEvaluated(() => {
  const node: HvigorNode = getNode(__filename);
  const appContext = node.getContext(OhosPluginId.OHOS_APP_PLUGIN) as OhosAppContext;
  const appJson5: AppJson.AppOptObj = appContext.getAppJsonOpt();

  // Match android/app/build.gradle and CMake/ProjectVersion.cmake without modifying source files.
  const describe = gitVersion(['describe', '--tags', '--abbrev=12'], '0.1');
  const commitCount = Number.parseInt(gitVersion(['rev-list', '--count', 'HEAD'], '1'), 10);
  appJson5.app.versionName = describe.replace(/^v/, '').replace('-', '.');
  appJson5.app.versionCode = Number.isSafeInteger(commitCount) && commitCount > 0 ? commitCount : 1;
  appContext.setAppJsonOpt(appJson5);
});

export default {
  system: appTasks,
  plugins: []
}
