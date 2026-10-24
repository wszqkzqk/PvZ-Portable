// Mirrors the arguments EntryAbility.ets passes; must match the SDL3 revision selected in harmony/build-deps.sh.
export const provideArkTSObjects: (ability: object, atManager: object, i18nSystem: object,
  setPointerVisibleSync: Function, imeController: object) => void;
