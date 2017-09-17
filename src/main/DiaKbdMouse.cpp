/**
 *	@file	DiaKbdMouse.cpp
 *	@brief	�L�[�{�[�h�Ń}�E�X�𐧌�.
 *	@author	Masashi KITAMURA
 *	@date	2006
 *	@note
 *		�t���[�\�[�X
 *		HACK HACK MORE HACK
 */

#include "stdafx.h"
#include "DiaKbdMouse.h"
#include <cstdlib>
#include <cstring>
#include "../cmn/DebugPrintf.h"
#include "../dll/DiaKbdMouseHook.h"
#include "KbdMouseCtrl.h"
#include "TrayIcon.h"
#include "ConfigFileReader.h"


/// �L�[�{�[�h�Ń}�E�X�𐧌䂷��A�v���E�N���X.
class CDiaKbdMouseApp {
public:
	CDiaKbdMouseApp();
	// ~CDiaKbdMouseApp() {;}

	int winMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);

private:
	ATOM			registerClass(HINSTANCE hInstance);
	bool			initInstance(HINSTANCE hInstance);

	static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK dlgProcAbout(HWND, UINT, WPARAM, LPARAM);

	DWORD			getConfigData(CDiaKbdMouseHook_ConvKeyTbl& tbl);
	LRESULT 		wmCreate (HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT 		wmCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	LRESULT 		wmUserTrayIcon(HWND hWnd, LPARAM lParam);
	void			checkMenuItem(int id, bool checkSw /*, int dispSw*/);

	static void		setMinmumWorkingSetSize();

private:
	enum { MAX_LOADSTRING	= 100		  };
	enum { WM_USER_TRAYICON = WM_USER + 1 };

	HWND		hWnd_;
	HINSTANCE	hInstance_;						///< ���݂̃C���^�[�t�F�C�X.
	HICON		hIconSm_;
	CTrayIcon	trayIcon_;
	TCHAR		szTitle_[MAX_LOADSTRING];		///< �^�C�g�� �o�[�̃e�L�X�g.
	TCHAR		szWindowClass_[MAX_LOADSTRING];	///< ���C�� �E�B���h�E �N���X��.
	//TCHAR		szIniName_[0x4000];				///< ���W���[����
	static CDiaKbdMouseApp*	s_pSelf_;			///< �������g�̕ϐ�(�C���X�^���X)�ւ̃|�C���^
};

/// �������g�ւ̃|�C���^.
CDiaKbdMouseApp*	CDiaKbdMouseApp::s_pSelf_ = 0;


/** �R���X�g���N�^
 */
CDiaKbdMouseApp::CDiaKbdMouseApp()
	: hWnd_(0)
	, hInstance_(0)
	, hIconSm_(0)
	, trayIcon_()
{
	std::memset(szTitle_      , 0, sizeof szTitle_);
	std::memset(szWindowClass_, 0, sizeof szWindowClass_);
	assert( s_pSelf_ == NULL );
	s_pSelf_ = this;
}


/** �G���g��.
 */
int CDiaKbdMouseApp::winMain(HINSTANCE /*hInstance0*/, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	// Window�C���X�^���X�̎擾.
	HINSTANCE hInstance = ::GetModuleHandle(NULL);

	// ���\�[�X��薼�O��������擾.
	::LoadString(hInstance, IDS_APP_TITLE, szTitle_ 	 , MAX_LOADSTRING);
	::LoadString(hInstance, IDS_APP_NAME , szWindowClass_, MAX_LOADSTRING);

	// ���d�N���h�~.
	::CreateMutex(NULL, 1, szWindowClass_);
	if (::GetLastError() == ERROR_ALREADY_EXISTS) {
		DEBUGPRINTF("���d�N��\n");
		return -2;
	}

	// �E�B���h�E �N���X��o�^.
	registerClass(hInstance);

	// �A�v���P�[�V�����̏�����.
	if (initInstance(hInstance/*, nCmdShow*/) == 0) {
		DEBUGPRINTF("�N�����������Ŏ��s\n");
		return -1;
	}

	// ���C�� ���b�Z�[�W ���[�v.
	HACCEL		hAccelTable;
	hAccelTable = ::LoadAccelerators(hInstance, (LPCTSTR)IDC_ACCELTABLE);
	MSG 		msg;
	while (::GetMessage(&msg, NULL, 0, 0)) {
		if (::TranslateAccelerator(msg.hwnd, hAccelTable, &msg) == 0) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		::Sleep(33);		// �K���ɐQ�Ƃ�.
	}

	// DLL�I��
	DiaKbdMouseHook_uninstall();

	return (int) msg.wParam;
}



///  �E�B���h�E �N���X��o�^.
///
ATOM CDiaKbdMouseApp::registerClass(HINSTANCE hInstance)
{
	WNDCLASSEX		wcex;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC) wndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= ::LoadIcon(hInstance, (LPCTSTR) IDI_ICON);
	wcex.hCursor		= ::LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszMenuName	= (LPCTSTR) IDC_MENU;
	wcex.lpszClassName	= szWindowClass_;
	wcex.hIconSm		= hIconSm_ = ::LoadIcon(wcex.hInstance, (LPCTSTR) IDI_SMALL);
	return ::RegisterClassEx(&wcex);
}


