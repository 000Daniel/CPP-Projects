#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/fileconf.h>
#include "resource.h"
#include "Windows.h"

wxIMPLEMENT_APP(App);

MainFrame* mainFrame;

bool App::OnInit()
{
	wxFileConfig* config = new wxFileConfig("AutoclickerPrefs");
	wxConfigBase::Set(config);

	mainFrame = new MainFrame("Auto Clicker");
	mainFrame->SetClientSize(300,445);
	mainFrame->Center();
	mainFrame->Show();
	mainFrame->SetWindowStyle(wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxSYSTEM_MENU);

	App::IconManager();

	return true;
}

void App::IconManager()
{
		//	I used 'SetIcon' but the caption icon wouldn't change, so I also used Win32's API to
		//	change ICON_SMALL.
	mainFrame->SetIcon(wxICON(AppIconLarge));

	HWND hwnd = (HWND)mainFrame->GetHWND();

	HICON hIconSmall = (HICON)LoadImage(
		GetModuleHandle(NULL), MAKEINTRESOURCE(AppIconSmall), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
}

