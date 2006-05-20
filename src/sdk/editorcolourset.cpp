/*
* This file is part of Code::Blocks Studio, an open-source cross-platform IDE
* Copyright (C) 2003  Yiannis An. Mandravellos
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact e-mail: Yiannis An. Mandravellos <mandrav@codeblocks.org>
* Program URL   : http://www.codeblocks.org
*
* $Revision$
* $Id$
* $HeadURL$
*/

#include "sdk_precomp.h"

#ifndef CB_PRECOMP
    #include "globals.h"
    #include "cbeditor.h"
    #include "configmanager.h"
    #include "messagemanager.h"
    #include "manager.h"
    #include <wx/log.h>
    #include <wx/dir.h>
#endif

#include "editorcolourset.h"
#include "editorlexerloader.h"
#include "filefilters.h"

const int cbHIGHLIGHT_LINE = -98; // highlight line under caret virtual style
const int cbSELECTION      = -99; // selection virtual style

EditorColourSet::EditorColourSet(const wxString& setName)
	: m_Name(setName)
{
	LoadAvailableSets();

	if (setName.IsEmpty())
		m_Name = COLORSET_DEFAULT;
	else
		Load();
}

EditorColourSet::EditorColourSet(const EditorColourSet& other) // copy ctor
{
	m_Name = other.m_Name;
	m_Sets.clear();

	for (OptionSetsMap::const_iterator it = other.m_Sets.begin(); it != other.m_Sets.end(); ++it)
	{
	    OptionSet& mset = m_Sets[it->first];

		mset.m_Langs = it->second.m_Langs;
		mset.m_Lexers = it->second.m_Lexers;
		for (int i = 0; i <= wxSCI_KEYWORDSET_MAX; ++i)
		{
            mset.m_Keywords[i] = it->second.m_Keywords[i];
		}
		mset.m_FileMasks = it->second.m_FileMasks;
		mset.m_SampleCode = it->second.m_SampleCode;
		mset.m_BreakLine = it->second.m_BreakLine;
		mset.m_DebugLine = it->second.m_DebugLine;
		mset.m_ErrorLine = it->second.m_ErrorLine;
		const OptionColours& value = it->second.m_Colours;
		for (unsigned int i = 0; i < value.GetCount(); ++i)
		{
			AddOption(it->first, value[i]);
        }
	}
}

EditorColourSet::~EditorColourSet()
{
	ClearAllOptionColours();
}

void EditorColourSet::ClearAllOptionColours()
{
       for (OptionSetsMap::iterator map_it = m_Sets.begin();
                                                       map_it != m_Sets.end(); ++map_it)
        for (OptionColours::iterator vec_it = (*map_it).second.m_Colours.begin();
                            vec_it != (*map_it).second.m_Colours.end(); ++vec_it)
                       delete (*vec_it);
    m_Sets.clear();
}

void EditorColourSet::LoadAvailableSets()
{
	wxString path = ConfigManager::GetDataFolder() + _T("/lexers");
    wxDir dir(path);

    if (!dir.IsOpened())
        return;

	EditorLexerLoader lex(this);
    wxString filename;
    bool ok = dir.GetFirst(&filename, _T("lexer_*.xml"), wxDIR_FILES);
    while (ok)
    {
        lex.Load(path + _T("/") + filename);
        ok = dir.GetNext(&filename);
    }

	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		wxString lang = it->second.m_Langs;
		if (lang.IsEmpty())
            continue;

        // keep the original filemasks and keywords, so we know what needs saving later
        for (int i = 0; i <= wxSCI_KEYWORDSET_MAX; ++i)
        {
            it->second.m_originalKeywords[i] = it->second.m_Keywords[i];
        }
        it->second.m_originalFileMasks = it->second.m_FileMasks;

        // remove old settings, no longer used
		unsigned int i = 0;
		while (i < it->second.m_Colours.GetCount())
		{
			OptionColour* opt = it->second.m_Colours.Item(i);
			// valid values are:
			if (opt->value < 0 &&               // styles >= 0
                opt->value != cbSELECTION &&    // cbSELECTION
                opt->value != cbHIGHLIGHT_LINE) // cbHIGHLIGHT_LINE
            {
                it->second.m_Colours.Remove(opt);
                delete opt;
            }
            else
                ++i;
		}
	}
}

