// Called from the game thread when the game quits; the handler should terminateSelf() the ability.
export const setTerminateHandler: (handler: () => void) => void;
// Forwards the system back gesture to the game as SDLK_AC_BACK (handled like Escape).
export const sendBackKey: () => void;
