/*
 * KeyboardLayoutReset - switches the keyboard layout back to a default (English
 * unless configured otherwise) when the computer has not been used for a while.
 *
 * Runs as a tiny tray application. Every CHECK seconds it looks at how long
 * the user has been idle (no keyboard or mouse input). When the idle time
 * reaches IDLE seconds and the focused window is not using the default
 * layout, it asks that window to switch.
 *
 * Settings come from HKCU\Software\KeyboardLayoutReset (written by the tray
 * menu and the installer); command-line options override them for one run.
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "resource.h"

#define APP_NAME L"KeyboardLayoutReset"
#define WM_TRAYICON (WM_APP + 1)
#define TIMER_ID 1
#define REG_KEY L"Software\\" APP_NAME
#define LAYOUTS_KEY L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts"
#define MAX_LAYOUTS 64

static const wchar_t USAGE[] =
    L"KeyboardLayoutReset.exe [--layout LAYOUT] [--idle SECONDS] [--check SECONDS]\n\n"
    L"--layout  Default keyboard to switch back to. A language or locale\n"
    L"          name (en, en-US, en-GB, sk-SK, de-DE) or a layout id\n"
    L"          (00000409). Default: en, the first installed English layout.\n\n"
    L"--idle    Seconds without keyboard or mouse input before switching.\n"
    L"          Default 60. Use 0 to switch on every check, even while typing.\n\n"
    L"--check   How often to check, in seconds. Default 5.\n\n"
    L"--layout and --idle override the settings chosen in the tray menu\n"
    L"for this run only.";

enum {
    CMD_SWITCH_NOW = 100, CMD_PAUSE, CMD_EXIT,
    CMD_LAYOUT_FIRST = 200,
    CMD_IDLE_FIRST = CMD_LAYOUT_FIRST + MAX_LAYOUTS,
};

static const DWORD IDLE_PRESETS[] = { 15, 30, 60, 120, 300, 600 };

static DWORD g_idle_ms = 60 * 1000;
static DWORD g_check_ms = 5 * 1000;
static HKL g_target = NULL;
static wchar_t g_target_name[256] = L"English";
static HKL g_menu_layouts[MAX_LAYOUTS];
static BOOL g_paused = FALSE;
static NOTIFYICONDATAW g_nid;

static LANGID g_ui_lang = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);

/* strings.rc has one block per language, e.g. Slovak as LANG_SLOVAK /
 * SUBLANG_NEUTRAL. Picks the one for the Windows display language, else
 * English. */
static LANGID pick_ui_language(LANGID display)
{
    LANGID lang = MAKELANGID(PRIMARYLANGID(display), SUBLANG_NEUTRAL);
    LPCWSTR block = MAKEINTRESOURCEW(IDS_SWITCH_NOW / 16 + 1);
    return FindResourceExW(NULL, RT_STRING, block, lang)
        ? lang : MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
}

/* UI text in g_ui_lang. String resources are stored in blocks of 16
 * length-prefixed UTF-16 strings. */
static void load_text(UINT id, wchar_t *buf, int len)
{
    buf[0] = 0;
    HRSRC res = FindResourceExW(NULL, RT_STRING, MAKEINTRESOURCEW(id / 16 + 1), g_ui_lang);
    const WCHAR *p = res ? LockResource(LoadResource(NULL, res)) : NULL;
    if (!p)
        return;
    for (UINT i = 0; i < id % 16; i++)
        p += 1 + *p;
    int n = min((int)*p, len - 1);
    wmemcpy(buf, p + 1, n);
    buf[n] = 0;
}

/* Milliseconds since the last keyboard or mouse input in this session. */
static DWORD idle_ms(void)
{
    LASTINPUTINFO lii = { sizeof(lii) };
    if (!GetLastInputInfo(&lii))
        return 0;
    return GetTickCount() - lii.dwTime;
}

/* Settings saved per user. Missing or unreadable values keep the defaults. */
static void load_settings(wchar_t *layout, DWORD layout_len)
{
    DWORD size = layout_len * sizeof(wchar_t);
    wchar_t buf[128];
    DWORD bufsize = sizeof(buf);
    if (RegGetValueW(HKEY_CURRENT_USER, REG_KEY, L"Layout", RRF_RT_REG_SZ, NULL, buf, &bufsize) == ERROR_SUCCESS
        && bufsize <= size)
        wcscpy_s(layout, layout_len, buf);

    DWORD idle, idle_size = sizeof(idle);
    if (RegGetValueW(HKEY_CURRENT_USER, REG_KEY, L"IdleSeconds", RRF_RT_REG_DWORD, NULL, &idle, &idle_size) == ERROR_SUCCESS
        && idle <= 24 * 60 * 60)
        g_idle_ms = idle * 1000;
}

