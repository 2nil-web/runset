
// clang-format off
#include <windows.h>
#include <richedit.h>
#include <cstdio>
// clang-format on

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED 0x00080000
#endif
#ifndef LWA_COLORKEY
#define LWA_COLORKEY 0x00000001
#endif
#ifndef LWA_ALPHA
#define LWA_ALPHA 0x00000002
#endif

/* A priori ces VK_keys ne sont pas définies dans winuser.h */
#define VK_1 49 /* 1 key  */
#define VK_2 50 /* 2 key  */
#define VK_3 51 /* 3 key  */
#define VK_4 52 /* 4 key  */
#define VK_5 53 /* 5 key  */
#define VK_6 54 /* 6 key  */
#define VK_7 55 /* 7 key  */
#define VK_8 56 /* 8 key  */
#define VK_9 57 /* 9 key  */
#define VK_A 65 /* A key  */
#define VK_B 66 /* B key  */
#define VK_C 67 /* C key  */
#define VK_D 68 /* D key  */
#define VK_E 69 /* E key  */
#define VK_F 70 /* F key  */
#define VK_G 71 /* G key  */
#define VK_H 72 /* H key  */
#define VK_I 73 /* I key  */
#define VK_J 74 /* J key  */
#define VK_K 75 /* K key  */
#define VK_L 76 /* L key  */
#define VK_M 77 /* M key  */
#define VK_N 78 /* N key  */
#define VK_O 79 /* O key  */
#define VK_P 80 /* P key  */
#define VK_Q 81 /* Q key  */
#define VK_R 82 /* R key  */
#define VK_S 83 /* S key  */
#define VK_T 84 /* T key  */
#define VK_U 85 /* U key  */
#define VK_V 86 /* V key  */
#define VK_W 87 /* W key  */
#define VK_X 88 /* X key  */
#define VK_Y 89 /* Y key  */
#define VK_Z 90 /* Z key  */

extern const char *hlpmsg;

// Affiche l'erreur windows sur stderr
DWORD WinError(const char *s)
{
  DWORD lasterr = GetLastError();

  LPVOID lpMsgBuf;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, lasterr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR)&lpMsgBuf, 0, NULL);

  // Display the string.
  fprintf(stderr, "%s: %s\n", s, (char *)lpMsgBuf);
  fflush(stderr);
  // Free the buffer.
  LocalFree(lpMsgBuf);

  return lasterr;
}

#define HLP_WIDTH 530
#define HLP_HEIGHT 650

#ifndef ST_DEFAULT
#define ST_DEFAULT 0
#define ST_KEEPUNDO 1
#define ST_SELECTION 2
#define ST_NEWCHARS 4

typedef struct _settextex
{
  DWORD flags;
  UINT codepage;
} SETTEXTEX;
#endif

LRESULT CALLBACK HelpProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  static HWND he = NULL;

  switch (uMsg)
  {
  case WM_CREATE: {
    SETTEXTEX ste;
    he = CreateWindow(
        "RICHEDIT_CLASS",
        "",
        WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_VISIBLE | ES_MULTILINE | ES_SAVESEL | ES_SELECTIONBAR | ES_SUNKEN | ES_READONLY,
        0,
        0,
        HLP_WIDTH,
        HLP_HEIGHT,
        hwnd,
        HMENU(nullptr),
        GetModuleHandle(nullptr),
        (LPVOID)nullptr);

    if (he)
    {
      ste.flags = ST_DEFAULT;
      ste.codepage = CP_ACP;
      SendMessage(he, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)hlpmsg);
    }
  }
    return 0;
  case WM_SHOWWINDOW:
    if ((BOOL)wParam)
    {
      if (he)
        SendMessage(he, EM_SETSEL, (WPARAM)-1, (LPARAM)0);
    }
    return 0;
  case WM_SIZE:
    if (he)
    {
#ifdef DEBUG
      printf("w %d, h %d\n", LOWORD(lParam), HIWORD(lParam));
      fflush(stdout);
#endif
      MoveWindow(he, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
    }
    return 0;
  case WM_COMMAND:
    if (LOWORD(wParam) == IDCANCEL)
      DestroyWindow(hwnd);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  }

  return (LONG)DefWindowProc(hwnd, uMsg, wParam, lParam);
}

