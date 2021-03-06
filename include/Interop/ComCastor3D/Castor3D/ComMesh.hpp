/* See LICENSE file in root folder */
#ifndef __COMC3D_COM_MESH_H__
#define __COMC3D_COM_MESH_H__

#include "ComCastor3D/ComAtlObject.hpp"

#include <Castor3D/Mesh/Mesh.hpp>

namespace CastorCom
{
	/*!
	\author 	Sylvain DOREMUS
	\version	0.7.0
	\date		10/09/2014
	\~english
	\brief		This class defines a CMesh object accessible from COM.
	\~french
	\brief		Cette classe définit un CMesh accessible depuis COM.
	*/
	class ATL_NO_VTABLE CMesh
		:	COM_ATL_OBJECT( Mesh )
	{
	public:
		/**
		 *\~english
		 *\brief		Default constructor.
		 *\~french
		 *\brief		Constructeur par défaut.
		 */
		CMesh();
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		virtual ~CMesh();

		inline castor3d::MeshSPtr getInternal()const
		{
			return m_internal;
		}

		inline void setInternal( castor3d::MeshSPtr internal )
		{
			m_internal = internal;
		}

		COM_PROPERTY_GET( SubmeshCount, unsigned int, makeGetter( m_internal.get(), &castor3d::Mesh::getSubmeshCount ) );
		COM_PROPERTY_GET( FaceCount, unsigned int, makeGetter( m_internal.get(), &castor3d::Mesh::getFaceCount ) );
		COM_PROPERTY_GET( VertexCount, unsigned int, makeGetter( m_internal.get(), &castor3d::Mesh::getVertexCount ) );

		STDMETHOD( GetSubmesh )( /* [in] */ unsigned int val, /* [out, retval] */ ISubmesh ** pVal );
		STDMETHOD( CreateSubmesh )( /* [out, retval] */ ISubmesh ** pVal );
		STDMETHOD( DeleteSubmesh )( /* [in] */ ISubmesh * val );
	private:
		castor3d::MeshSPtr m_internal;
	};
	//!\~english Enters the ATL object into the object map, updates the registry and creates an instance of the object	\~french Ecrit l'objet ATL dans la table d'objets, met à jour le registre et crée une instance de l'objet
	OBJECT_ENTRY_AUTO( __uuidof( Mesh ), CMesh );

	DECLARE_VARIABLE_PTR_GETTER( Mesh, castor3d, Mesh );
	DECLARE_VARIABLE_PTR_PUTTER( Mesh, castor3d, Mesh );
}

#endif
