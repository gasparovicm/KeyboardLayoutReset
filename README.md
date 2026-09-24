# KeyboardLayoutReset

A small Windows tray app that switches the keyboard layout back to **English**,
or another default keyboard you choose, after you stop using the computer for
a while.

If you often type in a second language (for example Slovak), you can come back
to the PC, type a password or a command, and find that the layout is still
wrong. KeyboardLayoutReset fixes that. Once there has been no keyboard or mouse input
for a minute, it switches the active window back to your default keyboard. It
never switches while you are typing.

It is written in plain C with the Win32 API. It has no dependencies or runtime
and builds to one ~150 KB `.exe`.

## Install

Download `KeyboardLayoutReset-Setup-x.y.z.exe` from
[Releases](https://github.com/gasparovicm/KeyboardLayoutReset/releases) (or
build it, see [Building](#building))
and run it. Administrator rights are not needed; it installs for your user only
into `%LOCALAPPDATA%\Programs\KeyboardLayoutReset`.

The setup asks for:

- **Default keyboard**: the layout to switch back to. The default is `en`.
- **Idle time**: seconds without input before switching. The default is `60`.
- **Start when I sign in**: adds a shortcut to your Startup folder.

You can change both settings later from the tray icon menu (see below).
Running setup again also works; it starts with your current settings. Uninstall it from
_Settings → Apps → Installed apps_.

Silent install, for scripts:

```
KeyboardLayoutReset-Setup-0.4.0.exe /SILENT /LAYOUT=en-US /IDLE=60
```

## How it works

- Every few seconds (`--check`, default 5 s), it asks Windows how long it has
  been since the last input (`GetLastInputInfo`).
- Once the idle time reaches `--idle` seconds (default 60) and the focused
  window is not using the default keyboard, it posts
  `WM_INPUTLANGCHANGEREQUEST` with that layout to the window.
- It prefers a layout already in your language list, so it doesn't add
  layouts you don't have.
- Only one copy runs at a time.

## Tray menu

Hovering over the tray icon shows the default keyboard and the idle time.
Clicking it opens the menu:

- **Switch to … now**: switches the window you were in right away.
- **Default keyboard ▸**: lists your installed keyboards. Pick the one to
  switch back to.
- **Idle time ▸**: 15 s, 30 s, 1, 2, 5 or 10 minutes. A value set in setup or
  on the command line that isn't in this list is shown as "(custom)".
- **Pause**: stops switching until you click it again.
- **Exit**

Changes apply immediately and are saved per user in
`HKCU\Software\KeyboardLayoutReset` (`Layout`, `IdleSeconds`). The installer
writes the same values.

## Languages

The tray menu and messages follow your Windows display language:

Bulgarian, Croatian, Czech, Danish, Dutch, English, Estonian, Finnish, French,
German, Greek, Hungarian, Irish, Italian, Latvian, Lithuanian, Maltese,
Norwegian, Polish, Portuguese, Romanian, Slovak, Slovenian, Spanish, Swedish
and Ukrainian. Other languages fall back to English. Keyboard names come from
Windows itself, so they are always in your display language.

The installer is translated into all of these except Croatian, Estonian, Greek,
Irish, Latvian, Lithuanian, Maltese and Romanian. Inno Setup has no built-in
translation for those, so setup shows a language choice there.

To add or fix a translation, edit `src\strings.rc` for the app and the
`[CustomMessages]` section of `installer\KeyboardLayoutReset.iss` for setup.
Keep the `%s`, `%lu` and `%1` placeholders.

## Options

Normally you don't need these, because the tray menu and setup store the
settings. When you do pass them, `--layout` and `--idle` override the saved
settings for that run only.

```
KeyboardLayoutReset.exe [--layout LAYOUT] [--idle SECONDS] [--check SECONDS]
```

| Option     | Default | Meaning                                                                                                       |
| ---------- | ------- | ------------------------------------------------------------------------------------------------------------- |
| `--layout` | `en`    | Default keyboard to switch back to (see below).                                                               |
| `--idle`   | `60`    | Seconds without keyboard or mouse input before switching. `0` means switch on every check, even while typing. |
| `--check`  | `5`     | How often to check, in seconds.                                                                               |
| `--help`   |         | Show the options.                                                                                             |

`--layout` accepts:

- a language name, such as `en` or `de`. It uses the first installed keyboard
  of that language.
- a locale name, such as `en-US`, `en-GB`, `sk-SK` or `cs-CZ`. It uses the
  installed keyboard for that locale, or loads the standard one for it.
- a layout id, such as `00000409` (US) or `00000809` (UK). Use this for
  variants like US-International (`00020409`).

An invalid value shows an error message and the app does not start.

Examples:

```
KeyboardLayoutReset.exe                            # English after 1 minute idle
KeyboardLayoutReset.exe --layout en-GB --idle 120  # UK English after 2 minutes idle
KeyboardLayoutReset.exe --layout sk-SK             # Slovak is the default instead
KeyboardLayoutReset.exe --idle 0 --check 60        # force English every minute, even while typing
```

## Administrator windows

Windows does not let normal programs send input messages to programs running
**as administrator** (for example an elevated terminal, Task Manager, or an
installer). This protection is called User Interface Privilege Isolation
(UIPI). KeyboardLayoutReset runs as a normal program, so when such a window has focus:

- its keyboard layout is **not** switched, and nothing else happens;
- as soon as you focus a normal window, switching works again.

With the default Windows setting of one input method for all windows, the
layout is shared anyway. It gets switched the next time a normal window is
focused while you are idle.

If you do want elevated windows switched too, run KeyboardLayoutReset as administrator.
For example, create a Task Scheduler task that starts it at sign-in with
_Run with highest privileges_. The installer does not do this, because a tray
app running as administrator is usually not worth the risk.

## Other limitations

- If _Settings → Time & language → Typing → Advanced keyboard settings → "Let me
  use a different input method for each app window"_ is on, only the focused
  window is switched. By default, Windows uses one layout for all windows.
- Setup and the app are not code-signed. Windows SmartScreen may warn on first
  run ("More info → Run anyway"). New unsigned installers can also trigger
  Microsoft Defender false positives. If that happens, report it at
  <https://www.microsoft.com/wdsi/filesubmission>.

## Building

You need:

- Visual Studio 2022 or later with the **Desktop development with C++**
  workload.
- [Inno Setup 6](https://jrsoftware.org/isinfo.php) for the installer
  (optional).

Then run:

```
build.bat
```

This produces `bin\KeyboardLayoutReset.exe`, built with the static runtime (`/MT`), and,
if Inno Setup is installed, `dist\KeyboardLayoutReset-Setup-<version>.exe`. The version
comes from `src\app.rc`.

### Microsoft Store package (MSIX)

`build.bat` also creates `dist\KeyboardLayoutReset-<version>.0.msix` for the
Microsoft Store, using `makeappx` from the Windows SDK. The package is
unsigned, because the Store signs it after certification. That also means it
can't be installed locally by double-clicking; use the Inno Setup installer for
that.

The Store version works without the installer:

- Settings are changed from the tray menu.
- Autostart is a startup task declared in the package. Windows turns it on after
  the app is opened once, and it can be switched off in _Task Manager → Startup
  apps_.

The package identity in `packaging\AppxManifest.xml` comes from Partner Center
(_Apps and games → KeyboardLayoutReset → Product identity_) and must match it
exactly.

| Path                                | What it is                     |
| ----------------------------------- | ------------------------------ |
| `src\main.c`                        | The app                        |
| `src\app.rc`, `src\app.ico`         | Version info and icon          |
| `src\strings.rc`                    | Translated UI text             |
| `installer\KeyboardLayoutReset.iss` | Inno Setup script              |
| `packaging\AppxManifest.xml`        | MSIX manifest (Store)          |
| `packaging\make-msix.ps1`           | Packs the MSIX                 |
| `tools\make-icon.ps1`               | Regenerates the icon and logos |

## Alternatives

Existing tools that do something similar:

- [Keyboard Auto Switcher](https://apps.microsoft.com/detail/9pcpx2n2r4lk)
  (Microsoft Store). Closest match. You press a hotkey to switch to your second
  layout, and after a timeout it switches back to your default layout. A tray
  icon shows the current layout. The timer starts from the hotkey, not from
  idle time, and switching with Win+Space or the taskbar may not start it.
- [OneKey Layout Switcher](https://apps.microsoft.com/detail/9nzvl338pldv)
  (Microsoft Store). Switches layouts with a single key. It has no automatic
  reset.
- [SwitchLang](https://github.com/Bumblebee621/SwitchLang). Real-time
  English/Hebrew auto-switcher. It resets context after 15 s without typing.
- [KeyboardSwitcher](https://github.com/kertser/KeyboardSwitcher). Guesses the
  language you are typing (Hebrew, English, Russian) with an ONNX model and
  switches to it.
- [KeyLayoutAutoSwitch](https://github.com/AlexVallat/KeyLayoutAutoSwitch).
  Browser extension that switches the layout by focused field or website.
- [RightKeyboard](https://github.com/gmcouto/RightKeyboard). Picks a layout per
  keyboard device.
- [keyboard-layout-switcher](https://github.com/Gaeritag/keyboard-layout-switcher).
  Picks a layout by keyboard hardware ID.
- [one-key-keyboard-switcher](https://github.com/asilichenko/one-key-keyboard-switcher).
  Switches layouts with a single key.
- [ahk-layout-switch](https://github.com/shokurov/ahk-layout-switch).
  AutoHotkey script that shows the layout under the caret and cycles layouts
  macOS-style.

KeyboardLayoutReset exists to be tiny, open, and do only this one job.

## Privacy

The app collects no data and never connects to the internet. See
[PRIVACY.md](PRIVACY.md).

## License

[MIT](LICENSE) © 2026 Martin Gašparovič