DWORD WINAPI DispHelp(HINSTANCE hinst)
{
  HWND hwnd;
  MSG msg;
  static int un = 1;

  if (un)
  {
    /*  Define Main Window Class */
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)HelpProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hinst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "AltEnterHelp";

    /*  Register Main Window Class */
    if (!RegisterClass(&wc))
    {
      WinError("RegisterClass");
      return 0;
    }

    LoadLibrary("RichEd32.Dll");

    un--;
  }

  hwnd = CreateWindowEx(WS_EX_TOPMOST, "AltEnterHelp", "", WS_POPUP | WS_THICKFRAME, (GetSystemMetrics(SM_CXSCREEN) - HLP_WIDTH) / 2, (GetSystemMetrics(SM_CYSCREEN) - HLP_HEIGHT) / 2, HLP_WIDTH, HLP_HEIGHT, NULL, (HMENU)NULL, hinst, (LPVOID)NULL);

  if (!hwnd)
  {
    WinError("CreateWindowEx AVI");
    return 0;
  }

  /*  Show and update the application */
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
  SetForegroundWindow(hwnd);

  /*  Traitement des messages */
  while (GetMessage(&msg, NULL, 0, 0))
  {
    if ((IsWindow(hwnd) && !IsDialogMessage(hwnd, &msg)))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}

/* Pour s'assurer que le programme ne se lance qu'une seule fois */
int FirstExec()
{
  CreateMutex(NULL, TRUE, "AltEnter0");
  if (GetLastError() == ERROR_ALREADY_EXISTS)
    return 0;
  else
    return 1;
}

/* Min/Max fenêtre 1er plan */
void PingPongTop()
{
  HWND haw = GetForegroundWindow();

  if (haw)
  {
    WINDOWPLACEMENT wndpl;

    GetWindowPlacement(haw, &wndpl);
    wndpl.showCmd = (wndpl.showCmd == SW_SHOWNORMAL) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL;
    SetWindowPlacement(haw, &wndpl);
  }
}

/* Min/Max fenêtre 1er plan */
void PingPongBottom()
{
  HWND haw = GetForegroundWindow();

  if (haw)
  {
    WINDOWPLACEMENT wndpl;

    GetWindowPlacement(haw, &wndpl);
    wndpl.showCmd = (wndpl.showCmd == SW_SHOWNORMAL) ? SW_SHOWMINIMIZED : SW_SHOWNORMAL;
    SetWindowPlacement(haw, &wndpl);
  }
}

void Regenerate(HWND hwnd)
{
  WINDOWPLACEMENT wp;
  if (IsZoomed(hwnd))
    ShowWindow(hwnd, SW_HIDE);
  GetWindowPlacement(hwnd, &wp);
  wp.rcNormalPosition.left++;
  SetWindowPlacement(hwnd, &wp);
  wp.rcNormalPosition.left--;
  SetWindowPlacement(hwnd, &wp);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
}

/* Decor/pas décor fenêtre 1er plan */
void PingPongDecor()
{
  HWND haw = GetForegroundWindow();

  if (haw)
  {
    if (GetWindowLong(haw, GWL_STYLE) & WS_CAPTION)
      SetWindowLong(haw, GWL_STYLE, GetWindowLong(haw, GWL_STYLE) & ~WS_CAPTION);
    else
      SetWindowLong(haw, GWL_STYLE, GetWindowLong(haw, GWL_STYLE) | WS_CAPTION);

    Regenerate(haw);
  }
}

#define START_HKID 0x1171

int hkid = START_HKID;

