#include "Castor3D/Mesh/SubmeshComponent/BonesComponent.hpp"

#include "Castor3D/Buffer/GpuBuffer.hpp"
#include "Castor3D/Mesh/Submesh.hpp"
#include "Castor3D/Mesh/Skeleton/BonedVertex.hpp"
#include "Castor3D/Miscellaneous/makeVkType.hpp"
#include "Castor3D/Render/RenderPass.hpp"
#include "Castor3D/Scene/Scene.hpp"

using namespace castor;

namespace castor3d
{
	String const BonesComponent::Name = cuT( "bones" );

	BonesComponent::BonesComponent( Submesh & submesh )
		: SubmeshComponent{ submesh, Name }
	{
	}

	BonesComponent::~BonesComponent()
	{
		cleanup();
		m_bones.clear();
	}

	void BonesComponent::addBoneDatas( VertexBoneData const * const begin
		, VertexBoneData const * const end )
	{
		m_bones.insert( m_bones.end(), begin, end );
	}

	SkeletonSPtr BonesComponent::getSkeleton()const
	{
		return getOwner()->getParent().getSkeleton();
	}

	void BonesComponent::gather( MaterialSPtr material
		, ashes::BufferCRefArray & buffers
		, std::vector< uint64_t > & offsets
		, ashes::PipelineVertexInputStateCreateInfoCRefArray & layouts )
	{
		buffers.emplace_back( m_bonesBuffer->getBuffer() );
		offsets.emplace_back( 0u );
		layouts.emplace_back( *m_bonesLayout );
	}

	bool BonesComponent::doInitialise()
	{
		auto & device = getCurrentRenderDevice( *getOwner() );

		if ( !m_bonesBuffer || m_bonesBuffer->getCount() != m_bones.size() )
		{
			m_bonesBuffer = makeVertexBuffer< VertexBoneData >( device
				, uint32_t( m_bones.size() )
				, 0u
				, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
				, getOwner()->getParent().getName() + "Submesh" + castor::string::toString( getOwner()->getId() ) + "BonesComponentBuffer" );
			
			m_bonesLayout = std::make_unique< ashes::PipelineVertexInputStateCreateInfo >( 0u
				, ashes::VkVertexInputBindingDescriptionArray
				{
					{ BindingPoint, sizeof( VertexBoneData ), VK_VERTEX_INPUT_RATE_VERTEX },
				}
				, ashes::VkVertexInputAttributeDescriptionArray
				{
					{ RenderPass::VertexInputs::BoneIds0Location, 0u, VK_FORMAT_R32G32B32A32_SINT, offsetof( VertexBoneData::Ids::ids, id0 ) },
					{ RenderPass::VertexInputs::BoneIds1Location, 0u, VK_FORMAT_R32G32B32A32_SINT, offsetof( VertexBoneData::Ids::ids, id1 ) },
					{ RenderPass::VertexInputs::Weights0Location, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof( VertexBoneData::Weights::weights, weight0 ) },
					{ RenderPass::VertexInputs::Weights1Location, 0u, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof( VertexBoneData::Weights::weights, weight1 ) },
				} );
		}

		return m_bonesBuffer != nullptr;
	}

	void BonesComponent::doCleanup()
	{
		m_bonesLayout.reset();
		m_bonesBuffer.reset();
	}

	void BonesComponent::doFill()
	{
	}

	void BonesComponent::doUpload()
	{
		auto count = uint32_t( m_bones.size() );
		auto itbones = m_bones.begin();

		if ( count )
		{
			if ( auto * buffer = m_bonesBuffer->lock( 0, count, 0u ) )
			{
				std::copy( m_bones.begin(), m_bones.end(), buffer );
				m_bonesBuffer->flush( 0u, count );
				m_bonesBuffer->unlock();
			}

			//m_bones.clear();
			//m_bonesData.clear();
		}
	}
}