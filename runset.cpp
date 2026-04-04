
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

std::wstring to_lower(const std::wstring s)
{
  std::wstring d = s;
  std::transform(d.begin(), d.end(), d.begin(), ::tolower);
  return d;
}

std::vector<std::wstring> sws = {L"hide", L"normal", L"showminimized", L"maximize", L"shownoactivate", L"show", L"minimize", L"showminnoactive", L"showna", L"restore", L"showdefault", L"forceminimize"};

int str_to_sw(std::wstring s)
{
  int ret = SW_NORMAL;

  int i = 0;
  for (std::wstring sw : sws)
  {
    std::wcout << s << std::endl;
    if (to_lower(s) == sw) return i;
    i++;
  }

  return SW_NORMAL;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  int argc;
  LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  std::vector<std::filesystem::path> args = std::vector<std::filesystem::path>(argv, argv + argc);

  if (args.size() < 2)
  {
    std::cerr << "Usage : " << std::filesystem::path(args[0]).stem().string() << R"EOF( [arguments]
Provide arguments in the following order: program x y w h show_mode
  prog program to run, at least this argument must be provided and also one of the other should be ...
  x horizontal position.
  y vertical position.
  w width.
  h height.
  show mode being one of: hide, normal, showminimized, maximize, shownoactivate, show, minimize, showminnoactive, showna, restore, showdefault, forceminimize
  (for more detail about show mode, see: https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-showwindow)

  This program works only for gui apps and of course if the gui app does not force it own position, size and show mode.
)EOF";

    return 1;
  }

  bool focus = false;
  STARTUPINFO si = {};
  si.wShowWindow = SW_NORMAL;
  si.dwFlags = STARTF_USESHOWWINDOW;

  if (args.size() >= 3)
  {
    si.dwX = std::stoi(args[2]);
    si.dwFlags |= STARTF_USEPOSITION;

    if (args.size() >= 4) si.dwY = std::stoi(args[3]);

    if (args.size() >= 5)
    {
      si.dwXSize = std::stoi(args[4]);
      si.dwFlags |= STARTF_USESIZE;

      if (args.size() >= 6) si.dwYSize = std::stoi(args[5]);
      if (args.size() >= 7) si.wShowWindow = str_to_sw(args[6]);
    }
  }

  PROCESS_INFORMATION pi;

  CreateProcess(nullptr, args[1].wstring().data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
  return 0;
}
