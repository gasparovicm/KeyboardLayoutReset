/*
 * KeyboardLayoutReset - switches the keyboard layout back to a default (English
 * unless configured otherwise) when the computer has not been used for a while.
 *
 * Runs as a tiny tray application. Every CHECK seconds it looks at how long
 * the user has been idle (no keyboard or mouse input). When the idle time
 * reaches IDLE seconds and the focused window is not using the default
 * layout, it asks that window to switch.
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#define APP_NAME L"KeyboardLayoutReset"
#define WM_TRAYICON (WM_APP + 1)
#define TIMER_ID 1

static const wchar_t USAGE[] =
    L"KeyboardLayoutReset.exe [--layout LAYOUT] [--idle SECONDS] [--check SECONDS]\n\n"
    L"--layout  Default keyboard to switch back to. A language or locale\n"
    L"          name (en, en-US, en-GB, sk-SK, de-DE) or a layout id\n"
    L"          (00000409). Default: en, the first installed English layout.\n\n"
    L"--idle    Seconds without keyboard or mouse input before switching.\n"
    L"          Default 60. Use 0 to switch on every check, even while typing.\n\n"
    L"--check   How often to check, in seconds. Default 5.";

enum { CMD_SWITCH_NOW = 100, CMD_PAUSE, CMD_EXIT };

static DWORD g_idle_ms = 60 * 1000;
static DWORD g_check_ms = 5 * 1000;
static HKL g_target = NULL;
static wchar_t g_target_name[LOCALE_NAME_MAX_LENGTH + 64] = L"English";
static BOOL g_paused = FALSE;
static NOTIFYICONDATAW g_nid;

/* Milliseconds since the last keyboard or mouse input in this session. */
static DWORD idle_ms(void)
{
    LASTINPUTINFO lii = { sizeof(lii) };
    if (!GetLastInputInfo(&lii))
        return 0;
    return GetTickCount() - lii.dwTime;
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

/* Resolves --layout: a layout id like 00000409, or a language/locale name
 * like en, en-US or sk-SK. Returns NULL if the value is not valid. */
static HKL resolve_layout(const wchar_t *spec)
{
    if (is_klid(spec))
        return LoadKeyboardLayoutW(spec, KLF_NOTELLSHELL);

    LCID lcid = LocaleNameToLCID(spec, LOCALE_ALLOW_NEUTRAL_NAMES);
    if (lcid == 0)
        return NULL;

    LANGID lang = LANGIDFROMLCID(lcid);
    HKL hkl = find_installed_layout(lang);
    if (hkl || SUBLANGID(lang) == 0)
        return hkl;

    /* Not installed yet: load the language's standard layout. */
    wchar_t klid[KL_NAMELENGTH];
    swprintf(klid, KL_NAMELENGTH, L"%08X", lang);
    return LoadKeyboardLayoutW(klid, KLF_NOTELLSHELL);
}

/* Human-readable name of the target layout's language for the tray. */
static void set_target_name(void)
{
    LCID lcid = MAKELCID(LOWORD((UINT_PTR)g_target), SORT_DEFAULT);
    GetLocaleInfoW(lcid, LOCALE_SENGLISHDISPLAYNAME, g_target_name, ARRAYSIZE(g_target_name));
}

static BOOL is_target(HKL hkl)
{
    /* The low word of an HKL is its language id (e.g. 0x0409 = en-US). */
    return LOWORD((UINT_PTR)hkl) == LOWORD((UINT_PTR)g_target);
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

static void update_tip(void)
{
    swprintf(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"%s - %s\n%s after %lu s idle",
             APP_NAME, g_paused ? L"paused" : L"active", g_target_name, g_idle_ms / 1000);
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

static void show_menu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    wchar_t label[ARRAYSIZE(g_target_name) + 32];
    swprintf(label, ARRAYSIZE(label), L"Switch to %s now", g_target_name);
    AppendMenuW(menu, MF_STRING, CMD_SWITCH_NOW, label);
    AppendMenuW(menu, MF_STRING | (g_paused ? MF_CHECKED : 0), CMD_PAUSE, L"Pause");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, CMD_EXIT, L"Exit");

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
    case WM_COMMAND:
        switch (LOWORD(wp)) {
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
    const wchar_t *layout = L"en";
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

    if (!err[0] && !*help) {
        g_target = resolve_layout(layout);
        if (!g_target)
            swprintf(err, ARRAYSIZE(err), L"Unknown or unavailable keyboard layout: %s", layout);
    }
    LocalFree(argv);
    return err[0] ? err : NULL;
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmd, int show)
{
    (void)prev; (void)cmd; (void)show;

    BOOL help;
    const wchar_t *err = parse_args(&help);
    if (help || err) {
        wchar_t text[ARRAYSIZE(USAGE) + 300];
        swprintf(text, ARRAYSIZE(text), L"%s%s%s", err ? err : L"", err ? L"\n\n" : L"", USAGE);
        MessageBoxW(NULL, text, APP_NAME, err ? MB_ICONERROR : MB_ICONINFORMATION);
        return err ? 1 : 0;
    }
    set_target_name();

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
