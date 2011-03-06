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
#ifndef ___C3D_Overlay___
#define ___C3D_Overlay___

#include "../Prerequisites.h"
#include "../render_system/Renderable.h"

namespace Castor3D
{
	//! The overlay abstract class
	/*!
	An overlay is a 2D element which is displayed at foreground
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class C3D_API Overlay : public Serialisable, public Textable, public Renderable<Overlay, OverlayRenderer>
	{
	public:
		typedef enum
		{
			eOverlayPanel,
			eOverlayBorderPanel,
			eOverlayText
		}
		eOVERLAY_TYPE;

	protected:
		Point2r m_ptPosition;				//!< The relative position (to parent or screen)
		Point2r m_ptSize;					//!< The relative size (to parent or screen)
		String m_strName;					//!< The overlay name
		bool m_bVisible;					//!< The visibility
		Overlay * m_pParent;				//!< The parent overlay, if any
		OverlayPtrIntMap m_mapOverlays;		//!< The childs, if any
		int m_iCurrentZIndex;				//!< The z index of next children to add
		MaterialPtr m_pMaterial;			//!< The material used by the overlay
		eOVERLAY_TYPE m_eType;
		OverlayManager * m_pManager;

	public:
		/**
		 * Constructor
		 *@param p_strName : [in] The overlay name
		 *@param p_pParent : [in] The parent overlay (if any)
		 */
		Overlay( OverlayManager * p_pManager, const String & p_strName, Overlay * p_pParent, eOVERLAY_TYPE p_eType);
		/**
		 * Destructor
		 */
		virtual ~Overlay()
		{
		}
		/**
		 * Adds a child to the overlay
		 *@param p_pOverlay : [in] The overlay to add
		 *@param p_iZIndex : [in] The wanted Z Index for the child
		 *@return true if successful, false if not
		 */
		bool AddChild( OverlayPtr p_pOverlay, int p_iZIndex);
		/**
		 * Sets the position of the overlay
		 *@param p_ptPosition : [in] The new position, relative to parent
		 */
		void SetPosition( const Point2r & p_ptPosition);
		/**
		 * Sets the size of the overlay
		 *@param p_ptSize : [in] The new size, relative to parent
		 */
		void SetSize( const Point2r & p_ptSize);

		/**@name Inherited methods from Textable */
		//@{
		virtual bool Write( File & p_file)const;
		virtual bool Read( File & p_file) { return false; }
		//@}

		/**@name Inherited methods from Serialisable */
		//@{
		virtual bool Save( File & p_file)const;
		virtual bool Load( File & p_file);
		//@}

