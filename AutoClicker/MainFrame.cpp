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
#include <condition_variable>
#include <mutex>

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
	BUTTON_SAVE_ID = 111,
	HOTKEY_F6_ID = 201
};

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_BUTTON(BUTTON_START_ID, MainFrame::OnButtonStartClicked)
	EVT_BUTTON(BUTTON_STOP_ID, MainFrame::OnButtonStopClicked)
	EVT_BUTTON(BUTTON_KEYBIND_ID, MainFrame::OnButtonKeybindClicked)
	EVT_BUTTON(BUTTON_RESET_ID, MainFrame::OnButtonResetClicked)
	EVT_BUTTON(BUTTON_GITHUB_ID, MainFrame::OnButtonGithubClicked)
	EVT_BUTTON(BUTTON_SAVE_ID, MainFrame::OnButtonSaveClicked)
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
wxButton* button_save;
wxCheckBox* checkBox;
wxCheckBox* checkBox2;
wxCheckBox* checkBox3;
wxSpinCtrl* spinCtrl;
wxSpinCtrl* spinCtrl2;
wxSpinCtrl* spinCtrl3;
wxSpinCtrl* spinCtrl4;
wxSpinCtrl* spinCtrl4b;
wxChoice* choice;
wxChoice* choicePreset;
wxChoice* choiceSimulationMode;

BOOL AutoClicking = false;
BOOL ChangingBind = false;
BOOL LetGoMouse = true;
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
int currectConfig = -1;

wxFileConfig* config1 = nullptr;
wxFileConfig* config2 = nullptr;
wxFileConfig* config3 = nullptr;
wxFileConfig& GetProfileConfig(int preset)
{
	switch (preset)
	{
	case 1: return *config2;
	case 2: return *config3;
	default: return *config1;
	}
}

		//	This function converts wxKeys to virtual keys (what windows uses).
		//	It converts, F1-F24 keys, Numpad numbers and Numpad operators.
		//	Regular Numbers and letters are left the same.
int WxKeyToVK(int key)
{
	if ((key >= 'A' && key <= 'Z') ||
		(key >= '0' && key <= '9'))
		return key;

	if (key >= WXK_F1 && key <= WXK_F24)
		return VK_F1 + (key - WXK_F1);

	if (key >= WXK_NUMPAD0 && key <= WXK_NUMPAD9)
		return VK_NUMPAD0 + (key - WXK_NUMPAD0);

	switch (key)
	{
	case WXK_NUMPAD_MULTIPLY: return VK_MULTIPLY;
	case WXK_NUMPAD_ADD:      return VK_ADD;
	case WXK_NUMPAD_SEPARATOR:return VK_SEPARATOR;
	case WXK_NUMPAD_SUBTRACT: return VK_SUBTRACT;
	case WXK_NUMPAD_DECIMAL:  return VK_DECIMAL;
	case WXK_NUMPAD_DIVIDE:   return VK_DIVIDE;
	}

	return key;
}

		//	'KeyboardHook' and 'MouseHook' receive inputs and notify the appropriate functions to handle those inputs.
HHOOK keyboardHook = nullptr;
HHOOK mouseHook = nullptr;
MainFrame* mainFrameInstance = nullptr;
bool hotkeyIsDown = false;
LRESULT CALLBACK KeyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		auto* key = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		
		if (key->vkCode == WxKeyToVK(capturedKey))
		{
			if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && !hotkeyIsDown)
			{
				hotkeyIsDown = true;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->HandleHotkeyDown();
					});
			}
			else if ((wParam == WM_KEYUP || wParam == WM_SYSKEYUP) && hotkeyIsDown)
			{
				hotkeyIsDown = false;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->HandleHotkeyUp();
					});
			}
		}
	}

	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}
