/*
This source file is part of Castor3D (http://dragonjoker.co.cc

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
the program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
*/
#ifndef ___GLMaterialRenderer___
#define ___GLMaterialRenderer___

#include "Module_GLRender.h"
#include "GLRenderSystem.h"

namespace Castor3D
{
	class CS3D_GL_API GLPassRenderer : public PassRenderer
	{
	protected:
		Point<float, 4>	m_varAmbient;
		Point<float, 4>	m_varDiffuse;
		Point<float, 4>	m_varSpecular;
		Point<float, 4>	m_varEmissive;
		real			m_varShininess;
		GLUBOVariable * m_pAmbient;
		GLUBOVariable * m_pDiffuse;
		GLUBOVariable * m_pEmissive;
		GLUBOVariable * m_pSpecular;
		GLUBOVariable * m_pShininess;

		GLUniformBufferObjectPtr m_pUniformBuffer;

	public:
		GLPassRenderer();
		virtual ~GLPassRenderer(){ Cleanup(); }

		virtual void Cleanup();
		virtual void Initialise();

		virtual void Apply( eDRAW_TYPE p_displayMode);
		virtual void Apply2D( eDRAW_TYPE p_displayMode);
		virtual void Remove();
		void DrawSubmesh( SubmeshPtr p_submesh, eDRAW_TYPE p_displayMode);
	};
}

#endif