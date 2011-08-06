#include "CastorShape/PrecompiledHeader.hpp"

#include "CastorShape/geometry/NewGeometryDialog.hpp"
#include "CastorShape/material/NewMaterialDialog.hpp"

using namespace Castor3D;
using namespace CastorShape;

NewGeometryDialog :: NewGeometryDialog( wxWindow * parent, wxWindowID p_id,
										const wxString & p_name,
										const wxPoint & pos, const wxSize & size,
										long style)
	:	wxDialog( parent, p_id, p_name, pos, size, style, p_name)
	,	m_actualY( 10)
{
}

NewGeometryDialog :: ~NewGeometryDialog()
{
}

void NewGeometryDialog :: AddTextCtrl( const wxString & p_name,
										 const wxString & p_defaultValue,
										 int p_width)
{
	if (m_textCtrls.find( p_name) != m_textCtrls.end())
	{
		return;
	}

	wxSize l_size = GetClientSize() - wxSize( 30 + p_width, 0);
	_updateY( 30);
	int l_width = l_size.x;
	new wxStaticText( this, wxID_ANY, p_name + cuT( " : "), wxPoint( 10, m_actualY - 30), wxSize( l_width, 20));
	wxTextCtrl * l_edit = new wxTextCtrl( this, wxID_ANY, p_defaultValue,
										  wxPoint( 80, m_actualY - 30), wxSize( p_width, 20),
										  wxBORDER_SIMPLE);
	m_textCtrls.insert( TextCtrlPtrStrMap::value_type( p_name, l_edit));
}

void NewGeometryDialog :: AddCubeBox( const wxString & p_name,
									  const wxArrayString & p_values,
									  const wxString & p_defaultValue,
									  wxWindowID p_id, int p_width)
{
	if (m_comboBoxes.find( p_name) != m_comboBoxes.end())
	{
		return;
	}

	wxSize l_size = GetClientSize() - wxSize( 30 + p_width, 0);
	_updateY( 30);
	int l_width = l_size.x;
	new wxStaticText( this, wxID_ANY, p_name + cuT( " : "), wxPoint( 10, m_actualY - 30), wxSize( l_width, 20));
	wxComboBox * l_box = new wxComboBox( this, p_id, p_defaultValue,
										 wxPoint( 80, m_actualY - 30), wxSize( p_width, 20),
										 p_values, wxBORDER_SIMPLE | wxCB_READONLY);
	m_comboBoxes.insert( CubeBoxPtrStrMap::value_type( p_name, l_box));
}

wxString NewGeometryDialog :: GetTextValue( const wxString & p_name)const
{
	if (m_textCtrls.find( p_name) != m_textCtrls.end())
	{
		return m_textCtrls.find( p_name)->second->GetValue();
	}
	return wxEmptyString;
}

wxString NewGeometryDialog :: GetCBValue( const wxString & p_name)const
{
	if (m_comboBoxes.find( p_name) != m_comboBoxes.end())
	{
		return m_comboBoxes.find( p_name)->second->GetValue();
	}
	return wxEmptyString;
}

void NewGeometryDialog :: RemoveTextCtrl( const wxString & p_name)
{
	TextCtrlPtrStrMap::iterator l_it = m_textCtrls.find( p_name);
	RemoveChild( l_it->second);
	delete l_it->second;
	m_textCtrls.erase( l_it);
}

void NewGeometryDialog :: RemoveCubeBox( const wxString & p_name)
{
	CubeBoxPtrStrMap::iterator l_it = m_comboBoxes.find( p_name);
	RemoveChild( l_it->second);
	delete l_it->second;
	m_comboBoxes.erase( l_it);
}

String NewGeometryDialog :: GetGeometryName()const
{
	return String( (const char *)GetTextValue( cuT( "Name")).c_str());
}

String NewGeometryDialog :: GetMaterialName()const
{
	wxString l_value = GetCBValue( cuT( "Material"));
	String l_res = (const char *)l_value.c_str();
	NewMaterialDialog * l_dialog;
	MaterialPtr l_material;

	while (l_res == cuT( "New..."))
	{
		l_material.reset();
		l_dialog = new NewMaterialDialog( NULL, wxID_ANY);

		if (l_dialog->ShowModal() == wxID_OK)
		{
			l_material = l_dialog->GetMaterial();

			if (l_material)
			{
				l_res = l_material->GetName();
			}
		}
		l_dialog->Destroy();
	}
	return l_res;
}

void NewGeometryDialog :: _updateY( int p_offset)
{
	SetSize( GetSize() + wxSize( 0, p_offset));
	m_actualY += p_offset;
}

void NewGeometryDialog :: _endInit()
{
	m_okButton = new wxButton( this, eOK, cuT( "OK"), wxPoint( 70, GetClientSize().y - 35), wxSize( 60, 30), wxBORDER_SIMPLE);
}

void NewGeometryDialog :: _defaultInit()
{
	AddTextCtrl( cuT( "Name"), cuT( "Geometry Name"), 110);

	StringArray l_choices1;
	Root::GetSingleton()->GetMaterialNames( l_choices1);
	wxArrayString l_choices;
	l_choices.push_back( cuT( "New..."));

	for (size_t i = 0 ; i < l_choices1.size() ; i++)
	{
		l_choices.push_back( wxString( l_choices1[i].c_str()));
	}

	AddCubeBox( cuT( "Material"), l_choices, cuT( "New..."), eMaterialName, 110);
}

BEGIN_EVENT_TABLE( NewGeometryDialog, wxDialog)
	EVT_BUTTON(		eOK,			NewGeometryDialog::_onOk)
	EVT_COMBOBOX(	eMaterialName,	NewGeometryDialog::_onMaterialSelect)
END_EVENT_TABLE()

void NewGeometryDialog :: _onOk(wxCommandEvent& event)
{
	EndModal( wxID_OK);
}

void NewGeometryDialog :: _onMaterialSelect( wxCommandEvent& event)
{
/*
	wxString l_value = GetCBValue( cuT( "Material"));
	wxString l_res;
	if (l_value == cuT( "New..."))
	{
		NewMaterialDialog * l_dialog = new NewMaterialDialog( nullptr, wxID_ANY);
		MaterialPtr l_material = nullptr;
		if (l_dialog->ShowModal() == wxID_OK)
		{
			l_material = l_dialog->GetMaterial();
		}
		l_dialog->Destroy();

		if (l_material)
		{
			C3DVector <String> l_choices1;
			Root::GetSingletonPtr()->GetMaterialManager()->GetMaterialNames( l_choices1);

			wxArrayString l_choices;
			l_choices.push_back( cuT( "New..."));
			for (size_t i = 0 ; i < l_choices1.size() ; i++)
			{
				l_choices.push_back( l_choices1[i].c_str());
			}
			l_res = l_material->GetName();

			CubeBoxPtrStrMap::iterator l_it = m_comboBoxes.find( cuT( "Material"));
			RemoveChild( l_it->second);
			delete l_it->second;
			l_it->second = new wxComboBox( this, ngpMaterialName, l_res, wxPoint( 80, 40), wxSize( 110, 20), l_choices, wxBORDER_SIMPLE | wxCB_READONLY);
		}
	}
*/
}