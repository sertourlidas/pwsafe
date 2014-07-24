/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

// CSDThread.h header file

#include "core/StringX.h"
#include "GetMasterPhrase.h"
#include "YubiMixin.h"

#include <vector>
#include <mutex>

struct st_MonitorImageInfo
{
  HDC hdcMonitor;
  HWND hwndBkGrndWindow;
  HBITMAP hbmDimmendMonitorImage;
  int left, top, width, height;

  st_MonitorImageInfo()
    : hdcMonitor(0), hwndBkGrndWindow(0), hbmDimmendMonitorImage(0), left(0), top(0), width(0), height(0)
  {}

  st_MonitorImageInfo(const st_MonitorImageInfo &that)
    : hdcMonitor(that.hdcMonitor), hwndBkGrndWindow(that.hwndBkGrndWindow),
    hbmDimmendMonitorImage(that.hbmDimmendMonitorImage),
    left(that.left), top(that.top), width(that.width), height(that.height)
  {}

  st_MonitorImageInfo &operator=(const st_MonitorImageInfo &that)
  {
    if (this != &that) {
      hdcMonitor = that.hdcMonitor;
      hwndBkGrndWindow = that.hwndBkGrndWindow;
      hbmDimmendMonitorImage = that.hbmDimmendMonitorImage;
      left = that.left;
      top = that.top;
      width = that.width;
      height = that.height;
    }
    return *this;
  }
};

class CVKeyBoardDlg;

class CSDThread : public CYubiMixin
{

public:
  CSDThread(int iDialogID, GetMasterPhrase *pGMP,
            HMONITOR hCurrentMonitor, bool bUseSecureDesktop);
  virtual ~CSDThread();

  StringX GetPhrase() const { return m_pGMP->sPhrase; }
  void GetThreadErrorCodes(int &irc, int &irc2, DWORD &dwError)
  {
    irc = m_irc; irc2 = m_irc2;  dwError = m_dwError;
  }

  enum {
    SUCCESS = 0,
    FAILED = -1,

    /*
      Thread can also return IDOK (1) or IDCANCEL (2) depending on user's
      response on main dialog shown on new Desktop
    */

    // Main error codes
    CREATE_SA_FAILED                          = 10,
    CREATE_NEW_DESKTOP_FAILED                 = 11,
    SET_THREAD_TO_NEW_DESKTOP_FAILED          = 12,
    REGISTER_BACKGROUND_WINDOW_CLASS_FAILED   = 13,
    CREATE_BACKGROUND_WINDOW_FAILED           = 14,
    SWITCH_DESKTOP_TO_NEW_DESKTOP_FAILED      = 15,
    CREATE_MASTER_PHRASE_DIALOG_FAILED        = 16,
    DESTROY_MASTER_PHRASE_DIALOG_FAILED       = 17,
    UNREGISTER_BACKGROUND_WINDOW_CLASS_FAILED = 18,
    SET_THREAD_TO_ORIGINAL_DESKTOP_FAILED     = 19,
    CLOSE_NEW_DESKTOP_FAILED                  = 20,

    // Detailed error codes for (currently) Create Security Attributes only
    CREATE_SA_ALLOCATEANDINITIALIZESID_FAILED         = 1,
    CREATE_SA_GETLOGONSID_CURRENTUSERSID_FAILED       = 2,
    CREATE_SA_SETENTRIESINACL_FAILED                  = 3,
    CREATE_SA_GETSECURITYDESCRIPTOR_LOCALALLOC_FAILED = 4,
    CREATE_SA_INITIALIZESECURITYDESCRIPTOR_FAILED     = 5,
    CREATE_SA_SETSECURITYDESCRIPTORDACL_FAILED        = 6,
  };

 protected:
   BOOL InitInstance();
   static DWORD WINAPI SDThreadProc(void *lpParameter);

   static INT_PTR CALLBACK MPDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
   static void CALLBACK TimerProc(void *lpParameter, BOOLEAN TimerOrWaitFired);
   static BOOL CALLBACK DesktopEnumProc(LPTSTR name, LPARAM lParam);
   static BOOL CALLBACK WindowEnumProc(HWND hwnd, LPARAM lParam);
   static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

