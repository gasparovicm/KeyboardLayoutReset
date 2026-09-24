/*
 * LayoutReset - switches the keyboard layout back to English when the
 * computer has not been used for a while.
 *
 * Runs as a tiny tray application. Every CHECK seconds it looks at how long
 * the user has been idle (no keyboard or mouse input). When the idle time
 * reaches IDLE seconds and the focused window is not using the English
 * layout, it asks that window to switch to English.
 *
 * Command line:
 *   LayoutReset.exe [--idle SECONDS] [--check SECONDS] [--layout KLID]
 *
 *   --idle    seconds without input before switching (default 60, 0 = always)
 *   --check   how often to check, in seconds          (default 5)
 *   --layout  keyboard layout id, e.g. 00000409 (US) or 00000809 (UK).
 *             Default: the first English layout already installed.
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <stdlib.h>
#include <wchar.h>

#define APP_NAME L"LayoutReset"
#define WM_TRAYICON (WM_APP + 1)
#define TIMER_ID 1

enum { CMD_SWITCH_NOW = 100, CMD_PAUSE, CMD_EXIT };

static DWORD g_idle_ms = 60 * 1000;
static DWORD g_check_ms = 5 * 1000;
static HKL g_target = NULL;
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

/* First installed layout whose language is English, so we never add a layout
 * the user does not have. Falls back to loading US English. */
static HKL find_english_layout(void)
{
    HKL list[64];
    int n = GetKeyboardLayoutList(64, list);
    for (int i = 0; i < n; i++) {
        LANGID lang = LOWORD((UINT_PTR)list[i]);
        if (PRIMARYLANGID(lang) == LANG_ENGLISH)
            return list[i];
    }
    return LoadKeyboardLayoutW(L"00000409", KLF_NOTELLSHELL);
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
    swprintf(g_nid.szTip, ARRAYSIZE(g_nid.szTip), L"%s - %s",
             APP_NAME, g_paused ? L"paused" : L"active");
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

static void show_menu(HWND hwnd)
{
    POINT pt;
    GetCursorPos(&pt);
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, CMD_SWITCH_NOW, L"Switch to English now");
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

static void parse_args(void)
{
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv)
        return;

    for (int i = 1; i + 1 < argc; i += 2) {
        if (wcscmp(argv[i], L"--idle") == 0)
            g_idle_ms = (DWORD)_wtoi(argv[i + 1]) * 1000;
        else if (wcscmp(argv[i], L"--check") == 0 && _wtoi(argv[i + 1]) > 0)
            g_check_ms = (DWORD)_wtoi(argv[i + 1]) * 1000;
        else if (wcscmp(argv[i], L"--layout") == 0)
            g_target = LoadKeyboardLayoutW(argv[i + 1], KLF_NOTELLSHELL);
    }
    LocalFree(argv);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE prev, LPWSTR cmd, int show)
{
    (void)prev; (void)cmd; (void)show;

    HANDLE mutex = CreateMutexW(NULL, TRUE, L"Local\\" APP_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    parse_args();
    if (!g_target)
        g_target = find_english_layout();
    if (!g_target) {
        MessageBoxW(NULL, L"No English keyboard layout found.", APP_NAME, MB_ICONERROR);
        return 1;
    }

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
    g_nid.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(1));
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