HighlightLanguage EditorColourSet::AddHighlightLanguage(int lexer, const wxString& name)
{
	if (lexer <= wxSCI_LEX_NULL ||
        lexer > wxSCI_LEX_FREEBASIC ||
        name.IsEmpty())
    {
        return HL_NONE;
    }

    // fix name to be XML compliant
    wxString newID;
    size_t pos = 0;
    while (pos < name.Length())
    {
        wxChar ch = name[pos];
        if (wxIsalnum(ch) || ch == _T('_'))
        {
            // valid character
            newID.Append(ch);
        }
        else if (wxIsspace(ch))
        {
            // convert spaces to underscores
            newID.Append(_T('_'));
        }
        ++pos;
    }
    // make sure it's not starting with a number or underscore.
    // if it is, prepend an 'A'
    if (wxIsdigit(newID.GetChar(0)) || newID.GetChar(0) == _T('_'))
        newID.Prepend(_T('A'));

    if (GetHighlightLanguage(newID) != HL_NONE)
    {
        return HL_NONE;
    }

    m_Sets[newID].m_Langs = name;
    m_Sets[newID].m_Lexers = lexer;
    return newID;
}

HighlightLanguage EditorColourSet::GetHighlightLanguage(const wxString& name)
{
	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		if (it->second.m_Langs.CmpNoCase(name) == 0)
            return it->first;
	}
	return HL_NONE;
}

// from scintilla lexer (wxSCI_LEX_*)
// Warning: the first one found is returned!
HighlightLanguage EditorColourSet::GetHighlightLanguage(int lexer)
{
	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		if (it->second.m_Lexers == lexer)
            return it->first;
	}
	return HL_NONE;
}

wxArrayString EditorColourSet::GetAllHighlightLanguages()
{
	wxArrayString ret;
	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		if (!it->second.m_Langs.IsEmpty())
            ret.Add(it->second.m_Langs);
	}
	ret.Sort();
	return ret;
}

void EditorColourSet::UpdateOptionsWithSameName(HighlightLanguage lang, OptionColour* base)
{
    if (!base)
        return;
    // first find the index of this option
    int idx = -1;
    OptionSet& mset = m_Sets[lang];
	for (unsigned int i = 0; i < mset.m_Colours.GetCount(); ++i)
	{
		OptionColour* opt = mset.m_Colours.Item(i);
		if (opt == base)
		{
			idx = i;
			break;
		}
	}
	if (idx == -1)
        return;

    // now loop again, but update the other options with the same name
	for (unsigned int i = 0; i < mset.m_Colours.GetCount(); ++i)
	{
		if ((int)i == idx)
            continue; // skip the base option
		OptionColour* opt = mset.m_Colours.Item(i);
		if (!opt->name.Matches(base->name))
            continue;
        opt->fore = base->fore;
        opt->back = base->back;
        opt->bold = base->bold;
        opt->italics = base->italics;
        opt->underlined = base->underlined;
	}
}

bool EditorColourSet::AddOption(HighlightLanguage lang, OptionColour* option, bool checkIfExists)
{
    if (lang == HL_NONE)
        return false;
	if (checkIfExists && GetOptionByValue(lang, option->value))
        return false;

    option->originalfore = option->fore;
    option->originalback = option->back;
    option->originalbold = option->bold;
    option->originalitalics = option->italics;
    option->originalunderlined = option->underlined;
    option->originalisStyle = option->isStyle;

	OptionColours& colours =  m_Sets[lang].m_Colours;
	colours.Add(new OptionColour(*option));
	return true;
}

