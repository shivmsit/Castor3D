#include "Castor3D/PrecompiledHeader.h"
#include "Castor3D/material/Pass.h"

#include "Castor3D/material/TextureUnit.h"
#include "Castor3D/shader/ShaderProgram.h"
#include "Castor3D/render_system/RenderSystem.h"
#include "Castor3D/main/Root.h"
#include "Castor3D/material/Material.h"

using namespace Castor3D;

Pass :: Pass( Material * p_parent)
	:	m_fShininess	( 50.0),
		m_bDoubleFace	( false),
		m_pParent		( p_parent),
		m_clrDiffuse	( 1, 1, 1, 1),
		m_clrAmbient	( 0, 0, 0, 1),
		m_clrSpecular	( 1, 1, 1, 1),
		m_clrEmissive	( 0, 0, 0, 1)
{
}

Pass :: Pass( const Pass & p_copy)
	:	m_clrDiffuse(		p_copy.m_clrDiffuse),
		m_clrAmbient(		p_copy.m_clrAmbient),
		m_clrSpecular(		p_copy.m_clrSpecular),
		m_clrEmissive(		p_copy.m_clrEmissive),
		m_fShininess(		p_copy.m_fShininess),
		m_fAlpha(			p_copy.m_fAlpha),
		m_clrTexBaseColour(	p_copy.m_clrTexBaseColour),
		m_bDoubleFace(		p_copy.m_bDoubleFace),
		m_pShaderProgram(	p_copy.m_pShaderProgram),
		m_pParent(			p_copy.m_pParent)
{
	for (size_t i = 0 ; i < p_copy.m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i] = p_copy.m_arrayTextureUnits[i];
	}

//	m_pRenderer = p_copy.m_pRenderer;
}

Pass & Pass :: operator =( const Pass & p_copy)
{
	m_clrDiffuse		= p_copy.m_clrDiffuse;
	m_clrAmbient		= p_copy.m_clrAmbient;
	m_clrSpecular		= p_copy.m_clrSpecular;
	m_clrEmissive		= p_copy.m_clrEmissive;
	m_fShininess		= p_copy.m_fShininess;
	m_fAlpha			= p_copy.m_fAlpha;
	m_clrTexBaseColour	= p_copy.m_clrTexBaseColour;
	m_bDoubleFace		= p_copy.m_bDoubleFace;
	m_pShaderProgram	= p_copy.m_pShaderProgram;
	m_pParent			= p_copy.m_pParent;

	for (size_t i = 0 ; i < p_copy.m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i] = p_copy.m_arrayTextureUnits[i];
	}

//	m_pRenderer = p_copy.m_pRenderer;

	return * this;
}

Pass :: ~Pass()
{
	m_arrayTextureUnits.clear();
}

void Pass :: Initialise()
{
	for (size_t i = 0 ; i < m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i]->Initialise();
	}

	m_pRenderer->Initialise();
}

void Pass :: Render( eDRAW_TYPE p_eDisplayMode)
{
	if ( ! m_pShaderProgram.expired())
	{
		ShaderProgramPtr l_pShader = m_pShaderProgram.lock();
		l_pShader->Initialise();
		l_pShader->Begin();
	}

	m_pRenderer->Render( p_eDisplayMode);

	for (size_t i = 0 ; i < m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i]->Render( p_eDisplayMode);
	}
}

void Pass :: Render2D()
{
	if ( ! m_pShaderProgram.expired())
	{
		ShaderProgramPtr l_pShader = m_pShaderProgram.lock();
		l_pShader->Initialise();
		l_pShader->Begin();
	}

	m_pRenderer->Render2D();

	for (size_t i = 0 ; i < m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i]->Render2D();
	}
}

void Pass :: EndRender()
{
	for (size_t i = 0 ; i < m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i]->EndRender();
	}

	m_pRenderer->EndRender();

	if ( ! m_pShaderProgram.expired())
	{
		ShaderProgramPtr l_pShader = m_pShaderProgram.lock();
		l_pShader->End();
	}
}

void Pass :: SetTexBaseColour( float p_fRed, float p_fGreen, float p_fBlue, float p_fAlphA)
{
	m_clrTexBaseColour = Colour( p_fRed, p_fGreen, p_fBlue, p_fAlphA);

	for (size_t i = 0 ; i < m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i]->SetPrimaryColour( m_clrTexBaseColour);
	}
}

void Pass :: SetTexBaseColour( const Colour & p_clrColour)
{
	m_clrTexBaseColour = p_clrColour;

	for (size_t i = 0 ; i < m_arrayTextureUnits.size() ; i++)
	{
		m_arrayTextureUnits[i]->SetPrimaryColour( m_clrTexBaseColour);
	}
}

TextureUnitPtr Pass :: AddTextureUnit()
{
	TextureUnitPtr l_pReturn( new TextureUnit);
	size_t l_uiID = m_arrayTextureUnits.size();
	l_pReturn->SetIndex( l_uiID);
	l_pReturn->SetPrimaryColour( m_clrTexBaseColour);
	m_arrayTextureUnits.push_back( l_pReturn);
	return l_pReturn;
}

