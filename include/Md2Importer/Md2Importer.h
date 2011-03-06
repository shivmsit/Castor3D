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
#ifndef ___C3D_Md2Importer___
#define ___C3D_Md2Importer___

#include <Castor3D/importer/Importer.h>

// These are the needed defines for the max values when loading .MD2 files
#define MD2_MAX_TRIANGLES		4096
#define MD2_MAX_VERTICES		2048
#define MD2_MAX_TEXCOORDS		2048
#define MD2_MAX_FRAMES			512
#define MD2_MAX_SKINS			32
#define MD2_MAX_FRAMESIZE		(MD2_MAX_VERTICES * 4 + 128)

#ifndef _WIN32
#	define C3D_Md2_API
#else
#	ifdef Md2Importer_EXPORTS
#		define C3D_Md2_API __declspec(dllexport)
#	else
#		define C3D_Md2_API __declspec(dllimport)
#	endif
#endif

namespace Castor3D
{
	//! MD2 file importer
	/*!
	Imports data from MD2 (Quake2) files
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class C3D_Md2_API Md2Importer : public Importer, public MemoryTraced<Md2Importer>
	{
	private:
		struct Md2Header
		{ 
		   int m_magic;
		   int m_version;
		   int m_skinWidth;
		   int m_skinHeight;
		   int m_frameSize;
		   int m_numSkins;
		   int m_numVertices;
		   int m_numTexCoords;
		   int m_numTriangles;
		   int m_numGlCommands;
		   int m_numFrames;
		   int m_offsetSkins;
		   int m_offsetTexCoords;
		   int m_offsetTriangles;
		   int m_offsetFrames;
		   int m_offsetGlCommands;
		   int om_ffsetEnd;
		};

		struct Md2AliasTriangle
		{
		   unsigned char m_vertex[3];
		   unsigned char m_lightNormalIndex;
		};

		struct Md2Triangle
		{
		   real m_vertex[3];
		   real m_normal[3];
		};

		struct Md2Face
		{
		   short m_vertexIndices[3];
		   short m_textureIndices[3];
		};

		struct Md2TexCoord
		{
		   short u, v;
		};

		struct Md2AliasFrame
		{
		   real m_scale[3];
		   real m_translate[3];
		   char m_name[16];
		   Md2AliasTriangle m_aliasVertices[1];
		};

		struct Md2Frame
		{
		   char m_strName[16];
		   Md2Triangle * m_vertices;
		};

		typedef char Md2Skin[64];

	private:
		Md2Header		m_header;
		Md2Skin		*	m_skins;
		Md2TexCoord	*	m_texCoords;
		Md2Face		*	m_triangles;
		Md2Frame	*	m_frames;
		String			m_textureName;
		File		*	m_pFile;

	public:
		Md2Importer( SceneManager * p_pManager, const String & p_textureName = C3DEmptyString);// This inits the data members

	private:
		virtual bool _import();
		void _readMD2Data();
		void _convertDataStructures( MeshPtr p_pMesh);
		void _cleanUp();
	};
}


#endif
