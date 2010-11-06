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
#ifndef ___GL_GLSLProgram___
#define ___GL_GLSLProgram___

#include "Module_GLSL.h"
#include "Module_GL.h"

namespace Castor3D
{
	class CS3D_GL_API GLShaderProgram : public ShaderProgram
	{
	private:   
		GLuint		m_programObject;
		String		m_linkerLog;
		GLUniformVariablePtrStrMap m_mapGLUniformVariables;

	public:
		GLShaderProgram();
		GLShaderProgram( const String & p_vertexShaderFile, const String & p_fragmentShaderFile, const String & p_geometryShaderFile);
		virtual ~GLShaderProgram();
		virtual void Cleanup();
		virtual void Initialise();
		/**
		 * Link all Shaders
		 */
		virtual bool Link();
		/**
		 * Get Linker Messages
		 */
		virtual void RetrieveLinkerLog( String & strLog);
		/**
		 * Use Shader. OpenGL calls will go through vertex, geometry and/or fragment shaders.
		 */
		virtual void Begin();
		/**
		 * Send uniform variables to the shader
		 */
		virtual void ApplyAllVariables();
		/**
		 * Stop using this shader. OpenGL calls will go through regular pipeline.
		 */
		virtual void End();

		virtual void AddUniformVariable( UniformVariablePtr p_pUniformVariable);

	private:
		GLUniformVariablePtr _createGLUniformVariable( UniformVariablePtr p_pUniformVariable);

	public:
		inline GLuint								GetProgramObject		()const { return m_programObject; }
		inline const GLUniformVariablePtrStrMap &	GetGLUniformVariables	()const { return m_mapGLUniformVariables; }
	};
}

#endif