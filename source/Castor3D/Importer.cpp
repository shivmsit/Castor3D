#include "Castor3D/PrecompiledHeader.hpp"

#include "Castor3D/Importer.hpp"
#include "Castor3D/Mesh.hpp"
#include "Castor3D/Submesh.hpp"
#include "Castor3D/Material.hpp"
#include "Castor3D/TextureUnit.hpp"
#include "Castor3D/Pass.hpp"
#include "Castor3D/Geometry.hpp"
#include "Castor3D/RenderSystem.hpp"
#include "Castor3D/Buffer.hpp"
#include "Castor3D/SceneNode.hpp"


using namespace Castor3D;

bool Importer :: Import( Path const & p_fileName)
{
	bool l_bReturn = false;

	m_fileName = p_fileName;
	m_filePath = m_fileName.GetPath();

	l_bReturn = _import();

	return l_bReturn;
}

SceneNodePtr Importer :: GetNode()
{
	return (m_nodes.empty() ? SceneNodePtr() : m_nodes[0]);
}
/*
void Importer :: _convertToMesh( MeshPtr p_mesh, Imported3DModel * p_model)
{
	ImportedMaterialInfo l_mat;
	MaterialPtr l_material;
	PassPtr l_pass;
	TextureUnitPtr l_unit;

	for (int i = 0 ; i < p_model->m_numOfMaterials ; i++)
	{
		l_mat = p_model->m_materials[i];
		l_material = Factor<Material>::Create( l_mat.m_strName, 1);
		l_pass = l_material->GetPass( 0);
		l_pass->SetDoubleFace( true);
		l_pass->SetAmbient( l_mat.m_fAmbient[0], l_mat.m_fAmbient[1], l_mat.m_fAmbient[2], 1.0f);
		l_pass->SetDiffuse( l_mat.m_fDiffuse[0], l_mat.m_fDiffuse[1], l_mat.m_fDiffuse[2], 1.0f);
		l_pass->SetSpecular( l_mat.m_fSpecular[0], l_mat.m_fSpecular[1], l_mat.m_fSpecular[2], 1.0f);
		l_pass->SetShininess( l_mat.m_fShininess);

		if ( ! String( l_mat.m_strFile).empty())
		{
			l_unit = new TextureUnit( RenderSystem::GetSingletonPtr()->CreateTextureRenderer());
			l_pass->AddTextureUnit( l_unit);
			FILE * l_file = nullptr;
			FOpen( l_file, String( l_mat.m_strFile).char_str(), "rb");

			if (l_file)
			{
				fclose( l_file);
				l_unit->SetTexture2D( l_mat.m_strFile);
			}
			else
			{
				l_unit->SetTexture2D( m_filePath + String( l_mat.m_strFile));
			}
		}
	}

	SubmeshPtr l_submesh;
	Imported3DObject l_object;
	FacePtr l_face;

	for (int i = 0 ; i < p_model->m_numOfObjects ; i++)
	{
		l_object = p_model->m_objects[i];

		if (l_object.m_numOfVerts > 0)
		{
			l_submesh = p_mesh->CreateSubmesh( 1);

			l_submesh->GetRenderer()->GetTriangles()->Cleanup();
			l_submesh->GetRenderer()->GetTrianglesTexCoords()->Cleanup();
			l_submesh->GetRenderer()->GetLines()->Cleanup();
			l_submesh->GetRenderer()->GetLinesTexCoords()->Cleanup();

			for (int j = 0 ; j < l_object.m_numOfVerts ; j++)
			{
				l_submesh->AddVertex( l_object.m_vertex[j][0], l_object.m_vertex[j][1], l_object.m_vertex[j][2]);
			}

			for (int j = 0 ; j < l_object.m_numOfFaces ; j++)
			{
				if ((l_face = l_submesh->AddFace( l_object.m_faces[j].m_vertIndex[0], l_object.m_faces[j].m_vertIndex[1], l_object.m_faces[j].m_vertIndex[2], 0)))
				{
					if (l_object.m_texVerts)
					{
						l_face->SetVertexTexCoords( 0, l_object.m_texVerts[l_object.m_faces[j].m_coordIndex[0]][0], l_object.m_texVerts[l_object.m_faces[j].m_coordIndex[0]][1]);
						l_face->SetVertexTexCoords( 1, l_object.m_texVerts[l_object.m_faces[j].m_coordIndex[1]][0], l_object.m_texVerts[l_object.m_faces[j].m_coordIndex[1]][1]);
						l_face->SetVertexTexCoords( 2, l_object.m_texVerts[l_object.m_faces[j].m_coordIndex[2]][0], l_object.m_texVerts[l_object.m_faces[j].m_coordIndex[2]][1]);
					}
				}
			}

			if (l_object.m_materialID >= 0 && p_model->m_numOfMaterials > 0)
			{
				Collection<Material, String> l_mtlCollection;
				l_material = l_mtlCollection.GetElement( p_model->m_materials[l_object.m_materialID].m_strName);
				l_submesh->SetMaterial( l_material);
			}

			l_submesh->ComputeContainers();
			l_submesh->GenerateBuffers();
		}

		delete [] l_object.m_faces;
		delete [] l_object.m_normals;
		delete [] l_object.m_texVerts;
		delete [] l_object.m_vertex;
	}

	p_mesh->SetNormals();
}
*/