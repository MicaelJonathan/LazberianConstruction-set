#include "windowComponents.h"
#include "DebugConsole.h"
#include "IsoReader.h"
#include "UnitDataScanner.h"
#include "UnitGroupScanner.h"
#include "UnitRecordReader.h"
#include "UnitRecordWriter.h"
#include "GrowthRecordReader.h"
#include "GrowthRecordWriter.h"
#include "ClassRecordReader.h"
#include "ClassRecordWriter.h"
#include "PortraitResolver.h"
#include "ItemIconResolver.h"
#include "GameOffsets.h"
#include "GameTables.h"
#include "ItemRecordReader.h"
#include "ItemRecordWriter.h"
#include "BitFieldIO.h"
#include "LepProject.h"

#include <wx/filedlg.h>
#include <wx/image.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace {
	class UnitDataTreeItemData : public wxTreeItemData {
	public:
		explicit UnitDataTreeItemData(uint64_t address) : address(address) {}
		uint64_t address;
	};
}

MainFrame::MainFrame() : wxFrame(nullptr, wxID_ANY, "Lazberian Construction Set - 1.0", wxDefaultPosition, wxSize(1000, 650)) {
	m_referenceTables = ReferenceTables::LoadFromAssets();

	CreateMenuBar();
	CreateStatusBarInfo();
	CreateLayout();
	CreateAccelerators();

	Bind(wxEVT_MENU, &MainFrame::OnOpenFile, this, ID_OpenFile);
	Bind(wxEVT_MENU, &MainFrame::OnExit, this, wxID_EXIT);
	Bind(wxEVT_MENU, &MainFrame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_MENU, &MainFrame::OnToggleConsole, this, ID_ToggleConsole);
	Bind(wxEVT_MENU, &MainFrame::OnLanguageChanged, this, ID_LangJapanese);
	Bind(wxEVT_MENU, &MainFrame::OnLanguageChanged, this, ID_LangEnglish);
	Bind(wxEVT_MENU, &MainFrame::OnExportProject, this, ID_ExportProject);
	Bind(wxEVT_MENU, &MainFrame::OnImportProject, this, ID_ImportProject);

	Center();
}

void MainFrame::CreateMenuBar() {
	wxMenu* menuFile = new wxMenu();
	menuFile->Append(ID_OpenFile, "&Open\tCtrl+O", "Open an ISO file");
	menuFile->AppendSeparator();
	menuFile->Append(ID_ExportProject, "&Export Project (.lep)...", "Export unit, class, item and growth data to a .lep file");
	menuFile->Append(ID_ImportProject, "&Import Project (.lep)...", "Import a .lep file and write it directly to the loaded ISO");
	menuFile->AppendSeparator();
	menuFile->Append(wxID_EXIT);

	wxMenu* menuView = new wxMenu();
	menuView->Append(ID_ToggleConsole, "Toggle Debug &Console\tF7", "Show or hide the debug console");

	wxMenu* menuLanguage = new wxMenu();
	menuLanguage->AppendRadioItem(ID_LangJapanese, "&Japanese (original)", "Use the Japanese offsets");
	menuLanguage->AppendRadioItem(ID_LangEnglish, "&English (translation patch Aethin 2.3)", "Use the English patch offsets");
	menuLanguage->Check(ID_LangEnglish, true); // default: m_languageUsed = 1

	wxMenu* menuHelp = new wxMenu();
	menuHelp->Append(wxID_ABOUT, "&About\tF1", "About this editor");

	wxMenuBar* menuBar = new wxMenuBar();
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuLanguage, "&Language");
	menuBar->Append(menuView, "&View");
	menuBar->Append(menuHelp, "&Help");
	SetMenuBar(menuBar);
}

void MainFrame::CreateStatusBarInfo() {
	CreateStatusBar(2);
	SetStatusText("ISO loaded");
	SetStatusText("No ISO loaded", 1);
}