LRESULT CALLBACK MouseHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		MSLLHOOKSTRUCT* mouse =
			reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);

		if (wParam == WM_XBUTTONDOWN)
		{
			if (HIWORD(mouse->mouseData) == XBUTTON1 && capturedKey == XBUTTON1)
			{
				hotkeyIsDown = true;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->HandleHotkeyDown();
					});
			}
			else if (HIWORD(mouse->mouseData) == XBUTTON2 && capturedKey == XBUTTON2)
			{
				hotkeyIsDown = true;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->HandleHotkeyDown();
					});
			}
		}
		else if (wParam == WM_XBUTTONUP)
		{
			if (ChangingBind)
			{
				capturedKey = HIWORD(mouse->mouseData) == XBUTTON1
					? XBUTTON1
					: XBUTTON2;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->SetKeyBind(capturedKey);
					});
			}
			if (HIWORD(mouse->mouseData) == XBUTTON1 && capturedKey == XBUTTON1)
			{
				hotkeyIsDown = false;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->HandleHotkeyUp();
					});
			}
			else if (HIWORD(mouse->mouseData) == XBUTTON2 && capturedKey == XBUTTON2)
			{
				hotkeyIsDown = false;
				mainFrameInstance->CallAfter([]()
					{
						mainFrameInstance->HandleHotkeyUp();
					});
			}
		}
	}
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);
	
	wxStaticText* staticText = new wxStaticText(panel, wxID_ANY, "C++ Auto Clicker Project", wxPoint(10, 10));
	wxStaticText* staticText6 = new wxStaticText(panel, wxID_ANY, "Preset:", wxPoint(10, 34));
	wxArrayString choices_array;
	choices_array.Add("Preset 1");
	choices_array.Add("Preset 2");
	choices_array.Add("Preset 3");
	choicePreset = new wxChoice(panel, wxID_ANY, wxPoint(50, 31), wxSize(100, -1), choices_array);
	choicePreset->Select(0);
	choicePreset->Bind(wxEVT_CHOICE, &MainFrame::OnPresetChanged,this);
	button_save = new wxButton(panel, BUTTON_SAVE_ID, "Save Preset", wxPoint(160, 30), wxSize(120, 25));

	button_start = new wxButton(panel, BUTTON_START_ID, "Start(F6)", wxPoint(10, 60), wxSize(85, 35));
	button_stop = new wxButton(panel, BUTTON_STOP_ID, "Stop(F6)", wxPoint(108, 60),	wxSize(85, 35));
	button_bind = new wxButton(panel, BUTTON_KEYBIND_ID, "Keybind",	wxPoint(205, 60), wxSize(85, 35));
	wxStaticText* staticText3 = new wxStaticText(panel, wxID_ANY, "Click Intervals (ms)", wxPoint(10,105));
	spinCtrl = new wxSpinCtrl(panel, wxID_ANY, "100", wxPoint(155, 101), wxSize(50,-1));
	checkBox = new wxCheckBox(panel, CB_CLICKS_ID, "Random Intervals (ms)", wxPoint(10, 135));
	spinCtrl2 = new wxSpinCtrl(panel, wxID_ANY, "10", wxPoint(155, 131), wxSize(50, -1));
	checkBox2 = new wxCheckBox(panel, CB2_CLICKS_ID, "Limit Clicks", wxPoint(10, 163));
	spinCtrl3 = new wxSpinCtrl(panel, wxID_ANY, "10", wxPoint(155, 161), wxSize(50, -1));
	
	spinCtrl4 = new wxSpinCtrl(panel, SPIN_X_POS_ID, "0", wxPoint(150, 191), wxSize(55, -1));
	spinCtrl4b = new wxSpinCtrl(panel, SPIN_Y_POS_ID, "0", wxPoint(230, 191), wxSize(55, -1));
	checkBox3 = new wxCheckBox(panel, CB3_CLICKS_ID, "Lock Click Position  X:                       Y:", wxPoint(10, 193));
	
	
	choices_array.clear();
	choices_array.Add("Left mouse");
	choices_array.Add("Right mouse");
	choices_array.Add("Middle mouse");
	choice = new wxChoice(panel, wxID_ANY, wxPoint(66, 221), wxSize(100, -1), choices_array);
	choice->Select(0);
	choices_array.clear();
	choices_array.Add("Toggle");
	choices_array.Add("Hold");
	choiceSimulationMode = new wxChoice(panel, wxID_ANY, wxPoint(214, 221), wxSize(76, -1), choices_array);
	choiceSimulationMode->Select(0);
	wxStaticText* staticText2 = new wxStaticText(panel, wxID_ANY, "Simulate:                                      Mode:", wxPoint(10, 224));

	wxStaticText* staticText4 = new wxStaticText(panel, wxID_ANY, "000DANIEL'S", wxPoint(10, 266));
	wxStaticText* staticText5 = new wxStaticText(panel, wxID_ANY, "AUTO CLICKER", wxPoint(35, 316));

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

	Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnClose, this);

	MainFrame::LoadPrefs();

	mainFrameInstance = this;

	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,KeyboardHook,GetModuleHandle(nullptr),0);
	mouseHook = SetWindowsHookEx(WH_MOUSE_LL,MouseHook,GetModuleHandle(nullptr),0);
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
void MainFrame::PressedBindKey(wxKeyEvent& event)
{
	int capturedKey_temp = event.GetKeyCode();
	
	if (capturedKey_temp == WXK_LBUTTON || capturedKey_temp == WXK_RBUTTON || capturedKey_temp == WXK_MBUTTON || capturedKey_temp == WXK_ESCAPE || capturedKey_temp == WXK_RETURN || capturedKey_temp == WXK_BACK)
	{
		return;
	}
	SetKeyBind(capturedKey_temp);
}
void MainFrame::SetKeyBind(int key)
{
	if (!ChangingBind) return;

	ChangingBind = false;
	capturedKey = key;
	KeyBindConfigMemory = key;

	button_bind->SetLabelText("Keybind");

	wxString KeybindName = "";
	if (capturedKey == XBUTTON1)
	{
		KeybindName = "Mouse4";
	}
	else if (capturedKey == XBUTTON2)
	{
		KeybindName = "Mouse5";
	}
	else
	{
		wxAcceleratorEntry entry;
		entry.Set(wxACCEL_NORMAL, capturedKey, wxID_ANY);
		KeybindName = entry.ToString();
	}

	button_start->SetLabelText(wxString::Format("Start(%s)", KeybindName));
	button_stop->SetLabelText(wxString::Format("Stop(%s)", KeybindName));

	hotkeyIsDown = false;
}

