#include "PrecompiledHeader.h"

#include "importer/Module_Importer.h"

#include "importer/SMaxImporter.h"
#include "geometry/mesh/MeshManager.h"
#include "geometry/mesh/Mesh.h"
#include "geometry/mesh/Submesh.h"
#include "geometry/primitives/Geometry.h"
#include "geometry/basic/Face.h"
#include "scene/Scene.h"
#include "scene/SceneNode.h"
#include "scene/SceneManager.h"
#include "material/MaterialManager.h"
#include "material/Material.h"
#include "material/Pass.h"
#include "material/TextureUnit.h"
#include "render_system/RenderSystem.h"


using namespace Castor3D;

char g_buffer[50000] = {0};

SMaxImporter :: SMaxImporter()
	:	ExternalImporter(),
		m_pFile( NULL),
		m_iNumOfMaterials( 0),
		m_texVerts( NULL)
{
}

bool SMaxImporter :: _import()
{
	m_pFile = new File( m_fileName, File::eRead);

	if (m_pFile->IsOk())
	{
		size_t l_uiSlashIndex = 0;

		if (m_fileName.find_last_of( CU_T( "\\")) != String::npos)
		{
			l_uiSlashIndex = m_fileName.find_last_of( CU_T( "\\")) + 1;
		}

		if (m_fileName.find_last_of( CU_T( "/")) != String::npos)
		{
			l_uiSlashIndex = std::max( l_uiSlashIndex, m_fileName.find_last_of( CU_T( "/")) + 1);
		}

		size_t l_uiDotIndex = m_fileName.find_last_of( CU_T( "."));

		UIntArray l_faces;
		FloatArray l_sizes;
		String l_name = m_fileName.substr( l_uiSlashIndex, l_uiDotIndex - l_uiSlashIndex);
		String l_meshName = m_fileName.substr( l_uiSlashIndex, l_uiDotIndex - l_uiSlashIndex);
		String l_materialName = m_fileName.substr( l_uiSlashIndex, l_uiDotIndex - l_uiSlashIndex);

		MeshPtr l_pMesh;

		if (MeshManager::HasElement( l_meshName))
		{
			l_pMesh.reset( MeshManager::GetElementByName( l_meshName));
		}
		else
		{
			l_pMesh.reset( MeshManager::CreateMesh( l_meshName, l_faces, l_sizes, Mesh::eCustom));
			Log::LogMessage( CU_T( "CreatePrimitive - Mesh %s created"), l_meshName.c_str());
		}

		SMaxChunk l_currentChunk = {0};

		_readChunk( & l_currentChunk);

		if (l_currentChunk.m_id != SMAX_PRIMARY)
		{
			delete m_pFile;
			return false;
		}

		_processNextChunk( l_pMesh, & l_currentChunk);
		l_pMesh->SetNormals();
		m_pScene = SceneManager::GetSingleton().GetElementByName( "MainScene");
		SceneNodePtr l_pNode = m_pScene->CreateSceneNode( l_name);

		GeometryPtr l_pGeometry = new Geometry( l_pMesh, l_pNode, l_name);
		Log::LogMessage( CU_T( "PlyImporter::_import - Geometry %s created"), l_name.c_str());

		m_geometries.insert( GeometryPtrStrMap::value_type( l_name, l_pGeometry));
	}

	delete m_pFile;

	return true;
}

