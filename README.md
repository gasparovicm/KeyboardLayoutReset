# LayoutReset

A small Windows tray app that switches the keyboard layout back to **English**
after you stop using the computer for a while.

If you often type in a second language (for example Slovak), you can come back
to the PC, type a password or a command, and find that the layout is still
wrong. LayoutReset fixes that. Once there has been no keyboard or mouse input
for a minute, it switches the active window back to English. It never switches
while you are typing.

It is written in plain C with the Win32 API. It has no dependencies or runtime
and builds to one ~140 KB `.exe`.

## How it works

- Every few seconds (`--check`, default 5 s), it asks Windows how long it has
  been since the last input (`GetLastInputInfo`).
- Once the idle time reaches `--idle` seconds (default 60) and the focused
  window is not using English, it posts `WM_INPUTLANGCHANGEREQUEST` with the
  English layout to that window.
- It uses the first English layout already in your language list, so it never
  adds a layout you don't have. `--layout` picks a specific one instead.
- A tray icon offers **Switch to English now**, **Pause** and **Exit**. Only one
  instance runs at a time.

## Usage

```
LayoutReset.exe [--idle SECONDS] [--check SECONDS] [--layout KLID]
```

| Option     | Default | Meaning                                                        |
| ---------- | ------- | -------------------------------------------------------------- |
| `--idle`   | `60`    | Seconds without input before switching. `0` = don't wait.      |
| `--check`  | `5`     | How often to check, in seconds.                                |
| `--layout` | auto    | Keyboard layout id, e.g. `00000409` (US) or `00000809` (UK).   |

Examples:

```
LayoutReset.exe                          # English after 1 minute idle
LayoutReset.exe --idle 0 --check 60      # force English every minute, even while typing
LayoutReset.exe --idle 120 --layout 00000809
```

### Start with Windows

Press `Win+R`, type `shell:startup`, and put a shortcut to `LayoutReset.exe`
(with any options) in that folder.

## Building

You need Visual Studio 2022 or later with the **Desktop development with C++**
workload. Then run:

```
build.bat
```

The output is `bin\LayoutReset.exe`, built with the static runtime (`/MT`).

## Limitations

- Windows blocks messages from normal apps to elevated (administrator) windows
  (UIPI), so those keep their layout. Running LayoutReset as administrator
  removes this limit.
- If *Settings → Time & language → Typing → Advanced keyboard settings → "Let me
  use a different input method for each app window"* is on, only the focused
  window is switched. By default, Windows uses one layout for all windows.

## Alternatives

Existing tools that do something similar:

- [Keyboard Auto Switcher](https://apps.microsoft.com/detail/9pcpx2n2r4lk)
  (Microsoft Store). A tray app that returns to your default layout after a
  timeout. This is the closest match.
- [SwitchLang](https://github.com/Bumblebee621/SwitchLang). Automatic
  English/Hebrew switcher with an idle-based reset.
- [RightKeyboard](https://github.com/gmcouto/RightKeyboard). Picks a layout per
  keyboard device.
- [keyboard-layout-switcher](https://github.com/Gaeritag/keyboard-layout-switcher).
  Picks a layout by keyboard hardware ID.

LayoutReset exists to be tiny, open, and do only this one job.
