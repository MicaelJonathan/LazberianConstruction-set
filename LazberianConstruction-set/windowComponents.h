#pragma once
#include <wx/wx.h>
#include <wx/splitter.h>
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/choice.h>
#include <wx/statbmp.h>
#include <wx/listbox.h>
#include <wx/checklst.h>
#include <wx/spinctrl.h>
#include <wx/statbox.h>
#include <cstdint>
#include <vector>

#include "UnitGroupScanner.h"
#include "ReferenceTables.h"
#include "ItemRecordReader.h"
#include "ItemFieldSpecs.h"
#include "UnitRecordReader.h"
#include "UnitFieldSpecs.h"
#include "GrowthRecordReader.h"
#include "GrowthFieldSpecs.h"
#include "ClassRecordReader.h"
#include "ClassFieldSpecs.h"

/* Main components, add new features as the editor develops. */

class MainFrame : public wxFrame {
public:
	MainFrame();

private:
	wxSplitterWindow* m_splitter = nullptr;
	wxTreeCtrl* m_tree = nullptr;
	wxNotebook* m_notebook = nullptr;
	wxPanel* m_unitDataPanel = nullptr;
	wxPanel* m_itemsDataPanel = nullptr;
	wxPanel* m_classesDataPanel = nullptr;

	wxChoice* m_characterChoice = nullptr;
	wxStaticBitmap* m_portraitBitmap = nullptr;

	enum class UnitStatField { Level, Hp, Strength, Speed, Luck, Defense, Magic, Count };
	wxSpinCtrl* m_unitStatSpin[static_cast<size_t>(UnitStatField::Count)] = {};
	wxChoice* m_mainhandChoice = nullptr; 
	wxChoice* m_offhandChoice = nullptr; 


	wxSpinCtrl* m_characterIdSpin = nullptr;
	wxSpinCtrl* m_textIdSpin = nullptr;
	wxSpinCtrl* m_portraitIdSpin = nullptr;
	wxChoice* m_unitClassIdChoice = nullptr; 
	uint16_t m_unitClassChoiceUnknownId = 0; 


	enum class UnitGrowthStatField { Hp, Str, Def, Spd, Mag, Count };
	wxSpinCtrl* m_growthStatSpin[static_cast<size_t>(UnitGrowthStatField::Count)] = {};
	wxStaticText* m_statsGrowthStatusLabel = nullptr;
	bool m_currentGrowthValid = false;


	wxSpinCtrl* m_proficiencySpin[static_cast<size_t>(WeaponCategory::Count)] = {};

	wxSpinCtrl* m_growthProficiencySpin[static_cast<size_t>(GrowthProficiencyCategory::Count)] = {};
	wxStaticText* m_profGrowthStatusLabel = nullptr;

	wxCheckListBox* m_unitSkillsCheckList = nullptr;
	std::vector<int> m_unitSkillsCheckListToId;

	wxChoice* m_inventoryItemChoice[UNIT_INVENTORY_SLOT_COUNT] = {};
	wxSpinCtrl* m_inventoryDurabilitySpin[UNIT_INVENTORY_SLOT_COUNT] = {};
	wxCheckBox* m_inventoryLockedCheck[UNIT_INVENTORY_SLOT_COUNT] = {};
	wxCheckBox* m_inventoryDroppedCheck[UNIT_INVENTORY_SLOT_COUNT] = {};

	wxButton* m_saveUnitButton = nullptr;
	int m_currentCharacterIndex = -1; 
	uint16_t m_loadedCharacterId = 0; 

// Class Editor
	wxChoice* m_classChoice = nullptr;
	int m_currentClassIndex = -1;


	enum class ClassStatField { Hp, Strength, Speed, Defense, Magic, Move, Experience, Count };
	wxSpinCtrl* m_classStatSpin[static_cast<size_t>(ClassStatField::Count)] = {};
	wxChoice* m_classMountChoice = nullptr;
	wxChoice* m_classTypeChoice = nullptr;
	wxChoice* m_classMovementChoice = nullptr;


	enum class ClassGrowthField { Hp, Strength, Speed, Defense, Magic, Count };
	wxSpinCtrl* m_classGrowthSpin[static_cast<size_t>(ClassGrowthField::Count)] = {};
	wxSpinCtrl* m_classCapSpin[static_cast<size_t>(WeaponCategory::Count)] = {};


	wxCheckListBox* m_classSkillsCheckList = nullptr;
	std::vector<int> m_classSkillsCheckListToId;

	wxButton* m_saveClassButton = nullptr;

// Items Editor
	wxChoice* m_itemChoice = nullptr;
	wxStaticBitmap* m_itemIconBitmap = nullptr;
	wxSpinCtrl* m_itemStatSpin[static_cast<size_t>(ItemNumericField::Count)] = {};
	wxChoice* m_durabilityChoice = nullptr;
	wxChoice* m_effectRateChoice = nullptr;
	std::vector<int> m_effectRateChoiceToId; // -1 = none
	wxSpinCtrl* m_effectRateValueSpin = nullptr;
	wxCheckListBox* m_itemEffectsCheckList = nullptr;
	std::vector<int> m_itemEffectsCheckListToId;
	wxButton* m_saveItemButton = nullptr;
	int m_currentItemIndex = -1;

	wxString m_isoPath;
	uint64_t m_dataOffset = 0; // Treat DATA3.DAT as point 0
	uint64_t m_dataSize = 0;

	// 0 = Japanese original, 1 = Aethin 2.3 patch english (default) 
	int m_languageUsed = 1;

	UnitGroup m_currentGroup;
	ReferenceTables m_referenceTables;

	void CreateMenuBar();
	void CreateStatusBarInfo();
	void CreateLayout();
	void CreateAccelerators();

	wxTreeCtrl* CreateNavigationTree(wxWindow* parent);
	wxNotebook* CreateEditorNotebook(wxWindow* parent);
	wxPanel* CreateUnitDataPanel(wxWindow* parent);
	wxPanel* CreateItemsDataPanel(wxWindow* parent);
	wxPanel* CreateClassesDataPanel(wxWindow* parent);

	void PopulateUnitDataTree();

	void LoadCharacterIntoPanel(size_t indexInGroup);

	void SaveCurrentCharacter();

	void LoadClassIntoPanel(int indexInClassesList);

	void SaveCurrentClass();

	void PopulateItemCatalogChoice(wxChoice* choice);

	int FindItemChoiceIndexForId(uint32_t itemId) const;

	wxString DescribeItemId(uint32_t itemId) const;

	void RebuildUnitClassChoice(uint16_t currentClassId);

	uint16_t GetSelectedUnitClassId() const;

	void LoadItemIntoPanel(int indexInItemsList);

	void SaveCurrentItem();

	// Event Handlers
	void OnOpenFile(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnToggleConsole(wxCommandEvent& event);
	void OnUnitDataTreeSelectionChanged(wxTreeEvent& event);
	void OnCharacterChoiceChanged(wxCommandEvent& event);
	void OnSaveUnitClicked(wxCommandEvent& event);
	void OnClassChoiceChanged(wxCommandEvent& event);
	void OnSaveClassClicked(wxCommandEvent& event);
	void OnLanguageChanged(wxCommandEvent& event);
	void OnItemChoiceChanged(wxCommandEvent& event);
	void OnSaveItemClicked(wxCommandEvent& event);
	void OnEffectRateChoiceChanged(wxCommandEvent& event);

	enum {
		ID_OpenFile = wxID_HIGHEST + 1,
		ID_ToggleConsole,
		ID_LangJapanese,
		ID_LangEnglish,
	};

};