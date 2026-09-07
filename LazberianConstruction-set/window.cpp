#include <iostream>
#include "window.h"
#include "windowComponents.h"

bool App::OnInit() {
	MainFrame* frame = new MainFrame();
	frame->Show();
	return true;
}

wxIMPLEMENT_APP(App);