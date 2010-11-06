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
#ifndef ___C3D_Subdiviser___
#define ___C3D_Subdiviser___

#include "../Module_Geometry.h"

namespace Castor3D
{
	//! Edge representation
	/*!
	An edge is defined between 2 vertices in a same face
	\author Sylvain DOREMUS
	\date 12/03/2010
	*/
	class Edge
	{
	private:
		VertexPtr m_firstVertex;
		VertexPtr m_secondVertex;
		FacePtr m_firstFace;
		FacePtr m_secondFace;

		VertexPtr m_createdVertex;
		bool m_divided;
		bool m_toDivide;
		bool m_toDelete;

		int m_refCount;

	public:
		/**
		 * Constructor
		 *@param p_v1 : [in] The 1st vector
		 *@param p_v2 : [in] The 2nd vector
		 *@param p_f1 : [in] The 1st face of the edge
		 *@param p_toDivide : [in] Tells if the edge is to be divided
		 */
		Edge( VertexPtr p_v1, VertexPtr p_v2, FacePtr p_f1, bool p_toDivide);
		/**
		 * Destructor
		 */
		~Edge();
		/**
		 * Adds a face to the edge (max 2 faces, 1 at each side of the edge)
		 */
		void AddFace( FacePtr p_face);
		/**
		 * Divides the edge id est adds a vertex in a portion of the edge determined by p_value (0.5 = middle).
		 * Doesn't divide the faces
		 */
		VertexPtr Divide( SubmeshPtr p_submesh, real p_value);

	public:
		inline bool			IsDivided	()						{ return m_divided; }
		inline VertexPtr 	GetVertex1	()						{ return m_firstVertex; }
		inline VertexPtr	GetVertex2	()						{ return m_secondVertex; }
		inline bool			IsInVertex	( VertexPtr p_vertex)	{ return m_firstVertex == p_vertex || m_secondVertex == p_vertex; }
		inline bool			IsToDelete	()						{ return m_toDelete; }

		inline int	Release		() { return --m_refCount; }
		inline void	Ref			() { m_refCount++; }
		inline void	SetToDivide	() { m_toDivide = true;m_divided = false;m_createdVertex = NULL; }
		inline void	SetToDelete	() { m_toDelete = true; }
	};


	//! Face edges representation
	/*!
	Holds the three edges constituted by a face
	\author Sylvain DOREMUS
	\date 25/08/2010
	*/
	class FaceEdges
	{
	private:
		SubmeshPtr m_submesh;
		FacePtr m_face;
		size_t m_sgIndex;
		EdgePtr m_edgeAB;
		EdgePtr m_edgeBC;
		EdgePtr m_edgeCA;

	public:
		/**
		 * Constructor
		 *@param p_submesh : [in] The submesh holding the face
		 *@param p_sgIndex : [in] The smoothing group index in the submesh
		 *@param p_face : [in] The face
		 *@param p_existingEdges : [in/out] The map of already existing edges, sorted by vertex
		 *@param p_vertexNeighbourhood : [in/out] The list of vertices neighbours
		 *@param p_allEdges : [in/out] All the edges
		 */
		FaceEdges( SubmeshPtr p_submesh, size_t p_sgIndex, FacePtr p_face, 
				   EdgePtrMapVPtrMap & p_existingEdges,
				   IntVPtrMap & p_vertexNeighbourhood,
				   std::set <EdgePtr> & p_allEdges);
		/**
		 * Constructor
		 *@param p_submesh : [in] The submesh holding the face
		 *@param p_sgIndex : [in] The smoothing group index in the submesh
		 *@param p_face : [in] The face
		 *@param l_ab : [in] Edge between 1st and 2nd vertex
		 *@param l_bc : [in] Edge between 2nd and 3rd vertex
		 *@param l_ca : [in] Edge between 3rd and 1st vertex
		 */
		FaceEdges( SubmeshPtr p_submesh, size_t p_sgIndex, FacePtr p_face, EdgePtr l_ab, EdgePtr l_bc, EdgePtr l_ca);
		/**
		 * Divides the edges held, creates needed faces to complete the division
		 *@param p_value : [in] The weight of division (if 0.5, divides all edges in the middle)
		 *@param p_existingEdges : [in/out] The map of already existing edges, sorted by vertex
		 *@param p_vertexNeighbourhood : [in/out] The list of vertices neighbours
		 *@param p_allEdges : [in/out] All the edges
		 *@param p_newFaces : [in/out] The array of newly created faces. All faces created by the subdivision are put inside it
		 *@param p_newVertices : [in/out] The array of newly created vertices. All vertices created by the subdivision are put inside it
		 */
		void Divide( real p_value, EdgePtrMapVPtrMap & p_existingEdges, IntVPtrMap & p_vertexNeighbourhood,
					 std::set <EdgePtr> & p_allEdges, FaceEdgesPtrArray & p_newFaces, VertexPtrArray & p_newVertices);

	private:
		EdgePtr _addEdge( VertexPtr p_v1, VertexPtr p_v2, bool p_toDivide, bool p_add1, bool p_add2,
						 EdgePtrMapVPtrMap & p_existingEdges, IntVPtrMap & p_vertexNeighbourhood, std::set <EdgePtr> & p_allEdges);
		void _removeEdge( EdgePtr p_edge, EdgePtrMapVPtrMap & p_existingEdges, std::set <EdgePtr> & p_allEdges);

	public:
		inline EdgePtr GetEdgesAB() { return m_edgeAB; }
		inline EdgePtr GetEdgesBC() { return m_edgeBC; }
		inline EdgePtr GetEdgesCA() { return m_edgeCA; }
	};
	//! Subdivisers main class
	/*!
	Abstract class for subdivisers, contains the header for the main Subdivide function
	\author Sylvain DOREMUS
	\date 12/03/2010
	*/
	class Subdiviser
	{
	protected:
		Submesh * m_submesh;					//!< The submesh being subdivided
		VertexPtrArray & m_vertex;				//!< All the vertices
		SmoothGroupPtrArray & m_smoothGroups;	//!< The submesh smoothing groups

	public:
		/**
		 * Constructor
		 *@param p_submesh : [in] The submesh to subdivide
		 */
		Subdiviser( Submesh * p_submesh);
		/**
		 * Destructor
		 */
		virtual ~Subdiviser(){}
		/**
		 * Main subdivision function, must be implemented by children classes
		 */
		virtual void Subdivide( Point3rPtr p_center)=0;

	protected:
		void _setTextCoords( FacePtr p_face, VertexPtr p_a, VertexPtr p_b, VertexPtr p_c, 
							 VertexPtr p_d, VertexPtr p_e, VertexPtr p_f, size_t p_sgIndex);
		VertexPtr _computeCenterFrom( const Vertex & p_a, real * p_aNormal,
									   const Vertex & p_b, real * p_bNormal);
		VertexPtr _computeCenterFrom( const Vertex & p_a, real * p_aNormal,
									   const Vertex & p_b, real * p_bNormal,
									   const Vertex & p_c, real * p_cNormal);
	};
}

#endif