void EditorColourSet::AddOption(HighlightLanguage lang,
								const wxString& name,
								int value,
								wxColour fore,
								wxColour back,
								bool bold,
								bool italics,
								bool underlined,
								bool isStyle)
{
    if (lang == HL_NONE)
        return;
	OptionColour* opt = new OptionColour;
	opt->name = name;
	opt->value = value;
	opt->fore = fore;
	opt->back = back;
	opt->bold = bold;
	opt->italics = italics;
	opt->underlined = underlined;
	opt->isStyle = isStyle;

	AddOption(lang, opt);
    delete opt;
}

OptionColour* EditorColourSet::GetOptionByName(HighlightLanguage lang, const wxString& name)
{
    if (lang == HL_NONE)
        return 0L;
    OptionSet& mset = m_Sets[lang];
	for (unsigned int i = 0; i < mset.m_Colours.GetCount(); ++i)
	{
		OptionColour* opt = mset.m_Colours.Item(i);
		if (opt->name == name)
			return opt;
	}
	return 0L;
}

OptionColour* EditorColourSet::GetOptionByValue(HighlightLanguage lang, int value)
{
    if (lang == HL_NONE)
        return 0L;
    OptionSet& mset = m_Sets[lang];
	for (unsigned int i = 0; i < mset.m_Colours.GetCount(); ++i)
	{
		OptionColour* opt = mset.m_Colours.Item(i);
		if (opt->value == value)
			return opt;
	}
	return 0L;
}

OptionColour* EditorColourSet::GetOptionByIndex(HighlightLanguage lang, int index)
{
    if (lang == HL_NONE)
        return 0L;
	return m_Sets[lang].m_Colours.Item(index);
}

int EditorColourSet::GetOptionCount(HighlightLanguage lang)
{
    return m_Sets[lang].m_Colours.GetCount();
}

HighlightLanguage EditorColourSet::GetLanguageForFilename(const wxString& filename)
{
    // convert filename to lowercase first (m_FileMasks already contains
    // lowercase-only strings)
    wxString lfname = filename.Lower();

	// first search in filemasks
	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		for (unsigned int x = 0; x < it->second.m_FileMasks.GetCount(); ++x)
		{
			if (lfname.Matches(it->second.m_FileMasks.Item(x)))
                return it->first;
		}
	}
    return HL_NONE;
}

wxString EditorColourSet::GetLanguageName(HighlightLanguage lang)
{
    if (lang == HL_NONE)
        return _("Unknown");
	wxString name = m_Sets[lang].m_Langs;
    if (!name.IsEmpty())
        return name;
    return _("Unknown");
}

void EditorColourSet::DoApplyStyle(cbStyledTextCtrl* control, int value, OptionColour* option)
{
	// option->value is ignored here...
	// value is used instead
	if (option->fore != wxNullColour)
        control->StyleSetForeground(value, option->fore);
	if (option->back != wxNullColour)
        control->StyleSetBackground(value, option->back);
	control->StyleSetBold(value, option->bold);
	control->StyleSetItalic(value, option->italics);
	control->StyleSetUnderline(value, option->underlined);
}

HighlightLanguage EditorColourSet::Apply(cbEditor* editor, HighlightLanguage lang)
{
	if (!editor)
		return HL_NONE;

    if (lang == HL_AUTO)
        lang = GetLanguageForFilename(editor->GetFilename());

	Apply(lang, editor->GetControl());

	return lang;
}


