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
#ifndef ___Castor_Plane___
#define ___Castor_Plane___

#include "Line.h"

namespace Castor
{	namespace Math
{
	/*!
	Plane representation : ax + by + cz + d = 0
	\author Sylvain DOREMUS
	\date 14/02/2010
	*/
	template <typename T>
	class Plane
	{
	private:
		typedef Templates::Value<T> value;
	public:
		Point<T, 3> m_origin;
		Point<T, 3> m_normal;	// a, b, c coefficients
		real m_d;			// d coefficient

	public:
		Plane( const Point<T, 3> & p_p1, const Point<T, 3> & p_p2, const Point<T, 3> & p_p3)
		{
			Point<T, 3> l_v( p_p2 - p_p1);
			Point<T, 3> l_w( p_p3 - p_p1);
			m_normal = l_v ^ l_w;

			m_normal.Normalise();
			m_origin = Point<T, 3>( p_p1);
			m_d = 0.0f - m_normal.dotProduct( m_origin);
		}

		bool IsParallel( const Plane & p_plane)const
		{
			T l_ratioA = m_normal[0] / p_plane.m_normal[0];
			T l_ratioB = m_normal[1] / p_plane.m_normal[1];
			T l_ratioC = m_normal[2] / p_plane.m_normal[2];

			return (value::equals( l_ratioA, l_ratioB) && value::equals( l_ratioA, l_ratioC));
		}

		bool operator ==( const Plane & p_plane)const
		{
			bool l_bReturn = false;

			if (IsParallel( p_plane))
			{
				T l_ratioA = m_normal[0] / p_plane.m_normal[0];
				T l_ratioD = m_d / p_plane.m_d;
				l_bReturn = value::equals( l_ratioA, l_ratioD);
			}

			return l_bReturn;
		}

		bool operator !=( const Plane & p_plane)const
		{
			return ( ! (*this == p_plane));
		}

		bool Intersects( const Plane & p_plane, Line<T> & p_line)const
		{
			bool l_bReturn = false;

			if ( ! IsParallel( p_plane))
			{
				Point3r l_normal = m_normal ^ p_plane.m_normal;
				//TODO : trouver le point d'origine
				l_bReturn = true;
			}

			return l_bReturn;
		}

		bool Intersects( const Plane & p_plane1, const Plane & p_plane2, Point<T, 3> & p_intersection)const
		{
			bool l_bReturn = false;

			if ( ! IsParallel( p_plane1) && ! IsParallel( p_plane2) && ! p_plane1.IsParallel( p_plane2))
			{
				T a1 = m_normal[0], a2 = p_plane1.m_normal[0], a3 = p_plane2.m_normal[0];
				T b1 = m_normal[1], b2 = p_plane1.m_normal[1], b3 = p_plane2.m_normal[1];
				T c1 = m_normal[2], c2 = p_plane1.m_normal[2], c3 = p_plane2.m_normal[2];
				T alpha, beta;

				alpha = (a3 - (a2 * (b3 - (a3 / a1)) / (b2 - (a2 / a1)))) / a1;
				beta = (b3 - (a3 / a1)) / (b2 - (a2 / a1));

				T l_c3 = (c1 * alpha) + (c2 * beta);

				if ( ! value::equals( c3, l_c3))
				{
					alpha = ((a2 * c1) / (a1 * (b2 - (a2 * b1) / a1))) - (c2 / (b2 - (a2 * b1) / a1));
					beta = ((a2 * m_d) / (a1 * (b2 - (a2 * b1) / a1))) - (p_plane1.m_d / (b2 - (a2 * b1) / a1));

					T x, y, z;

					z = ((a3 * ((m_d + (beta * b1)) / a1)) - p_plane2.m_d) / ((b3 * alpha) + c3 - (a3 * ((alpha * b1) + c1) / a1));
					y = (alpha * z) + beta;
					x = (z * (0.0f - ((alpha * b1) + c1)) / a1) - ((m_d + (b1 * beta)) / a1);

					p_intersection[0] = x;
					p_intersection[1] = y;
					p_intersection[2] = z;

					l_bReturn = true;
				}
			}

			return l_bReturn;
		}
	};
}
}

#endif