///	���C�� �v���O���� �E�B���h�E���쐬.
///
bool CDiaKbdMouseApp::initInstance(HINSTANCE hInstance /*, int nCmdShow*/)
{
	hInstance_	= hInstance;
	hWnd_		= ::CreateWindowEx(
						0/*dwExStyle*/,
						szWindowClass_,
						szTitle_,
						WS_CAPTION | /*WS_SYSMENU |*/ WS_POPUP,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						0,
						0,
						NULL,
						NULL,
						hInstance,
						NULL);

	if (hWnd_ == 0)
		return false;

	::ShowWindow(hWnd_, SW_HIDE/*nCmdShow*/);
	::UpdateWindow(hWnd_);

	return true;
}


/// ���C�� �E�B���h�E�̃��b�Z�[�W������.
///
LRESULT CALLBACK CDiaKbdMouseApp::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDiaKbdMouseApp*	pSelf = s_pSelf_;
	assert( pSelf != 0 );
	switch (uMsg) {
	case WM_CREATE:
		return pSelf->wmCreate(hWnd, wParam, lParam);

	case WM_COMMAND:		// ���j���[���ڂ̏���.
		if (pSelf->wmCommand(hWnd, wParam, lParam) == 0)
			return 0;
		break;

	case WM_USER_TRAYICON:	// �g���C�A�C�R���ŃN���b�N���ꂽ�Ƃ��̏���.
		return pSelf->wmUserTrayIcon(hWnd, lParam);

	case WM_DESTROY:
		// �L�[�{�[�h�Ń}�E�X���삷�鏈���̃X���b�h���I��.
		CKbdMouseCtrl::release();
		// �g���C�A�C�R���̏I��.
		pSelf->trayIcon_.release();
		// ���b�Z�[�W���[�v���I��������.
		::PostQuitMessage(0);
		return 0;

	default:
		pSelf->trayIcon_.checkRecreate(uMsg);	// �^�X�N�o�[���č쐬���ꂽ��A�C�R�����ēo�^.
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#if 1
DWORD	CDiaKbdMouseApp::getConfigData(CDiaKbdMouseHook_ConvKeyTbl& tbl)
{
	DWORD	dwKeyCode = 0;
	TCHAR	szIniName[0x4000];
	::GetModuleFileName(NULL, szIniName, 0x4000 );
	std::size_t l = ::lstrlen(szIniName);
	if (l > 4) {
		std::memcpy( &szIniName[l-3], _T("cfg"), 4 * sizeof(TCHAR) );	// �g���q��cfg��.
		//x DEBUGPRINTF("%s\n", path);
		CConfigFileReader	configData(szIniName, tbl);
		dwKeyCode	= configData.getData();
	}
	return dwKeyCode;
}
#else
DWORD getKeyCodeFromPrivateProfile()
{
	enum { EX_KEY = VK_APPS/*VK_RWIN*/ };
	DWORD	dwKeyCode = EX_KEY;
	TCHAR	szIniName[0x4000];
	::GetModuleFileName(NULL, szIniName, 0x4000 );
	std::size_t l = ::lstrlen(szIniName);
	if (l > 4) {
		std::::memcpy( &szIniName[l-3], _T("ini"), 4 * sizeof(TCHAR) );	// �g���q��ini��.
		//x DEBUGPRINTF("%s\n", path);
		dwKeyCode = ::GetPrivateProfileInt(_T("COMMON"), _T("TRIGGER"), dwKeyCode, szIniName);
		if (dwKeyCode == 0)
			dwKeyCode = EX_KEY;
	}
	return dwKeyCode;
}
#endif


/// WM_CREATE �ōs������.
///
LRESULT CDiaKbdMouseApp::wmCreate(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// �g���C�ɃA�C�R����o�^.
	int rc = trayIcon_.create(
			WM_USER_TRAYICON,
			hInstance_,
			hWnd,
			hIconSm_,
			_T("DiaKbdMouse"),
			IDC_MENU
		);
	if (rc == 0) {		// �g���C�A�C�R���̍쐬�Ɏ��s������I��.
		::DestroyWindow(hWnd);
		return 0;
	}

  #ifdef USE_LWINKEY
	// �g���C�A�C�R�����j���[�̏�Ԃ�ݒ�
	checkMenuItem(IDM_LWIN_SW, DiaKbdMouseHook_setLWinMode(-1));
  #endif	

	// ��`�t�@�C���ǂݍ���
 #if 1
	CDiaKbdMouseHook_ConvKeyTbl	tbl;
	tbl.clear();
	DWORD	dwKeyCode = getConfigData(tbl);	// VK_RWIN;	// VK_RMENU;
	if (dwKeyCode == 0) {	// ��`�t�@�C���ŃG���[���������ꍇ.
		::DestroyWindow(hWnd);
		return 0;
	}
	DiaKbdMouseHook_install(dwKeyCode, tbl);
 #else
	DWORD	dwKeyCode = getKeyCodeFromPrivateProfile();
	// DLL���J�n
	DiaKbdMouseHook_install(dwKeyCode);
 #endif

	// �L�[�{�[�h�Ń}�E�X���삷�鏈���̃X���b�h���N��.
	CKbdMouseCtrl::create();

	// �q�[�v�������̒���
	setMinmumWorkingSetSize();
	return 0;
}


/// WM_COMMAND �ōs������.
///
LRESULT CDiaKbdMouseApp::wmCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	DWORD wmId	  = LOWORD(wParam);
	//x DWORD wmEvent = HIWORD(wParam);
	// �I�����ꂽ���j���[�̉��
	switch (wmId) {
  #ifdef USE_LWINKEY
	case IDM_LWIN_SW:
		{
			bool sw = DiaKbdMouseHook_setLWinMode(-1);
			sw = (sw == 0);
			DiaKbdMouseHook_setLWinMode(sw);

			checkMenuItem(IDM_LWIN_SW, sw);
		}
		return 0;
  #endif

	case IDM_HELP:
		{
			TCHAR path[MAX_PATH+16]; // ���s�t�@�C�������i�[����o�b�t�@
			::GetModuleFileName( NULL, path, MAX_PATH );
			std::size_t l = ::_tcslen(path);
			if (l > 4) {
				std::memcpy(&path[l-3], _T("htm"), 4*sizeof(TCHAR));	// �g���q��htm�ɂ��Ă�����J�����Ƃɂ���.
				//x DEBUGPRINTF("%s\n", path);
				::ShellExecute (hWnd, _T("open"), path, NULL, NULL, SW_SHOW);
				setMinmumWorkingSetSize();
			}
		}
		return 0;

	case IDM_ABOUT:
		::DialogBox(hInstance_, (LPCTSTR) IDD_ABOUTBOX, hWnd, (DLGPROC) dlgProcAbout);
		return 0;

	case IDM_EXIT:
		::DestroyWindow(hWnd);
		return 0;

	default:
		;
	}
	return 1;
}


