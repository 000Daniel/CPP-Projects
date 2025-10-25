#include "MainFrame.h"
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/fileconf.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <format>
#include <random>
#include <iostream>

		//	For more information on how this code works, please visit my Github page (000Daniel),
		//	And check out the documentation provided.
		//	Please remember that this is my first exersice project with C++ and wxWidgets.
enum IDs {
	BUTTON_START_ID = 101,
	BUTTON_STOP_ID = 102,
	CB_CLICKS_ID = 103,
	CB2_CLICKS_ID = 104,
	CB3_CLICKS_ID = 105,
	BUTTON_KEYBIND_ID = 106,
	BUTTON_RESET_ID = 107,
	SPIN_X_POS_ID = 108,
	SPIN_Y_POS_ID = 109,
	BUTTON_GITHUB_ID = 110,
	HOTKEY_F6_ID = 201
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_BUTTON(BUTTON_START_ID, MainFrame::OnButtonStartClicked)
	EVT_BUTTON(BUTTON_STOP_ID, MainFrame::OnButtonStopClicked)
	EVT_BUTTON(BUTTON_KEYBIND_ID, MainFrame::OnButtonKeybindClicked)
	EVT_BUTTON(BUTTON_RESET_ID, MainFrame::OnButtonResetClicked)
	EVT_BUTTON(BUTTON_GITHUB_ID, MainFrame::OnButtonGithubClicked)
	EVT_CHECKBOX(CB_CLICKS_ID, MainFrame::OnCBClicksClicked)
	EVT_CHECKBOX(CB2_CLICKS_ID, MainFrame::OnCB2ClicksClicked)
	EVT_CHECKBOX(CB3_CLICKS_ID, MainFrame::OnCB3ClicksClicked)
	EVT_TEXT(SPIN_X_POS_ID, MainFrame::OnSpinXUpdate)
	EVT_TEXT(SPIN_Y_POS_ID, MainFrame::OnSpinYUpdate)
END_EVENT_TABLE()

wxButton* button_start;
wxButton* button_stop;
wxButton* button_bind;
wxButton* button_reset;
wxButton* button_github;
wxCheckBox* checkBox;
wxCheckBox* checkBox2;
wxCheckBox* checkBox3;
wxSpinCtrl* spinCtrl;
wxSpinCtrl* spinCtrl2;
wxSpinCtrl* spinCtrl3;
wxSpinCtrl* spinCtrl4;
wxSpinCtrl* spinCtrl4b;
wxChoice* choice;

BOOL AutoClicking = false;
BOOL ChangingBind = false;
BOOL RandomIntervals = false;
BOOL LimitNumberOfClicks = false;
BOOL LimitMousePosition = false;
POINT cursorLockPos;
int clicksLeft = 0;
int capturedKey = WXK_F6;
int msIntervals = 0;
POINT cursorPos;
const int mouseTrackIntervals = 10;
std::mt19937 gen;
int KeyBindConfigMemory = WXK_F6;

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);
	
	wxStaticText* staticText = new wxStaticText(panel, wxID_ANY, "C++ Auto Clicker Project", wxPoint(10, 10));
	button_start = new wxButton(panel, BUTTON_START_ID, "Start(F6)",		wxPoint(10, 35),	wxSize(85, 35));
	button_stop = new wxButton(panel, BUTTON_STOP_ID, "Stop(F6)", wxPoint(108, 35),	wxSize(85, 35));
	button_bind = new wxButton(panel, BUTTON_KEYBIND_ID, "Keybind",	wxPoint(205, 35),	wxSize(85, 35));
	wxStaticText* staticText3 = new wxStaticText(panel, wxID_ANY, "Click Intervals (ms)", wxPoint(10,82));
	spinCtrl = new wxSpinCtrl(panel, wxID_ANY, "100", wxPoint(155, 80), wxSize(50,-1));
	checkBox = new wxCheckBox(panel, CB_CLICKS_ID, "Random Intervals (ms)", wxPoint(10, 116));
	spinCtrl2 = new wxSpinCtrl(panel, wxID_ANY, "10", wxPoint(155, 114), wxSize(45, -1));
	checkBox2 = new wxCheckBox(panel, CB2_CLICKS_ID, "Limit Clicks", wxPoint(10, 150));
	spinCtrl3 = new wxSpinCtrl(panel, wxID_ANY, "10", wxPoint(150, 148), wxSize(45, -1));
	
	spinCtrl4 = new wxSpinCtrl(panel, SPIN_X_POS_ID, "0", wxPoint(150, 182), wxSize(55, -1));
	spinCtrl4b = new wxSpinCtrl(panel, SPIN_Y_POS_ID, "0", wxPoint(230, 182), wxSize(55, -1));
	checkBox3 = new wxCheckBox(panel, CB3_CLICKS_ID, "Lock Click Position  X:                       Y:", wxPoint(10, 184));
	
	wxStaticText* staticText2 = new wxStaticText(panel, wxID_ANY, "Simulate:", wxPoint(10, 218));

	wxArrayString choices_array;
	choices_array.Add("Left mouse");
	choices_array.Add("Right mouse");
	choices_array.Add("Middle mouse");
	choice = new wxChoice(panel, wxID_ANY, wxPoint(66, 216), wxSize(100, -1), choices_array);
	choice->Select(0);

	wxStaticText* staticText4 = new wxStaticText(panel, wxID_ANY, "000DANIEL'S", wxPoint(10, 270));
	wxStaticText* staticText5 = new wxStaticText(panel, wxID_ANY, "AUTO CLICKER", wxPoint(35, 320));

	button_reset = new wxButton(panel, BUTTON_RESET_ID, "Reset to defaults", wxPoint(10, 400), wxSize(100, 35));
	button_github = new wxButton(panel, BUTTON_GITHUB_ID, "Github page", wxPoint(190, 400), wxSize(100, 35));

	spinCtrl->SetMax(9999);
	spinCtrl->SetMin(1);
	spinCtrl2->SetMax(999);
	spinCtrl2->Disable();
	spinCtrl3->SetMax(9999);
	spinCtrl3->SetMin(1);
	spinCtrl3->Disable();
	spinCtrl4->SetMax(99999);
	spinCtrl4->SetMin(-99999);
	spinCtrl4->Disable();
	spinCtrl4b->SetMax(99999);
	spinCtrl4b->SetMin(-99999);
	spinCtrl4b->Disable();
	button_bind->Bind(wxEVT_KEY_UP, &MainFrame::PressedBindKey, this);

	gen.seed(std::random_device{}());

	wxFont font = staticText4->GetFont();
	font.SetPointSize(28);
	font.MakeBold();
	staticText4->SetFont(font);
	staticText5->SetFont(font);
	staticText4->SetForegroundColour(wxColour(200,200,200));
	staticText5->SetForegroundColour(wxColour(200, 200, 200));
	staticText4->Refresh();
	staticText5->Refresh();

	if (!RegisterHotKey(HOTKEY_F6_ID, wxMOD_NONE, WXK_F6))
	{
		wxLogMessage("The program could not register the F6 bind!\nPlease restart the application.");
	}

	Bind(wxEVT_HOTKEY, &MainFrame::PressedF6,this,HOTKEY_F6_ID);
	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

	MainFrame::LoadPrefs();
}