int RegisterKeys()
{
  /* Enregistre la combinaison de touche SHIFT WIN Flêches */
  if (!RegisterHotKey(NULL, hkid++, MOD_SHIFT | MOD_WIN, VK_LEFT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_SHIFT | MOD_WIN, VK_RIGHT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_SHIFT | MOD_WIN, VK_UP))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_SHIFT | MOD_WIN, VK_DOWN))
    return 1;

  /* Enregistre la combinaison de touche WIN Flêches */
  if (!RegisterHotKey(NULL, hkid++, MOD_WIN, VK_LEFT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_WIN, VK_RIGHT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_WIN, VK_UP))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_WIN, VK_DOWN))
    return 1;

  /* Enregistre la combinaison de touche ALT Flêches */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_LEFT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_RIGHT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_UP))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_DOWN))
    return 1;

  /* Enregistre la combinaison de touche ALT WIN Flêches */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT | MOD_WIN, VK_LEFT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT | MOD_WIN, VK_RIGHT))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT | MOD_WIN, VK_UP))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT | MOD_WIN, VK_DOWN))
    return 1;

  /* Enregistre la combinaison de touche Alt ADD */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_ADD))
    return 1;
  /* Enregistre la combinaison de touche Alt SUBTRACT */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_SUBTRACT))
    return 1;
  /* Enregistre la combinaison de touche Alt DEL */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_DELETE))
    return 1;

  /* Enregistre la combinaison de touche Alt Enter */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_RETURN))
    return 1;
  /* Enregistre la combinaison de touche Alt End */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_END))
    return 1;
  /* Enregistre la combinaison de touche Alt BackSpace */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_BACK))
    return 1;
  /* Enregistre la combinaison de touche Alt F9 */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_F9))
    return 1;
  if (!RegisterHotKey(NULL, hkid++, MOD_WIN, VK_C))
    return 1;

  /* Enregistre la combinaison de touche Alt F10 */
  if (!RegisterHotKey(NULL, hkid++, MOD_ALT, VK_F10))
    return 1;

  return 0;
}

void UnregisterKeys()
{
  while (hkid > START_HKID)
    UnregisterHotKey(NULL, hkid--);
}

typedef BOOL(WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);
typedef BOOL(WINAPI *PGLWA)(HWND, DWORD *, BYTE *, DWORD *);
PSLWA pSetLayeredWindowAttributes = NULL;
PSLWA SecondarySetLayeredWindowAttributes = NULL;
PGLWA pGetLayeredWindowAttributes = NULL;

#if (_WIN32_WINNT < 0x0500)

#define MAX_LAYERED 1000

typedef struct sLayeredWindowAttributes
{
  HWND hwnd;
  COLORREF crKey;
  BYTE bAlpha;
  DWORD dwFlags;
} sLayeredWindowAttributes;

sLayeredWindowAttributes lwa_tab[MAX_LAYERED];
int nLayered = 0;

BOOL WINAPI mySetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
  int intab = 0, i;

  for (i = 0; i < nLayered; i++)
  {
    if (hwnd == lwa_tab[i].hwnd)
    {
      lwa_tab[i].crKey = crKey;
      lwa_tab[i].bAlpha = bAlpha;
      lwa_tab[i].dwFlags = dwFlags;
      intab = 1;
      break;
    }
  }

  if (!intab)
  {
    if (nLayered >= MAX_LAYERED)
      nLayered = 0;
    lwa_tab[nLayered].hwnd = hwnd;
    lwa_tab[nLayered].crKey = crKey;
    lwa_tab[nLayered].bAlpha = bAlpha;
    lwa_tab[nLayered].dwFlags = dwFlags;
    nLayered++;
  }

  return SecondarySetLayeredWindowAttributes(hwnd, crKey, bAlpha, dwFlags);
}

BOOL WINAPI myGetLayeredWindowAttributes(HWND hwnd, COLORREF *pcrKey, BYTE *pbAlpha, DWORD *pdwFlags)
{
  int i;
  for (i = 0; i < nLayered; i++)
  {
    if (hwnd == lwa_tab[i].hwnd)
    {
      *pcrKey = lwa_tab[i].crKey;
      *pbAlpha = lwa_tab[i].bAlpha;
      *pdwFlags = lwa_tab[i].dwFlags;
      return 1;
    }
  }

  return 0;
}

