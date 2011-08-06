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
#ifndef ___C3D_Camera___
#define ___C3D_Camera___

#include "Renderable.hpp"
#include "MovableObject.hpp"
#include "Viewport.hpp"

namespace Castor
{	namespace Resources
{
	//! Camera loader
	/*!
	Loads and saves cameras from/into a file
	\author Sylvain DOREMUS
	\date 14/02/2010
	*/
	template <> class C3D_API Loader<Castor3D::Camera>
	{
	public:
		/**
		 * Loads a cameras from a text file
		 *@param p_file : [in/out] the file to save the cameras in
		 *@param p_camera : [in/out] the cameras to save
		 */
		static bool Load( Castor3D::Camera & p_camera, Utils::File & p_file);
		/**
		 * Saves a cameras into a text file
		 *@param p_file : [in] the file to save the cameras in
		 *@param p_camera : [in] the cameras to save
		 */
		static bool Save( const Castor3D::Camera & p_camera, Utils::File & p_file);
		/**
		 * Writes a cameras into a text file
		 *@param p_file : [in] the file to save the cameras in
		 *@param p_camera : [in] the cameras to save
		 */
		static bool Write( const Castor3D::Camera & p_camera, Utils::File & p_file);
	};
}
}

namespace Castor3D
{
	//! Class which represents a Camera
	/*!
	Class to represent a Camera, giving its position, orientation, viewport...
	\author Sylvain DOREMUS
	\version 0.1
	\date 09/02/2010
	*/
	class C3D_API Camera : public MovableObject, public Renderable<Camera, CameraRenderer>
	{
	public:
		//! The selection mode enumerator
		/*!
		There are four modes of selection when you click over an object
		*/
		typedef enum
		{	eSELECTION_MODE_VERTEX		//!< Vertex selection mode
		,	eSELECTION_MODE_EDGE		//!< Edge selection mode
		,	eSELECTION_MODE_FACE		//!< Face selection mode
		,	eSELECTION_MODE_SUBMESH		//!< Submesh selection mode
		,	eSELECTION_MODE_OBJECT		//!< Geometry selection mode
		,	eSELECTION_MODE_COUNT
		}	eSELECTION_MODE;

	private:
		friend class Scene;
		friend class CameraRenderer;
		Viewport m_viewport;			//!< The viewport of the camera
		unsigned int m_uiWindowWidth;	//!< Display window width
		unsigned int m_uiWindowHeight;	//!< Display window height

	public:
		/**@name Construction / Destruction */
		//@{
		/**
		* Constructor, needs the camera renderer, the name, window size and projection type. Creates a viewport renderer and a viewport
		* Not to be used by the user, use Scene::CreateCamera instead
		*@param p_strName : [in] The camera name
		*@param p_uiWidth, p_uiHeight : [in] Display window size
		*@param p_pNode : [in] The parent camera node
		*@param p_eType : [in] Projection type
		*/
		Camera( Scene * p_pScene, String const & p_strName, unsigned int p_uiWidth, unsigned int p_uiHeight, const SceneNodePtr p_pNode, eVIEWPORT_TYPE p_eType);
		/**
		 * Destructor
		 */
		virtual ~Camera();
		//@}

		/**
		 * Applies the viewport, the rotation, and renders all
		 *@param p_eDisplayMode : [in] The display mode
		 */
		virtual void Render( ePRIMITIVE_TYPE p_eDisplayMode);
		/**
		 * Removes the transformations
		 */
		virtual void EndRender();
		/**
		 * Resizes the viewport
		 *@param p_uiWidth, p_uiHeight : [in] Display window size
		 */
		void Resize( unsigned int p_uiWidth, unsigned int p_uiHeight);
		/**
		 * Sets the orientation to 0
		 */
		void ResetOrientation();
		/**
		 * Sets the position to 0
		 */
		void ResetPosition();
		/**
		* Returns the first object at mouse coords x and y
		*@param p_pScene : [in] The scene used for the selection
		*@param p_eMode : [in] The selection mode (vertex, face, submesh, geometry)
		*@param p_ppFound : [out] The found object (vertex, face, submesh, geometry)
		*@param p_iX : [in] The x mouse coordinate
		*@param p_iY : [in] The y mouse coordinate
		*@return true if something was found, false if not
		*/
		bool Select( ScenePtr p_pScene, eSELECTION_MODE p_eMode, void ** p_ppFound, int p_iX, int p_iY);

		/**@name Accessors */
		//@{
		inline const Viewport *	GetViewport	()const	{ return & m_viewport; }
		inline Viewport *		GetViewport	()		{ return & m_viewport; }
		//@}

		DECLARE_SERIALISE_MAP()
	};
	//! The Camera renderer
	/*!
	Applies the camera transformations, also removes it
	\author Sylvain DOREMUS
	\version 0.1
	\date 09/02/2010
	*/
	class C3D_API CameraRenderer : public Renderer<Camera, CameraRenderer>
	{
	protected:
		bool m_bResize;

	protected:
		/**
		 * Constructor
		 */
		CameraRenderer()
			:	Renderer<Camera, CameraRenderer>()
			,	m_bResize( false)
		{}
	public:
		/**
		 * Destructor
		 */
		virtual ~CameraRenderer(){}
		/**
		* Returns the first object at mouse coords x and y
		*@param p_pScene : [in] The scene from which the selection is made
		*@param p_eMode : [in] The selection mode (vertex, face, submesh, geometry)
		*@param p_ppFound : [out] The found object (vertex, face, submesh, geometry)
		*@param p_iX : [in] The x mouse coordinate
		*@param p_iY : [in] The y mouse coordinate
		*@return true if something was found, false if not
		*/
		virtual bool Select( ScenePtr p_pScene, Camera::eSELECTION_MODE p_eMode, void ** p_ppFound, int p_iX, int p_iY) = 0;
		/**
		 * Renders the target
		 *@param p_eDisplayMode : [in] The display mode
		 */
		virtual void Render( ePRIMITIVE_TYPE p_eDisplayMode) = 0;
		virtual void EndRender() = 0;
		virtual void Resize( Point2i const & p_ptNewSize) { m_bResize = true; }
	};
}

#endif