void MainFrame::OnCBClicksClicked(wxCommandEvent& evt)
{
	if (checkBox->GetValue())
	{
		spinCtrl2->Enable();
		RandomIntervals = true;
	}
	else
	{
		spinCtrl2->Disable();
		RandomIntervals = false;
	}
}
void MainFrame::OnCB2ClicksClicked(wxCommandEvent& evt)
{
	if (checkBox2->GetValue())
	{
		spinCtrl3->Enable();
		LimitNumberOfClicks = true;
	}
	else
	{
		spinCtrl3->Disable();
		LimitNumberOfClicks = false;
	}
}
void MainFrame::OnCB3ClicksClicked(wxCommandEvent& evt)
{
	if (checkBox3->GetValue())
	{
		spinCtrl4->Enable();
		spinCtrl4b->Enable();
		LimitMousePosition = true;
	}
	else
	{
		spinCtrl4->Disable();
		spinCtrl4b->Disable();
		LimitMousePosition = false;
	}
}
void MainFrame::OnButtonStartClicked(wxCommandEvent& evt)
{
	button_start->Disable();
	AutoClicking = true;
	MainFrame::StartLoop();
}
void MainFrame::OnButtonStopClicked(wxCommandEvent& evt)
{
	button_start->Enable();
	AutoClicking = false;
	MainFrame::StopLoop();
}
void MainFrame::PressedF6(wxKeyEvent& event)
{
	if (ChangingBind) return;

	AutoClicking = !AutoClicking;

	if (AutoClicking)
	{
		MainFrame::StartLoop();
	}
	else
	{
		MainFrame::StopLoop();
	}
}

