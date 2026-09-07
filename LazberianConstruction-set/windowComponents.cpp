#include "windowComponents.h"
#include "DebugConsole.h"
#include <iostream>

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "Lazberian Construction Set - UnitData & Items", wxDefaultPosition, wxSize(1000, 650)) {
	CreateMenuBar();
	CreateStatusBarInfo();
	CreateLayout();
	CreateAcelerators();

	Bind(wxEVT_MENU, &MainFrame::OnOpenFile, this, ID_OpenFile);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnToggleConsole, this, ID_ToggleConsole);

	Center();
}

void MainFrame::CreateMenuBar() {
	wxMenu* menuFile = new wxMenu();
	menuFile->Append(ID_OpenFile, "&Open\tCtrl+O", "Open a file");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuView = new wxMenu();
	menuView->Append(ID_ToggleConsole, "Toggle Debug &Console\tF7", "Show or hide debug console");

	wxMenu* menuHelp = new wxMenu();
	menuHelp->Append(wxID_ABOUT, "&About\tF1", "About this editor");

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuView, "&View");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
}

void MainFrame::CreateStatusBarInfo() {
	CreateStatusBar(2);
	SetStatusText("File status");
	SetStatusText("No data loaded", 1);
}

void MainFrame::CreateAcelerators() {
	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_NORMAL, WXK_F7, ID_ToggleConsole);
	wxAcceleratorTable accel(1, entries);
	SetAcceleratorTable(accel);
}

void MainFrame::CreateLayout() {
	m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);
	m_splitter->SetMinimumPaneSize(180);
	m_tree = CreateNavigationTree(m_splitter);
	m_notebook = CreateEditorNotebook(m_splitter);
	m_splitter->SplitVertically(m_tree, m_notebook, 250);
	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	rootSizer->Add(m_splitter, 1, wxEXPAND);
	SetSizer(rootSizer);
}

wxTreeCtrl* MainFrame::CreateNavigationTree(wxWindow* parent) {
	wxTreeCtrl* tree = new wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT);

	wxTreeItemId root = tree->AddRoot("root");
	tree->AppendItem(root, "No file loaded");

	return tree;
}

wxNotebook* MainFrame::CreateEditorNotebook(wxWindow* parent) {
	wxNotebook* notebook = new wxNotebook(parent, wxID_ANY);

	m_unitDataPanel = CreateUnitDataPanel(notebook);
	m_itemsDataPanel = CreateItemsDataPanel(notebook);
	m_classesDataPanel = CreateClassesDataPanel(notebook);


	notebook->AddPage(m_unitDataPanel, "Unit editor");
	notebook->AddPage(m_itemsDataPanel, "Items editor");
	notebook->AddPage(m_classesDataPanel, "Classes editor");

	return notebook;
}

wxPanel* MainFrame::CreateUnitDataPanel(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* placeholder = new wxStaticText(panel, wxID_ANY,
		"Unit Data Editor (WIP)", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	sizer->AddStretchSpacer();
	sizer->Add(placeholder, 0, wxALIGN_CENTER);
	sizer->AddStretchSpacer();
	panel->SetSizer(sizer);

	return panel;
}

wxPanel* MainFrame::CreateItemsDataPanel(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* placeholder = new wxStaticText(panel, wxID_ANY,
		"Items Data Editor (WIP)", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	sizer->AddStretchSpacer();
	sizer->Add(placeholder, 0, wxALIGN_CENTER);
	sizer->AddStretchSpacer();
	panel->SetSizer(sizer);

	return panel;
}

wxPanel* MainFrame::CreateClassesDataPanel(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* placeholder = new wxStaticText(panel, wxID_ANY,
		"Classes Data Editor (WIP)", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE_HORIZONTAL);
	sizer->AddStretchSpacer();
	sizer->Add(placeholder, 0, wxALIGN_CENTER);
	sizer->AddStretchSpacer();
	panel->SetSizer(sizer);
	return panel;
}

void MainFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event)) {
	// Placeholder for file opening logic
	SetStatusText("Open file dialog would appear here.");
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	wxMessageBox("Lazberian Construction Set - UnitData & Items \nJohanMikes (Main Programmer) \nLordLouie (Tester and Research) \nLightgazer (Special Thanks) \nVersion 1.0 (XX/YY/2026)",
		"About", wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnToggleConsole(wxCommandEvent& WXUNUSED(event)) {
	if (IsDebugConsoleOpen()) {
		CloseDebugConsole();
		SetStatusText("Debug console closed");
	}
	else {
		OpenDebugConsole();
		std::cout << "Debug console opened." << std::endl;
		std::cout << "WARNING!! Closing this console will close the application, if you still have unsaved data, save it before closing." << std::endl;
		SetStatusText("Debug console opened");
	}
}