void MainFrame::CreateAccelerators() {
	// Grant F7 to always work
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
	tree->AppendItem(root, "No ISO loaded..");

	tree->Bind(wxEVT_TREE_SEL_CHANGED, &MainFrame::OnUnitDataTreeSelectionChanged, this);

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

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	m_characterChoice = new wxChoice(panel, wxID_ANY);
	m_characterChoice->Append("Select an unitdata on the left");
	m_characterChoice->SetSelection(0);
	rootSizer->Add(m_characterChoice, 0, wxEXPAND | wxALL, 8);

	wxNotebook* subNotebook = new wxNotebook(panel, wxID_ANY);

	// portrait + stats base + mainhand/offhand 
	wxPanel* statsTab = new wxPanel(subNotebook);
	wxBoxSizer* statsTabSizer = new wxBoxSizer(wxHORIZONTAL);

	wxFlexGridSizer* statsGrid = new wxFlexGridSizer(0, 2, 4, 8);
	auto addUnitStatRow = [&](const wxString& labelText, UnitStatField field, int minVal, int maxVal) {
		statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, labelText));
		wxSpinCtrl* spin = new wxSpinCtrl(statsTab, wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, minVal, maxVal);
		m_unitStatSpin[static_cast<size_t>(field)] = spin;
		statsGrid->Add(spin);
		};
	addUnitStatRow("Level:", UnitStatField::Level, 1, 50);
	addUnitStatRow("HP:", UnitStatField::Hp, 0, 127);
	addUnitStatRow("Strength:", UnitStatField::Strength, -15, 15);
	addUnitStatRow("Speed:", UnitStatField::Speed, -15, 15);
	addUnitStatRow("Luck:", UnitStatField::Luck, -15, 15);
	addUnitStatRow("Defense:", UnitStatField::Defense, -15, 15);
	addUnitStatRow("Magic:", UnitStatField::Magic, -15, 15);

	statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, "Mainhand:"));
	m_mainhandChoice = new wxChoice(statsTab, wxID_ANY, wxDefaultPosition, wxSize(180, -1));
	m_mainhandChoice->Append("(unequipped)");
	statsGrid->Add(m_mainhandChoice);

	statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, "Offhand:"));
	m_offhandChoice = new wxChoice(statsTab, wxID_ANY, wxDefaultPosition, wxSize(180, -1));
	m_offhandChoice->Append("(unequipped)");
	statsGrid->Add(m_offhandChoice);

	statsTabSizer->Add(statsGrid, 1, wxALL | wxEXPAND, 8);

	wxStaticBoxSizer* identityBox = new wxStaticBoxSizer(wxVERTICAL, statsTab, "Unit dentity");
	wxFlexGridSizer* identityGrid = new wxFlexGridSizer(0, 2, 4, 8);
	identityGrid->Add(new wxStaticText(identityBox->GetStaticBox(), wxID_ANY, "Character ID:"));
	m_characterIdSpin = new wxSpinCtrl(identityBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
		wxSP_ARROW_KEYS, 0, 65535);
	identityGrid->Add(m_characterIdSpin);
	identityGrid->Add(new wxStaticText(identityBox->GetStaticBox(), wxID_ANY, "Name ID:"));
	m_textIdSpin = new wxSpinCtrl(identityBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
		wxSP_ARROW_KEYS, 0, 65535);
	identityGrid->Add(m_textIdSpin);
	identityGrid->Add(new wxStaticText(identityBox->GetStaticBox(), wxID_ANY, "Portrait ID:"));
	m_portraitIdSpin = new wxSpinCtrl(identityBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
		wxSP_ARROW_KEYS, 0, 65535);
	identityGrid->Add(m_portraitIdSpin);
	identityGrid->Add(new wxStaticText(identityBox->GetStaticBox(), wxID_ANY, "Class ID:"));
	m_unitClassIdChoice = new wxChoice(identityBox->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxSize(180, -1));
	identityGrid->Add(m_unitClassIdChoice);
	identityBox->Add(identityGrid, 0, wxALL, 4);
	identityBox->Add(new wxStaticText(identityBox->GetStaticBox(), wxID_ANY,
		"Warning: Careful changing the character ID of playable units \n changing them can mess with the unit growths."), 0, wxALL, 4);

	statsTabSizer->Add(identityBox, 0, wxALL | wxEXPAND, 8);

	wxStaticBoxSizer* growthBox = new wxStaticBoxSizer(wxVERTICAL, statsTab, "Growth (%)");
	m_statsGrowthStatusLabel = new wxStaticText(growthBox->GetStaticBox(), wxID_ANY, "");
	growthBox->Add(m_statsGrowthStatusLabel, 0, wxBOTTOM, 4);

	wxFlexGridSizer* growthGrid = new wxFlexGridSizer(0, 2, 4, 8);
	auto addGrowthStatRow = [&](const wxString& labelText, UnitGrowthStatField field) {
		growthGrid->Add(new wxStaticText(growthBox->GetStaticBox(), wxID_ANY, labelText));
		wxSpinCtrl* spin = new wxSpinCtrl(growthBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(60, -1),
			wxSP_ARROW_KEYS, 0, static_cast<int>(MaxValueForBits(GROWTH_STAT_BIT_WIDTH)));
		m_growthStatSpin[static_cast<size_t>(field)] = spin;
		growthGrid->Add(spin);
		};
	addGrowthStatRow("HP:", UnitGrowthStatField::Hp);
	addGrowthStatRow("Str:", UnitGrowthStatField::Str);
	addGrowthStatRow("Def:", UnitGrowthStatField::Def);
	addGrowthStatRow("Spd:", UnitGrowthStatField::Spd);
	addGrowthStatRow("Mag:", UnitGrowthStatField::Mag);
	growthBox->Add(growthGrid, 0, wxEXPAND);

	statsTabSizer->Add(growthBox, 0, wxALL | wxEXPAND, 8);

	wxImage placeholderImage(256, 256);
	placeholderImage.SetRGB(wxRect(0, 0, 256, 256), 200, 200, 200);
	m_portraitBitmap = new wxStaticBitmap(statsTab, wxID_ANY, wxBitmap(placeholderImage));
	statsTabSizer->Add(m_portraitBitmap, 0, wxALL | wxALIGN_TOP, 8);

	statsTab->SetSizer(statsTabSizer);
	subNotebook->AddPage(statsTab, "Stats");

	// Weapon skills
	wxPanel* profTab = new wxPanel(subNotebook);
	wxBoxSizer* profContentSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer* profBaseBox = new wxStaticBoxSizer(wxVERTICAL, profTab, "Base Weapon Proficiency");
	wxFlexGridSizer* profGrid = new wxFlexGridSizer(0, 2, 4, 8);
	for (size_t i = 0; i < WeaponCategories.size(); ++i) {
		profGrid->Add(new wxStaticText(profBaseBox->GetStaticBox(), wxID_ANY, WeaponCategories[i] + ":"));
		wxSpinCtrl* spin = new wxSpinCtrl(profBaseBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, 0, static_cast<int>(MaxValueForBits(PROFICIENCY_BIT_WIDTH)));
		m_proficiencySpin[i] = spin;
		profGrid->Add(spin);
	}
	profBaseBox->Add(new wxStaticText(profBaseBox->GetStaticBox(), wxID_ANY,
		"Raw value, game display it as (value/10) in game."), 0, wxALL, 4);
	profBaseBox->Add(profGrid, 0, wxALL, 4);
	profContentSizer->Add(profBaseBox, 0, wxALL, 8);

	wxStaticBoxSizer* profGrowthBox = new wxStaticBoxSizer(wxVERTICAL, profTab, "Growth Rate (%)");
	m_profGrowthStatusLabel = new wxStaticText(profGrowthBox->GetStaticBox(), wxID_ANY, "");
	profGrowthBox->Add(m_profGrowthStatusLabel, 0, wxALL, 4);
	wxFlexGridSizer* profGrowthGrid = new wxFlexGridSizer(0, 2, 4, 8);
	for (size_t i = 0; i < GrowthProficiencyCategories.size(); ++i) {
		profGrowthGrid->Add(new wxStaticText(profGrowthBox->GetStaticBox(), wxID_ANY, GrowthProficiencyCategories[i] + ":"));
		wxSpinCtrl* spin = new wxSpinCtrl(profGrowthBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, 0, static_cast<int>(MaxValueForBits(GROWTH_PROFICIENCY_BIT_WIDTH)));
		m_growthProficiencySpin[i] = spin;
		profGrowthGrid->Add(spin);
	}
	profGrowthBox->Add(profGrowthGrid, 0, wxALL, 4);
	profContentSizer->Add(profGrowthBox, 0, wxALL, 8);

	profTab->SetSizer(profContentSizer);
	subNotebook->AddPage(profTab, "Proficiencies");

	// Skills
	wxPanel* skillsTab = new wxPanel(subNotebook);
	wxBoxSizer* skillsTabSizer = new wxBoxSizer(wxVERTICAL);
	m_unitSkillsCheckList = new wxCheckListBox(skillsTab, wxID_ANY);
	m_unitSkillsCheckListToId.clear();
	for (size_t i = 0; i < Skills.size(); ++i) {
		if (Skills[i] == "--") continue;
		m_unitSkillsCheckList->Append(Skills[i]);
		m_unitSkillsCheckListToId.push_back(static_cast<int>(i));
	}
	skillsTabSizer->Add(m_unitSkillsCheckList, 1, wxEXPAND | wxALL, 8);
	skillsTab->SetSizer(skillsTabSizer);
	subNotebook->AddPage(skillsTab, "Skills");

	// Inventory
	wxPanel* invTab = new wxPanel(subNotebook);
	wxFlexGridSizer* invGrid = new wxFlexGridSizer(0, 5, 4, 8);
	invGrid->Add(new wxStaticText(invTab, wxID_ANY, "Slot"));
	invGrid->Add(new wxStaticText(invTab, wxID_ANY, "Item"));
	invGrid->Add(new wxStaticText(invTab, wxID_ANY, "Durability"));
	invGrid->Add(new wxStaticText(invTab, wxID_ANY, "Locked"));
	invGrid->Add(new wxStaticText(invTab, wxID_ANY, "Dropped"));
	for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
		invGrid->Add(new wxStaticText(invTab, wxID_ANY, wxString::Format("%zu", slot + 1)));

		wxChoice* itemChoice = new wxChoice(invTab, wxID_ANY, wxDefaultPosition, wxSize(220, -1));
		PopulateItemCatalogChoice(itemChoice);
		m_inventoryItemChoice[slot] = itemChoice;
		invGrid->Add(itemChoice);

		wxSpinCtrl* durSpin = new wxSpinCtrl(invTab, wxID_ANY, "0", wxDefaultPosition, wxSize(60, -1),
			wxSP_ARROW_KEYS, 0, 255);
		m_inventoryDurabilitySpin[slot] = durSpin;
		invGrid->Add(durSpin);

		wxCheckBox* lockedCheck = new wxCheckBox(invTab, wxID_ANY, "");
		m_inventoryLockedCheck[slot] = lockedCheck;
		invGrid->Add(lockedCheck);

		wxCheckBox* droppedCheck = new wxCheckBox(invTab, wxID_ANY, "");
		m_inventoryDroppedCheck[slot] = droppedCheck;
		invGrid->Add(droppedCheck);
	}
	wxBoxSizer* invTabSizer = new wxBoxSizer(wxVERTICAL);
	invTabSizer->Add(invGrid, 1, wxALL | wxEXPAND, 8);
	invTab->SetSizer(invTabSizer);
	subNotebook->AddPage(invTab, "Inventory");

	rootSizer->Add(subNotebook, 1, wxEXPAND | wxALL, 4);

	m_saveUnitButton = new wxButton(panel, wxID_ANY, "Save changes to disc");
	rootSizer->Add(m_saveUnitButton, 0, wxEXPAND | wxALL, 8);

	panel->SetSizer(rootSizer);

	m_characterChoice->Bind(wxEVT_CHOICE, &MainFrame::OnCharacterChoiceChanged, this);
	m_saveUnitButton->Bind(wxEVT_BUTTON, &MainFrame::OnSaveUnitClicked, this);

	return panel;
}

void MainFrame::PopulateItemCatalogChoice(wxChoice* choice) {
	choice->Append("(empty)");
	for (const auto& item : m_referenceTables.items) {
		choice->Append(wxString::Format("0x%03x - %s", item.id, item.name));
	}
}

int MainFrame::FindItemChoiceIndexForId(uint32_t itemId) const {
	if (itemId == 0) {
		return 0;
	}
	for (size_t i = 0; i < m_referenceTables.items.size(); ++i) {
		if (m_referenceTables.items[i].id == itemId) {
			return static_cast<int>(i) + 1;
		}
	}
	return 0; // No value, treat as empty
}

wxString MainFrame::DescribeItemId(uint32_t itemId) const {
	if (itemId == 0) {
		return "(empty)";
	}
	for (const auto& item : m_referenceTables.items) {
		if (item.id == itemId) {
			return item.name;
		}
	}
	return "(unknown item or position)";
}

void MainFrame::RebuildUnitClassChoice(uint16_t currentClassId) {
	m_unitClassIdChoice->Clear();
	int selectIndex = -1;
	for (size_t i = 0; i < m_referenceTables.classesOrdered.size(); ++i) {
		const auto& cls = m_referenceTables.classesOrdered[i];
		m_unitClassIdChoice->Append(wxString::Format("0x%03x - %s", cls.id, cls.name));
		if (cls.id == currentClassId) {
			selectIndex = static_cast<int>(i);
		}
	}
	if (selectIndex < 0) {
		// If ID don't match any known class, add a "(unknown class)" entry at the end and select it.
		m_unitClassChoiceUnknownId = currentClassId;
		m_unitClassIdChoice->Append(wxString::Format("0x%04X - (unknown class)", currentClassId));
		selectIndex = static_cast<int>(m_unitClassIdChoice->GetCount()) - 1;
	}
	m_unitClassIdChoice->SetSelection(selectIndex);
}