#endif

void VaryTransparency(int stepfactor)
{
  static int usefull = 1;

  if (usefull)
  {

    HWND hwnd = GetForegroundWindow();

    if (hwnd)
    {
      COLORREF lrgb;
      BYTE factor = 255;
      DWORD flags = LWA_COLORKEY | LWA_ALPHA;

      /* Fait une seule fois */
      if (!pSetLayeredWindowAttributes || !pGetLayeredWindowAttributes)
      {
#if (_WIN32_WINNT >= 0x0500)
        pSetLayeredWindowAttributes = SetLayeredWindowAttributes;
        pGetLayeredWindowAttributes = GetLayeredWindowAttributes;
#else
        HMODULE hDLL = LoadLibrary("user32");
        pSetLayeredWindowAttributes = (PSLWA)GetProcAddress(hDLL, "SetLayeredWindowAttributes");
        pGetLayeredWindowAttributes = (PGLWA)GetProcAddress(hDLL, "GetLayeredWindowAttributes");

        if (!pSetLayeredWindowAttributes)
        {
          usefull = 0;
          return;
        }

        if (!pGetLayeredWindowAttributes)
        {
          SecondarySetLayeredWindowAttributes = pSetLayeredWindowAttributes;
          pSetLayeredWindowAttributes = mySetLayeredWindowAttributes;
          pGetLayeredWindowAttributes = myGetLayeredWindowAttributes;
        }
#endif
      }

      if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED)
      {
        if (pGetLayeredWindowAttributes(hwnd, &lrgb, &factor, &flags) == 0)
          WinError("GLWA");
      }
      else
      {
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        lrgb = RGB(20, 20, 255);
      }

      if (stepfactor > 0)
      {
        if (factor + stepfactor < 255)
          factor += stepfactor;
        else
          factor = 255;
      }
      else if (stepfactor < 0)
      {
        if (factor + stepfactor > 0)
          factor += stepfactor;
        else
          factor = 0;
      }

      if (pSetLayeredWindowAttributes(hwnd, lrgb, factor, flags) == 0)
        WinError("GLWA");

      UpdateWindow(hwnd);
    }
  }
}

void PingPongTrans()
{
  HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_LAYERED)
    {
      SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
      Regenerate(hwnd);
    }
    else
    {
      SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
      VaryTransparency(0);
      UpdateWindow(hwnd);
    }
  }
}

void SizeWin(int hor, int ver)
{
  HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    if (IsZoomed(hwnd))
      return;
    else
    {
      WINDOWPLACEMENT wp;
      GetWindowPlacement(hwnd, &wp);
      wp.rcNormalPosition.right += hor;
      wp.rcNormalPosition.bottom += ver;
      SetWindowPlacement(hwnd, &wp);
      UpdateWindow(hwnd);
    }
  }
}

void PasteWinTitle2Clip()
{
  HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    size_t len;
    char *s;
    HGLOBAL cdata;
    void *lock;

    len = GetWindowTextLength(hwnd);
    s = (char *)malloc(len + 1);
    GetWindowText(hwnd, s, (int)len + 1);
    cdata = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, len + 1);
    if (!cdata)
      return;
    lock = GlobalLock(cdata);
    if (!lock)
      return;
    memcpy(lock, s, len);
    ((unsigned char *)lock)[len] = '\0';
    GlobalUnlock(cdata);

    if (OpenClipboard(hwnd))
    {
      EmptyClipboard();
      SetClipboardData(CF_TEXT, cdata);
      CloseClipboard();
    }
    else
      GlobalFree(cdata);
  }
}

void MoveWin(int hor, int ver)
{
  HWND hwnd = GetForegroundWindow();

  if (hwnd)
  {
    if (IsZoomed(hwnd))
      return;
    else
    {
      WINDOWPLACEMENT wp;
      GetWindowPlacement(hwnd, &wp);
      wp.rcNormalPosition.left += hor;
      wp.rcNormalPosition.right += hor;
      wp.rcNormalPosition.top += ver;
      wp.rcNormalPosition.bottom += ver;
      SetWindowPlacement(hwnd, &wp);
      UpdateWindow(hwnd);
    }
  }
}