std::thread loopThread;
std::atomic<bool> loopRunning;
std::mutex loopMutex;
std::condition_variable loopCV;

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

			std::unique_lock<std::mutex> lock(loopMutex);
			loopCV.wait_for(lock,std::chrono::milliseconds(msIntervals + randomInterval),[]{
					return !loopRunning.load();
				});
		}
	});
}

void MainFrame::StopLoop()
{
	if (loopRunning)
	{
		loopRunning = false;
		loopCV.notify_all();

		wxTheApp->CallAfter([this]()
			{
				button_start->Enable();
				spinCtrl->Enable();
				AutoClicking = false;
				if (loopThread.joinable())
					loopThread.join();
			});
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
	MainFrame::StopLoop();
	button_bind->SetLabelText("...");
	ChangingBind = true;
}
void MainFrame::OnButtonResetClicked(wxCommandEvent& evt)
{
	MainFrame::StopLoop();
	
	capturedKey = WXK_F6;
	hotkeyIsDown = false;

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
	choiceSimulationMode->Select(0);

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
		wxLogError("Couldn't open the URL, please visit this URL: https://github.com/000Daniel/CPP-Projects/tree/main/AutoClicker");
	}
}
void MainFrame::OnButtonSaveClicked(wxCommandEvent& evt)
{
	SavePrefs();
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

	if (keyboardHook)
	{
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = nullptr;
	}
	if (mouseHook)
	{
		UnhookWindowsHookEx(mouseHook);
		mouseHook = nullptr;
	}

	mainFrameInstance = nullptr;
	evt.Skip();
}