void MainFrame::PressedBindKey(wxKeyEvent& event)
{
	if (!ChangingBind) return;

	ChangingBind = false;
	button_bind->SetLabelText("Keybind");

	int capturedKey_temp = event.GetKeyCode();
	
	if (capturedKey_temp == WXK_LBUTTON || capturedKey_temp == WXK_RBUTTON || capturedKey_temp == WXK_MBUTTON || capturedKey_temp == WXK_ESCAPE || capturedKey_temp == WXK_RETURN || capturedKey_temp == WXK_BACK)
	{
		return;
	}
	
	capturedKey = capturedKey_temp;
	KeyBindConfigMemory = capturedKey;

	wxAcceleratorEntry entry;
	entry.Set(wxACCEL_NORMAL, capturedKey, wxID_ANY);
	
	button_start->SetLabelText(wxString::Format("Start(%s)", entry.ToString()));
	button_stop->SetLabelText(wxString::Format("Stop(%s)", entry.ToString()));

	UnregisterHotKey(HOTKEY_F6_ID);
	if (!RegisterHotKey(HOTKEY_F6_ID, wxMOD_NONE, capturedKey))
	{
		wxLogMessage("The program could not register a keybind!\nPlease restart the application.");
	}
}

std::thread loopThread;
std::atomic<bool> loopRunning;

void MainFrame::StartLoop()
{
	if (loopRunning)
	{
		loopRunning = false;
		if (loopThread.joinable())
			loopThread.join();
	}

	button_start->Disable();
	msIntervals = spinCtrl->GetValue();
	spinCtrl->Disable();

	if (LimitNumberOfClicks)
	{
		clicksLeft = spinCtrl3->GetValue();
	}

	loopRunning = true;

	loopThread = std::thread([this]() {

		std::uniform_int_distribution<> dist(0, spinCtrl2->GetValue());
		int randomInterval = 0;

		while (loopRunning)
		{
			MainFrame::MouseClick();

			if (RandomIntervals)
				randomInterval = dist(gen);
			if (LimitNumberOfClicks)
			{
				if (clicksLeft <= 1)
				{
					button_start->Enable();
					spinCtrl->Enable();
					AutoClicking = false;
					break;
				}
				clicksLeft--;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(msIntervals + randomInterval));
		}
	});
}

void MainFrame::StopLoop()
{
	if (loopRunning)
	{
		button_start->Enable();
		spinCtrl->Enable();

		loopRunning = false;
		if (loopThread.joinable())
			loopThread.join();
	}
}

void MainFrame::StartMouseTrackingLoop()
{
	loopThread = std::thread([this]() {
		while (true)
		{
			GetCursorPos(&cursorPos);

			std::this_thread::sleep_for(std::chrono::milliseconds(mouseTrackIntervals));
		}
		});
}

void MainFrame::MouseClick()
{
	int evt_down;
	int evt_up;
	switch (choice->GetSelection())
	{
	case (0):
		evt_down = MOUSEEVENTF_LEFTDOWN;
		evt_up = MOUSEEVENTF_LEFTUP;
		break;
	case (1):
		evt_down = MOUSEEVENTF_RIGHTDOWN;
		evt_up = MOUSEEVENTF_RIGHTUP;
		break;
	case (2):
		evt_down = MOUSEEVENTF_MIDDLEDOWN;
		evt_up = MOUSEEVENTF_MIDDLEUP;
		break;
	default:
		evt_down = MOUSEEVENTF_LEFTDOWN;
		evt_up = MOUSEEVENTF_LEFTUP;
		break;
	}
	

	if (LimitMousePosition)
	{
		GetCursorPos(&cursorPos);
		SetCursorPos(cursorLockPos.x, cursorLockPos.y);
		mouse_event(evt_down, cursorLockPos.x, cursorLockPos.y, 0, 0);
		mouse_event(evt_up, cursorLockPos.x, cursorLockPos.y, 0, 0);
		SetCursorPos(cursorPos.x, cursorPos.y);
	}
	else
	{
		mouse_event(evt_down, cursorPos.x, cursorPos.y, 0, 0);
		mouse_event(evt_up, cursorPos.x, cursorPos.y, 0, 0);
	}
}

void MainFrame::OnButtonKeybindClicked(wxCommandEvent& evt)
{
	button_bind->SetLabelText("...");
	ChangingBind = true;
}
void MainFrame::OnButtonResetClicked(wxCommandEvent& evt)
{
	MainFrame::StopLoop();
	
	UnregisterHotKey(HOTKEY_F6_ID);
	if (HOTKEY_F6_ID != WXK_F6 && !RegisterHotKey(HOTKEY_F6_ID, wxMOD_NONE, WXK_F6))
	{
		wxLogMessage("The program could not register the F6 bind!\nPlease restart the application.");
	}
	button_start->SetLabelText(wxString::Format("Start(F6)"));
	button_stop->SetLabelText(wxString::Format("Stop(F6)"));
	ChangingBind = false;
	KeyBindConfigMemory = WXK_F6;

	spinCtrl->SetValue(100);
	checkBox->SetValue(0);
	checkBox2->SetValue(0);
	checkBox3->SetValue(0);
	spinCtrl->SetValue(100);
	spinCtrl2->SetValue(10);
	spinCtrl3->SetValue(10);
	spinCtrl4->SetValue(0);
	spinCtrl4b->SetValue(0);

	choice->Select(0);

	RandomIntervals = false;
	LimitNumberOfClicks = false;
	LimitMousePosition = false;
	cursorLockPos.x = 0;
	cursorLockPos.y = 0;
}
void MainFrame::OnButtonGithubClicked(wxCommandEvent& evt)
{
	if (!wxLaunchDefaultBrowser("https://github.com/000Daniel/CPP-Projects/tree/main/AutoClicker"))
	{
		wxLogError("Couldn't open URL, please visit this URL: https://github.com/000Daniel/CPP-Projects/tree/main/AutoClicker");
	}
}