void SMaxImporter :: _processNextChunk( MeshPtr p_pMesh, SMaxChunk * p_previousChunk)
{
	SubmeshPtr l_pSubmesh = NULL;

	SMaxChunk l_currentChunk = {0};
	SMaxChunk l_tempChunk = {0};

	while (m_pFile->IsOk() && p_previousChunk->m_bytesRead < p_previousChunk->m_length)
	{
		_readChunk( & l_currentChunk);

		switch (l_currentChunk.m_id)
		{
		case SMAX_VERSION:
			l_currentChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_currentChunk.m_length - l_currentChunk.m_bytesRead);
			if ((l_currentChunk.m_length - l_currentChunk.m_bytesRead == 4) && (((int *)g_buffer)[0] > 0x03))
			{
				Log::LogMessage( "File version is over version 3 and may load incorrectly");
			}
		break;

		case SMAX_OBJECTINFO:
			_readChunk( & l_tempChunk);
			l_tempChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_tempChunk.m_length - l_tempChunk.m_bytesRead);
			l_currentChunk.m_bytesRead += l_tempChunk.m_bytesRead;
			_processNextChunk( p_pMesh, & l_currentChunk);
		break;

		case SMAX_MATERIAL:
			m_iNumOfMaterials++;
			_processNextMaterialChunk( & l_currentChunk);
		break;

		case SMAX_OBJECT:
		{
			l_pSubmesh = p_pMesh->CreateSubmesh( 1);
			String l_strName;
			l_currentChunk.m_bytesRead += _getString( l_strName);
			_processNextObjectChunk( p_pMesh, l_pSubmesh, & l_currentChunk);
			l_pSubmesh->ComputeContainers();
			l_pSubmesh->GenerateBuffers();
		}
		break;

		case SMAX_EDITKEYFRAME:
			l_currentChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_currentChunk.m_length - l_currentChunk.m_bytesRead);
		break;

		default: 
			l_currentChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_currentChunk.m_length - l_currentChunk.m_bytesRead);
		break;
		}

		p_previousChunk->m_bytesRead += l_currentChunk.m_bytesRead;
	}
}

void SMaxImporter :: _processNextObjectChunk( MeshPtr p_pMesh, SubmeshPtr p_pSubmesh, SMaxChunk * p_previousChunk)
{
	SMaxChunk l_currentChunk = {0};

	while (m_pFile->IsOk() && p_previousChunk->m_bytesRead < p_previousChunk->m_length)
	{
		_readChunk( & l_currentChunk);

		switch (l_currentChunk.m_id)
		{
		case SMAX_OBJECT_MESH:
			_processNextObjectChunk( p_pMesh, p_pSubmesh, & l_currentChunk);
		break;

		case SMAX_OBJECT_VERTICES:
			_readVertices( p_pSubmesh, & l_currentChunk);
		break;

		case SMAX_OBJECT_FACES:
			_readVertexIndices( p_pSubmesh, & l_currentChunk);
		break;

		case SMAX_OBJECT_MATERIAL:
			_readObjectMaterial( p_pSubmesh, & l_currentChunk);			
		break;

		case SMAX_OBJECT_UV:
			_readUVCoordinates( p_pSubmesh, & l_currentChunk);
		break;

		default:  
			l_currentChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_currentChunk.m_length - l_currentChunk.m_bytesRead);
		break;
		}

		p_previousChunk->m_bytesRead += l_currentChunk.m_bytesRead;
	}
}