 private:
  enum {
    MONITORIMAGESCREATED        = 0x0001,
    NEWDESKTOCREATED            = 0x0002,
    SETTHREADDESKTOP            = 0x0004,
    SWITCHEDDESKTOP             = 0x0008,
    REGISTEREDWINDOWCLASS       = 0x0010,
    BACKGROUNDWINDOWSCREATED    = 0x0020,
    MASTERPHRASEDIALOGCREATED   = 0x0040,
    VIRTUALKEYBOARDCREATED      = 0x0080,
    MASTERPHRASEDIALOGENDED     = 0x0100,
  };

   friend class CPKBaseDlg;
   friend class CPasskeyEntry;
   friend class CPasskeyChangeDlg;

   INT_PTR DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
   DWORD ThreadProc();

   void OnInitDialog();
   void OnVirtualKeyboard();
   void OnOK();
   void OnCancel();
   void OnQuit();
   void OnInsertBuffer();

   BOOL AddTooltip(UINT uiControlID, UINT uiToolString, UINT uiFormat = NULL);
   BOOL AddTooltip(UINT uiControlID, stringT sText);

   void CheckDesktop();
   void CheckWindow();
   void ResetTimer();

   bool GetLogonSID(PSID &logonSID);
   int CreateSA(SECURITY_ATTRIBUTES &sa, PSECURITY_DESCRIPTOR &pSD, PACL &pACL,
     PSID &pOwnerSID, PSID &pCurrentUserSID);
   void CancelSecureDesktop();

   bool TerminateProcesses();
   bool GetChildProcesses(const bool bStart);

   stringT m_masterphrase;
   stringT m_sBkGrndClassName;
   stringT m_sDesktopName;

   HINSTANCE m_hInstResDLL;

   GetMasterPhrase *m_pGMP;
   CVKeyBoardDlg *m_pVKeyBoardDlg;
   std::vector<DWORD> m_vPIDs;
   std::mutex m_mutex;
   DWORD m_dwAccessMask, m_dwForbidden, m_dwError;
   int m_irc, m_irc2;

   // Yubi stuff
   CProgressCtrl m_yubi_timeout;
   CEdit m_yubi_status;
   CBitmap m_yubiLogo;
   CBitmap m_yubiLogoDisabled;
   void YubiControlsUpdate(bool insertedOrRemoved); // enable/disable/show/hide
   // Callbacks interfaces
   virtual void yubiShowChallengeSent(); // request's in the air, setup GUI to wait for reply
   virtual void yubiProcessCompleted(YKLIB_RC yrc, unsigned short ts, const BYTE *respBuf);
   virtual void yubiInserted(void);
   virtual void yubiRemoved(void);
   int m_passkeyID; // either IDC_PASSKEY or IDC_NEWPASSKEY
   StringX m_yubiResp[2]; // [0] set via IDC_PASSKEY, [1] via IDC_NEWPASSKEY

   HINSTANCE m_hInstance;
   HWND m_hwndVKeyBoard, m_hwndMasterPhraseDlg;
   HDESK m_hOriginalDesk, m_hNewDesktop;

   unsigned int m_iStartTime;
   HANDLE m_hTimer;
   HANDLE m_hWaitableTimer;
   HWND m_hwndStaticTimer, m_hwndStaticTimerText, m_hwndStaticSeconds;
   HWND m_hwndDlg, m_hwndTooltip;
   int m_iMinutes, m_iSeconds;
   int m_iUserTimeLimit;
   WORD m_wDialogID;
   HMONITOR m_hCurrentMonitor;
   COLORREF m_cfMask;
   int m_IDB;

   bool m_bDoTimerProcAction, m_bMPWindowBeingShown, m_bVKWindowBeingShown;
   bool m_bUseSecureDesktop, m_bDesktopPresent, m_bWindowPresent;
   short xFlags;

   // Secure Desktop - screen related
   void CaptureMonitorImage(HDC &hdcMonitor, HDC &hdcCapture, HBITMAP &hbmDimmendScreen,
     const int left, const int top, const int nScreenWidth, const int nScreenHeight);
   void DimMonitorImage(HDC &hdcMonitor, HDC &hdcCapture, HBITMAP &hbmDimmendScreen);
   void GetMonitorImages();

   // Vector of monitors and related image information
   typedef std::vector<st_MonitorImageInfo> MonitorImageInfo;
   typedef std::vector<st_MonitorImageInfo>::iterator MonitorImageInfoIter;
   MonitorImageInfo m_vMonitorImageInfo;

   // Secure Desktop related
   int m_iLastFocus;

   // Thread return code
   DWORD m_dwRC;
};
