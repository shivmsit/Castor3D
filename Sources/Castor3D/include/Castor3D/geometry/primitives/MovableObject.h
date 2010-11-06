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
#ifndef ___C3D_MovableObject___
#define ___C3D_MovableObject___

#include "../../scene/Module_Scene.h"

namespace Castor3D
{
	//! Movable objects handler class
	/*!
	Movable objects description : name, center, orientation, parent node
	\author Sylvain DOREMUS
	\version 0.1
	\date 09/02/2010
	*/
	class CS3D_API MovableObject
	{
	protected:
		String m_name;				//!< The name
		Point3r m_center;			//!< The center's position, all vertexes position are relative to the center
		Quaternion m_orientation;	//!< The orientation
		real * m_matrix;			//!< The rotation matrix
		SceneNode * m_sceneNode;	//!< The parent scene node, the center's position is relative to the parent node's position

	public :
		/**
		 * Constructor
		 *@param p_sn : [in] Parent node
		 *@param p_name : [in] The name
		 */
		MovableObject( SceneNodePtr p_sn, const String & p_name);
		/**
		 * Destructor
		 */
		virtual ~MovableObject();
		/**
		 * Cleans the pointers the object has created and that are not necessary (currently none)
		 */
		void Cleanup();
		/**
		 * Writes the object in a file
		 *@param p_pFile : [in] the file to write in
		 *@return true if successful, false if not
		 */
		bool Write( Castor::Utils::File & p_pFile)const;
		/**
		 * Computes then returns the rotation matrix
		 *@return The rotation matrix
		 */
		real * Get4x4RotationMatrix();
		/**
		 * Detaches the movable object from it's parent
		 */
		void Detach();

	public:
		/**@name Accessors */
		//@{
		/**
		 * @return The object name
		 */
		inline String				GetName			()const	{ return m_name; }
		/**
		 * @return The object center position pointer
		 */
		inline const Point3r	*	GetCenter		()const { return & m_center; }
		/**
		 * @return The orientation
		 */
		inline const Quaternion	&	GetOrientation	()const	{ return m_orientation; }
		/**
		 * @return The object center position
		 */
		inline const Point3r	&	GetPosition		()const { return m_center; }
		/**
		 * @return The parent node
		 */
		inline SceneNode		*	GetParent		()const { return m_sceneNode; }
		/**
		 * @param p_position : [in] The new position
		 */
		inline void SetPosition		( const Point3r & p_position)			{ m_center = p_position; }
		/**
		 * @param p_orientation : [in] The new orientation
		 */
		inline void SetOrientation	( const Quaternion & p_orientation)		{ m_orientation = p_orientation; }
		/**
		 * @param p_node : [in] The new parent node
		 */
		inline void SetParent		( SceneNode * p_node)					{ m_sceneNode = p_node; }
		//@}
	};
}

#endif