void SMaxImporter :: _processNextMaterialChunk( SMaxChunk * p_previousChunk)
{
	SMaxChunk l_currentChunk = {0};
	MaterialPtr l_pMaterial;
	Colour l_crDiffuse( 0, 0, 0, 1);
	Colour l_crAmbient( 0, 0, 0, 1);
	Colour l_crSpecular( 0, 0, 0, 1);
	real l_fShininess = 0;
	String l_strMatName;
	String l_strTexture;

	while (m_pFile->IsOk() && p_previousChunk->m_bytesRead < p_previousChunk->m_length)
	{
		_readChunk( & l_currentChunk);

		switch (l_currentChunk.m_id)
		{
		case SMAX_MATNAME:
			l_currentChunk.m_bytesRead += _getString( l_strMatName);
			l_currentChunk.m_bytesRead = l_currentChunk.m_length;
		break;

		case SMAX_MATDIFFUSE:
			_readColorChunk( l_crDiffuse, & l_currentChunk);
		break;

		case SMAX_MATSPECULAR:
			_readColorChunk( l_crSpecular, & l_currentChunk);
		break;

		case SMAX_MATSHININESS:
		break;

		case SMAX_MATTWOSIDED:
		break;

		case SMAX_MATMAP:
			_processMaterialMapChunk( l_strTexture, & l_currentChunk);
		break;

		case SMAX_MATMAPFILE:
			l_currentChunk.m_bytesRead += _getString( l_strTexture);
			l_currentChunk.m_bytesRead = l_currentChunk.m_length;
		break;

		default:  
			l_currentChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_currentChunk.m_length - l_currentChunk.m_bytesRead);
		break;
		}

		p_previousChunk->m_bytesRead += l_currentChunk.m_bytesRead;
	}

	if ( ! l_strMatName.empty())
	{
		l_pMaterial = MaterialManager::CreateMaterial( l_strMatName);
		PassPtr l_pPass = l_pMaterial->GetPass( 0);
		l_pPass->SetAmbient( l_crAmbient);
		l_pPass->SetDiffuse( l_crDiffuse);
		l_pPass->SetSpecular( l_crSpecular);
		l_pPass->SetDoubleFace( true);

		if ( ! l_strTexture.empty())
		{
			TextureUnitPtr l_pUnit = new TextureUnit();
			l_pUnit->SetTexture2D( m_pFile->GetFilePath() + "/" + l_strTexture);
			l_pPass->AddTextureUnit( l_pUnit);
			l_pUnit->SetPrimaryColour( 1.0, 1.0, 1.0, 1.0);
		}

		MaterialManager::SetToInitialise( l_pMaterial);
	}
}

void SMaxImporter :: _processMaterialMapChunk( String & p_strName, SMaxChunk * p_previousChunk)
{
	SMaxChunk l_currentChunk = {0};

	while (m_pFile->IsOk() && p_previousChunk->m_bytesRead < p_previousChunk->m_length)
	{
		_readChunk( & l_currentChunk);

		switch (l_currentChunk.m_id)
		{
		case SMAX_MATMAP:
			_processMaterialMapChunk( p_strName, & l_currentChunk);
		break;

		case SMAX_MATMAPFILE:
			l_currentChunk.m_bytesRead += _getString( p_strName);
			l_currentChunk.m_bytesRead = l_currentChunk.m_length;
		break;

		default:  
			l_currentChunk.m_bytesRead += m_pFile->ReadArray<char>( g_buffer, l_currentChunk.m_length - l_currentChunk.m_bytesRead);
		break;
		}

		p_previousChunk->m_bytesRead += l_currentChunk.m_bytesRead;
	}
}

void SMaxImporter :: _readChunk( SMaxChunk * p_chunk)
{
	p_chunk->m_bytesRead = m_pFile->Read<unsigned short>( p_chunk->m_id);
	p_chunk->m_bytesRead += m_pFile->Read<unsigned int>( p_chunk->m_length);
}

int SMaxImporter :: _getString( Char * p_pBuffer)
{
	char l_pBuffer[255];
	int index = 0;
	m_pFile->Read<char>( *l_pBuffer);

	while (m_pFile->IsOk() && *(l_pBuffer + index++) != 0)
	{
		m_pFile->Read<char>( *(l_pBuffer + index));
	}

	Strcpy( p_pBuffer, 255, String( l_pBuffer).c_str());

	return strlen( l_pBuffer) + 1;
}

int SMaxImporter :: _getString( String & p_strString)
{
	char l_pBuffer[255];
	int index = 0;
	m_pFile->Read<char>( * l_pBuffer);

	while (m_pFile->IsOk() && *(l_pBuffer + index++) != 0)
	{
		m_pFile->Read<char>( *(l_pBuffer + index));
	}

	p_strString = l_pBuffer;

	return strlen( l_pBuffer) + 1;
}

