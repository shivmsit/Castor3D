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
#ifndef ___C3D_Vertex___
#define ___C3D_Vertex___

#include "../../Prerequisites.h"
#include "../../render_system/BufferElement.h"

namespace Castor3D
{
	//! A simple Point with an index
	template <typename T, size_t Count>
	class C3D_API IdPoint : public Point<T, Count>, public MemoryTraced< IdPoint<T, Count> >
	{
	public:
		typedef T									value_type;
		typedef Policy<value_type>					policy;

	private:
		typedef value_type &						reference;
		typedef value_type *						pointer;
		typedef const value_type &					const_reference;
		typedef const value_type *					const_pointer;
		typedef Point<value_type, Count>			point;
		typedef Point<value_type, Count> &			point_reference;
		typedef Point<value_type, Count> *			point_pointer;
		typedef const Point<value_type, Count> &	const_point_reference;
		typedef const Point<value_type, Count> *	const_point_pointer;

	private:
		size_t m_uiIndex;

	public:
		IdPoint( size_t p_uiIndex = 0)
			:	point(),
				m_uiIndex( p_uiIndex)
		{
		}
		IdPoint( const IdPoint<T, Count> & p_ptPoint, bool p_bLinked = false)
			:	point( p_ptPoint),
				m_uiIndex( p_ptPoint.m_uiIndex)
		{
			if (p_bLinked)
			{
				LinkCoords( p_ptPoint.const_ptr());
			}
		}
		template <typename _Ty>
		IdPoint( const _Ty * p_pCoords, size_t p_uiIndex = 0, bool p_bLinked = false)
			:	point( p_pCoords),
				m_uiIndex( p_uiIndex)
		{
			if (p_bLinked)
			{
				LinkCoords( p_pCoords);
			}
		}
		IdPoint( const_point_reference p_ptPoint, size_t p_uiIndex = 0, bool p_bLinked = false)
			:	point( p_ptPoint),
				m_uiIndex( p_uiIndex)
		{
			if (p_bLinked)
			{
				LinkCoords( p_ptPoint.const_ptr());
			}
		}
		template <typename _Ty>
		IdPoint( const _Ty & p_a, size_t p_uiIndex = 0)
			:	point( p_a),
				m_uiIndex( p_uiIndex)
		{
		}
		template <typename _Ty>
		IdPoint( const _Ty & p_a, const _Ty & p_b, size_t p_uiIndex = 0)
			:	point( p_a, p_b),
				m_uiIndex( p_uiIndex)
		{
		}
		template <typename _Ty>
		IdPoint( const _Ty & p_a, const _Ty & p_b, const _Ty & p_c, size_t p_uiIndex = 0)
			:	point( p_a, p_b, p_c),
				m_uiIndex( p_uiIndex)
		{
		}
		template <typename _Ty>
		IdPoint( const _Ty & p_a, const _Ty & p_b, const _Ty & p_c, const _Ty & p_d, size_t p_uiIndex = 0)
			:	point( p_a, p_b, p_c, p_d),
			m_uiIndex( p_uiIndex)
		{
		}
		template <typename _Ty, size_t _Count>
		IdPoint( const Point<_Ty, _Count> & p_ptPoint, size_t p_uiIndex = 0, bool p_bLinked = false)
			:	point( p_ptPoint),
				m_uiIndex( p_uiIndex)
		{
			if (p_bLinked)
			{
				LinkCoords( p_ptPoint.const_ptr());
			}
		}
		virtual ~IdPoint()
		{
		}
	public:
		/**@name Operators */
		//@{
		inline IdPoint<T, Count> &	operator =( const IdPoint<T, Count> & p_toCopy)
		{
			Point<T, Count>::operator =( p_toCopy);
			m_uiIndex = p_toCopy.m_uiIndex;
			return * this;
		}
		//@}

		/**@name Accessors */
		//@{
		inline size_t	GetIndex	()const { return m_uiIndex; }
		inline void	SetIndex	( const size_t & val)	{ m_uiIndex = val; }
		//@}
	};

	//! Representation of a vertex
	/*!
	The 3 coordinates of a vertex, it's normal, tangent and texture coordinates, an optional index for smoothing groups
	\author Sylvain DOREMUS
	\version 0.1
	\date 09/02/2010
	*/
	class C3D_API Vertex : public BufferElement, public MemoryTraced<Vertex>
	{
	public:
		static size_t Size;					//!< Number of components of a Vertex (total of coordinates for coords, normal, tangent, texture coords).
		static unsigned long long Count;	//!< Total number of created vertex
		static BufferElementDeclaration VertexDeclarationElements[];
		static BufferDeclaration VertexDeclaration;

