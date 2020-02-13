/*
See LICENSE file in root folder
*/
#ifndef ___C3D_MeshAnimation_H___
#define ___C3D_MeshAnimation_H___

#include "MeshAnimationModule.hpp"

#include "Castor3D/Binary/BinaryModule.hpp"
#include "Castor3D/Scene/Animation/AnimationModule.hpp"

#include "Castor3D/Animation/Animation.hpp"
#include "Castor3D/Model/Mesh/Animation/MeshAnimationSubmesh.hpp"

namespace castor3d
{
	class MeshAnimation
		: public Animation
	{
	public:
		/**
		 *\~english
		 *name Copy / Move.
		 *\~french
		 *name Copie / Déplacement.
		 **/
		/**@{*/
		C3D_API MeshAnimation( MeshAnimation && rhs ) = default;
		C3D_API MeshAnimation & operator=( MeshAnimation && rhs ) = delete;
		C3D_API MeshAnimation( MeshAnimation const & rhs ) = delete;
		C3D_API MeshAnimation & operator=( MeshAnimation const & rhs ) = delete;
		/**@}*/
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	mesh	The parent mesh.
		 *\param[in]	name	The name of the animation.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	mesh	Le maillage parent.
		 *\param[in]	name	Le nom de l'animation.
		 */
		C3D_API explicit MeshAnimation( Mesh & mesh
			, castor::String const & name = castor::cuEmptyString );
		/**
		 *\~english
		 *\brief		Destructor.
		 *\~french
		 *\brief		Destructeur.
		 */
		C3D_API ~MeshAnimation();
		/**
		 *\~english
		 *\brief		adds a child to this object.
		 *\remarks		The child's transformations are affected by this object's ones.
		 *\param[in]	object	The child.
		 *\~french
		 *\brief		Ajoute un objet enfant à celui-ci.
		 *\remarks		Les transformations de l'enfant sont affectées par celles de cet objet.
		 *\param[in]	object	L'enfant.
		 */
		C3D_API void addChild( MeshAnimationSubmesh object );
		/**
		 *\~english
		 *\param[in]	index	The index.
		 *\return		The animated submesh at given index.
		 *\~french
		 *\param[in]	index	L'indice.
		 *\return		Le sous-maillage animé à l'indice donné.
		 */
		inline MeshAnimationSubmesh & getSubmesh( uint32_t index )
		{
			CU_Require( index < m_submeshes.size() );
			return m_submeshes[index];
		}

	protected:
		//!\~english	The animated submesh.
		//!\~french		Les sous-maillages animés.
		MeshAnimationSubmeshArray m_submeshes;

		friend class BinaryWriter< MeshAnimation >;
		friend class BinaryParser< MeshAnimation >;
		friend class MeshAnimationInstance;
	};
}

#endif
