#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <windows.h>

std::atomic<HWND> g_hwnd(nullptr);
DWORD g_targetPID = 0;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
  DWORD pid;
  GetWindowThreadProcessId(hwnd, &pid);

  if (pid == g_targetPID && GetWindow(hwnd, GW_OWNER) == NULL && IsWindowVisible(hwnd))
  {
    g_hwnd = hwnd;
    return FALSE; // stop
  }
  return TRUE;
}

// Thread qui cherche la fenêtre en boucle
void findWindowLoop()
{
  while (g_hwnd == nullptr)
  {
    EnumWindows(EnumWindowsProc, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

int main()
{
  STARTUPINFO si = {sizeof(si)};
  PROCESS_INFORMATION pi;

  wchar_t cmd[] = L"notepad.exe";

  if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
  {
    std::cerr << "CreateProcess failed\n";
    return 1;
  }

  g_targetPID = pi.dwProcessId;

  // Lancer un thread pour chercher la fenêtre
  std::thread finder(findWindowLoop);

  // Attendre que le process soit prêt (optionnel mais recommandé)
  WaitForInputIdle(pi.hProcess, 5000);

  // Attendre que le thread trouve la fenêtre
  finder.join();

  HWND hwnd = g_hwnd.load();

  if (hwnd)
  {
    std::cout << "HWND trouvé: " << hwnd << std::endl;

    // Exemple : mettre la fenêtre au premier plan
    SetForegroundWindow(hwnd);
  }
  else
  {
    std::cout << "Fenêtre non trouvée.\n";
  }

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);

  return 0;
}