static void save_layout(const wchar_t *klid)
{
    RegSetKeyValueW(HKEY_CURRENT_USER, REG_KEY, L"Layout", REG_SZ, klid,
                    (DWORD)((wcslen(klid) + 1) * sizeof(wchar_t)));
}

static void save_idle(DWORD seconds)
{
    RegSetKeyValueW(HKEY_CURRENT_USER, REG_KEY, L"IdleSeconds", REG_DWORD, &seconds, sizeof(seconds));
}

/* Installed layout for the language, so we never add a layout the user does
 * not have. A neutral language (sublanguage 0, e.g. plain "en") matches any
 * of its regional variants. */
static HKL find_installed_layout(LANGID lang)
{
    HKL list[64];
    int n = GetKeyboardLayoutList(64, list);
    for (int i = 0; i < n; i++) {
        LANGID l = LOWORD((UINT_PTR)list[i]);
        if (l == lang || (SUBLANGID(lang) == 0 && PRIMARYLANGID(l) == PRIMARYLANGID(lang)))
            return list[i];
    }
    return NULL;
}

static BOOL is_klid(const wchar_t *s)
{
    if (wcslen(s) != KL_NAMELENGTH - 1)
        return FALSE;
    for (; *s; s++)
        if (!iswxdigit(*s))
            return FALSE;
    return TRUE;
}

/* LoadKeyboardLayout returns the current layout for unknown ids, so check
 * that Windows has the layout first. */
static HKL load_layout(const wchar_t *klid)
{
    wchar_t sub[128];
    HKEY key;
    swprintf(sub, ARRAYSIZE(sub), LAYOUTS_KEY L"\\%s", klid);
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, sub, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return NULL;
    RegCloseKey(key);
    return LoadKeyboardLayoutW(klid, KLF_NOTELLSHELL);
}

/* Resolves --layout: a layout id like 00000409, or a language/locale name
 * like en, en-US or sk-SK. Returns NULL if the value is not valid. */
static HKL resolve_layout(const wchar_t *spec)
{
    if (is_klid(spec))
        return load_layout(spec);

    if (!IsValidLocaleName(spec))
        return NULL;
    LCID lcid = LocaleNameToLCID(spec, LOCALE_ALLOW_NEUTRAL_NAMES);
    if (lcid == 0 || lcid == LOCALE_CUSTOM_UNSPECIFIED)
        return NULL;

    LANGID lang = LANGIDFROMLCID(lcid);
    HKL hkl = find_installed_layout(lang);
    if (hkl || SUBLANGID(lang) == 0)
        return hkl;

    /* Not installed yet: load the language's standard layout. */
    wchar_t klid[KL_NAMELENGTH];
    swprintf(klid, KL_NAMELENGTH, L"%08X", lang);
    return load_layout(klid);
}

/* Layout id (KLID, e.g. 0001041B) of a loaded layout. The low word of an HKL
 * is its language; the high word is either the layout's own id (04090409 ->
 * 00000409), 0xFnnn pointing to the registry "Layout Id" nnn of a layout
 * variant (F013041B -> Slovak QWERTY), or a full IME id (E0010411). */
