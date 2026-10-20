import { common } from '@kit.AbilityKit';
import { fileIo as fs, picker } from '@kit.CoreFileKit';
import { zlib } from '@kit.BasicServicesKit';

const SAVE_DIR_NAME = 'userdata';
const EXPORT_FILE_NAME = 'pvz-portable-savedata.zip';
const RESOURCE_ENTRIES: string[] = [
  'main.pak', 'properties', 'data', 'images', 'particles', 'reanim', 'sounds', 'compiled'
];

// Resource import and save data management, the HarmonyOS counterpart of Android's ResourceImportActivity.
// The app is sandboxed, so everything is copied into the directory the game reads from.
export class GameData {
  private context: common.UIAbilityContext;
  // Must match SDL_GetPrefPath("io.github.wszqkzqk", "PvZPortable") on HarmonyOS, which SDL3 builds from the
  // *application* context's filesDir (the ability context's filesDir is a per-module directory).
  readonly gameDir: string;
  private cacheDir: string;

  constructor(context: common.UIAbilityContext) {
    this.context = context;
    const appContext = context.getApplicationContext();
    this.gameDir = appContext.filesDir + '/PvZPortable';
    this.cacheDir = appContext.cacheDir;
    if (!fs.accessSync(this.gameDir)) {
      fs.mkdirSync(this.gameDir, true);
    }
  }

  hasResources(): boolean {
    return isFile(this.gameDir + '/main.pak') && isDirectory(this.gameDir + '/properties');
  }

  hasSaveData(): boolean {
    return isFile(this.gameDir + '/' + SAVE_DIR_NAME + '/users.dat');
  }

  // Resolves to false if the user cancelled the picker.
  async importResourcesZip(): Promise<boolean> {
    const uri = await this.pickZip();
    if (uri === undefined) {
      return false;
    }
    const extracted = await this.extractToTemp(uri);
    try {
      const root = findResourceRoot(extracted);
      if (root === undefined) {
        throw new Error('main.pak and properties/ not found');
      }
      // Only import known resource entries. Writable data such as userdata/ and registry.regemu is never touched.
      for (const name of RESOURCE_ENTRIES) {
        const source = root + '/' + name;
        if (fs.accessSync(source)) {
          await copyPath(source, this.gameDir + '/' + name);
        }
      }
    } finally {
      await removePath(extracted);
    }
    return true;
  }

  async importSaveZip(): Promise<boolean> {
    const uri = await this.pickZip();
    if (uri === undefined) {
      return false;
    }
    const extracted = await this.extractToTemp(uri);
    try {
      const saveRoot = findSaveRoot(extracted);
      if (saveRoot === undefined) {
        throw new Error('users.dat not found in save ZIP');
      }
      // Like Android, merge files into userdata instead of deleting the existing directory first.
      await copyPath(saveRoot, this.gameDir + '/' + SAVE_DIR_NAME);
    } finally {
      await removePath(extracted);
    }
    return true;
  }

  async exportSaveZip(): Promise<boolean> {
    const documentPicker = new picker.DocumentViewPicker(this.context);
    const saveOptions = new picker.DocumentSaveOptions();
    saveOptions.newFileNames = [EXPORT_FILE_NAME];
    const uris = await documentPicker.save(saveOptions);
    if (uris.length === 0) {
      return false;
    }

    const tempZip = this.cacheDir + '/' + EXPORT_FILE_NAME;
    await removePath(tempZip);
    const options: zlib.Options = {
      level: zlib.CompressLevel.COMPRESS_LEVEL_DEFAULT_COMPRESSION,
      memLevel: zlib.MemLevel.MEM_LEVEL_DEFAULT,
      strategy: zlib.CompressStrategy.COMPRESS_STRATEGY_DEFAULT_STRATEGY
    };
    try {
      // Compressing the directory keeps "userdata/" as the archive's top level, like the Android export.
      await zlib.compressFile(this.gameDir + '/' + SAVE_DIR_NAME, tempZip, options);
      const out = fs.openSync(uris[0], fs.OpenMode.WRITE_ONLY | fs.OpenMode.TRUNC);
      try {
        await fs.copyFile(tempZip, out.fd);
      } finally {
        fs.closeSync(out);
      }
    } finally {
      await removePath(tempZip);
    }
    return true;
  }