void EditorColourSet::Apply(HighlightLanguage lang, cbStyledTextCtrl* control)
{
    if (lang == HL_NONE || !control)
        return;
	control->StyleClearAll();

	if (lang == HL_NONE)
        return;

    // first load the default colours to all styles (ignoring some built-in styles)
    OptionColour* defaults = GetOptionByName(lang, _("Default"));
    if (defaults)
    {
        for (int i = 0; i < wxSCI_STYLE_MAX; ++i)
        {
            if (i < 33 || i > 39)
                DoApplyStyle(control, i, defaults);
        }
    }
	// for some strange reason, when switching styles, the line numbering changes colour
	// too, though we didn't ask it to...
	// this makes sure it stays the correct colour
    control->StyleSetForeground(wxSCI_STYLE_LINENUMBER, wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));

    OptionSet& mset = m_Sets[lang];
	for (unsigned int i = 0; i < mset.m_Colours.GetCount(); ++i)
	{
		OptionColour* opt = mset.m_Colours.Item(i);

		if (opt->isStyle)
		{
			DoApplyStyle(control, opt->value, opt);
		}
		else
		{
            if (opt->value == cbHIGHLIGHT_LINE)
            {
                control->SetCaretLineBackground(opt->back);
                Manager::Get()->GetConfigManager(_T("editor"))->Write(_T("/highlight_caret_line_colour"), opt->back);
            }
            else if (opt->value == cbSELECTION)
            {
                if (opt->back != wxNullColour)
                {
                    control->SetSelBackground(true, opt->back);
                    Manager::Get()->GetConfigManager(_T("editor"))->Write(_T("/selection_colour"), opt->back);
                }
                else
                    control->SetSelBackground(false, wxColour(0xC0, 0xC0, 0xC0));
            }
//            else
//            {
//                control->MarkerDefine(-opt->value, 1);
//                control->MarkerSetBackground(-opt->value, opt->back);
//            }
		}
	}
	control->SetLexer(mset.m_Lexers);
	for (int i = 0; i <= wxSCI_KEYWORDSET_MAX; ++i)
	{
        control->SetKeyWords(i, mset.m_Keywords[i]);
	}
    control->Colourise(0, -1); // the *most* important part!
}

void EditorColourSet::Save()
{
	wxString key;
	ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));

    //FIXME: Commenting out the following line is no definite cure, but it hides the annoying disappearing colourset for now
	//cfg->DeleteSubPath(_T("/colour_sets/") + m_Name);

	// write the theme name
	cfg->Write(_T("/colour_sets/") + m_Name + _T("/name"), m_Name);

	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		if (it->first == HL_NONE || it->first == HL_AUTO)
            continue;
		wxString lang = it->first;

        bool gsaved = false;

        key.Clear();
		key << _T("/colour_sets/") << m_Name << _T('/') << lang;
		for (unsigned int i = 0; i < it->second.m_Colours.GetCount(); ++i)
		{
			OptionColour* opt = it->second.m_Colours.Item(i);
			wxString tmpKey;
			tmpKey << key << _T("/style") << wxString::Format(_T("%d"), i);

            bool saved = false;

			if (opt->fore != opt->originalfore && opt->fore != wxNullColour)
			{
                cfg->Write(tmpKey + _T("/fore"), opt->fore);
                saved = true;
			}
			if (opt->back != opt->originalback && opt->back != wxNullColour)
			{
                cfg->Write(tmpKey + _T("/back"), opt->back);
                saved = true;
			}
            if (opt->bold != opt->originalbold)
			{
                cfg->Write(tmpKey + _T("/bold"),       opt->bold);
                saved = true;
			}
            if (opt->italics != opt->originalitalics)
			{
                cfg->Write(tmpKey + _T("/italics"),    opt->italics);
                saved = true;
			}
            if (opt->underlined != opt->originalunderlined)
			{
                cfg->Write(tmpKey + _T("/underlined"), opt->underlined);
                saved = true;
			}
            if (opt->isStyle != opt->originalisStyle)
			{
                cfg->Write(tmpKey + _T("/isStyle"),    opt->isStyle);
                saved = true;
			}

            if (saved)
            {
                cfg->Write(tmpKey + _T("/name"), opt->name, true);
                gsaved = true;
            }
		}
        wxString tmpkey;
		for (int i = 0; i <= wxSCI_KEYWORDSET_MAX; ++i)
		{
		    if (it->second.m_Keywords[i] != it->second.m_originalKeywords[i])
		    {
                tmpkey.Printf(_T("%s/editor/keywords/set%d"), key.c_str(), i);
                cfg->Write(tmpkey, it->second.m_Keywords[i]);
                gsaved = true;
		    }
		}
        tmpkey.Printf(_T("%s/editor/filemasks"), key.c_str());
        wxString tmparr = GetStringFromArray(it->second.m_FileMasks, _T(","));
        wxString tmparrorig = GetStringFromArray(it->second.m_originalFileMasks, _T(","));
        if (tmparr != tmparrorig)
        {
            cfg->Write(tmpkey, tmparr);
            gsaved = true;
        }

        if (gsaved)
            cfg->Write(key + _T("/name"), it->second.m_Langs);
	}
}