	private:
		size_t m_uiIndex;
		bool m_bManual;

	public:
		/**
		* Specified constructor
		*/
		Vertex( real x = 0, real y = 0, real z = 0, size_t p_uiIndex = 0);
		/**
		* Copy constructor
		*/
		explicit Vertex( const Vertex & p_vertex);
		/**
		* Constructor from point
		*/
		Vertex( const Point3r & p_point);
		/**
		* Constructor from point
		*/
		Vertex( const IdPoint3r & p_idPoint);
		/**
		* Constructor from a real array
		*/
		Vertex( real * p_pBuffer);
		/**
		* Constructor from the difference between second and first argument
		*/
		Vertex( const Vertex & p_v1, const Vertex & p_v2);
		/**
		* Destructor
		*/
		virtual ~Vertex();
		/**
		 * Links the vertex coords to the ones in parameter.
		 * The vertex no longer owns it's coords
		 *@param p_pBuffer : [in] The coordinates buffer
		 */
		void LinkCoords( real * p_pBuffer);
		/**
		 * Unlinks the vertex coords.
		 * The vertex now owns it's coords
		 */
		void UnlinkCoords();

		/**@name Assignment */
		//@{
		void SetNormal	( const Point3r & val, bool p_bManual = false);
		void SetNormal	( real x, real y, real z, bool p_bManual = false);
		void SetNormal	( const real * p_pCoords, bool p_bManual = false);

		void SetTexCoord	( const Point2r & val, bool p_bManual = false);
		void SetTexCoord	( real x, real y, bool p_bManual = false);
		void SetTexCoord	( const real * p_pCoords, bool p_bManual = false);

		void SetTangent	( const Point3r & val, bool p_bManual = false);
		void SetTangent	( real x, real y, real z, bool p_bManual = false);
		void SetTangent	( const real * p_pCoords, bool p_bManual = false);
		//@}

		/**@name Operators */
		//@{
		inline real &			operator []( size_t p_uiIndex)		{ return GetElement<Point3r>( 0)[p_uiIndex]; }
		inline const real &		operator []( size_t p_uiIndex)const	{ return GetElement<Point3r>( 0)[p_uiIndex]; }
		virtual Vertex & 		operator =( const Vertex & p_vertex);
		virtual Vertex & 		operator =( const IdPoint3r & p_idPoint);
		//@}

		/**@name Accessors */
		//@{
		inline Point3r &		GetCoords	()		{ return GetElement<Point3r>( 0); }
		inline Point3r &		GetNormal	()		{ return GetElement<Point3r>( 1); }
		inline Point3r &		GetTangent	()		{ return GetElement<Point3r>( 2); }
		inline Point2r &		GetTexCoord	()		{ return GetElement<Point2r>( 3); }
		inline const Point3r &	GetCoords	()const	{ return GetElement<Point3r>( 0); }
		inline const Point3r &	GetNormal	()const	{ return GetElement<Point3r>( 1); }
		inline const Point3r &	GetTangent	()const	{ return GetElement<Point3r>( 2); }
		inline const Point2r &	GetTexCoord	()const	{ return GetElement<Point2r>( 3); }
		inline const real *		const_ptr	()const	{ return (const real *)( BufferElement::const_ptr()); }
		inline real *			ptr			()		{ return (real *)( BufferElement::ptr()); }
		inline size_t			GetIndex	()const { return m_uiIndex; }
		inline bool				IsManual	()const	{ return m_bManual; }
		inline void SetCoords	( const Point3r & val)						{ GetElement<Point3r>( 0) = val; }
		inline void SetCoords	( real x, real y, real z)					{ GetElement<Point3r>( 0)[0] = x;GetElement<Point3r>( 0)[1] = y;GetElement<Point3r>( 0)[2] = z; }
		inline void SetCoords	( const real * p_pCoords)					{ GetElement<Point3r>( 0)[0] = GetElement<Point3r>( 0)[0];GetElement<Point3r>( 0)[1] = p_pCoords[1];GetElement<Point3r>( 0)[2] = p_pCoords[2]; }
		inline void SetIndex	( const size_t & val)						{ m_uiIndex = val; }
		//@}

	private:
		void _link();
		void _computeTangent();
	};
}

#endif
