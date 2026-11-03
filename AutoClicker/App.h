#pragma once
#include <wx/wx.h>
#include <wx/fileconf.h>
#include "MainFrame.h"

class App : public wxApp
{
public:
	bool OnInit();
	void IconManager();
	void ConfigManager();
};

