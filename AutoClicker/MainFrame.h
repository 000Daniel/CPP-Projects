#pragma once
#include <wx/wx.h>
#include <wx/fileconf.h>

extern wxFileConfig* config1;
extern wxFileConfig* config2;
extern wxFileConfig* config3;

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
	void HandleHotkeyDown();
	void HandleHotkeyUp();
	void SetKeyBind(int key);
private:
	void OnButtonStartClicked(wxCommandEvent& evt);
	void OnButtonStopClicked(wxCommandEvent& evt);
	void OnButtonKeybindClicked(wxCommandEvent& evt);
	void OnButtonResetClicked(wxCommandEvent& evt);
	void OnButtonGithubClicked(wxCommandEvent& evt);
	void OnButtonSaveClicked(wxCommandEvent& evt);
	void OnCBClicksClicked(wxCommandEvent& evt);
	void OnCB2ClicksClicked(wxCommandEvent& evt);
	void OnCB3ClicksClicked(wxCommandEvent& evt);
	void OnSpinXUpdate(wxCommandEvent& evt);
	void OnSpinYUpdate(wxCommandEvent& evt);
	void PressedBindKey(wxKeyEvent& event);
	void StartLoop();
	void StopLoop();
	void StartMouseTrackingLoop();
	void MouseClick();
	void OnClose(wxCloseEvent& evt);
	void LoadPrefs();
	void SavePrefs();
	void OnPresetChanged(wxCommandEvent& event);
	int WxKeyToVK(int key);
	wxDECLARE_EVENT_TABLE();
};