uint16_t MainFrame::GetSelectedUnitClassId() const {
	int selection = m_unitClassIdChoice->GetSelection();
	if (selection < 0) {
		return 0;
	}
	if (static_cast<size_t>(selection) < m_referenceTables.classesOrdered.size()) {
		return static_cast<uint16_t>(m_referenceTables.classesOrdered[selection].id);
	}
	return m_unitClassChoiceUnknownId; // Return the unknown class ID.
}

wxPanel* MainFrame::CreateItemsDataPanel(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	m_itemChoice = new wxChoice(panel, wxID_ANY);
	if (m_referenceTables.items.empty()) {
		m_itemChoice->Append("No item found in assets/refs/items (make sure the file exists)");
	}
	else {
		for (const auto& item : m_referenceTables.items) {
			m_itemChoice->Append(wxString::Format("0x%03x - %s", item.id, item.name));
		}
	}
	m_itemChoice->SetSelection(0);
	rootSizer->Add(m_itemChoice, 0, wxEXPAND | wxALL, 8);

	wxBoxSizer* contentSizer = new wxBoxSizer(wxHORIZONTAL);

	wxFlexGridSizer* statsGrid = new wxFlexGridSizer(0, 4, 4, 8);
	auto addStatRow = [&](const wxString& labelText, ItemNumericField field) {
		const ItemFieldSpec* spec = nullptr;
		for (const auto& s : ItemFieldSpecs) {
			if (s.field == field) { spec = &s; break; }
		}

		bool isUnsignedField = (field == ItemNumericField::Uses || field == ItemNumericField::Level
			|| field == ItemNumericField::Might || field == ItemNumericField::Accuracy);

		int minAllowed;
		int maxAllowed;
		if (isUnsignedField) {
			minAllowed = static_cast<int>(spec->minValue);
			maxAllowed = static_cast<int>((spec->maxValueOverride != 0) ? spec->maxValueOverride : MaxValueForBits(spec->bitWidth));
		}
		else {
			minAllowed = -static_cast<int>(1u << (spec->bitWidth - 1));
			maxAllowed = (spec->maxValueOverride != 0) ? static_cast<int>(spec->maxValueOverride) : static_cast<int>((1u << (spec->bitWidth - 1)) - 1);
		}

		statsGrid->Add(new wxStaticText(panel, wxID_ANY, labelText));
		wxSpinCtrl* spin = new wxSpinCtrl(panel, wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, minAllowed, maxAllowed);
		m_itemStatSpin[static_cast<size_t>(field)] = spin;
		statsGrid->Add(spin);
		};

	addStatRow("Might:", ItemNumericField::Might);
	addStatRow("Hex:", ItemNumericField::Hex);
	addStatRow("Accuracy:", ItemNumericField::Accuracy);
	addStatRow("Weight:", ItemNumericField::Weight);
	addStatRow("Max Range:", ItemNumericField::MaxRange);
	addStatRow("Min Range:", ItemNumericField::MinRange);
	addStatRow("Crit:", ItemNumericField::Crit);
	addStatRow("Uses:", ItemNumericField::Uses);
	addStatRow("Level:", ItemNumericField::Level);
	addStatRow("Price:", ItemNumericField::Price);
	addStatRow("Defense:", ItemNumericField::Defense);
	addStatRow("Speed:", ItemNumericField::Speed);
	addStatRow("Avoid:", ItemNumericField::Avoid);
	addStatRow("Hit:", ItemNumericField::Hit);
	addStatRow("Magic:", ItemNumericField::Magic);
	addStatRow("Strength:", ItemNumericField::Strength);
	addStatRow("Rounds:", ItemNumericField::Rounds);
	addStatRow("Fire Res:", ItemNumericField::FireRes);
	addStatRow("Thunder Res:", ItemNumericField::ThunderRes);
	addStatRow("Wind Res:", ItemNumericField::WindRes);
	addStatRow("Dark Res:", ItemNumericField::DarkRes);
	addStatRow("Holy Res:", ItemNumericField::HolyRes);
	addStatRow("Crit Avoid:", ItemNumericField::CritAvoidPenalty);

	statsGrid->Add(new wxStaticText(panel, wxID_ANY, "Durability:"));
	m_durabilityChoice = new wxChoice(panel, wxID_ANY);
	for (const auto& d : Durability) {
		m_durabilityChoice->Append(d);
	}
	statsGrid->Add(m_durabilityChoice);

	contentSizer->Add(statsGrid, 1, wxALL | wxEXPAND, 8);

	wxBoxSizer* middleSizer = new wxBoxSizer(wxVERTICAL);

	middleSizer->Add(new wxStaticText(panel, wxID_ANY, "Effect Rate:"), 0, wxBOTTOM, 4);
	wxBoxSizer* effectRateRow = new wxBoxSizer(wxHORIZONTAL);
	m_effectRateChoice = new wxChoice(panel, wxID_ANY);
	m_effectRateChoiceToId.clear();
	m_effectRateChoice->Append("(none)");
	m_effectRateChoiceToId.push_back(-1);
	for (size_t i = 0; i < ItemEffectRates.size(); ++i) {
		if (ItemEffectRates[i] == "--") continue;
		m_effectRateChoice->Append(ItemEffectRates[i]);
		m_effectRateChoiceToId.push_back(static_cast<int>(i));
	}
	effectRateRow->Add(m_effectRateChoice, 1, wxRIGHT, 4);
	m_effectRateValueSpin = new wxSpinCtrl(panel, wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
		wxSP_ARROW_KEYS, 0, static_cast<int>(MaxValueForBits(ITEM_EFFECT_RATE_VALUE_BIT_WIDTH)));
	effectRateRow->Add(m_effectRateValueSpin, 0);
	middleSizer->Add(effectRateRow, 0, wxEXPAND | wxBOTTOM, 8);

	middleSizer->Add(new wxStaticText(panel, wxID_ANY, "Active Effects:"), 0, wxBOTTOM, 4);
	m_itemEffectsCheckList = new wxCheckListBox(panel, wxID_ANY, wxDefaultPosition, wxSize(220, -1));
	m_itemEffectsCheckListToId.clear();
	for (size_t i = 0; i < ItemEffects.size(); ++i) {
		if (ItemEffects[i] == "--") continue;
		wxString label = (ItemEffects[i] == "??")
			? wxString::Format("?? (id %zu)", i)
			: wxString(ItemEffects[i]);
		m_itemEffectsCheckList->Append(label);
		m_itemEffectsCheckListToId.push_back(static_cast<int>(i));
	}
	middleSizer->Add(m_itemEffectsCheckList, 1, wxEXPAND);

	m_saveItemButton = new wxButton(panel, wxID_ANY, "Save changes to disc");
	middleSizer->Add(m_saveItemButton, 0, wxEXPAND | wxTOP, 8);

	contentSizer->Add(middleSizer, 0, wxALL | wxEXPAND, 8);

	// Items are 256x97
	wxImage itemPlaceholder(256, 97);
	itemPlaceholder.SetRGB(wxRect(0, 0, 256, 97), 200, 200, 200);
	m_itemIconBitmap = new wxStaticBitmap(panel, wxID_ANY, wxBitmap(itemPlaceholder));
	contentSizer->Add(m_itemIconBitmap, 0, wxALL | wxALIGN_TOP, 8);

	rootSizer->Add(contentSizer, 1, wxEXPAND);
	panel->SetSizer(rootSizer);

	m_itemChoice->Bind(wxEVT_CHOICE, &MainFrame::OnItemChoiceChanged, this);
	m_effectRateChoice->Bind(wxEVT_CHOICE, &MainFrame::OnEffectRateChoiceChanged, this);
	m_saveItemButton->Bind(wxEVT_BUTTON, &MainFrame::OnSaveItemClicked, this);

	return panel;
}