  private async pickZip(): Promise<string | undefined> {
    const documentPicker = new picker.DocumentViewPicker(this.context);
    const selectOptions = new picker.DocumentSelectOptions();
    selectOptions.maxSelectNumber = 1;
    selectOptions.fileSuffixFilters = ['ZIP|.zip'];
    const uris = await documentPicker.select(selectOptions);
    return uris.length > 0 ? uris[0] : undefined;
  }

  // Copies the picked document into the sandbox and unpacks it; returns the extraction directory.
  private async extractToTemp(uri: string): Promise<string> {
    const tempZip = this.cacheDir + '/import.zip';
    const tempDir = this.cacheDir + '/import';
    await removePath(tempZip);
    await removePath(tempDir);
    fs.mkdirSync(tempDir, true);

    const src = fs.openSync(uri, fs.OpenMode.READ_ONLY);
    try {
      await fs.copyFile(src.fd, tempZip);
    } finally {
      fs.closeSync(src);
    }

    try {
      const options: zlib.Options = {
        level: zlib.CompressLevel.COMPRESS_LEVEL_DEFAULT_COMPRESSION,
        memLevel: zlib.MemLevel.MEM_LEVEL_DEFAULT,
        strategy: zlib.CompressStrategy.COMPRESS_STRATEGY_DEFAULT_STRATEGY
      };
      await zlib.decompressFile(tempZip, tempDir, options);
    } catch (e) {
      await removePath(tempDir);
      throw e as Error;
    } finally {
      await removePath(tempZip);
    }
    return tempDir;
  }
}

function isDirectory(path: string): boolean {
  return fs.accessSync(path) && fs.statSync(path).isDirectory();
}

function isFile(path: string): boolean {
  return fs.accessSync(path) && fs.statSync(path).isFile();
}

function onlySubdirectory(dir: string): string | undefined {
  const subdirs = fs.listFileSync(dir).filter((entry: string) => isDirectory(dir + '/' + entry));
  return subdirs.length === 1 ? dir + '/' + subdirs[0] : undefined;
}

function isResourceRoot(dir: string): boolean {
  return isFile(dir + '/main.pak') && isDirectory(dir + '/properties');
}

function findResourceRoot(dir: string): string | undefined {
  if (isResourceRoot(dir)) {
    return dir;
  }
  const subdir = onlySubdirectory(dir);
  return subdir !== undefined && isResourceRoot(subdir) ? subdir : undefined;
}

function isSaveRoot(dir: string): boolean {
  return isFile(dir + '/users.dat');
}

function findSaveRoot(dir: string): string | undefined {
  if (isSaveRoot(dir)) {
    return dir;
  }
  if (isSaveRoot(dir + '/' + SAVE_DIR_NAME)) {
    return dir + '/' + SAVE_DIR_NAME;
  }
  const subdir = onlySubdirectory(dir);
  if (subdir !== undefined) {
    if (isSaveRoot(subdir)) {
      return subdir;
    }
    if (isSaveRoot(subdir + '/' + SAVE_DIR_NAME)) {
      return subdir + '/' + SAVE_DIR_NAME;
    }
  }
  return undefined;
}

async function copyPath(src: string, dest: string): Promise<void> {
  if (isDirectory(src)) {
    if (!fs.accessSync(dest)) {
      fs.mkdirSync(dest, true);
    }
    for (const name of fs.listFileSync(src)) {
      await copyPath(src + '/' + name, dest + '/' + name);
    }
  } else {
    const parent = dest.substring(0, dest.lastIndexOf('/'));
    if (!fs.accessSync(parent)) {
      fs.mkdirSync(parent, true);
    }
    await fs.copyFile(src, dest);
  }
}

async function removePath(path: string): Promise<void> {
  if (!fs.accessSync(path)) {
    return;
  }
  if (fs.statSync(path).isDirectory()) {
    await fs.rmdir(path);
  } else {
    await fs.unlink(path);
  }
}
