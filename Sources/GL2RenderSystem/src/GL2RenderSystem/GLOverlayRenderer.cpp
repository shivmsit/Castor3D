#include "PrecompiledHeader.h"

#include "GLOverlayRenderer.h"

using namespace Castor3D;

void GLOverlayRenderer :: DrawPanel()
{
	MaterialPtr l_pMaterial = ((PanelOverlayPtr)m_target)->GetMaterial();

	if ( ! l_pMaterial.null())
	{
		l_pMaterial->Apply( DTQuads);

		glBegin( GL_QUADS);
		{
			glVertex2( m_target->GetPosition()[0], m_target->GetPosition()[1]);
			glTexCoord2( 0.0, 0.0);
			glVertex2( m_target->GetPosition()[0], m_target->GetPosition()[1] + m_target->GetSize()[1]);
			glTexCoord2( 0.0, 1.0);
			glVertex2( m_target->GetPosition()[0] + m_target->GetSize()[0], m_target->GetPosition()[1] + m_target->GetSize()[1]);
			glTexCoord2( 1.0, 1.0);
			glVertex2( m_target->GetPosition()[0] + m_target->GetSize()[0], m_target->GetPosition()[1]);
			glTexCoord2( 1.0, 0.0);
		}
		glEnd();

		l_pMaterial->Remove();
	}
}

void GLOverlayRenderer :: DrawBorderPanel()
{
}

void GLOverlayRenderer :: DrawText()
{
}