wxPanel* MainFrame::CreateClassesDataPanel(wxWindow* parent) {
	wxPanel* panel = new wxPanel(parent);

	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	m_classChoice = new wxChoice(panel, wxID_ANY);
	if (m_referenceTables.classesOrdered.empty()) {
		m_classChoice->Append("No class found in assets/refs/classes (make sure the file exists)");
	}
	else {
		for (const auto& cls : m_referenceTables.classesOrdered) {
			m_classChoice->Append(wxString::Format("0x%03x - %s", cls.id, cls.name));
		}
	}
	m_classChoice->SetSelection(0);
	rootSizer->Add(m_classChoice, 0, wxEXPAND | wxALL, 8);

	wxNotebook* subNotebook = new wxNotebook(panel, wxID_ANY);

	wxPanel* statsTab = new wxPanel(subNotebook);
	wxFlexGridSizer* statsGrid = new wxFlexGridSizer(0, 2, 4, 8);
	auto addClassStatRow = [&](const wxString& labelText, ClassStatField field, int minVal, int maxVal) {
		statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, labelText));
		wxSpinCtrl* spin = new wxSpinCtrl(statsTab, wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, minVal, maxVal);
		m_classStatSpin[static_cast<size_t>(field)] = spin;
		statsGrid->Add(spin);
		};
	addClassStatRow("HP:", ClassStatField::Hp, 0, 31);
	addClassStatRow("Strength:", ClassStatField::Strength, 0, 31);
	addClassStatRow("Speed:", ClassStatField::Speed, 0, 31);
	addClassStatRow("Defense:", ClassStatField::Defense, 0, 31);
	addClassStatRow("Magic:", ClassStatField::Magic, 0, 31);
	addClassStatRow("Move:", ClassStatField::Move, 0, static_cast<int>(MaxValueForBits(CLASS_MOVE_BIT_WIDTH)));
	addClassStatRow("Experience:", ClassStatField::Experience, 0, static_cast<int>(MaxValueForBits(CLASS_EXP_BIT_WIDTH)));

	statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, "Mount:"));
	m_classMountChoice = new wxChoice(statsTab, wxID_ANY);
	for (const auto& m : MountStatus) m_classMountChoice->Append(m);
	statsGrid->Add(m_classMountChoice);

	statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, "Type:"));
	m_classTypeChoice = new wxChoice(statsTab, wxID_ANY);
	for (const auto& t : UnitTypes) m_classTypeChoice->Append(t);
	statsGrid->Add(m_classTypeChoice);

	statsGrid->Add(new wxStaticText(statsTab, wxID_ANY, "Movement:"));
	m_classMovementChoice = new wxChoice(statsTab, wxID_ANY);
	for (const auto& m : MovementTypes) m_classMovementChoice->Append(m);
	statsGrid->Add(m_classMovementChoice);

	wxBoxSizer* statsTabSizer = new wxBoxSizer(wxVERTICAL);
	statsTabSizer->Add(statsGrid, 1, wxALL | wxEXPAND, 8);
	statsTab->SetSizer(statsTabSizer);
	subNotebook->AddPage(statsTab, "Stats");

	wxPanel* growthCapsTab = new wxPanel(subNotebook);
	wxBoxSizer* growthCapsSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticBoxSizer* growthBox = new wxStaticBoxSizer(wxVERTICAL, growthCapsTab, "Growth (%)");
	auto addClassGrowthRow = [&](const wxString& labelText, ClassGrowthField field) {
		wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
		row->Add(new wxStaticText(growthBox->GetStaticBox(), wxID_ANY, labelText), 0, wxALIGN_CENTER_VERTICAL);
		wxSpinCtrl* spin = new wxSpinCtrl(growthBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, 0, static_cast<int>(MaxValueForBits(CLASS_GROWTH_BIT_WIDTH)));
		m_classGrowthSpin[static_cast<size_t>(field)] = spin;
		row->Add(spin, 0, wxLEFT, 4);
		growthBox->Add(row, 0, wxALL, 4);
		};
	addClassGrowthRow("HP:", ClassGrowthField::Hp);
	addClassGrowthRow("Strength:", ClassGrowthField::Strength);
	addClassGrowthRow("Speed:", ClassGrowthField::Speed);
	addClassGrowthRow("Defense:", ClassGrowthField::Defense);
	addClassGrowthRow("Magic:", ClassGrowthField::Magic);
	growthCapsSizer->Add(growthBox, 0, wxALL, 8);

	wxStaticBoxSizer* capsBox = new wxStaticBoxSizer(wxVERTICAL, growthCapsTab, "Weapon skill caps");
	wxFlexGridSizer* capsGrid = new wxFlexGridSizer(0, 2, 4, 8);
	for (size_t i = 0; i < WeaponCategories.size(); ++i) {
		capsGrid->Add(new wxStaticText(capsBox->GetStaticBox(), wxID_ANY, WeaponCategories[i] + ":"));
		wxSpinCtrl* spin = new wxSpinCtrl(capsBox->GetStaticBox(), wxID_ANY, "0", wxDefaultPosition, wxSize(70, -1),
			wxSP_ARROW_KEYS, 0, static_cast<int>(MaxValueForBits(CLASS_CAP_BIT_WIDTH)));
		m_classCapSpin[i] = spin;
		capsGrid->Add(spin);
	}
	capsBox->Add(capsGrid, 0, wxALL, 4);
	growthCapsSizer->Add(capsBox, 0, wxALL, 8);

	growthCapsTab->SetSizer(growthCapsSizer);
	subNotebook->AddPage(growthCapsTab, "Growth && Caps");

	// --- Sub-aba "Skills" ---
	wxPanel* skillsTab = new wxPanel(subNotebook);
	wxBoxSizer* skillsTabSizer = new wxBoxSizer(wxVERTICAL);
	m_classSkillsCheckList = new wxCheckListBox(skillsTab, wxID_ANY);
	m_classSkillsCheckListToId.clear();
	for (size_t i = 0; i < Skills.size(); ++i) {
		if (Skills[i] == "--") continue;
		m_classSkillsCheckList->Append(Skills[i]);
		m_classSkillsCheckListToId.push_back(static_cast<int>(i));
	}
	skillsTabSizer->Add(m_classSkillsCheckList, 1, wxEXPAND | wxALL, 8);
	skillsTab->SetSizer(skillsTabSizer);
	subNotebook->AddPage(skillsTab, "Skills");

	rootSizer->Add(subNotebook, 1, wxEXPAND | wxALL, 4);

	m_saveClassButton = new wxButton(panel, wxID_ANY, "Save changes to disc");
	rootSizer->Add(m_saveClassButton, 0, wxEXPAND | wxALL, 8);

	panel->SetSizer(rootSizer);

	m_classChoice->Bind(wxEVT_CHOICE, &MainFrame::OnClassChoiceChanged, this);
	m_saveClassButton->Bind(wxEVT_BUTTON, &MainFrame::OnSaveClassClicked, this);

	return panel;
}

void MainFrame::LoadClassIntoPanel(int indexInClassesList) {
	if (indexInClassesList < 0 || static_cast<size_t>(indexInClassesList) >= m_referenceTables.classesOrdered.size()) {
		return;
	}
	if (m_isoPath.IsEmpty()) {
		std::cout << "No ISO loaded." << std::endl;
		return;
	}

	m_currentClassIndex = indexInClassesList;
	const ItemDefinition& classDef = m_referenceTables.classesOrdered[indexInClassesList];

	std::cout << "Loading class '" << classDef.name << "' (ID 0x" << std::hex << classDef.id << std::dec
		<< ", language=" << (m_languageUsed == 0 ? "JP" : "EN") << ")..." << std::endl;

	ClassRecord record = ReadClassRecord(m_isoPath.ToStdString(), m_dataOffset, m_languageUsed, classDef.id);

	std::cout << "  HP=" << record.hp << " Str=" << record.strength << " Spd=" << record.speed
		<< " Def=" << record.defense << " Magic=" << record.magic << " Move=" << record.move
		<< " Exp=" << record.experience << std::endl;
	std::cout << "  Mount=" << (record.mountIndex >= 0 && static_cast<size_t>(record.mountIndex) < MountStatus.size() ? MountStatus[record.mountIndex] : "?")
		<< " Type=" << (record.typeIndex >= 0 ? UnitTypes[record.typeIndex] : "invalid (not power of 2.. don't ask me why...)")
		<< " Movement=" << (record.movementIndex >= 0 && static_cast<size_t>(record.movementIndex) < MovementTypes.size() ? MovementTypes[record.movementIndex] : "?")
		<< std::endl;
	std::cout << "  ActiveSkills=" << record.activeSkillIds.size() << std::endl;

	auto setStat = [&](ClassStatField field, int value) { m_classStatSpin[static_cast<size_t>(field)]->SetValue(value); };
	setStat(ClassStatField::Hp, record.hp);
	setStat(ClassStatField::Strength, record.strength);
	setStat(ClassStatField::Speed, record.speed);
	setStat(ClassStatField::Defense, record.defense);
	setStat(ClassStatField::Magic, record.magic);
	setStat(ClassStatField::Move, record.move);
	setStat(ClassStatField::Experience, record.experience);

	m_classMountChoice->SetSelection(record.mountIndex >= 0 && static_cast<size_t>(record.mountIndex) < MountStatus.size() ? record.mountIndex : 0);
	m_classTypeChoice->SetSelection(record.typeIndex >= 0 ? record.typeIndex : 0);
	m_classMovementChoice->SetSelection(record.movementIndex >= 0 && static_cast<size_t>(record.movementIndex) < MovementTypes.size() ? record.movementIndex : 0);

	auto setGrowth = [&](ClassGrowthField field, int value) { m_classGrowthSpin[static_cast<size_t>(field)]->SetValue(value); };
	setGrowth(ClassGrowthField::Hp, record.growthHp);
	setGrowth(ClassGrowthField::Strength, record.growthStrength);
	setGrowth(ClassGrowthField::Speed, record.growthSpeed);
	setGrowth(ClassGrowthField::Defense, record.growthDefense);
	setGrowth(ClassGrowthField::Magic, record.growthMagic);

	for (size_t i = 0; i < WeaponCategories.size(); ++i) {
		m_classCapSpin[i]->SetValue(record.caps[i]);
	}

	for (size_t i = 0; i < m_classSkillsCheckListToId.size(); ++i) {
		int realId = m_classSkillsCheckListToId[i];
		bool isActive = std::find(record.activeSkillIds.begin(), record.activeSkillIds.end(), realId) != record.activeSkillIds.end();
		m_classSkillsCheckList->Check(static_cast<unsigned int>(i), isActive);
	}
}

