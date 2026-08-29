#include <iostream>
#include "window.h"
#include "windowComponents.h"

bool App::OnInit() {
	MainFrame* frame = new MainFrame();
	frame->Show();
	std::cout << "Main frame initialized." << std::endl;
	return true;
}

wxIMPLEMENT_APP(App);