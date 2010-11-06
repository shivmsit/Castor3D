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

#ifndef ___C3D_Module_Importer___
#define ___C3D_Module_Importer___

#include "../Module_General.h"

namespace Castor3D
{
	class ExternalImporter;
	class AseImporter;
	class BspImporter;
	class Md2Importer;
	class Md3Importer;
	class ObjImporter;
	class SMaxImporter;
	
	typedef Templates::SharedPtr<ExternalImporter>	ExternalImporterPtr;
	typedef Templates::SharedPtr<AseImporter>		AseImporterPtr;
	typedef Templates::SharedPtr<BspImporter>		BspImporterPtr;
	typedef Templates::SharedPtr<Md2Importer>		Md2ImporterPtr;
	typedef Templates::SharedPtr<Md3Importer>		Md3ImporterPtr;
	typedef Templates::SharedPtr<ObjImporter>		ObjImporterPtr;
	typedef Templates::SharedPtr<SMaxImporter>		SMaxImporterPtr;
}

#endif