static BOOL hkl_to_klid(HKL hkl, wchar_t klid[KL_NAMELENGTH])
{
    WORD dev = HIWORD((UINT_PTR)hkl);
    if ((dev & 0xF000) == 0xE000) {
        swprintf(klid, KL_NAMELENGTH, L"%08lX", (unsigned long)(UINT_PTR)hkl);
        return TRUE;
    }
    if ((dev & 0xF000) != 0xF000) {
        swprintf(klid, KL_NAMELENGTH, L"%08X", dev);
        return TRUE;
    }

    HKEY key;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, LAYOUTS_KEY, 0, KEY_READ, &key) != ERROR_SUCCESS)
        return FALSE;
    BOOL found = FALSE;
    wchar_t name[KL_NAMELENGTH + 8], id[16];
    for (DWORD i = 0; !found; i++) {
        DWORD name_len = ARRAYSIZE(name), id_size = sizeof(id);
        if (RegEnumKeyExW(key, i, name, &name_len, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            break;
        if (name_len == KL_NAMELENGTH - 1
            && RegGetValueW(key, name, L"Layout Id", RRF_RT_REG_SZ, NULL, id, &id_size) == ERROR_SUCCESS
            && wcstoul(id, NULL, 16) == (dev & 0x0FFFu)) {
            wcscpy_s(klid, KL_NAMELENGTH, name);
            found = TRUE;
        }
    }
    RegCloseKey(key);
    return found;
}

/* Label like "English (United States) - US" for the menu and tray tip. */
static void describe_layout(HKL hkl, wchar_t *out, size_t out_len)
{
    wchar_t lang[128] = L"", layout[128] = L"", klid[KL_NAMELENGTH], sub[128];
    GetLocaleInfoW(MAKELCID(LOWORD((UINT_PTR)hkl), SORT_DEFAULT), LOCALE_SLOCALIZEDDISPLAYNAME,
                   lang, ARRAYSIZE(lang));

    if (hkl_to_klid(hkl, klid)) {
        swprintf(sub, ARRAYSIZE(sub), LAYOUTS_KEY L"\\%s", klid);
        wchar_t raw[MAX_PATH];
        DWORD size = sizeof(raw);
        /* The display name is an indirect string like "@%SystemRoot%\...,-5000". */
        if (RegGetValueW(HKEY_LOCAL_MACHINE, sub, L"Layout Display Name", RRF_RT_REG_SZ, NULL, raw, &size) != ERROR_SUCCESS
            || FAILED(SHLoadIndirectString(raw, layout, ARRAYSIZE(layout), NULL))) {
            size = sizeof(layout);
            if (RegGetValueW(HKEY_LOCAL_MACHINE, sub, L"Layout Text", RRF_RT_REG_SZ, NULL, layout, &size) != ERROR_SUCCESS)
                layout[0] = 0;
        }
    }

    if (layout[0] && lang[0])
        _snwprintf_s(out, out_len, _TRUNCATE, L"%s \u2013 %s", lang, layout);
    else
        _snwprintf_s(out, out_len, _TRUNCATE, L"%s", lang[0] ? lang : layout);
}

static void set_target(HKL hkl)
{
    g_target = hkl;
    describe_layout(hkl, g_target_name, ARRAYSIZE(g_target_name));
}

static BOOL is_target(HKL hkl)
{
    return hkl == g_target;
}

/* Ask the window that owns keyboard focus to switch to the target layout.
 * Returns TRUE if a switch was requested. */
static BOOL switch_foreground(void)
{
    HWND fg = GetForegroundWindow();
    if (!fg)
        return FALSE;

    DWORD tid = GetWindowThreadProcessId(fg, NULL);
    if (is_target(GetKeyboardLayout(tid)))
        return FALSE;

    GUITHREADINFO gti = { sizeof(gti) };
    HWND dest = fg;
    if (GetGUIThreadInfo(tid, &gti) && gti.hwndFocus)
        dest = gti.hwndFocus;

    /* DefWindowProc handles this by activating the layout. Windows with
     * higher integrity (elevated apps) ignore it because of UIPI. */
    return PostMessageW(dest, WM_INPUTLANGCHANGEREQUEST, 0, (LPARAM)g_target);
}

static void on_timer(void)
{
    if (g_paused)
        return;
    if (idle_ms() >= g_idle_ms)
        switch_foreground();
}

/* "1 min" or "30 s": short units avoid plural rules in translations. */
static void format_seconds(DWORD s, wchar_t *out, size_t out_len)
{
    wchar_t fmt[32];
    BOOL minutes = s >= 60 && s % 60 == 0;
    load_text(minutes ? IDS_MINUTES : IDS_SECONDS, fmt, ARRAYSIZE(fmt));
    _snwprintf_s(out, out_len, _TRUNCATE, fmt, minutes ? s / 60 : s);
}

static void update_tip(void)
{
    wchar_t status[32], idle_label[64], idle[32];
    load_text(g_paused ? IDS_PAUSED : IDS_ACTIVE, status, ARRAYSIZE(status));
    load_text(IDS_IDLE_TIME, idle_label, ARRAYSIZE(idle_label));
    format_seconds(g_idle_ms / 1000, idle, ARRAYSIZE(idle));
    _snwprintf_s(g_nid.szTip, ARRAYSIZE(g_nid.szTip), _TRUNCATE, L"%s \u2013 %s\n%s\n%s: %s",
                 APP_NAME, status, g_target_name, idle_label, idle);
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

/* Installed keyboards; the ids map to g_menu_layouts. */
static HMENU layout_menu(void)
{
    HMENU menu = CreatePopupMenu();
    int n = GetKeyboardLayoutList(MAX_LAYOUTS, g_menu_layouts);
    for (int i = 0; i < n; i++) {
        wchar_t label[256];
        describe_layout(g_menu_layouts[i], label, ARRAYSIZE(label));
        AppendMenuW(menu, MF_STRING | (g_menu_layouts[i] == g_target ? MF_CHECKED : 0),
                    CMD_LAYOUT_FIRST + i, label);
    }
    return menu;
}

static HMENU idle_menu(void)
{
    HMENU menu = CreatePopupMenu();
    DWORD current = g_idle_ms / 1000;
    BOOL preset = FALSE;
    for (int i = 0; i < ARRAYSIZE(IDLE_PRESETS); i++) {
        wchar_t label[64];
        format_seconds(IDLE_PRESETS[i], label, ARRAYSIZE(label));
        preset |= IDLE_PRESETS[i] == current;
        AppendMenuW(menu, MF_STRING | (IDLE_PRESETS[i] == current ? MF_CHECKED : 0),
                    CMD_IDLE_FIRST + i, label);
    }
    /* Show a value set on the command line or in setup that is not a preset. */
    if (!preset) {
        wchar_t label[64], fmt[64], custom[128];
        format_seconds(current, label, ARRAYSIZE(label));
        load_text(IDS_CUSTOM, fmt, ARRAYSIZE(fmt));
        _snwprintf_s(custom, ARRAYSIZE(custom), _TRUNCATE, fmt, label);
        AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(menu, MF_STRING | MF_CHECKED | MF_GRAYED, 0, custom);
    }
    return menu;
}

static void show_menu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    wchar_t fmt[64], label[ARRAYSIZE(g_target_name) + 64];
    load_text(IDS_SWITCH_NOW, fmt, ARRAYSIZE(fmt));
    _snwprintf_s(label, ARRAYSIZE(label), _TRUNCATE, fmt, g_target_name);
    AppendMenuW(menu, MF_STRING, CMD_SWITCH_NOW, label);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    load_text(IDS_DEFAULT_KEYBOARD, label, ARRAYSIZE(label));
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)layout_menu(), label);
    load_text(IDS_IDLE_TIME, label, ARRAYSIZE(label));
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)idle_menu(), label);
    load_text(IDS_PAUSE, label, ARRAYSIZE(label));
    AppendMenuW(menu, MF_STRING | (g_paused ? MF_CHECKED : 0), CMD_PAUSE, label);
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    load_text(IDS_EXIT, label, ARRAYSIZE(label));
    AppendMenuW(menu, MF_STRING, CMD_EXIT, label);

    /* Required so the menu closes when clicking elsewhere. */
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessageW(hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
}