		/**@name Accessors */
		//@{
		inline String			GetName		()const { return m_strName; }
		inline Point2r			GetPosition	()const { return m_ptPosition; }
		inline Point2r			GetSize		()const { return m_ptSize; }
		inline bool				IsVisible	()const { return m_bVisible; }
		inline Overlay *		GetParent	()const { return m_pParent; }
		inline MaterialPtr		GetMaterial	()const { return m_pMaterial; }
		inline void SetVisible	( bool val)					{ m_bVisible = val; }
		inline void SetMaterial	( MaterialPtr p_pMaterial)	{ m_pMaterial = p_pMaterial; }
		//@}
	};
	//! A simple overlay area
	/*!
	Describes an actually rectangular overlay. Uses a material so it can contain an image
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class C3D_API PanelOverlay : public Overlay
	{
	public:
		/**
		 * Constructor
		 *@param p_strName : [in] The overlay name
		 *@param p_pParent : [in] The parent overlay (if any)
		 */
		PanelOverlay( OverlayManager * p_pManager, const String & p_strName, Overlay * p_pParent)
			:	Overlay( p_pManager, p_strName, p_pParent, eOverlayPanel)
		{
		}
		/**
		 * Destructor
		 */
		virtual ~PanelOverlay(){}
		/**
		 * Draws the overlay
		 */
		virtual void Render( ePRIMITIVE_TYPE p_displayMode);

		/**@name Inherited methods from Textable */
		//@{
		virtual bool Write( File & p_file)const;
		virtual bool Read( File & p_file) { return false; }
		//@}

		/**@name Inherited methods from Serialisable */
		//@{
		virtual bool Save( File & p_file)const;
		virtual bool Load( File & p_file);
		//@}
	};
	//! A simple overlay bordered area
	/*!
	Describes an actually rectangular overlay with a border. Uses a material for the panel and a material for the border
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class C3D_API BorderPanelOverlay : public PanelOverlay
	{
	protected:
		MaterialPtr m_pBorderMaterial;	//!< The border material
		Point4r m_ptBorderSize;			//!< The border size

	public:
		/**
		 * Constructor
		 *@param p_strName : [in] The overlay name
		 *@param p_pParent : [in] The parent overlay (if any)
		 */
		BorderPanelOverlay( OverlayManager * p_pManager, const String & p_strName, Overlay * p_pParent)
			:	PanelOverlay( p_pManager, p_strName, p_pParent)
		{
			m_eType = eOverlayBorderPanel;
		}
		/**
		 * Destructor
		 */
		virtual ~BorderPanelOverlay(){}
		/**
		 * Draws the overlay
		 */
		virtual void Render( ePRIMITIVE_TYPE p_displayMode);

		/**@name Inherited methods from Textable */
		//@{
		virtual bool Write( File & p_file)const;
		virtual bool Read( File & p_file) { return false; }
		//@}

		/**@name Inherited methods from Serialisable */
		//@{
		virtual bool Save( File & p_file)const;
		virtual bool Load( File & p_file);
		//@}

		/**@name Accessors */
		//@{
		inline real					GetLeftBorderSize	()const { return m_ptBorderSize[0]; }
		inline real					GetRightBorderSize	()const { return m_ptBorderSize[1]; }
		inline real					GetTopBorderSize	()const { return m_ptBorderSize[2]; }
		inline real					GetBottomBorderSize	()const { return m_ptBorderSize[3]; }
		inline const Point4r		GetBorderSize		()const { return m_ptBorderSize; }
		inline void SetLeftBorderSize	( real p_fSize)						{ m_ptBorderSize[0] = p_fSize; }
		inline void SetRightBorderSize	( real p_fSize)						{ m_ptBorderSize[1] = p_fSize; }
		inline void SetTopBorderSize	( real p_fSize)						{ m_ptBorderSize[2] = p_fSize; }
		inline void SetBottomBorderSize	( real p_fSize)						{ m_ptBorderSize[3] = p_fSize; }
		inline void SetBorderSize		( const Point4r & p_ptSize)			{ m_ptBorderSize = p_ptSize; }
		//@}
	};
	//! A text overlay
	/*!
	Describes an overlay containing a text. To be implemented
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class C3D_API TextOverlay : public Overlay
	{
	protected:
		String m_strCaption;		//!< The overlay caption
		TextFontPtr m_pFont;

	public:
		/**
		 * Constructor
		 *@param p_strName : [in] The overlay name
		 *@param p_pParent : [in] The parent overlay (if any)
		 */
		TextOverlay( OverlayManager * p_pManager, const String & p_strName, Overlay * p_pParent)
			:	Overlay( p_pManager, p_strName, p_pParent, eOverlayText)
		{
		}
		/**
		 * Destructor
		 */
		virtual ~TextOverlay();
		/**
		 * Draws the overlay
		 */
		virtual void Render( ePRIMITIVE_TYPE p_displayMode);
		void SetFont( const String & p_strFont, FontManager * p_pManager);
		void SetMaterial( MaterialPtr p_pMaterial);

		/**@name Inherited methods from Textable */
		//@{
		virtual bool Write( File & p_file)const;
		virtual bool Read( File & p_file) { return false; }
		//@}

		/**@name Inherited methods from Serialisable */
		//@{
		virtual bool Save( File & p_file)const;
		virtual bool Load( File & p_file);
		//@}

	public:
		/**@name Accessors */
		//@{
		inline String		GetCaption	()const { return m_strCaption; }
		inline TextFontPtr	GetFont		()const { return m_pFont; }

		inline void SetCaption	( const String & p_strCaption)	{ m_strCaption = p_strCaption; }

		VertexBufferPtr	GetVertexBuffer	()const;
		IndexBufferPtr	GetIndexBuffer	()const;
		//@}

	private:
		void _updateMaterial();
	};
	//! The Overlay renderer
	/*!
	\author Sylvain DOREMUS
	\version 0.1
	\date 09/02/2010
	*/
	class C3D_API OverlayRenderer : public Renderer<Overlay, OverlayRenderer>
	{
	protected:
		/**
		 * Constructor
		 */
		OverlayRenderer( SceneManager * p_pManager)
			:	Renderer<Overlay, OverlayRenderer>( p_pManager)
		{}
	public:
		/**
		 * Destructor
		 */
		virtual ~OverlayRenderer(){}

		virtual void DrawPanel()=0;
		virtual void DrawBorderPanel()=0;
		virtual void DrawText()=0;
	};
}

#endif