void EditorColourSet::Load()
{
    static bool s_notifiedUser = false;

	wxString key;
	ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("editor"));

	// read the theme name
    m_Name = cfg->Read(_T("/colour_sets/") + m_Name + _T("/name"), m_Name);

    int x = 0;
	for (OptionSetsMap::iterator it = m_Sets.begin(); it != m_Sets.end(); ++it)
	{
		if (it->first == HL_NONE || it->first == HL_AUTO)
            continue;

		// look for old-style configuration
		key.Clear();
		key << _T("/colour_sets/") << m_Name << _T("/set") << wxString::Format(_T("%d"), x++);
		if (cfg->Exists(key + _T("/name")))
		{
		    // old-style configuration
		    // delete it and tell the user about it
		    cfg->DeleteSubPath(key);
		    if (!s_notifiedUser)
		    {
		        cbMessageBox(_("The way editor syntax highlighting configuration is saved, has changed.\n"
                                "Syntax highlighting for all supported languages will revert to defaults now.\n"
                                "We 're sorry for the inconvenience..."),
                                _("Information"),
                                wxICON_INFORMATION);
		        s_notifiedUser = true;
		    }
		    continue;
		}
		// make sure we didn't create it accidentally
        cfg->DeleteSubPath(key);

		// new-style configuration key
		key.Clear();
		key << _T("/colour_sets/") << m_Name << _T('/') << it->first;
		if (!cfg->Exists(key + _T("/name")))
		{
            // make sure we didn't create it accidentally
            cfg->DeleteSubPath(key);
			continue;
		}

		for (unsigned int i = 0; i < it->second.m_Colours.GetCount(); ++i)
		{
			OptionColour* opt = it->second.m_Colours.Item(i);
			if (!opt)
                continue;
			wxString tmpKey;
			tmpKey << key << _T("/style") << wxString::Format(_T("%d"), i);

			if (cfg->Exists(tmpKey + _T("/name")))
                opt->name = cfg->Read(tmpKey + _T("/name"));
            else
            {
                // make sure we didn't create it accidentally
                cfg->DeleteSubPath(tmpKey);
                continue;
            }

			if (cfg->Exists(tmpKey + _T("/fore")))
                opt->fore = cfg->ReadColour(tmpKey + _T("/fore"), opt->fore);
			if (cfg->Exists(tmpKey + _T("/back")))
                opt->back = cfg->ReadColour(tmpKey + _T("/back"), opt->back);
			if (cfg->Exists(tmpKey + _T("/bold")))
                opt->bold = cfg->ReadBool(tmpKey + _T("/bold"), opt->bold);
			if (cfg->Exists(tmpKey + _T("/italics")))
                opt->italics = cfg->ReadBool(tmpKey + _T("/italics"), opt->italics);
			if (cfg->Exists(tmpKey + _T("/underlined")))
                opt->underlined = cfg->ReadBool(tmpKey + _T("/underlined"), opt->underlined);

			if (cfg->Exists(tmpKey + _T("/isStyle")))
                opt->isStyle = cfg->ReadBool(tmpKey + _T("/isStyle"), opt->isStyle);
		}
        wxString tmpkey;
        for (int i = 0; i <= wxSCI_KEYWORDSET_MAX; ++i)
        {
            tmpkey.Printf(_T("%s/editor/keywords/set%d"), key.c_str(), i);
            if (cfg->Exists(tmpkey))
                it->second.m_Keywords[i] = cfg->Read(tmpkey, wxEmptyString);
        }
        tmpkey.Printf(_T("%s/editor/filemasks"), key.c_str());
        if (cfg->Exists(tmpkey))
            it->second.m_FileMasks = GetArrayFromString(cfg->Read(tmpkey, wxEmptyString), _T(","));
	}
}

