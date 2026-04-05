
#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <windows.h>

// Evolutions à envisager :
//  Combiner ce petit outil avec AltEnter pour avoir une fenêtre "projet" permettant de regrouper des applis et les rappeler et mettre sur le devant avec un raccourci clavier

std::wstring to_lower(const std::wstring s)
{
  std::wstring d = s;
  std::transform(d.begin(), d.end(), d.begin(), ::tolower);
  return d;
}

//std::vector<std::wstring> sws = {L"hide", L"normal", L"showminimized", L"maximize", L"shownoactivate", L"show", L"minimize", L"showminnoactive", L"showna", L"restore", L"showdefault", L"forceminimize"};
std::vector<std::wstring> sws = {{}, L"normal", {}, L"maximize", {}, {}, L"minimize",{}, {}, {}, {}, {}};

int str_to_sw(std::wstring s)
{
  int i = 0;
  for (std::wstring sw : sws)
  {
    // std::wcout << s << std::endl;
    if ((to_lower(s) == sw || to_lower(s) == sw.substr(0, 3)) && (i == SW_MAXIMIZE || i == SW_MINIMIZE || i == SW_NORMAL)) return i;
    i++;
  }

  return SW_HIDE;
}
struct ScreenArray {
  std::vector<RECT> Monitors;
  POINT max_work_pt = { 0, 0 }, max_monitor_pt = { 0, 0 };
  std::wstringstream ss;
  
   static BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC, LPRECT rect, LPARAM data) {
    MONITORINFOEX mi = {};
    
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(monitor, &mi);
    auto p = reinterpret_cast<ScreenArray *>(data);
    
    if (mi.dwFlags == DISPLAY_DEVICE_MIRRORING_DRIVER) {
      p->ss << "Using mirroring driver for multi-screen." << std::endl;
    } else {
      p->Monitors.push_back(*rect);
      p->ss << "Work area rectangle for ";
      if (mi.dwFlags == MONITORINFOF_PRIMARY) p->ss << "primary ";
      p->ss << mi.szDevice << " is : (" << mi.rcWork.left << ", " << mi.rcWork.top << ", " << mi.rcWork.right << ", " << mi.rcWork.bottom << ").";

      p->ss << " And the whole display rectangle for this monitor is : (" << mi.rcMonitor.left << ", " << mi.rcMonitor.top << ", " << mi.rcMonitor.right << ", " << mi.rcMonitor.bottom << ").";
      p-> ss << std::endl;

      if (p->max_work_pt.x < mi.rcWork.right) p->max_work_pt.x = mi.rcWork.right;
      if (p->max_work_pt.y < mi.rcWork.bottom) p->max_work_pt.y = mi.rcWork.bottom;

      if (p->max_monitor_pt.x < mi.rcMonitor.right) p->max_monitor_pt.x = mi.rcMonitor.right;
      if (p->max_monitor_pt.y < mi.rcMonitor.bottom) p->max_monitor_pt.y = mi.rcMonitor.bottom;
    }
    return TRUE;
  }
  
  ScreenArray() {
    Monitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)this);
  }

  void DisplayInfo() {
    std::wcout << ss.str();
    std::cout << "Max work area surrounding the whole set of area, has the following size: (" << max_work_pt.x << ", " << max_work_pt.y << ").";
    std::cout << " And the max display area surrounding the whole set of monitor, has the following size: (" << max_monitor_pt.x << ", " << max_monitor_pt.y << ")." << std::endl;
  }
};

void correct_geometry(int& x, int& y, int& w, int& h)
{
  ScreenArray myArray;
  //myArray.DisplayInfo();
  // Correct size first
  if (w > myArray.max_work_pt.x) w=myArray.max_work_pt.x;
  if (h > myArray.max_work_pt.y) h=myArray.max_work_pt.y;

  // Then correct position by taking into account the size
  if (x < 0) x=0;
  if (y < 0) y=0;
  if (x > myArray.max_work_pt.x-60) x=myArray.max_work_pt.x-w;
  if (y > myArray.max_work_pt.y-60) y=myArray.max_work_pt.y-h;
}

