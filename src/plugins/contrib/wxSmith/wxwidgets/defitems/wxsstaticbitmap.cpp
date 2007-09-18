/*
* This file is part of wxSmith plugin for Code::Blocks Studio
* Copyright (C) 2006  Bartlomiej Swiecki
*
* wxSmith is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* wxSmith is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with wxSmith; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*
* $Revision$
* $Id$
* $HeadURL$
*/

#include <wx/statbmp.h>
#include "wxsstaticbitmap.h"

namespace
{
    wxsRegisterItem<wxsStaticBitmap> Reg(_T("StaticBitmap"),wxsTWidget,_T("Standard"),70);

    WXS_ST_BEGIN(wxsStaticBitmapStyles,_T(""))
        WXS_ST_CATEGORY("wxStaticBitmap")
        WXS_ST_DEFAULTS()
    WXS_ST_END()


    WXS_EV_BEGIN(wxsStaticBitmapEvents)
    WXS_EV_END()
}

wxsStaticBitmap::wxsStaticBitmap(wxsItemResData* Data):
    wxsWidget(
        Data,
        &Reg.Info,
        wxsStaticBitmapEvents,
        wxsStaticBitmapStyles)
{}

void wxsStaticBitmap::OnBuildCreatingCode(wxString& Code,const wxString& WindowParent,wxsCodingLang Language)
{
    switch ( Language )
    {
        case wxsCPP:
        {
            wxsSizeData& SizeData = GetBaseProps()->m_Size;
            bool DontResize = SizeData.IsDefault;
            wxString SizeCodeStr = SizeCode(WindowParent,wxsCPP);
            wxString BmpCode = Bitmap.IsEmpty() ? _T("wxNullBitmap") : Bitmap.BuildCode(DontResize,SizeCodeStr,wxsCPP,_T("wxART_OTHER"));

            Code << Codef(Language,_T("%C(%W, %I, %s, %P, %S, %T, %N);\n"),BmpCode.c_str());
            SetupWindowCode(Code,WindowParent,Language);
            return;
        }

        default:
        {
            wxsCodeMarks::Unknown(_T("wxsStaticBitmap::OnBuildCreatingCode"),Language);
        }
    }
}


wxObject* wxsStaticBitmap::OnBuildPreview(wxWindow* Parent,long Flags)
{
    wxStaticBitmap* Preview = new wxStaticBitmap(Parent,GetId(),Bitmap.GetPreview(Size(Parent)),Pos(Parent),Size(Parent),Style());
    return SetupWindow(Preview,Flags);
}

void wxsStaticBitmap::OnEnumWidgetProperties(long Flags)
{
   WXS_BITMAP(wxsStaticBitmap,Bitmap,_("Bitmap"),_T("bitmap"),_T("wxART_OTHER"))
}

void wxsStaticBitmap::OnEnumDeclFiles(wxArrayString& Decl,wxArrayString& Def,wxsCodingLang Language)
{
    switch ( Language )
    {
        case wxsCPP:
        {
            Decl.Add(_T("<wx/statbmp.h>"));
            Def.Add(_T("<wx/bitmap.h>"));
            Def.Add(_T("<wx/image.h>"));
            Def.Add(_T("<wx/artprov.h>"));
            return;
        }

        default:
        {
            wxsCodeMarks::Unknown(_T("wxsStaticBitmap::OnEnumDeclFiles"),Language);
        }
    }
}
