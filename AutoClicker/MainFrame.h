#pragma once
#include <wx/wx.h>

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
private:
	void OnButtonStartClicked(wxCommandEvent& evt);
	void OnButtonStopClicked(wxCommandEvent& evt);
	void OnButtonKeybindClicked(wxCommandEvent& evt);
	void OnButtonResetClicked(wxCommandEvent& evt);
	void OnCBClicksClicked(wxCommandEvent& evt);
	void OnCB2ClicksClicked(wxCommandEvent& evt);
	void OnCB3ClicksClicked(wxCommandEvent& evt);
	void OnSpinXUpdate(wxCommandEvent& evt);
	void OnSpinYUpdate(wxCommandEvent& evt);
	void PressedF6(wxKeyEvent& event);
	void PressedBindKey(wxKeyEvent& event);
	void OnButtonGithubClicked(wxCommandEvent& evt);
	void StartLoop();
	void StopLoop();
	void StartMouseTrackingLoop();
	void MouseClick();
	void OnClose(wxCloseEvent& evt);
	void LoadPrefs();
	void SavePrefs();
	wxDECLARE_EVENT_TABLE();
};