void MainFrame::SaveCurrentClass() {
	if (m_currentClassIndex < 0 || static_cast<size_t>(m_currentClassIndex) >= m_referenceTables.classesOrdered.size()) {
		std::cout << "No class selected for saving." << std::endl;
		return;
	}
	if (m_isoPath.IsEmpty()) {
		std::cout << "No ISO loaded - not possible to save." << std::endl;
		return;
	}

	const ItemDefinition& classDef = m_referenceTables.classesOrdered[m_currentClassIndex];

	ClassRecord record;
	auto getStat = [&](ClassStatField field) { return m_classStatSpin[static_cast<size_t>(field)]->GetValue(); };
	record.hp = static_cast<uint16_t>(getStat(ClassStatField::Hp));
	record.strength = static_cast<uint16_t>(getStat(ClassStatField::Strength));
	record.speed = static_cast<uint16_t>(getStat(ClassStatField::Speed));
	record.defense = static_cast<uint16_t>(getStat(ClassStatField::Defense));
	record.magic = static_cast<uint16_t>(getStat(ClassStatField::Magic));
	record.move = static_cast<uint16_t>(getStat(ClassStatField::Move));
	record.experience = static_cast<uint16_t>(getStat(ClassStatField::Experience));

	record.mountIndex = m_classMountChoice->GetSelection();
	record.typeIndex = m_classTypeChoice->GetSelection();
	record.movementIndex = m_classMovementChoice->GetSelection();

	auto getGrowth = [&](ClassGrowthField field) { return m_classGrowthSpin[static_cast<size_t>(field)]->GetValue(); };
	record.growthHp = static_cast<uint16_t>(getGrowth(ClassGrowthField::Hp));
	record.growthStrength = static_cast<uint16_t>(getGrowth(ClassGrowthField::Strength));
	record.growthSpeed = static_cast<uint16_t>(getGrowth(ClassGrowthField::Speed));
	record.growthDefense = static_cast<uint16_t>(getGrowth(ClassGrowthField::Defense));
	record.growthMagic = static_cast<uint16_t>(getGrowth(ClassGrowthField::Magic));

	for (size_t i = 0; i < WeaponCategories.size(); ++i) {
		record.caps[i] = static_cast<uint16_t>(m_classCapSpin[i]->GetValue());
	}

	for (unsigned int i = 0; i < m_classSkillsCheckList->GetCount(); ++i) {
		if (m_classSkillsCheckList->IsChecked(i)) {
			record.activeSkillIds.push_back(m_classSkillsCheckListToId[i]);
		}
	}

	bool ok = WriteClassRecord(m_isoPath.ToStdString(), m_dataOffset, m_languageUsed, classDef.id, record);

	if (ok) {
		SetStatusText("Class saved successfully.");
	}
	else {
		SetStatusText("Failed to save class - open debug console and try again for error log.");
		wxMessageBox("Could not save all changes to the class.\nOpen the debug console (F7) for details and try again for error log.",
			"Error saving class", wxOK | wxICON_ERROR, this);
	}

	LoadClassIntoPanel(m_currentClassIndex);
}

void MainFrame::PopulateUnitDataTree() {
	m_tree->DeleteAllItems();
	wxTreeItemId root = m_tree->AddRoot("root");

	std::vector<UnitDataEntry> entries = LoadUnitDataRefSheet();
	if (entries.empty()) {
		m_tree->AppendItem(root, "No unitdata found");
		return;
	}

	std::unordered_map<std::string, std::string> nameRef = LoadUnitDataNameRef();

	for (const auto& entry : entries) {
		auto it = nameRef.find(entry.name);
		wxString displayName = (it != nameRef.end()) ? wxString(it->second) : wxString("(unknown)");
		wxString itemText = wxString::Format("%s = %s", entry.name, displayName);

		wxTreeItemId item = m_tree->AppendItem(root, itemText);
		m_tree->SetItemData(item, new UnitDataTreeItemData(entry.address));
	}
}

void MainFrame::LoadCharacterIntoPanel(size_t indexInGroup) {
	if (indexInGroup >= m_currentGroup.characters.size()) {
		return;
	}

	m_currentCharacterIndex = static_cast<int>(indexInGroup);
	uint64_t offset = m_currentGroup.characters[indexInGroup].offset;
	std::cout << "Loading character.. " << (indexInGroup + 1) << "/" << m_currentGroup.characters.size()
		<< " (offset 0x" << std::hex << offset << std::dec << ")..." << std::endl;

	UnitRecord record = ReadUnitRecord(m_isoPath.ToStdString(), m_dataOffset, offset);

	std::string currentName = m_referenceTables.ResolveName(record.textId, record.classId);
	m_characterChoice->SetString(static_cast<unsigned int>(indexInGroup),
		wxString::Format("%zu - %s", indexInGroup + 1, currentName));

	std::cout << "  Level=" << record.level << " HP=" << record.hp << " Str=" << record.strength
		<< " Spd=" << record.speed << " Luck=" << record.luck << " Def=" << record.defense
		<< " Magic=" << record.magic << std::endl;
	std::cout << "  CharacterID=0x" << std::hex << record.characterId << " TextID=0x" << record.textId
		<< " PortraitID=0x" << record.portraitId << " ClassID=0x" << record.classId << std::dec << std::endl;
	std::cout << "  Mainhand=" << (record.mainhandSlot >= 0 ? wxString::Format("slot %d (%s)", record.mainhandSlot + 1, DescribeItemId(record.inventory[record.mainhandSlot].itemId)).ToStdString() : "none")
		<< " Offhand=" << (record.offhandSlot >= 0 ? wxString::Format("slot %d (%s)", record.offhandSlot + 1, DescribeItemId(record.inventory[record.offhandSlot].itemId)).ToStdString() : "none") << std::endl;
	std::cout << "  ActiveSkills=" << record.activeSkillIds.size() << std::endl;
	for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
		const InventorySlot& item = record.inventory[slot];
		std::cout << "  Slot" << (slot + 1) << ": itemId=0x" << std::hex << item.itemId << std::dec
			<< " durability=" << static_cast<int>(item.durability)
			<< " locked=" << item.locked << " dropped=" << item.dropped << std::endl;
	}

	auto setSpin = [&](UnitStatField field, int value) { m_unitStatSpin[static_cast<size_t>(field)]->SetValue(value); };
	setSpin(UnitStatField::Level, record.level);
	setSpin(UnitStatField::Hp, record.hp);
	setSpin(UnitStatField::Strength, record.strength);
	setSpin(UnitStatField::Speed, record.speed);
	setSpin(UnitStatField::Luck, record.luck);
	setSpin(UnitStatField::Defense, record.defense);
	setSpin(UnitStatField::Magic, record.magic);

	m_characterIdSpin->SetValue(record.characterId);
	m_textIdSpin->SetValue(record.textId);
	m_portraitIdSpin->SetValue(record.portraitId);
	RebuildUnitClassChoice(record.classId);


	auto rebuildEquipChoice = [&](wxChoice* choice, int selectedSlot) {
		choice->Clear();
		choice->Append("(unequipped)");
		for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
			wxString itemName = DescribeItemId(record.inventory[slot].itemId);
			choice->Append(wxString::Format("Slot %zu - %s", slot + 1, itemName));
		}
		choice->SetSelection(selectedSlot < 0 ? 0 : selectedSlot + 1);
		};
	rebuildEquipChoice(m_mainhandChoice, record.mainhandSlot);
	rebuildEquipChoice(m_offhandChoice, record.offhandSlot);


	for (size_t i = 0; i < WeaponCategories.size(); ++i) {
		m_proficiencySpin[i]->SetValue(record.proficiency[i]);
	}


	for (size_t i = 0; i < m_unitSkillsCheckListToId.size(); ++i) {
		int realId = m_unitSkillsCheckListToId[i];
		bool isActive = std::find(record.activeSkillIds.begin(), record.activeSkillIds.end(), realId) != record.activeSkillIds.end();
		m_unitSkillsCheckList->Check(static_cast<unsigned int>(i), isActive);
	}


	for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
		const InventorySlot& item = record.inventory[slot];
		m_inventoryItemChoice[slot]->SetSelection(FindItemChoiceIndexForId(item.itemId));
		m_inventoryDurabilitySpin[slot]->SetValue(item.durability);
		m_inventoryLockedCheck[slot]->SetValue(item.locked);
		m_inventoryDroppedCheck[slot]->SetValue(item.dropped);
	}

	// Changed character ID hotfix to not reload growth 
	m_loadedCharacterId = record.characterId;
	GrowthRecord growth = ReadGrowthRecord(m_isoPath.ToStdString(), m_dataOffset, m_dataSize, m_languageUsed, record.characterId);
	m_currentGrowthValid = growth.valid;

	std::cout << "  Growth=" << (growth.valid ? "available" : "N/A (no growths)") << std::endl;

	wxString growthStatusText = growth.valid ? "" : "N/A (no growths).";
	m_statsGrowthStatusLabel->SetLabel(growthStatusText);
	m_profGrowthStatusLabel->SetLabel(growthStatusText);

	auto setGrowthStatSpin = [&](UnitGrowthStatField field, int value) {
		wxSpinCtrl* spin = m_growthStatSpin[static_cast<size_t>(field)];
		spin->SetValue(growth.valid ? value : 0);
		spin->Enable(growth.valid);
		};
	setGrowthStatSpin(UnitGrowthStatField::Hp, growth.hp);
	setGrowthStatSpin(UnitGrowthStatField::Str, growth.str);
	setGrowthStatSpin(UnitGrowthStatField::Def, growth.def);
	setGrowthStatSpin(UnitGrowthStatField::Spd, growth.spd);
	setGrowthStatSpin(UnitGrowthStatField::Mag, growth.mag);

	for (size_t i = 0; i < GrowthProficiencyCategories.size(); ++i) {
		m_growthProficiencySpin[i]->SetValue(growth.valid ? growth.proficiency[i] : 0);
		m_growthProficiencySpin[i]->Enable(growth.valid);
	}

	std::string portraitPath = ResolvePortraitPath(record.portraitId);
	wxImage image;
	if (image.LoadFile(portraitPath)) {
		image.Rescale(256, 256);
		m_portraitBitmap->SetBitmap(wxBitmap(image));
	}

	m_unitDataPanel->Layout();
}