static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static UINT taskbar_created;

    switch (msg) {
    case WM_CREATE:
        taskbar_created = RegisterWindowMessageW(L"TaskbarCreated");
        return 0;
    case WM_TIMER:
        if (wp == TIMER_ID)
            on_timer();
        return 0;
    case WM_TRAYICON:
        if (LOWORD(lp) == WM_RBUTTONUP || LOWORD(lp) == WM_LBUTTONUP)
            show_menu(hwnd);
        return 0;
    case WM_COMMAND: {
        WORD id = LOWORD(wp);
        if (id >= CMD_LAYOUT_FIRST && id < CMD_LAYOUT_FIRST + MAX_LAYOUTS) {
            wchar_t klid[KL_NAMELENGTH];
            set_target(g_menu_layouts[id - CMD_LAYOUT_FIRST]);
            if (hkl_to_klid(g_target, klid))
                save_layout(klid);
            update_tip();
            return 0;
        }
        if (id >= CMD_IDLE_FIRST && id < CMD_IDLE_FIRST + ARRAYSIZE(IDLE_PRESETS)) {
            DWORD seconds = IDLE_PRESETS[id - CMD_IDLE_FIRST];
            g_idle_ms = seconds * 1000;
            save_idle(seconds);
            update_tip();
            return 0;
        }
        switch (id) {
        case CMD_SWITCH_NOW:
            /* The menu took focus; give it back before switching. */
            Sleep(100);
            switch_foreground();
            break;
        case CMD_PAUSE:
            g_paused = !g_paused;
            update_tip();
            break;
        case CMD_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        return 0;
    }
    case WM_QUERYENDSESSION:
        return TRUE;
    case WM_ENDSESSION:
        /* Signing out or shutting down. */
        if (wp)
            DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        Shell_NotifyIconW(NIM_DELETE, &g_nid);
        PostQuitMessage(0);
        return 0;
    }

    /* Explorer restarted: put the tray icon back. */
    if (taskbar_created && msg == taskbar_created) {
        Shell_NotifyIconW(NIM_ADD, &g_nid);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static BOOL parse_seconds(const wchar_t *s, DWORD min, DWORD *out_ms)
{
    wchar_t *end;
    unsigned long v = wcstoul(s, &end, 10);
    if (end == s || *end || v < min || v > 24 * 60 * 60)
        return FALSE;
    *out_ms = (DWORD)v * 1000;
    return TRUE;
}

/* Returns an error message for bad arguments, or NULL when all are valid. */
static const wchar_t *parse_args(BOOL *help)
{
    static wchar_t err[256];
    const wchar_t *layout = NULL;
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv)
        return L"Cannot read the command line.";

    *help = FALSE;
    err[0] = 0;
    for (int i = 1; i < argc && !err[0]; i++) {
        const wchar_t *opt = argv[i], *val = i + 1 < argc ? argv[i + 1] : NULL;
        if (wcscmp(opt, L"--help") == 0 || wcscmp(opt, L"-h") == 0 || wcscmp(opt, L"/?") == 0)
            *help = TRUE;
        else if (!val)
            swprintf(err, ARRAYSIZE(err), L"Missing value for %s.", opt);
        else if (wcscmp(opt, L"--idle") == 0) {
            if (!parse_seconds(val, 0, &g_idle_ms))
                swprintf(err, ARRAYSIZE(err), L"Invalid --idle value: %s", val);
            i++;
        } else if (wcscmp(opt, L"--check") == 0) {
            if (!parse_seconds(val, 1, &g_check_ms))
                swprintf(err, ARRAYSIZE(err), L"Invalid --check value: %s", val);
            i++;
        } else if (wcscmp(opt, L"--layout") == 0) {
            layout = val;
            i++;
        } else
            swprintf(err, ARRAYSIZE(err), L"Unknown option: %s", opt);
    }

    if (!err[0] && !*help && layout) {
        HKL hkl = resolve_layout(layout);
        if (hkl)
            set_target(hkl);
        else
            swprintf(err, ARRAYSIZE(err), L"Unknown or unavailable keyboard layout: %s", layout);
    }
    LocalFree(argv);
    return err[0] ? err : NULL;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmd, int show)
{
    (void)prev; (void)cmd; (void)show;

    g_ui_lang = pick_ui_language(GetUserDefaultUILanguage());

    wchar_t saved_layout[64] = L"en";
    load_settings(saved_layout, ARRAYSIZE(saved_layout));

    BOOL help;
    const wchar_t *err = parse_args(&help);
    if (help || err) {
        wchar_t text[ARRAYSIZE(USAGE) + 300];
        swprintf(text, ARRAYSIZE(text), L"%s%s%s", err ? err : L"", err ? L"\n\n" : L"", USAGE);
        MessageBoxW(NULL, text, APP_NAME, err ? MB_ICONERROR : MB_ICONINFORMATION);
        return err ? 1 : 0;
    }
    if (!g_target) {
        /* A saved layout that was removed from Windows falls back to English. */
        HKL hkl = resolve_layout(saved_layout);
        if (!hkl)
            hkl = resolve_layout(L"en");
        if (!hkl) {
            wchar_t text[256];
            load_text(IDS_NO_LAYOUT, text, ARRAYSIZE(text));
            MessageBoxW(NULL, text, APP_NAME, MB_ICONERROR);
            return 1;
        }
        set_target(hkl);
    }

    HANDLE mutex = CreateMutexW(NULL, TRUE, L"Local\\" APP_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = inst;
    wc.lpszClassName = APP_NAME;
    RegisterClassW(&wc);

    /* Hidden top-level window: receives tray and timer messages. */
    HWND hwnd = CreateWindowW(APP_NAME, APP_NAME, 0, 0, 0, 0, 0, NULL, NULL, inst, NULL);
    if (!hwnd)
        return 1;

    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    /* Small icon size, so the tray gets the crisp 16 px (or DPI-scaled) image. */
    g_nid.hIcon = LoadImageW(inst, MAKEINTRESOURCEW(1), IMAGE_ICON,
                             GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    if (!g_nid.hIcon)
        g_nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    Shell_NotifyIconW(NIM_ADD, &g_nid);
    update_tip();

    SetTimer(hwnd, TIMER_ID, g_check_ms, NULL);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    ReleaseMutex(mutex);
    return 0;
}