void handle_window(HWND hwnd, int x, int y, int w, int h, int show_mode)
{
  ShowWindow(hwnd, show_mode);
  if (show_mode == SW_NORMAL) {
    MoveWindow(hwnd, x, y, w, h, true);
    SetForegroundWindow(hwnd);
  }
}

HWND g_hwnd = NULL;
DWORD g_targetPID = 0;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM /*lParam*/)
{
  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);

  if (pid == g_targetPID && GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd))
  {
    g_hwnd = hwnd;
    return FALSE;
  }
  return TRUE;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  int argc;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  std::vector<std::filesystem::path> args = std::vector<std::filesystem::path>(argv, argv + argc);

  if (args.size() < 2)
  {
    std::cerr << "Usage : " << std::filesystem::path(args[0]).stem().string() << R"EOF( "Command" [other parameters]
Provide your parameters respecting the following order: "Command" x y w h show_mode
This program may only works correctly with gui apps.
  At least the first parameter must be provided. It corresponds to the app to be run with its eventual parameters and if it contains spaces then it must be surrounded with quotation marks.
  The other parameters are optional but must respect the order, they correspond to: horizontal position, vertical position, width and height of the window.
  And finally the show mode of the window may also be provided, being one of: normal (this is the default), minimize or maximize (They may be shorten to min or max). But be aware that by using the minimize or maximize show mode, the provided geometry may not be fully respected when restoring the window.
)EOF";

    return 1;
  }

  STARTUPINFO si = {};
  si.wShowWindow = SW_MINIMIZE;
  si.dwFlags = STARTF_USESHOWWINDOW;
  int show_mode=SW_NORMAL;
  std::wstring sflags = L"", sShow_mode=L"";
  std::wstringstream ss;
  ss << "Command: " << args[1];

  int x=0, y=0, w, h;

  if (args.size() >= 3)
  {
    x = std::stoi(args[2]);
    si.dwFlags |= STARTF_USEPOSITION;
    sflags = L"STARTF_USEPOSITION";

    if (args.size() >= 4) y = std::stoi(args[3]);

    if (args.size() >= 5)
    {
      w = std::stoi(args[4]);
      si.dwFlags |= STARTF_USESIZE;
      sflags += L" | STARTF_USESIZE";

      if (args.size() >= 6) h = std::stoi(args[5]);
      if (args.size() >= 7)
      {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        sflags += L" | STARTF_USESHOWWINDOW";
        show_mode = str_to_sw(args[6]);
        si.wShowWindow = SW_MINIMIZE; //show_mode;
        sShow_mode = args[6];
      }
    }
  }

  correct_geometry(x, y, w, h);
  si.dwX=(DWORD)x;
  si.dwY=(DWORD)y;
  si.dwXSize=(DWORD)w;
  si.dwYSize=(DWORD)h;

  if (!sflags.empty())
    ss << std::endl << "flags: (" << sflags << ") = " << si.dwFlags;
  if (!sShow_mode.empty())
    ss << std::endl << "show mode: (" << sShow_mode << ") = " << show_mode;
  ss << std::endl << "geometry: (" << si.dwX << ", " << si.dwY << ", " << si.dwXSize << ", " << si.dwYSize << ')' << std::endl;
  std::wcout << ss.str() << std::endl;
  //MessageBox(nullptr, ss.str().c_str(), L"RunsSet", MB_OK);
  PROCESS_INFORMATION pi;

  CreateProcess(nullptr, args[1].wstring().data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);

  g_targetPID = pi.dwProcessId;

  auto start = std::chrono::steady_clock::now();

  while (g_hwnd == nullptr) {
    EnumWindows(EnumWindowsProc, 0);
    if (std::chrono::steady_clock::now() - start > std::chrono::seconds(5)) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  if (g_hwnd)
  {
    wchar_t s[1024];
    GetWindowText(g_hwnd, s, 1024);
    std::wcout << "Found window with handle: " << g_hwnd << " and name: " << s << std::endl;
    handle_window(g_hwnd, si.dwX, si.dwY, si.dwXSize, si.dwYSize, show_mode);
    return 0;
  }
  else
  {
    std::cout << "Window not found" << std::endl;
    return 1;
  }
}