void MainFrame::LoadPrefs()
{
			//	This loads the settings config file, and then it loads the appropriate preset config file.
	wxConfigBase* settings = wxConfigBase::Get();

	long preset = 0;
	settings->Read("SelectedPreset",&preset,0);

	if (preset < 0 || preset > 2)
		preset = 0;
	if (currectConfig >= 0)
		preset = currectConfig;
	else
		currectConfig = preset;

	choicePreset->SetSelection(preset);
	wxFileConfig& config = GetProfileConfig(preset);

	bool cbValue = false;
	if (config.Read("checkBox", &cbValue))
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
	if (config.Read("checkBox2", &cbValue))
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
	if (config.Read("checkBox3", &cbValue))
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
	if (config.Read("spinCtrl", &numValue))
		spinCtrl->SetValue(numValue);
	if (config.Read("spinCtrl2", &numValue))
		spinCtrl2->SetValue(numValue);
	if (config.Read("spinCtrl3", &numValue))
		spinCtrl3->SetValue(numValue);
	if (config.Read("spinCtrl4", &numValue))
	{
		spinCtrl4->SetValue(numValue);
		cursorLockPos.x = numValue;
	}
	if (config.Read("spinCtrl4b", &numValue))
	{
		spinCtrl4b->SetValue(numValue);
		cursorLockPos.y = numValue;
	}
	if (config.Read("choice", &numValue))
		choice->Select(numValue);
	if (config.Read("choiceMode", &numValue))
		choiceSimulationMode->Select(numValue);

	if (config.Read("keybind", &KeyBindConfigMemory))
	{
		ChangingBind = true;
		mainFrameInstance->SetKeyBind(KeyBindConfigMemory);
	}
}
void MainFrame::SavePrefs()
{
	wxConfigBase* settings = wxConfigBase::Get();
	int preset = currectConfig;
	settings->Write("SelectedPreset", preset);
	settings->Flush();
	wxFileConfig& config = GetProfileConfig(preset);

	config.Write("configPreset", currectConfig);
	config.Write("checkBox", checkBox->GetValue());
	config.Write("checkBox2", checkBox2->GetValue());
	config.Write("checkBox3", checkBox3->GetValue());
	config.Write("spinCtrl", spinCtrl->GetValue());
	config.Write("spinCtrl2", spinCtrl2->GetValue());
	config.Write("spinCtrl3", spinCtrl3->GetValue());
	config.Write("spinCtrl4", spinCtrl4->GetValue());
	config.Write("spinCtrl4b", spinCtrl4b->GetValue());
	config.Write("choice", choice->GetSelection());
	config.Write("choiceMode", choiceSimulationMode->GetSelection());
	config.Write("keybind", KeyBindConfigMemory);

	config.Flush();
}
void MainFrame::OnPresetChanged(wxCommandEvent& event)
{
	SavePrefs();
	currectConfig = choicePreset->GetSelection();
	StopLoop();
	LoadPrefs();
}

void MainFrame::HandleHotkeyDown()
{
	if (ChangingBind)
		return;

	if (choiceSimulationMode->GetSelection() == 0)
	{
		AutoClicking = !AutoClicking;
		if (AutoClicking)
		{
			MainFrame::StartLoop();
		}
		else
		{
			MainFrame::StopLoop();
		}
		return;
	}
	else
	{
		if (AutoClicking) return;
		AutoClicking = true;
		StartLoop();
	}
}

void MainFrame::HandleHotkeyUp()
{
	if (ChangingBind || !AutoClicking || choiceSimulationMode->GetSelection() == 0)
		return;

	AutoClicking = false;
	StopLoop();
}