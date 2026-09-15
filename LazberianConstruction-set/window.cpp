#include <iostream>
#include <wx/image.h>
#include "window.h"
#include "windowComponents.h"

bool App::OnInit() {
	wxInitAllImageHandlers();
	MainFrame* frame = new MainFrame();
	frame->SetIcon(wxIcon("appicon"));
	frame->Maximize(true);
	frame->Show();
	return true;
}

wxIMPLEMENT_APP(App);