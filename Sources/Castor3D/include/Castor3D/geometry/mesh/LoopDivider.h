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
#ifndef ___C3D_LoopDivider___
#define ___C3D_LoopDivider___

#include "Subdiviser.h"

namespace Castor3D
{
	//! Loop subdivision algorithm Edge List
	/*!
	Specialisation of EdgeList for Loop subdivision algorithm. Creates LoopEdge instead of Edge
	\author Sylvain DOREMUS
	\date 12/03/2010
	*/
	class LoopEdgeList
	{
	private:
		SubmeshPtr m_submesh;
		std::set <EdgePtr> m_allEdges;
		EdgePtrMapVPtrMap m_vertexEdges;
		VertexPtrVPtrMap m_originalVertices;
		IntVPtrMap m_vertexNeighbourhood;
		FaceEdgesPtrArray m_facesEdges;

	public:
		LoopEdgeList( SubmeshPtr p_submesh);
		~LoopEdgeList();

		void Divide();
		void Average( Point3rPtr p_center);

	private:
		void _fill();
	};
	//! Loop subdivision algorithm Subdiviser
	/*!
	This class implements the Loop subdivision algorithm
	\author Sylvain DOREMUS
	\date 12/03/2010
	*/
	class LoopSubdiviser : public Subdiviser
	{
	private:
		LoopEdgeList * m_edges;

	public:
		LoopSubdiviser( Submesh * p_submesh);
		virtual ~LoopSubdiviser();

		virtual void Subdivide( Point3rPtr p_center);
	};
}

#endif