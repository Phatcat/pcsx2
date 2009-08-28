/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2009  Pcsx2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "PrecompiledHeader.h"
#include "ConfigurationPanels.h"

#include <wx/stdpaths.h>
#include <wx/file.h>
#include <wx/dir.h>

using namespace wxHelpers;

static wxString GetNormalizedConfigFolder( FoldersEnum_t folderId )
{
	const bool isDefault = g_Conf->Folders.IsDefault( folderId );
	wxDirName normalized( isDefault ? PathDefs::Get(folderId) : g_Conf->Folders[folderId] );
	normalized.Normalize( wxPATH_NORM_ALL );
	return normalized.ToString();
}

// Pass me TRUE if the default path is to be used, and the DirPcikerCtrl disabled from use.
void Panels::DirPickerPanel::UpdateCheckStatus( bool someNoteworthyBoolean )
{
	m_pickerCtrl->Enable( !someNoteworthyBoolean );
	if( someNoteworthyBoolean )
	{
		wxDirName normalized( PathDefs::Get( m_FolderId ) );
		normalized.Normalize( wxPATH_NORM_ALL );
		m_pickerCtrl->SetPath( normalized.ToString() );

		wxFileDirPickerEvent event( m_pickerCtrl->GetEventType(), m_pickerCtrl, m_pickerCtrl->GetId(), normalized.ToString() );
		m_pickerCtrl->GetEventHandler()->ProcessEvent(event);
	}
}

void Panels::DirPickerPanel::UseDefaultPath_Click( wxCommandEvent &evt )
{
	evt.Skip();
	wxASSERT( m_pickerCtrl != NULL && m_checkCtrl != NULL );
	UpdateCheckStatus( m_checkCtrl->IsChecked() );
}

void Panels::DirPickerPanel::Explore_Click( wxCommandEvent &evt )
{
	wxHelpers::Explore( m_pickerCtrl->GetPath() );
}

// ------------------------------------------------------------------------
// If initPath is NULL, then it's assumed the default folder is to be used, which is
// obtained from invoking the specified getDefault() function.
//
Panels::DirPickerPanel::DirPickerPanel( wxWindow* parent, FoldersEnum_t folderid, const wxString& label, const wxString& dialogLabel ) :
	BaseApplicableConfigPanel( parent, wxDefaultCoord )
,	m_FolderId( folderid )
,	m_pickerCtrl( NULL )
,	m_checkCtrl( NULL )
{
	wxStaticBoxSizer& s_box( *new wxStaticBoxSizer( wxVERTICAL, this, label ) );
	wxFlexGridSizer& s_lower( *new wxFlexGridSizer( 2, 0, 4 ) );

	s_lower.AddGrowableCol( 1 );

	// Force the Dir Picker to use a text control.  This isn't standard on Linux/GTK but it's much
	// more usable, so to hell with standards.

	wxString normalized( GetNormalizedConfigFolder( m_FolderId ) );

	if( wxFile::Exists( normalized ) )
	{
		// The default path is invalid... What should we do here? hmm..
	}

	if( !wxDir::Exists( normalized ) )
		wxMkdir( normalized );

	m_pickerCtrl = new wxDirPickerCtrl( this, wxID_ANY, wxEmptyString, dialogLabel,
		wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST
	);

	s_box.Add( m_pickerCtrl, wxSizerFlags().Border(wxLEFT | wxRIGHT | wxTOP, 5).Expand() );

	m_checkCtrl = &AddCheckBox( s_lower, _("Use default setting") );
	m_checkCtrl->SetToolTip( pxE( ".Tooltip:DirPicker:UseDefault",
		L"When checked this folder will automatically reflect the default associated with PCSX2's current usermode setting. "
		L"" )
	);

#ifndef __WXGTK__
	// GTK+ : The wx implementation of Explore isn't reliable, so let's not even put the
	// button on the dialogs for now.

	wxButton* b_explore( new wxButton( this, wxID_ANY, _("Open in Explorer") ) );
	b_explore->SetToolTip( _("Open an explorer window to this folder.") );
	s_lower.Add( b_explore, SizerFlags::StdButton().Align( wxALIGN_RIGHT ) );
	Connect( b_explore->GetId(),	wxEVT_COMMAND_BUTTON_CLICKED,	wxCommandEventHandler( DirPickerPanel::Explore_Click ) );
#endif

	s_box.Add( &s_lower, wxSizerFlags().Expand() );

	SetSizer( &s_box );

	// Apply default values
	const bool isDefault = g_Conf->Folders.IsDefault( m_FolderId );
	m_checkCtrl->SetValue( isDefault );
	m_pickerCtrl->Enable( !isDefault );

	Connect( m_checkCtrl->GetId(),	wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler( DirPickerPanel::UseDefaultPath_Click ) );

	// Finally, assign the real value from the config.
	//  (done here because wxGTK fails to init the control when provisioning the initial path
	//   via the contructor)
	m_pickerCtrl->SetPath( GetNormalizedConfigFolder( m_FolderId ) );
}

Panels::DirPickerPanel& Panels::DirPickerPanel::SetStaticDesc( const wxString& msg )
{
	InsertStaticTextAt( this, *GetSizer(), 0, msg );
	//SetSizer( GetSizer(), false );
	return *this;
}

void Panels::DirPickerPanel::Reset()
{
	m_pickerCtrl->SetPath( GetNormalizedConfigFolder( m_FolderId ) );
}

void Panels::DirPickerPanel::Apply( AppConfig& conf )
{
	conf.Folders.Set( m_FolderId, m_pickerCtrl->GetPath(), m_checkCtrl->GetValue() );
}
