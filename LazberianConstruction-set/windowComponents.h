#pragma once
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>

/* Main components, add new features as the editor develops. */

class MainFrame : public wxFrame {
public:
	MainFrame();

private:
	// Main Widgets
	wxSplitterWindow* m_splitter = nullptr;
	wxTreeCtrl* m_tree = nullptr;
	wxNotebook* m_notebook = nullptr;
	// Main Panels
	wxPanel* m_unitDataPanel = nullptr;
	wxPanel* m_itemsDataPanel = nullptr;
	wxPanel* m_classesDataPanel = nullptr;
	// Window builds
	void CreateMenuBar();
	void CreateStatusBarInfo();
	void CreateLayout();

	wxTreeCtrl* CreateNavigationTree(wxWindow* parent);
	wxNotebook* CreateEditorNotebook(wxWindow* parent);
	wxPanel* CreateUnitDataPanel(wxWindow* parent);
	wxPanel* CreateItemsDataPanel(wxWindow* parent);
	wxPanel* CreateClassesDataPanel(wxWindow* parent);
	// Event Handlers
	void OnOpenFile(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

	enum {
		ID_OpenFile = wxID_HIGHEST + 1,
	};

};