/* Permute l'état NORMAL/MAXIMISER de la fenêtre de 1er plan à l'appuie sur Alt Enter */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE , LPSTR , int )
{
  if (FirstExec())
  {
    MSG msg;

    RegisterKeys();

    /* Ne récupére que les messages HOTKEY */
    while (GetMessage(&msg, NULL, WM_HOTKEY, WM_HOTKEY))
    {
      switch (LOWORD(msg.lParam))
      {
      case MOD_ALT:
        switch (HIWORD(msg.lParam))
        {
          /* Retaillage fenetre */
        case VK_RIGHT:
          SizeWin(20, 0);
          break;
        case VK_DOWN:
          SizeWin(0, 20);
          break;
        case VK_LEFT:
          SizeWin(-20, 0);
          break;
        case VK_UP:
          SizeWin(0, -20);
          break;

        /* Si Alt Enter, maxi/restore la fenêtre de 1er plan */
        case VK_RETURN:
          PingPongTop();
          break;
        case VK_END:
          PingPongBottom();
          break;
        /* Eléve/remet les décors fenêtre 1er plan */
        case VK_BACK:
          PingPongDecor();
          break;
        case VK_ADD:
          VaryTransparency(10);
          break;
        case VK_SUBTRACT:
          VaryTransparency(-10);
          break;
        case VK_DELETE:
          PingPongTrans();
          break;
        /* Si Alt F10, sort du programme */
        case VK_F10:
          if (MessageBox(NULL, "Confirmer la sortie de AltEnter", "AltEnter", MB_YESNO | MB_ICONWARNING | MB_SYSTEMMODAL) == IDYES)
          {
            RegisterKeys();
            return (int)msg.wParam;
          }
          break;
        /* Si Alt F9, aide du programme */
        case VK_F9:
          DispHelp(hInstance);
          break;
        default:
          break;
        }
        break;

      case MOD_ALT | MOD_WIN:
        switch (HIWORD(msg.lParam))
        { /* Retaillage fin de la fenetre */
        case VK_RIGHT:
          SizeWin(1, 0);
          break;
        case VK_LEFT:
          SizeWin(-1, 0);
          break;
        case VK_UP:
          SizeWin(0, -1);
          break;
        case VK_DOWN:
          SizeWin(0, 1);
          break;
        default:
          break;
        }
        break;

      case MOD_WIN:
        switch (HIWORD(msg.lParam))
        {
        /* Déplace à droite la fenêtre de 1er plan */
        case VK_RIGHT:
          MoveWin(20, 0);
          break;
        /* Déplace à gauche la fenêtre de 1er plan */
        case VK_LEFT:
          MoveWin(-20, 0);
          break;
        /* Remonte la fenêtre de 1er plan */
        case VK_UP:
          MoveWin(0, -20);
          break;
        /* Descend la fenêtre de 1er plan */
        case VK_DOWN:
          MoveWin(0, 20);
          break;
        case VK_C:
          PasteWinTitle2Clip();
          break;
        default:
          break;
        }
        break;

      case MOD_SHIFT | MOD_WIN:
        switch (HIWORD(msg.lParam))
        {
        /* Déplace à droite la fenêtre de 1er plan */
        case VK_RIGHT:
          MoveWin(1, 0);
          break;
        /* Déplace à gauche la fenêtre de 1er plan */
        case VK_LEFT:
          MoveWin(-1, 0);
          break;
        /* Remonte la fenêtre de 1er plan */
        case VK_UP:
          MoveWin(0, -1);
          break;
        /* Descend la fenêtre de 1er plan */
        case VK_DOWN:
          MoveWin(0, 1);
          break;

        default:
          break;
        }
        break;
      }
    }
  }

  return 0;
}