void EditorColourSet::Reset(HighlightLanguage lang)
{
    wxLogNull ln;
    wxString key;
    key << _T("/colour_sets/") << m_Name << _T('/') << lang;
    if (Manager::Get()->GetConfigManager(_T("editor"))->Exists(key + _T("name")))
        Manager::Get()->GetConfigManager(_T("editor"))->DeleteSubPath(key);

    ClearAllOptionColours();
    LoadAvailableSets();
    Load();
}

wxString& EditorColourSet::GetKeywords(HighlightLanguage lang, int idx)
{
    if (idx < 0 || idx > wxSCI_KEYWORDSET_MAX)
        idx = 0;
    return m_Sets[lang].m_Keywords[idx];
}

void EditorColourSet::SetKeywords(HighlightLanguage lang, int idx, const wxString& keywords)
{
    if (lang != HL_NONE && idx >=0 && idx <= wxSCI_KEYWORDSET_MAX)
    {
        wxString tmp(_T(' '), keywords.length()); // faster than using Alloc()

        const wxChar *src = keywords.c_str();
        wxChar *dst = (wxChar *) tmp.c_str();
        wxChar c;
        size_t len = 0;

        while(c = *src)
        {
            ++src;
            if(c > _T(' '))
            {
                *dst = c;
            }
            else // white space
            {
                *dst = _T(' ');
                while(*src && *src < _T(' '))
                    ++src;
            }

            ++dst;
            ++len;
        }

        tmp.Truncate(len);

        OptionSet& mset = m_Sets[lang];
        mset.m_Keywords[idx] = tmp;
    }
}

const wxArrayString& EditorColourSet::GetFileMasks(HighlightLanguage lang)
{
	return m_Sets[lang].m_FileMasks;
}

void EditorColourSet::SetFileMasks(HighlightLanguage lang, const wxString& masks, const wxString& separator)
{
    if (lang != HL_NONE)
    {
        m_Sets[lang].m_FileMasks = GetArrayFromString(masks.Lower(), separator);

        // let's add these filemasks in the file filters master list ;)
        FileFilters::Add(wxString::Format(_("%s files"), m_Sets[lang].m_Langs.c_str()), masks);
    }
}

wxString EditorColourSet::GetSampleCode(HighlightLanguage lang, int* breakLine, int* debugLine, int* errorLine)
{
	if (lang == HL_NONE)
        return wxEmptyString;
    OptionSet& mset = m_Sets[lang];
    if (breakLine)
        *breakLine = mset.m_BreakLine;
    if (debugLine)
        *debugLine = mset.m_DebugLine;
    if (errorLine)
        *errorLine = mset.m_ErrorLine;
	wxString path = ConfigManager::GetDataFolder() + _T("/lexers/");
    if (!mset.m_SampleCode.IsEmpty())
        return path + mset.m_SampleCode;
    return wxEmptyString;
}

void EditorColourSet::SetSampleCode(HighlightLanguage lang, const wxString& sample, int breakLine, int debugLine, int errorLine)
{
	if (lang == HL_NONE)
        return;
    OptionSet& mset = m_Sets[lang];
    mset.m_SampleCode = sample;
    mset.m_BreakLine = breakLine;
    mset.m_DebugLine = debugLine;
    mset.m_ErrorLine = errorLine;
}