void MainFrame::OnSpinXUpdate(wxCommandEvent& evt)
{
	cursorLockPos.x = spinCtrl4->GetValue();
}
void MainFrame::OnSpinYUpdate(wxCommandEvent& evt)
{
	cursorLockPos.y = spinCtrl4b->GetValue();
}

void MainFrame::OnClose(wxCloseEvent& evt)
{
	loopRunning = false;
	if (loopThread.joinable())
		loopThread.join();

	MainFrame::SavePrefs();
	evt.Skip();
}

void MainFrame::LoadPrefs()
{
	wxConfigBase* config = wxConfigBase::Get();

	bool cbValue = false;
	if (config->Read("checkBox", &cbValue))
	{
		checkBox->SetValue(cbValue);
		if (cbValue)
		{
			spinCtrl2->Enable();
			RandomIntervals = true;
		}
		else
		{
			spinCtrl2->Disable();
			RandomIntervals = false;
		}
	}
	if (config->Read("checkBox2", &cbValue))
	{
		checkBox2->SetValue(cbValue);
		if (cbValue)
		{
			spinCtrl3->Enable();
			LimitNumberOfClicks = true;
		}
		else
		{
			spinCtrl3->Disable();
			LimitNumberOfClicks = false;
		}
	}
	if (config->Read("checkBox3", &cbValue))
	{
		checkBox3->SetValue(cbValue);
		if (cbValue)
		{
			spinCtrl4->Enable();
			spinCtrl4b->Enable();
			LimitMousePosition = true;
		}
		else
		{
			spinCtrl4->Disable();
			spinCtrl4b->Disable();
			LimitMousePosition = false;
		}
	}

	long numValue = 0;
	if (config->Read("spinCtrl", &numValue))
		spinCtrl->SetValue(numValue);
	if (config->Read("spinCtrl2", &numValue))
		spinCtrl2->SetValue(numValue);
	if (config->Read("spinCtrl3", &numValue))
		spinCtrl3->SetValue(numValue);
	if (config->Read("spinCtrl4", &numValue))
	{
		spinCtrl4->SetValue(numValue);
		cursorLockPos.x = numValue;
	}
	if (config->Read("spinCtrl4b", &numValue))
	{
		spinCtrl4b->SetValue(numValue);
		cursorLockPos.y = numValue;
	}
	if (config->Read("choice", &numValue))
		choice->Select(numValue);

	if (config->Read("keybind", &KeyBindConfigMemory))
	{
		wxAcceleratorEntry entry;
		entry.Set(wxACCEL_NORMAL, KeyBindConfigMemory, wxID_ANY);

		button_start->SetLabelText(wxString::Format("Start(%s)", entry.ToString()));
		button_stop->SetLabelText(wxString::Format("Stop(%s)", entry.ToString()));

		UnregisterHotKey(HOTKEY_F6_ID);
		if (!RegisterHotKey(HOTKEY_F6_ID, wxMOD_NONE, KeyBindConfigMemory))
		{
			wxLogMessage("The program could not register a keybind!\nPlease restart the application.");
		}
	}
}
void MainFrame::SavePrefs()
{
	wxConfigBase* config = wxConfigBase::Get();

	config->Write("checkBox", checkBox->GetValue());
	config->Write("checkBox2", checkBox2->GetValue());
	config->Write("checkBox3", checkBox3->GetValue());
	config->Write("spinCtrl", spinCtrl->GetValue());
	config->Write("spinCtrl2", spinCtrl2->GetValue());
	config->Write("spinCtrl3", spinCtrl3->GetValue());
	config->Write("spinCtrl4", spinCtrl4->GetValue());
	config->Write("spinCtrl4b", spinCtrl4b->GetValue());
	config->Write("choice", choice->GetSelection());
	config->Write("keybind", KeyBindConfigMemory);

	config->Flush();
}