void MainFrame::SaveCurrentCharacter() {
	if (m_currentCharacterIndex < 0 || static_cast<size_t>(m_currentCharacterIndex) >= m_currentGroup.characters.size()) {
		std::cout << "No character selected for saving." << std::endl;
		return;
	}
	if (m_isoPath.IsEmpty()) {
		std::cout << "No ISO loaded - not possible to save." << std::endl;
		return;
	}

	uint64_t offset = m_currentGroup.characters[m_currentCharacterIndex].offset;

	UnitRecord record = ReadUnitRecord(m_isoPath.ToStdString(), m_dataOffset, offset);

	auto getSpin = [&](UnitStatField field) { return m_unitStatSpin[static_cast<size_t>(field)]->GetValue(); };
	record.level = static_cast<uint16_t>(getSpin(UnitStatField::Level));
	record.hp = static_cast<uint16_t>(getSpin(UnitStatField::Hp));
	record.strength = static_cast<int16_t>(getSpin(UnitStatField::Strength));
	record.speed = static_cast<int16_t>(getSpin(UnitStatField::Speed));
	record.luck = static_cast<int16_t>(getSpin(UnitStatField::Luck));
	record.defense = static_cast<int16_t>(getSpin(UnitStatField::Defense));
	record.magic = static_cast<int16_t>(getSpin(UnitStatField::Magic));

	record.characterId = static_cast<uint16_t>(m_characterIdSpin->GetValue());
	record.textId = static_cast<uint16_t>(m_textIdSpin->GetValue());
	record.portraitId = static_cast<uint16_t>(m_portraitIdSpin->GetValue());
	record.classId = GetSelectedUnitClassId();

	int mainhandSel = m_mainhandChoice->GetSelection();
	record.mainhandSlot = (mainhandSel <= 0) ? -1 : mainhandSel - 1;
	int offhandSel = m_offhandChoice->GetSelection();
	record.offhandSlot = (offhandSel <= 0) ? -1 : offhandSel - 1;

	for (size_t i = 0; i < WeaponCategories.size(); ++i) {
		record.proficiency[i] = static_cast<uint16_t>(m_proficiencySpin[i]->GetValue());
	}

	record.activeSkillIds.clear();
	for (unsigned int i = 0; i < m_unitSkillsCheckList->GetCount(); ++i) {
		if (m_unitSkillsCheckList->IsChecked(i)) {
			record.activeSkillIds.push_back(m_unitSkillsCheckListToId[i]);
		}
	}

	for (size_t slot = 0; slot < UNIT_INVENTORY_SLOT_COUNT; ++slot) {
		InventorySlot& item = record.inventory[slot];
		int choiceSel = m_inventoryItemChoice[slot]->GetSelection();
		item.itemId = (choiceSel <= 0) ? 0 : m_referenceTables.items[choiceSel - 1].id;
		item.durability = static_cast<uint8_t>(m_inventoryDurabilitySpin[slot]->GetValue());
		item.locked = m_inventoryLockedCheck[slot]->GetValue();
		item.dropped = m_inventoryDroppedCheck[slot]->GetValue();
	}

	bool ok = WriteUnitRecord(m_isoPath.ToStdString(), m_dataOffset, offset, record);

	// Only load if growth exist, avoid byte shifting the disk until proper code is added to handle it.
	if (m_currentGrowthValid) {
		GrowthRecord growth;
		growth.valid = true;
		growth.hp = static_cast<uint16_t>(m_growthStatSpin[static_cast<size_t>(UnitGrowthStatField::Hp)]->GetValue());
		growth.str = static_cast<uint16_t>(m_growthStatSpin[static_cast<size_t>(UnitGrowthStatField::Str)]->GetValue());
		growth.def = static_cast<uint16_t>(m_growthStatSpin[static_cast<size_t>(UnitGrowthStatField::Def)]->GetValue());
		growth.spd = static_cast<uint16_t>(m_growthStatSpin[static_cast<size_t>(UnitGrowthStatField::Spd)]->GetValue());
		growth.mag = static_cast<uint16_t>(m_growthStatSpin[static_cast<size_t>(UnitGrowthStatField::Mag)]->GetValue());
		for (size_t i = 0; i < GrowthProficiencyCategories.size(); ++i) {
			growth.proficiency[i] = static_cast<uint16_t>(m_growthProficiencySpin[i]->GetValue());
		}

		bool growthOk = WriteGrowthRecord(m_isoPath.ToStdString(), m_dataOffset, m_dataSize, m_languageUsed, m_loadedCharacterId, growth);
		ok = ok && growthOk;
	}
	else {
		std::cout << "Growth not saved (character has no real growths)." << std::endl;
	}

	if (ok) {
		SetStatusText("Character saved successfully!!!.");
	}
	else {
		SetStatusText("Failed to save character - open debug console and try again for error log.");
		wxMessageBox("Could not save all character changes.\nOpen debug console (F7) and try again for error log.",
			"ERROR SAVING!!", wxOK | wxICON_ERROR, this);
	}

	// Auto reload portrait in change case
	LoadCharacterIntoPanel(static_cast<size_t>(m_currentCharacterIndex));
}

