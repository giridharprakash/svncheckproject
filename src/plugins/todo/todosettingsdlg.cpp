#include <sdk.h>
#include "todosettingsdlg.h"
#include <wx/xrc/xmlres.h>
#include <wx/checkbox.h>
#include <old_configmanager.h>

ToDoSettingsDlg::ToDoSettingsDlg()
{
	//ctor
	wxXmlResource::Get()->LoadDialog(this, 0L, _T("ToDoSettingsDlg"));
	bool checked = OldConfigManager::Get()->Read(_T("todo_list/auto_refresh"), true);
	XRCCTRL(*this, "chkAutoRefresh", wxCheckBox)->SetValue(checked);
}

ToDoSettingsDlg::~ToDoSettingsDlg()
{
	//dtor
}

void ToDoSettingsDlg::EndModal(int retCode)
{
    if (retCode == wxID_OK)
    {
        bool checked = XRCCTRL(*this, "chkAutoRefresh", wxCheckBox)->GetValue();
        OldConfigManager::Get()->Write(_T("todo_list/auto_refresh"), checked);
    }

    wxDialog::EndModal(retCode);
}
