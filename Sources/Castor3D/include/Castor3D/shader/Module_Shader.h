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
#ifndef ___C3D_Module_Shader___
#define ___C3D_Module_Shader___

#include "../Module_General.h"

namespace Castor3D
{
	class ShaderManager;
	class ShaderObject;
	class ShaderProgram;
	class UniformVariable;

	typedef Templates::SharedPtr<UniformVariable>	UniformVariablePtr;
	typedef Templates::SharedPtr<ShaderProgram>		ShaderProgramPtr;
	typedef Templates::SharedPtr<ShaderObject>		ShaderObjectPtr;

	typedef C3DVector(	ShaderProgramPtr)			ShaderProgramPtrArray;
	typedef C3DVector(	ShaderObjectPtr)			ShaderObjectPtrArray;
	typedef C3DMap(		String,	UniformVariablePtr)	UniformVariablePtrStrMap;
}

#endif