void SMaxImporter :: _readColorChunk( Colour & p_colour, SMaxChunk * p_chunk)
{
	SMaxChunk l_tempChunk = {0};
	_readChunk( & l_tempChunk);
	unsigned char l_pColour[3];
	l_tempChunk.m_bytesRead += m_pFile->ReadArray<unsigned char>( l_pColour, l_tempChunk.m_length - l_tempChunk.m_bytesRead);
	p_colour[0] = float( l_pColour[0]) / 255.0f;
	p_colour[1] = float( l_pColour[1]) / 255.0f;
	p_colour[2] = float( l_pColour[2]) / 255.0f;
	p_chunk->m_bytesRead += l_tempChunk.m_bytesRead;
}

void SMaxImporter :: _readVertexIndices( SubmeshPtr p_pSubmesh, SMaxChunk * p_chunk)
{
	unsigned short l_index1 = 0;
	unsigned short l_index2 = 0;
	unsigned short l_index3 = 0;
	unsigned short l_dump = 0;
	int l_iNumOfFaces = 0;
	p_chunk->m_bytesRead += m_pFile->Read<short>( (short &)l_iNumOfFaces);
	FacePtr l_pFace;

	for (int i = 0 ; m_pFile->IsOk() && i < l_iNumOfFaces ; i++)
	{
		p_chunk->m_bytesRead += m_pFile->Read<unsigned short>( l_index1);
		p_chunk->m_bytesRead += m_pFile->Read<unsigned short>( l_index2);
		p_chunk->m_bytesRead += m_pFile->Read<unsigned short>( l_index3);
		p_chunk->m_bytesRead += m_pFile->Read<unsigned short>( l_dump);

		l_pFace = p_pSubmesh->AddFace( l_index1, l_index2, l_index3, 0);

		if (m_texVerts != NULL)
		{
			l_pFace->SetTexCoordV1( m_texVerts[l_index1][0], m_texVerts[l_index1][1]);
			l_pFace->SetTexCoordV2( m_texVerts[l_index2][0], m_texVerts[l_index2][1]);
			l_pFace->SetTexCoordV3( m_texVerts[l_index3][0], m_texVerts[l_index3][1]);
		}
	}
}

void SMaxImporter :: _readUVCoordinates( SubmeshPtr p_pSubmesh, SMaxChunk * p_chunk)
{
	short l_sNumTexVertex = 0;
	p_chunk->m_bytesRead += m_pFile->Read<short>( l_sNumTexVertex);
	m_texVerts = new Point2r[l_sNumTexVertex];
	p_chunk->m_bytesRead += m_pFile->ReadArray<char>( (char *)m_texVerts, p_chunk->m_length - p_chunk->m_bytesRead);
}

void SMaxImporter :: _readVertices( SubmeshPtr p_pSubmesh, SMaxChunk * p_chunk)
{
	short l_sNumOfVerts = 0;
	p_chunk->m_bytesRead += m_pFile->Read<short>( l_sNumOfVerts);

	for (short i = 0 ; i < l_sNumOfVerts && p_chunk->m_bytesRead < p_chunk->m_length ; i++)
	{
		Point3r l_ptVertex;
		p_chunk->m_bytesRead += m_pFile->ReadArray<char>( (char *)(& l_ptVertex), sizeof( Point3r));
		p_pSubmesh->AddVertex( l_ptVertex[0], l_ptVertex[2], -l_ptVertex[1]);
	}
}

void SMaxImporter :: _readObjectMaterial( SubmeshPtr p_pSubmesh, SMaxChunk * p_chunk)
{
	String l_materialName;
	p_chunk->m_bytesRead += _getString( l_materialName);

	MaterialPtr l_pMaterial = MaterialManager::GetElementByName( l_materialName);

	if ( ! l_pMaterial.null())
	{
		p_pSubmesh->SetMaterial( l_pMaterial);
	}

	p_chunk->m_bytesRead += m_pFile->ReadArray<char>( g_buffer, p_chunk->m_length - p_chunk->m_bytesRead);
}