void MainFrame::LoadItemIntoPanel(int indexInItemsList) {
	if (indexInItemsList < 0 || static_cast<size_t>(indexInItemsList) >= m_referenceTables.items.size()) {
		return;
	}

	if (m_isoPath.IsEmpty()) {
		std::cout << "No ISO loaded yet - not possible to seek item values." << std::endl;
		return;
	}

	m_currentItemIndex = indexInItemsList;
	const ItemDefinition& itemDef = m_referenceTables.items[indexInItemsList];
	uint64_t itemOffset = ComputeItemOffset(m_languageUsed, itemDef.id);

	std::cout << "Loading item '" << itemDef.name << "' (ID 0x" << std::hex << itemDef.id << std::dec
		<< ", offset 0x" << std::hex << itemOffset << std::dec
		<< ", language=" << (m_languageUsed == 0 ? "JP" : "EN") << ")..." << std::endl;

	ItemStats stats = ReadItemStats(m_isoPath.ToStdString(), m_dataOffset, itemOffset);

	std::cout << "  Might=" << stats.might << " Hex=" << stats.hexValue << " Accuracy=" << stats.accuracy
		<< " Weight=" << stats.weight << " MaxRange=" << stats.maxRange << " MinRange=" << stats.minRange
		<< " Crit=" << stats.crit << " Uses=" << stats.uses << " Level=" << stats.level << " Price=" << stats.price
		<< std::endl;
	std::cout << "  Defense=" << stats.defense << " Speed=" << stats.speed << " Avoid=" << stats.avoid
		<< " Hit=" << stats.hit << " Magic=" << stats.magic << " Strength=" << stats.strength
		<< " Rounds=" << stats.rounds << std::endl;
	std::cout << "  FireRes=" << stats.fireRes << " ThunderRes=" << stats.thunderRes << " WindRes=" << stats.windRes
		<< " DarkRes=" << stats.darkRes << " HolyRes=" << stats.holyRes
		<< " CritAvoidPenalty=" << stats.critAvoidPenalty << std::endl;
	std::cout << "  Durability=" << (stats.durabilityIndex < Durability.size() ? Durability[stats.durabilityIndex] : "?")
		<< " EffectRate=" << (stats.effectRateId >= 0 && static_cast<size_t>(stats.effectRateId) < ItemEffectRates.size()
			? ItemEffectRates[stats.effectRateId] : "none")
		<< " (" << stats.effectRateValue << ")" << std::endl;
	std::cout << "  ActiveEffects=" << stats.activeEffectIds.size() << std::endl;

	auto setSpin = [&](ItemNumericField field, int value) {
		if (m_itemStatSpin[static_cast<size_t>(field)]) {
			m_itemStatSpin[static_cast<size_t>(field)]->SetValue(value);
		}
		};

	setSpin(ItemNumericField::Might, stats.might);
	setSpin(ItemNumericField::Hex, stats.hexValue);
	setSpin(ItemNumericField::Accuracy, stats.accuracy);
	setSpin(ItemNumericField::Weight, stats.weight);
	setSpin(ItemNumericField::MaxRange, stats.maxRange);
	setSpin(ItemNumericField::MinRange, stats.minRange);
	setSpin(ItemNumericField::Crit, stats.crit);
	setSpin(ItemNumericField::Uses, stats.uses);
	setSpin(ItemNumericField::Level, stats.level);
	setSpin(ItemNumericField::Price, static_cast<int>(stats.price));
	setSpin(ItemNumericField::Defense, stats.defense);
	setSpin(ItemNumericField::Speed, stats.speed);
	setSpin(ItemNumericField::Avoid, stats.avoid);
	setSpin(ItemNumericField::Hit, stats.hit);
	setSpin(ItemNumericField::Magic, stats.magic);
	setSpin(ItemNumericField::Strength, stats.strength);
	setSpin(ItemNumericField::Rounds, stats.rounds);
	setSpin(ItemNumericField::FireRes, stats.fireRes);
	setSpin(ItemNumericField::ThunderRes, stats.thunderRes);
	setSpin(ItemNumericField::WindRes, stats.windRes);
	setSpin(ItemNumericField::DarkRes, stats.darkRes);
	setSpin(ItemNumericField::HolyRes, stats.holyRes);
	setSpin(ItemNumericField::CritAvoidPenalty, stats.critAvoidPenalty);

	m_durabilityChoice->SetSelection(stats.durabilityIndex < Durability.size() ? static_cast<int>(stats.durabilityIndex) : 0);

	int effectChoiceSelection = 0; // 0 = "(none)"
	for (size_t i = 0; i < m_effectRateChoiceToId.size(); ++i) {
		if (m_effectRateChoiceToId[i] == stats.effectRateId) {
			effectChoiceSelection = static_cast<int>(i);
			break;
		}
	}
	m_effectRateChoice->SetSelection(effectChoiceSelection);
	m_effectRateValueSpin->SetValue(static_cast<int>(stats.effectRateValue));
	m_effectRateValueSpin->Enable(stats.effectRateId >= 0);

	for (size_t i = 0; i < m_itemEffectsCheckListToId.size(); ++i) {
		int realId = m_itemEffectsCheckListToId[i];
		bool isActive = std::find(stats.activeEffectIds.begin(), stats.activeEffectIds.end(), realId) != stats.activeEffectIds.end();
		m_itemEffectsCheckList->Check(static_cast<unsigned int>(i), isActive);
	}

	std::string iconPath = ResolveItemIconPath(itemDef.id);
	wxImage iconImage;
	if (iconImage.LoadFile(iconPath)) {
		iconImage.Rescale(256, 97);
		m_itemIconBitmap->SetBitmap(wxBitmap(iconImage));
	}

	m_itemsDataPanel->Layout();
}

void MainFrame::SaveCurrentItem() {
	if (m_currentItemIndex < 0 || static_cast<size_t>(m_currentItemIndex) >= m_referenceTables.items.size()) {
		std::cout << "No item selected for saving." << std::endl;
		return;
	}
	if (m_isoPath.IsEmpty()) {
		std::cout << "No ISO loaded - not possible to save." << std::endl;
		return;
	}

	const ItemDefinition& itemDef = m_referenceTables.items[m_currentItemIndex];

	ItemStats stats;
	auto getSpin = [&](ItemNumericField field) -> int {
		return m_itemStatSpin[static_cast<size_t>(field)] ? m_itemStatSpin[static_cast<size_t>(field)]->GetValue() : 0;
		};

	stats.might = static_cast<uint16_t>(getSpin(ItemNumericField::Might));
	stats.hexValue = getSpin(ItemNumericField::Hex);
	stats.accuracy = static_cast<uint16_t>(getSpin(ItemNumericField::Accuracy));
	stats.weight = getSpin(ItemNumericField::Weight);
	stats.maxRange = getSpin(ItemNumericField::MaxRange);
	stats.minRange = getSpin(ItemNumericField::MinRange);
	stats.crit = getSpin(ItemNumericField::Crit);
	stats.uses = static_cast<uint16_t>(getSpin(ItemNumericField::Uses));
	stats.level = static_cast<uint16_t>(getSpin(ItemNumericField::Level));
	stats.price = getSpin(ItemNumericField::Price);
	stats.defense = getSpin(ItemNumericField::Defense);
	stats.speed = getSpin(ItemNumericField::Speed);
	stats.avoid = getSpin(ItemNumericField::Avoid);
	stats.hit = getSpin(ItemNumericField::Hit);
	stats.magic = getSpin(ItemNumericField::Magic);
	stats.strength = getSpin(ItemNumericField::Strength);
	stats.rounds = getSpin(ItemNumericField::Rounds);
	stats.fireRes = getSpin(ItemNumericField::FireRes);
	stats.thunderRes = getSpin(ItemNumericField::ThunderRes);
	stats.windRes = getSpin(ItemNumericField::WindRes);
	stats.darkRes = getSpin(ItemNumericField::DarkRes);
	stats.holyRes = getSpin(ItemNumericField::HolyRes);
	stats.critAvoidPenalty = getSpin(ItemNumericField::CritAvoidPenalty);

	stats.durabilityIndex = static_cast<uint16_t>(m_durabilityChoice->GetSelection());

	int effectSelection = m_effectRateChoice->GetSelection();
	stats.effectRateId = (effectSelection >= 0 && effectSelection < static_cast<int>(m_effectRateChoiceToId.size()))
		? m_effectRateChoiceToId[effectSelection] : -1;
	stats.effectRateValue = static_cast<uint16_t>(m_effectRateValueSpin->GetValue());

	for (unsigned int i = 0; i < m_itemEffectsCheckList->GetCount(); ++i) {
		if (m_itemEffectsCheckList->IsChecked(i)) {
			stats.activeEffectIds.push_back(m_itemEffectsCheckListToId[i]);
		}
	}

	bool ok = WriteItemStats(m_isoPath.ToStdString(), m_dataOffset, m_languageUsed, itemDef.id, stats);

	if (ok) {
		SetStatusText(wxString::Format("Item '%s' saved successfully.", itemDef.name));
	}
	else {
		SetStatusText("Failed to save item - open debug console and try again for error log.");
		wxMessageBox("Could not save all item changes.\nOpen debug console (F7) and try again for error log.",
			"ERROR SAVING!!!", wxOK | wxICON_ERROR, this);
	}

	LoadItemIntoPanel(m_currentItemIndex);
}