/// ���j���[���ڂɃ`�F�b�N�}�[�N�����͂���
///
void CDiaKbdMouseApp::checkMenuItem(int id, bool checkSw /*, int dispSw*/)
{
	int flags		= checkSw ? MFS_CHECKED : MFS_UNCHECKED;
	// HMENU	hMenu = ::GetSubMenu(::GetMenu(hWnd_), 0);
	HMENU	hMenu = trayIcon_.getMenuHandle();
	//x int checked =
	::CheckMenuItem(hMenu, id, MF_BYCOMMAND|flags);

	//x int dispFlags	= (dispMode < 0) ? MF_GLAY : (dispMode == 0) ? MF_DISABLE : MF_ENABLE;
	//x EnableMenuItem(hMenu, id, MF_BYCOMMAND|dispFlags);
}


/// �ʒm�̈�̃A�C�R���ɑ΂��đ��삪�s��ꂽ�ꍇ.
///
LRESULT CDiaKbdMouseApp::wmUserTrayIcon(HWND hWnd, LPARAM lParam)
{
	switch (lParam) {
	case WM_RBUTTONDOWN:	// �E�N���b�N�̂Ƃ��A�|�b�v�A�b�v���j���[�őI��.
		{
			unsigned no = trayIcon_.trackPopupMenu();
			wmCommand(hWnd, no, lParam);
		}
		break;
	default:
		;
	}
	return 0;
}


/// �o�[�W�������{�b�N�X�̃��b�Z�[�W �n���h��.
///
LRESULT CALLBACK CDiaKbdMouseApp::dlgProcAbout(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	lParam;
	switch (uMsg) {
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			::EndDialog(hDlg, LOWORD(wParam));
			setMinmumWorkingSetSize();
			return TRUE;
		}
		break;

	default:
		;
	}
	return FALSE;
}


/// �q�[�v�������̒���
///
void CDiaKbdMouseApp::setMinmumWorkingSetSize()
{
	HINSTANCE hDll = ::LoadLibrary( _T( "kernel32.dll" ) );
	if ( hDll ) {
		typedef BOOL ( WINAPI *fn_t_SetProcessWorkingSetSize )( HANDLE, SIZE_T, SIZE_T );
		fn_t_SetProcessWorkingSetSize fn_SetProcessWorkingSetSize = reinterpret_cast< fn_t_SetProcessWorkingSetSize >( ::GetProcAddress( hDll, "SetProcessWorkingSetSize" ) );
		if ( fn_SetProcessWorkingSetSize ) {
			HANDLE hCurrentProcess = ::GetCurrentProcess();
		  #ifdef _WIN64
			fn_SetProcessWorkingSetSize(hCurrentProcess, 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL );
		  #else
			fn_SetProcessWorkingSetSize(hCurrentProcess, 0xFFFFFFFF, 0xFFFFFFFF );
		  #endif
		}
		::FreeLibrary( hDll );
	}
}



// ===========================================================================
/** �N���G���g��
 */
// ===========================================================================
int APIENTRY ::_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CDiaKbdMouseApp		diaCursorApp;
	return diaCursorApp.winMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