bool Pass :: DestroyTextureUnit( size_t p_uiIndex)
{
	CASTOR_ASSERT( p_uiIndex < m_arrayTextureUnits.size());
	bool l_bReturn = false;
	Logger::LogMessage( CU_T( "Destroying TextureUnit %d"), p_uiIndex);

	TextureUnitPtrArray::iterator l_it = m_arrayTextureUnits.begin();
	m_arrayTextureUnits.erase( l_it + p_uiIndex);
	size_t i = p_uiIndex;

	for (l_it = m_arrayTextureUnits.begin() + p_uiIndex ; l_it != m_arrayTextureUnits.end() ; ++l_it)
	{
		(* l_it)->SetIndex( i++);
	}

	l_bReturn = true;

	return l_bReturn;
}

TextureUnitPtr Pass :: GetTextureUnit( size_t p_uiIndex)
{
	CASTOR_ASSERT( p_uiIndex < m_arrayTextureUnits.size());
	return m_arrayTextureUnits[p_uiIndex];
}

String Pass :: GetTexturePath( size_t p_uiIndex)
{
	CASTOR_ASSERT( p_uiIndex < m_arrayTextureUnits.size());
	return m_arrayTextureUnits[p_uiIndex]->GetTexturePath();
}

bool Pass :: Write( Castor::Utils::File & p_file)const
{
	bool l_bReturn = p_file.WriteLine( "\tpass\n\t{\n");

	if (l_bReturn)
	{
		p_file.Print( 256, "\t\tambient %f %f %f %f\n", m_clrAmbient[0], m_clrAmbient[1], m_clrAmbient[2], m_clrAmbient[3]);
	}
	
	if (l_bReturn)
	{
		p_file.Print( 256, "\t\tdiffuse %f %f %f %f\n", m_clrDiffuse[0], m_clrDiffuse[1], m_clrDiffuse[2], m_clrDiffuse[3]);
	}
	
	if (l_bReturn)
	{
		p_file.Print( 256, "\t\temissive %f %f %f %f\n", m_clrEmissive[0], m_clrEmissive[1], m_clrEmissive[2], m_clrEmissive[3]);
	}
	
	if (l_bReturn)
	{
		p_file.Print( 256, "\t\tspecular %f %f %f %f\n", m_clrSpecular[0], m_clrSpecular[1], m_clrSpecular[2], m_clrSpecular[3]);
	}
	
	if (l_bReturn)
	{
		p_file.Print( 256, "\t\tshininess %f\n", m_fShininess);
	}
	
	if (l_bReturn)
	{
		p_file.Print( 256, "\t\ttex_base %f %f %f %f\n", m_clrTexBaseColour[0], m_clrTexBaseColour[1], m_clrTexBaseColour[2], m_clrTexBaseColour[3]);
	}
	
	if (l_bReturn)
	{
		p_file.WriteLine( "\t\tdouble_face " + String( m_bDoubleFace ? "true" : "false") + "\n");
	}

	if (l_bReturn)
	{
		size_t l_uiNbTextureUnits = m_arrayTextureUnits.size();
		bool l_bFirst = true;

		for (size_t i = 0 ; i < l_uiNbTextureUnits && l_bReturn ; i++)
		{
			if (l_bFirst)
			{
				l_bFirst = false;
			}
			else
			{
				p_file.WriteLine( "\n");
			}

			l_bReturn = m_arrayTextureUnits[i]->Write( p_file);
		}
	}
	
	if (l_bReturn)
	{
		l_bReturn = p_file.WriteLine( "\t}\n");
	}

	return l_bReturn;
}

bool Pass :: Read( Castor::Utils::File & p_file)
{
	bool l_bReturn = m_clrAmbient.Read( p_file);

	if (l_bReturn)
	{
		l_bReturn = m_clrDiffuse.Read( p_file);
	}

	if (l_bReturn)
	{
		l_bReturn = m_clrSpecular.Read( p_file);
	}

	if (l_bReturn)
	{
		l_bReturn = m_clrEmissive.Read( p_file);
	}

	if (l_bReturn)
	{
		l_bReturn = (p_file.Read( m_fShininess) == sizeof( real));
	}

	if (l_bReturn)
	{
		l_bReturn = m_clrTexBaseColour.Read( p_file);
	}

	if (l_bReturn)
	{
		l_bReturn = (p_file.Read( m_bDoubleFace) == sizeof( bool));
	}

	if (l_bReturn)
	{
		size_t l_uiNbTextureUnits = 0;
		l_bReturn = (p_file.Read( l_uiNbTextureUnits) == sizeof( size_t));

		for (size_t i = 0 ; i < l_uiNbTextureUnits && l_bReturn ; i++)
		{
			TextureUnitPtr l_pTextureUnit = AddTextureUnit();
			l_bReturn = l_pTextureUnit->Read( p_file);

			if ( ! l_bReturn)
			{
				DestroyTextureUnit( l_pTextureUnit->GetIndex());
			}
		}
	}

	return l_bReturn;
}

void Pass :: SetShader( ShaderProgramPtr p_pProgram)
{
	m_pShaderProgram = p_pProgram;
	m_pRenderer->Initialise();
}

bool Pass :: HasShader()const
{
	return ! m_pShaderProgram.expired();
}