void MainFrame::OnOpenFile(wxCommandEvent& WXUNUSED(event)) {
	wxFileDialog openDialog(this, "Open Game ISO", "", "",
		"ISO Images (*.iso)|*.iso|All Files (*.*)|*.*",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openDialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	wxString isoPath = openDialog.GetPath();
	SetStatusText("Seeking DATA3.DAT...");

	IsoFileLocation location = FindFileInIso(isoPath.ToStdString(), "DATA3.DAT");

	if (!location.found) {
		std::cout << "DATA3.DAT not found in: " << isoPath.ToStdString() << std::endl;
		SetStatusText("DATA3.DAT not found", 1);
		wxMessageBox("DATA3.DAT not found within this ISO.\n"
			"Verify that the file is a standard ISO9660 image (2048 bytes/sector).",
			"File not found!!!", wxOK | wxICON_ERROR, this);
		return;
	}

	m_isoPath = isoPath;
	m_dataOffset = location.offset;
	m_dataSize = location.sizeBytes;

	std::cout << "DATA3.DAT found in: " << isoPath.ToStdString() << std::endl;
	std::cout << "  LBA: " << location.lba << std::endl;
	std::cout << "  Offset in ISO: " << m_dataOffset << " bytes" << std::endl;
	std::cout << "  Size: " << m_dataSize << " bytes" << std::endl;

	SetStatusText(wxString::Format("DATA3.DAT: %llu bytes | scanning... please wait...", (unsigned long long)m_dataSize), 1);

	int unitsFound = ScanUnitData(isoPath.ToStdString(), m_dataOffset, m_dataSize);

	SetStatusText(wxString::Format("DATA3.DAT: %llu bytes | %d units found",
		(unsigned long long)m_dataSize, unitsFound), 1);

	PopulateUnitDataTree();
}

void MainFrame::OnExit(wxCommandEvent& WXUNUSED(event)) {
	Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	wxMessageBox("Lazberian Construction Set - 1.0 (09/2026)\nBerwick Saga Editor - PS2\n** CREDITS ~\n- JohanMikes (Main programmer, assets making, technical research)\n- LordLouie  (Main beta tester, assets making, fine tuning, technical research)\n- TearRing Saga: Berwick Saga: Lazberia Chronicle Chapter 174 Discord server members (Beta testing)\n** SPECIAL THANKS ~\n- MintX (MinN-11), the original creator of the Berwick saga randomizer tool for allowing his code to be open source.\n- Eugene Pavlyuk (Lightgazer), creator of the extraction tool and original english/german translation software for giving me advice on text entries and game compression.",
		"About", wxOK | wxICON_INFORMATION, this);
}

void MainFrame::OnToggleConsole(wxCommandEvent& WXUNUSED(event)) {
	if (IsDebugConsoleOpen()) {
		CloseDebugConsole();
		SetStatusText("Debug console closed.");
	}
	else {
		OpenDebugConsole();
		std::cout << "Debug console opened." << std::endl;
		SetStatusText("Debug console opened.");
	}
}

void MainFrame::OnUnitDataTreeSelectionChanged(wxTreeEvent& event) {
	wxTreeItemId itemId = event.GetItem();
	if (!itemId.IsOk()) {
		return;
	}

	auto* data = dynamic_cast<UnitDataTreeItemData*>(m_tree->GetItemData(itemId));
	if (!data) {
		return;
	}

	if (m_isoPath.IsEmpty()) {
		return;
	}

	wxString groupName = m_tree->GetItemText(itemId);
	std::cout << "Group '" << groupName.ToStdString() << "' selected (offset 0x"
		<< std::hex << data->address << std::dec << "). Scanning characters..." << std::endl;

	SetStatusText("Scanning group...");
	m_currentGroup = ScanUnitGroup(m_isoPath.ToStdString(), m_dataOffset, m_dataSize, data->address);

	m_characterChoice->Clear();
	for (size_t i = 0; i < m_currentGroup.characters.size(); ++i) {
		UnitRecord record = ReadUnitRecord(m_isoPath.ToStdString(), m_dataOffset, m_currentGroup.characters[i].offset);
		std::string name = m_referenceTables.ResolveName(record.textId, record.classId);
		m_characterChoice->Append(wxString::Format("%zu - %s", i + 1, name));
	}

	std::cout << "Group '" << groupName.ToStdString() << "': " << m_currentGroup.characters.size()
		<< " character(s) found." << std::endl;

	if (!m_currentGroup.characters.empty()) {
		m_characterChoice->SetSelection(0);
		LoadCharacterIntoPanel(0);
	}

	SetStatusText(wxString::Format("%zu character(s) in this group", m_currentGroup.characters.size()));
}

void MainFrame::OnCharacterChoiceChanged(wxCommandEvent& WXUNUSED(event)) {
	int selection = m_characterChoice->GetSelection();
	if (selection == wxNOT_FOUND) {
		return;
	}
	LoadCharacterIntoPanel(static_cast<size_t>(selection));
}

void MainFrame::OnSaveUnitClicked(wxCommandEvent& WXUNUSED(event)) {
	SaveCurrentCharacter();
}

void MainFrame::OnClassChoiceChanged(wxCommandEvent& WXUNUSED(event)) {
	int selection = m_classChoice->GetSelection();
	if (selection == wxNOT_FOUND) {
		return;
	}
	LoadClassIntoPanel(selection);
}

void MainFrame::OnSaveClassClicked(wxCommandEvent& WXUNUSED(event)) {
	SaveCurrentClass();
}

void MainFrame::OnLanguageChanged(wxCommandEvent& event) {
	m_languageUsed = (event.GetId() == ID_LangJapanese) ? 0 : 1;

	std::cout << "Language changed to: " << (m_languageUsed == 0 ? "Japanese (original)" : "English (translation patch)") << std::endl;
	SetStatusText(m_languageUsed == 0 ? "Language: Japanese" : "Language: English (translation patch)");

	// Reload if language change
	if (m_currentItemIndex >= 0) {
		LoadItemIntoPanel(m_currentItemIndex);
	}
	if (m_currentClassIndex >= 0) {
		LoadClassIntoPanel(m_currentClassIndex);
	}
}

void MainFrame::OnItemChoiceChanged(wxCommandEvent& WXUNUSED(event)) {
	int selection = m_itemChoice->GetSelection();
	if (selection == wxNOT_FOUND) {
		return;
	}
	LoadItemIntoPanel(selection);
}

void MainFrame::OnSaveItemClicked(wxCommandEvent& WXUNUSED(event)) {
	SaveCurrentItem();
}

void MainFrame::OnEffectRateChoiceChanged(wxCommandEvent& WXUNUSED(event)) {
	int selection = m_effectRateChoice->GetSelection();
	bool isNone = (selection <= 0); // indice 0 = "(none)"
	m_effectRateValueSpin->Enable(!isNone);
	if (isNone) {
		m_effectRateValueSpin->SetValue(0);
	}
}

void MainFrame::OnExportProject(wxCommandEvent& WXUNUSED(event)) {
	if (m_isoPath.IsEmpty()) {
		wxMessageBox("Load an ISO first.", "No ISO loaded", wxOK | wxICON_WARNING, this);
		return;
	}

	wxFileDialog saveDialog(this, "Export Project", "", "",
		"Lazberian Export Project (*.lep)|*.lep",
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (saveDialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	SetStatusText("Exporting project...");
	bool ok = ExportLepProject(saveDialog.GetPath().ToStdString(), m_isoPath.ToStdString(), m_dataOffset, m_dataSize,
		m_languageUsed, m_referenceTables);

	if (ok) {
		SetStatusText("Project exported successfully.");
	}
	else {
		SetStatusText("Failed to export project - open debug console and try again for error log.");
		wxMessageBox("Could not export the project.\nOpen the debug console (F7) for details and try again for error log.",
			"Error exporting project", wxOK | wxICON_ERROR, this);
	}
}

void MainFrame::OnImportProject(wxCommandEvent& WXUNUSED(event)) {
	if (m_isoPath.IsEmpty()) {
		wxMessageBox("Load an ISO first.", "No ISO loaded", wxOK | wxICON_WARNING, this);
		return;
	}

	wxFileDialog openDialog(this, "Import Project", "", "",
		"Lazberian Export Project (*.lep)|*.lep",
		wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openDialog.ShowModal() == wxID_CANCEL) {
		return;
	}

	int confirm = wxMessageBox("Importing will overwrite unit, class, item and growth data directly on the loaded ISO.\nThis cannot be undone. Continue?",
		"Confirm import", wxYES_NO | wxICON_WARNING, this);
	if (confirm != wxYES) {
		return;
	}

	SetStatusText("Importing project...");
	bool ok = ImportLepProject(openDialog.GetPath().ToStdString(), m_isoPath.ToStdString(), m_dataOffset, m_dataSize);

	if (ok) {
		SetStatusText("Project imported successfully.");
	}
	else {
		SetStatusText("Failed to import project - open debug console and try again for error log.");
		wxMessageBox("Could not import all of the project.\nOpen the debug console (F7) for details and try again for error log.",
			"Error importing project", wxOK | wxICON_ERROR, this);
	}

	if (m_currentCharacterIndex >= 0) {
		LoadCharacterIntoPanel(static_cast<size_t>(m_currentCharacterIndex));
	}
	if (m_currentClassIndex >= 0) {
		LoadClassIntoPanel(m_currentClassIndex);
	}
	if (m_currentItemIndex >= 0) {
		LoadItemIntoPanel(m_currentItemIndex);
	}
}