/*
This source file is part of Castor3D (http://castor3d.developpez.com/castor3d.htm)

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
#ifndef ___C3D_PlyImporter___
#define ___C3D_PlyImporter___

#include <Castor3D/Importer.hpp>

#ifndef _WIN32
#	define C3D_Ply_API
#else
#	ifdef PlyImporter_EXPORTS
#		define C3D_Ply_API __declspec(dllexport)
#	else
#		define C3D_Ply_API __declspec(dllimport)
#	endif
#endif

namespace Castor3D
{
	//! PLY file importer
	/*!
	Imports data from PLY files
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class PlyImporter : public Importer, public MemoryTraced<PlyImporter>
	{
	private:
		friend class Scene;

	public:
		/**
		 * Constructor
		 */
		PlyImporter();

	private:
		virtual bool _import();
	};
}

#endif