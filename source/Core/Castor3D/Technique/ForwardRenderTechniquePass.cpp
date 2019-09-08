#include "Castor3D/Technique/ForwardRenderTechniquePass.hpp"

#include "Castor3D/Cache/MaterialCache.hpp"
#include "Castor3D/EnvironmentMap/EnvironmentMap.hpp"
#include "Castor3D/Mesh/Submesh.hpp"
#include "Castor3D/Render/RenderPassTimer.hpp"
#include "Castor3D/Render/RenderPipeline.hpp"
#include "Castor3D/Render/RenderTarget.hpp"
#include "Castor3D/Render/RenderNode/RenderNode_Render.hpp"
#include "Castor3D/Scene/Animation/AnimatedObjectGroup.hpp"
#include "Castor3D/Shader/PassBuffer/PassBuffer.hpp"
#include "Castor3D/Shader/TextureConfigurationBuffer/TextureConfigurationBuffer.hpp"
#include "Castor3D/Shader/Shaders/GlslFog.hpp"
#include "Castor3D/Shader/Shaders/GlslMaterial.hpp"
#include "Castor3D/Shader/Shaders/GlslMetallicPbrReflection.hpp"
#include "Castor3D/Shader/Shaders/GlslMetallicBrdfLighting.hpp"
#include "Castor3D/Shader/Shaders/GlslPhongLighting.hpp"
#include "Castor3D/Shader/Shaders/GlslPhongReflection.hpp"
#include "Castor3D/Shader/Shaders/GlslSpecularBrdfLighting.hpp"
#include "Castor3D/Shader/Shaders/GlslSpecularPbrReflection.hpp"
#include "Castor3D/Shader/Shaders/GlslTextureConfiguration.hpp"
#include "Castor3D/Shader/Shaders/GlslUtils.hpp"
#include "Castor3D/Shader/Ubos/TexturesUbo.hpp"

#include <ShaderWriter/Source.hpp>

#include <ashespp/Image/ImageView.hpp>
#include <ashespp/RenderPass/RenderPassCreateInfo.hpp>

using namespace castor;

namespace castor3d
{
	//*********************************************************************************************

	namespace
	{
		void doBindTexture( ashes::ImageView const & view
			, ashes::Sampler const & sampler
			, ashes::WriteDescriptorSetArray & writes
			, uint32_t & index )
		{
			writes.push_back( ashes::WriteDescriptorSet
				{
					index++,
					0u,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					{
						1u,
						{
							sampler,
							view,
							VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
						},
					}
				} );
		}

		void doBindShadowMaps( ShadowMapRefArray const & shadowMaps
			, ashes::WriteDescriptorSetArray & writes
			, uint32_t & index )
		{
			std::vector< VkDescriptorImageInfo > shadowMapWrites;

			for ( auto & shadowMap : shadowMaps )
			{
				shadowMapWrites.push_back( {
					shadowMap.first.get().getLinearSampler(),
					shadowMap.first.get().getLinearView(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					} );
				shadowMapWrites.push_back( {
					shadowMap.first.get().getVarianceSampler(),
					shadowMap.first.get().getVarianceView(),
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					} );
			}

			writes.push_back( ashes::WriteDescriptorSet
				{
					index,
					0u,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					shadowMapWrites,
				} );
			index += uint32_t( shadowMapWrites.size() );
		}
	}

	//*********************************************************************************************

	ForwardRenderTechniquePass::ForwardRenderTechniquePass( String const & name
		, MatrixUbo const & matrixUbo
		, SceneCuller & culler
		, bool environment
		, SceneNode const * ignored
		, SsaoConfig const & config )
		: RenderTechniquePass{ name
			, name
			, matrixUbo
			, culler
			, environment
			, ignored
			, config }
	{
	}

	ForwardRenderTechniquePass::ForwardRenderTechniquePass( String const & name
		, MatrixUbo const & matrixUbo
		, SceneCuller & culler
		, bool oit
		, bool environment
		, SceneNode const * ignored
		, SsaoConfig const & config )
		: RenderTechniquePass{ name
			, name
			, matrixUbo
			, culler
			, oit
			, environment
			, ignored
			, config }
	{
	}

	ForwardRenderTechniquePass::~ForwardRenderTechniquePass()
	{
	}

	void ForwardRenderTechniquePass::initialiseRenderPass( ashes::ImageView const & colourView
		, ashes::ImageView const & depthView
		, castor::Size const & size
		, bool clear )
	{
		ashes::VkAttachmentDescriptionArray attaches
		{
			{
				0u,
				depthView.getFormat(),
				VK_SAMPLE_COUNT_1_BIT,
				( clear
					? VK_ATTACHMENT_LOAD_OP_CLEAR
					: VK_ATTACHMENT_LOAD_OP_LOAD ),
				VK_ATTACHMENT_STORE_OP_STORE,
				( ( clear && ashes::isDepthStencilFormat( depthView.getFormat() ) )
					? VK_ATTACHMENT_LOAD_OP_CLEAR
					: VK_ATTACHMENT_LOAD_OP_DONT_CARE ),
				( ashes::isDepthStencilFormat( depthView.getFormat() )
					? VK_ATTACHMENT_STORE_OP_STORE
					: VK_ATTACHMENT_STORE_OP_DONT_CARE ),
				( clear
					? VK_IMAGE_LAYOUT_UNDEFINED
					: VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ),
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			},
			{
				1u,
				colourView.getFormat(),
				VK_SAMPLE_COUNT_1_BIT,
				( clear
					? VK_ATTACHMENT_LOAD_OP_CLEAR
					: VK_ATTACHMENT_LOAD_OP_LOAD ),
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				( clear
					? VK_IMAGE_LAYOUT_UNDEFINED
					: VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL ),
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			},
		};
		ashes::SubpassDescriptionArray subpasses;
		subpasses.emplace_back( ashes::SubpassDescription
			{
				0u,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				{},
				{ { 1u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
				{},
				VkAttachmentReference{ 0u, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL },
				{},
			} );
		ashes::VkSubpassDependencyArray dependencies
		{
			{
				VK_SUBPASS_EXTERNAL,
				0u,
				VkPipelineStageFlags( clear
					? VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
					: VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT ),
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkAccessFlags( clear
					? VK_ACCESS_SHADER_READ_BIT
					: VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ),
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
			},
			{
				0u,
				VK_SUBPASS_EXTERNAL,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VkPipelineStageFlags( clear
					? VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
					: VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT ),
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				VkAccessFlags( clear
					? VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
					: VK_ACCESS_SHADER_READ_BIT ),
				VK_DEPENDENCY_BY_REGION_BIT,
			}
		};
		ashes::RenderPassCreateInfo createInfo
		{
			0u,
			std::move( attaches ),
			std::move( subpasses ),
			std::move( dependencies ),
		};
		auto & device = getCurrentRenderDevice( *this );
		m_renderPass = device->createRenderPass( std::move( createInfo ) );
		ashes::ImageViewCRefArray fbAttaches;
		fbAttaches.emplace_back( depthView );
		fbAttaches.emplace_back( colourView );

		m_frameBuffer = m_renderPass->createFrameBuffer( { colourView.image->getDimensions().width, colourView.image->getDimensions().height }
			, std::move( fbAttaches ) );

		m_nodesCommands = device.graphicsCommandPool->createCommandBuffer();
		m_fence = device->createFence( VK_FENCE_CREATE_SIGNALED_BIT );
	}

	void ForwardRenderTechniquePass::accept( RenderTechniqueVisitor & visitor )
	{
		auto shaderProgram = getEngine()->getShaderProgramCache().getAutomaticProgram( *this
			, visitor.getFlags() );
		visitor.visit( cuT( "Object" )
			, VK_SHADER_STAGE_VERTEX_BIT
			, *shaderProgram->getSource( VK_SHADER_STAGE_VERTEX_BIT ).shader );
		visitor.visit( cuT( "Object" )
			, VK_SHADER_STAGE_FRAGMENT_BIT
			, *shaderProgram->getSource( VK_SHADER_STAGE_FRAGMENT_BIT ).shader );
	}

	void ForwardRenderTechniquePass::update( RenderInfo & info
		, Point2r const & jitter )
	{
		getSceneUbo().update( m_scene, m_camera );
		doUpdate( info, jitter );
	}

	ashes::Semaphore const & ForwardRenderTechniquePass::render( ashes::Semaphore const & toWait )
	{
		ashes::Semaphore const * result = &toWait;

		if ( hasNodes() )
		{
			static ashes::VkClearValueArray const clearValues
			{
				ashes::makeClearValue( VkClearDepthStencilValue{ 1.0, 0 } ),
				ashes::makeClearValue( VkClearColorValue{ 0.0f, 0.0f, 0.0f, 1.0f } ),
			};

			getEngine()->setPerObjectLighting( true );
			auto timerBlock = getTimer().start();
			auto & device = getCurrentRenderDevice( *this );

			m_nodesCommands->begin( VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );
			getTimer().beginPass( *m_nodesCommands );
			getTimer().notifyPassRender();
			m_nodesCommands->beginRenderPass( getRenderPass()
				, *m_frameBuffer
				, clearValues
				, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS );
			m_nodesCommands->executeCommands( { getCommandBuffer() } );
			m_nodesCommands->endRenderPass();
			getTimer().endPass( *m_nodesCommands );
			m_nodesCommands->end();

			device.graphicsQueue->submit( { *m_nodesCommands }
				, { *result }
				, { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }
				, { getSemaphore() }
				, nullptr );
			result = &getSemaphore();
		}

		return *result;
	}

	void ForwardRenderTechniquePass::doCleanup()
	{
		m_fence.reset();
		m_nodesCommands.reset();
		m_frameBuffer.reset();
		RenderTechniquePass::doCleanup();
	}

	ashes::VkDescriptorSetLayoutBindingArray ForwardRenderTechniquePass::doCreateUboBindings( PipelineFlags const & flags )const
	{
		ashes::VkDescriptorSetLayoutBindingArray uboBindings;
		uboBindings.emplace_back( getEngine()->getMaterialCache().getPassBuffer().createLayoutBinding() );
		uboBindings.emplace_back( getEngine()->getMaterialCache().getTextureBuffer().createLayoutBinding() );

		if ( checkFlag( flags.programFlags, ProgramFlag::eLighting ) )
		{
			uboBindings.emplace_back( makeDescriptorSetLayoutBinding( LightBufferIndex
				, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
				, VK_SHADER_STAGE_FRAGMENT_BIT ) );
		}

		uboBindings.emplace_back( makeDescriptorSetLayoutBinding( MatrixUbo::BindingPoint
			, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT ) );
		uboBindings.emplace_back( makeDescriptorSetLayoutBinding( SceneUbo::BindingPoint
			, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT ) );

		if ( !checkFlag( flags.programFlags, ProgramFlag::eInstantiation ) )
		{
			uboBindings.emplace_back( makeDescriptorSetLayoutBinding( ModelMatrixUbo::BindingPoint
				, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, VK_SHADER_STAGE_VERTEX_BIT ) );
		}

		uboBindings.emplace_back( makeDescriptorSetLayoutBinding( ModelUbo::BindingPoint
			, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT ) );
		uboBindings.emplace_back( makeDescriptorSetLayoutBinding( TexturesUbo::BindingPoint
			, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );

		if ( checkFlag( flags.programFlags, ProgramFlag::eSkinning ) )
		{
			uboBindings.push_back( SkinningUbo::createLayoutBinding( SkinningUbo::BindingPoint
				, flags.programFlags ) );
		}

		if ( checkFlag( flags.programFlags, ProgramFlag::eMorphing ) )
		{
			uboBindings.emplace_back( makeDescriptorSetLayoutBinding( MorphingUbo::BindingPoint
				, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, VK_SHADER_STAGE_VERTEX_BIT ) );
		}

		if ( checkFlag( flags.programFlags, ProgramFlag::eBillboards ) )
		{
			uboBindings.emplace_back( makeDescriptorSetLayoutBinding( BillboardUbo::BindingPoint
				, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, VK_SHADER_STAGE_VERTEX_BIT ) );
		}

		return uboBindings;
	}

	ashes::VkDescriptorSetLayoutBindingArray ForwardRenderTechniquePass::doCreateTextureBindings( PipelineFlags const & flags )const
	{
		auto index = MinTextureIndex;
		ashes::VkDescriptorSetLayoutBindingArray textureBindings;

		if ( flags.texturesCount )
		{
			textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT
				, flags.texturesCount ) );
			index += flags.texturesCount;
		}

		if ( checkFlag( flags.textures, TextureFlag::eReflection )
			|| checkFlag( flags.textures, TextureFlag::eRefraction ) )
		{
			textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT ) );
		}

		if ( checkFlag( flags.passFlags, PassFlag::eMetallicRoughness )
			|| checkFlag( flags.passFlags, PassFlag::eSpecularGlossiness ) )
		{
			textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapIrradiance
			textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapPrefiltered
			textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapBrdf
		}

		textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapDepthDirectional
		textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapShadowDirectional
		textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapDepthSpot
		textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapShadowSpot
		textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapDepthPoint
		textureBindings.emplace_back( makeDescriptorSetLayoutBinding( index++
			, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
			, VK_SHADER_STAGE_FRAGMENT_BIT ) );	// c3d_mapShadowPoint
		return textureBindings;
	}

	void ForwardRenderTechniquePass::doFillTextureDescriptor( ashes::DescriptorSetLayout const & layout
		, uint32_t & index
		, BillboardListRenderNode & node
		, ShadowMapLightTypeArray const & shadowMaps )
	{
		ashes::WriteDescriptorSetArray writes;
		node.passNode.fillDescriptor( layout
			, index
			, writes );

		if ( node.passNode.pass.hasEnvironmentMapping() )
		{
			auto & envMap = m_scene.getEnvironmentMap( node.sceneNode );
			doBindTexture( envMap.getTexture().getTexture()->getDefaultView()
				, envMap.getTexture().getSampler()->getSampler()
				, writes
				, index );
		}

		if ( node.passNode.pass.getType() != MaterialType::ePhong )
		{
			auto & background = *node.sceneNode.getScene()->getBackground();

			if ( background.hasIbl() )
			{
				auto & ibl = background.getIbl();
				doBindTexture( ibl.getIrradianceTexture()
					, ibl.getIrradianceSampler()
					, writes
					, index );
				doBindTexture( ibl.getPrefilteredEnvironmentTexture()
					, ibl.getPrefilteredEnvironmentSampler()
					, writes
					, index );
				doBindTexture( ibl.getPrefilteredBrdfTexture()
					, ibl.getPrefilteredBrdfSampler()
					, writes
					, index );
			}
		}

		doBindShadowMaps( shadowMaps[size_t( LightType::eDirectional )], writes, index );
		doBindShadowMaps( shadowMaps[size_t( LightType::eSpot )], writes, index );
		doBindShadowMaps( shadowMaps[size_t( LightType::ePoint )], writes, index );
		node.texDescriptorSet->setBindings( writes );
	}

	void ForwardRenderTechniquePass::doFillTextureDescriptor( ashes::DescriptorSetLayout const & layout
		, uint32_t & index
		, SubmeshRenderNode & node
		, ShadowMapLightTypeArray const & shadowMaps )
	{
		ashes::WriteDescriptorSetArray writes;
		node.passNode.fillDescriptor( layout
			, index
			, writes );

		if ( node.passNode.pass.hasEnvironmentMapping() )
		{
			auto & envMap = m_scene.getEnvironmentMap( node.sceneNode );
			doBindTexture( envMap.getTexture().getTexture()->getDefaultView()
				, envMap.getTexture().getSampler()->getSampler()
				, writes
				, index );
		}

		if ( node.passNode.pass.getType() != MaterialType::ePhong )
		{
			auto & background = *node.sceneNode.getScene()->getBackground();

			if ( background.hasIbl() )
			{
				auto & ibl = background.getIbl();
				doBindTexture( ibl.getIrradianceTexture()
					, ibl.getIrradianceSampler()
					, writes
					, index );
				doBindTexture( ibl.getPrefilteredEnvironmentTexture()
					, ibl.getPrefilteredEnvironmentSampler()
					, writes
					, index );
				doBindTexture( ibl.getPrefilteredBrdfTexture()
					, ibl.getPrefilteredBrdfSampler()
					, writes
					, index );
			}
		}

		doBindShadowMaps( shadowMaps[size_t( LightType::eDirectional )], writes, index );
		doBindShadowMaps( shadowMaps[size_t( LightType::eSpot )], writes, index );
		doBindShadowMaps( shadowMaps[size_t( LightType::ePoint )], writes, index );
		node.texDescriptorSet->setBindings( writes );
	}

	ShaderPtr ForwardRenderTechniquePass::doGetVertexShaderSource( PipelineFlags const & flags )const
	{
		// Since their vertex attribute locations overlap, we must not have both set at the same time.
		CU_Require( ( checkFlag( flags.programFlags, ProgramFlag::eInstantiation ) ? 1 : 0 )
			+ ( checkFlag( flags.programFlags, ProgramFlag::eMorphing ) ? 1 : 0 ) < 2 );
		using namespace sdw;
		VertexWriter writer;
		// Vertex inputs
		auto position = writer.declInput< Vec4 >( cuT( "position" )
			, RenderPass::VertexInputs::PositionLocation );
		auto normal = writer.declInput< Vec3 >( cuT( "normal" )
			, RenderPass::VertexInputs::NormalLocation );
		auto tangent = writer.declInput< Vec3 >( cuT( "tangent" )
			, RenderPass::VertexInputs::TangentLocation );
		auto uv = writer.declInput< Vec3 >( cuT( "uv" )
			, RenderPass::VertexInputs::TextureLocation );
		auto bone_ids0 = writer.declInput< IVec4 >( cuT( "bone_ids0" )
			, RenderPass::VertexInputs::BoneIds0Location
			, checkFlag( flags.programFlags, ProgramFlag::eSkinning ) );
		auto bone_ids1 = writer.declInput< IVec4 >( cuT( "bone_ids1" )
			, RenderPass::VertexInputs::BoneIds1Location
			, checkFlag( flags.programFlags, ProgramFlag::eSkinning ) );
		auto weights0 = writer.declInput< Vec4 >( cuT( "weights0" )
			, RenderPass::VertexInputs::Weights0Location
			, checkFlag( flags.programFlags, ProgramFlag::eSkinning ) );
		auto weights1 = writer.declInput< Vec4 >( cuT( "weights1" )
			, RenderPass::VertexInputs::Weights1Location
			, checkFlag( flags.programFlags, ProgramFlag::eSkinning ) );
		auto transform = writer.declInput< Mat4 >( cuT( "transform" )
			, RenderPass::VertexInputs::TransformLocation
			, checkFlag( flags.programFlags, ProgramFlag::eInstantiation ) );
		auto material = writer.declInput< Int >( cuT( "material" )
			, RenderPass::VertexInputs::MaterialLocation
			, checkFlag( flags.programFlags, ProgramFlag::eInstantiation ) );
		auto position2 = writer.declInput< Vec4 >( cuT( "position2" )
			, RenderPass::VertexInputs::Position2Location
			, checkFlag( flags.programFlags, ProgramFlag::eMorphing ) );
		auto normal2 = writer.declInput< Vec3 >( cuT( "normal2" )
			, RenderPass::VertexInputs::Normal2Location
			, checkFlag( flags.programFlags, ProgramFlag::eMorphing ) );
		auto tangent2 = writer.declInput< Vec3 >( cuT( "tangent2" )
			, RenderPass::VertexInputs::Tangent2Location
			, checkFlag( flags.programFlags, ProgramFlag::eMorphing ) );
		auto texture2 = writer.declInput< Vec3 >( cuT( "texture2" )
			, RenderPass::VertexInputs::Texture2Location
			, checkFlag( flags.programFlags, ProgramFlag::eMorphing ) );
		auto in = writer.getIn();

		UBO_MATRIX( writer, MatrixUbo::BindingPoint, 0 );
		UBO_MODEL_MATRIX( writer, ModelMatrixUbo::BindingPoint, 0 );
		UBO_SCENE( writer, SceneUbo::BindingPoint, 0 );
		UBO_MODEL( writer, ModelUbo::BindingPoint, 0 );
		auto skinningData = SkinningUbo::declare( writer, SkinningUbo::BindingPoint, 0, flags.programFlags );
		UBO_MORPHING( writer, MorphingUbo::BindingPoint, 0, flags.programFlags );

		// Outputs
		auto vtx_worldPosition = writer.declOutput< Vec3 >( cuT( "vtx_worldPosition" )
			, RenderPass::VertexOutputs::WorldPositionLocation );
		auto vtx_viewPosition = writer.declOutput< Vec3 >( cuT( "vtx_viewPosition" )
			, RenderPass::VertexOutputs::ViewPositionLocation );
		auto vtx_curPosition = writer.declOutput< Vec3 >( cuT( "vtx_curPosition" )
			, RenderPass::VertexOutputs::CurPositionLocation );
		auto vtx_prvPosition = writer.declOutput< Vec3 >( cuT( "vtx_prvPosition" )
			, RenderPass::VertexOutputs::PrvPositionLocation );
		auto vtx_tangentSpaceFragPosition = writer.declOutput< Vec3 >( cuT( "vtx_tangentSpaceFragPosition" )
			, RenderPass::VertexOutputs::TangentSpaceFragPositionLocation );
		auto vtx_tangentSpaceViewPosition = writer.declOutput< Vec3 >( cuT( "vtx_tangentSpaceViewPosition" )
			, RenderPass::VertexOutputs::TangentSpaceViewPositionLocation );
		auto vtx_normal = writer.declOutput< Vec3 >( cuT( "vtx_normal" )
			, RenderPass::VertexOutputs::NormalLocation );
		auto vtx_tangent = writer.declOutput< Vec3 >( cuT( "vtx_tangent" )
			, RenderPass::VertexOutputs::TangentLocation );
		auto vtx_bitangent = writer.declOutput< Vec3 >( cuT( "vtx_bitangent" )
			, RenderPass::VertexOutputs::BitangentLocation );
		auto vtx_texture = writer.declOutput< Vec3 >( cuT( "vtx_texture" )
			, RenderPass::VertexOutputs::TextureLocation );
		auto vtx_instance = writer.declOutput< UInt >( cuT( "vtx_instance" )
			, RenderPass::VertexOutputs::InstanceLocation );
		auto vtx_material = writer.declOutput< UInt >( cuT( "vtx_material" )
			, RenderPass::VertexOutputs::MaterialLocation );
		auto out = writer.getOut();

		std::function< void() > main = [&]()
		{
			auto curPosition = writer.declLocale( cuT( "curPosition" )
				, vec4( position.xyz(), 1.0 ) );
			auto v4Normal = writer.declLocale( cuT( "v4Normal" )
				, vec4( normal, 0.0 ) );
			auto v4Tangent = writer.declLocale( cuT( "v4Tangent" )
				, vec4( tangent, 0.0 ) );
			auto v3Texture = writer.declLocale( cuT( "v3Texture" )
				, uv );

			if ( checkFlag( flags.programFlags, ProgramFlag::eSkinning ) )
			{
				auto curMtxModel = writer.declLocale( cuT( "curMtxModel" )
					, SkinningUbo::computeTransform( skinningData, writer, flags.programFlags ) );
				auto prvMtxModel = writer.declLocale( cuT( "prvMtxModel" )
					, curMtxModel );
			}
			else if ( checkFlag( flags.programFlags, ProgramFlag::eInstantiation ) )
			{
				auto curMtxModel = writer.declLocale( cuT( "curMtxModel" )
					, transform );
				auto prvMtxModel = writer.declLocale( cuT( "prvMtxModel" )
					, curMtxModel );
			}
			else
			{
				auto curMtxModel = writer.declLocale( cuT( "curMtxModel" )
					, c3d_curMtxModel );
				auto prvMtxModel = writer.declLocale( cuT( "prvMtxModel" )
					, c3d_prvMtxModel );
			}

			auto curMtxModel = writer.getVariable< Mat4 >( cuT( "curMtxModel" ) );
			auto prvMtxModel = writer.getVariable< Mat4 >( cuT( "prvMtxModel" ) );
			auto mtxNormal = writer.declLocale( cuT( "mtxNormal" )
				, transpose( inverse( mat3( curMtxModel ) ) ) );

			if ( checkFlag( flags.programFlags, ProgramFlag::eInstantiation ) )
			{
				vtx_material = writer.cast< UInt >( material );
			}
			else
			{
				vtx_material = writer.cast< UInt >( c3d_materialIndex );
			}

			if ( checkFlag( flags.programFlags, ProgramFlag::eMorphing ) )
			{
				curPosition = vec4( sdw::mix( curPosition.xyz(), position2.xyz(), vec3( c3d_time ) ), 1.0 );
				v4Normal = vec4( sdw::mix( v4Normal.xyz(), normal2.xyz(), vec3( c3d_time ) ), 1.0 );
				v4Tangent = vec4( sdw::mix( v4Tangent.xyz(), tangent2.xyz(), vec3( c3d_time ) ), 1.0 );
				v3Texture = sdw::mix( v3Texture, texture2, vec3( c3d_time ) );
			}

			vtx_texture = v3Texture;
			auto prvPosition = writer.declLocale( cuT( "prvPosition" )
				, prvMtxModel * curPosition );
			curPosition = curMtxModel * curPosition;
			vtx_worldPosition = curPosition.xyz();
			prvPosition = c3d_prvView * curPosition;
			curPosition = c3d_curView * curPosition;
			vtx_viewPosition = curPosition.xyz();

			if ( checkFlag( flags.programFlags, ProgramFlag::eInvertNormals ) )
			{
				vtx_normal = normalize( mtxNormal * -v4Normal.xyz() );
			}
			else
			{
				vtx_normal = normalize( mtxNormal * v4Normal.xyz() );
			}

			vtx_tangent = normalize( mtxNormal * v4Tangent.xyz() );
			vtx_tangent = normalize( sdw::fma( -vtx_normal, vec3( dot( vtx_tangent, vtx_normal ) ), vtx_tangent ) );
			vtx_bitangent = cross( vtx_normal, vtx_tangent );
			vtx_instance = writer.cast< UInt >( in.gl_InstanceID );
			prvPosition = c3d_projection * prvPosition;
			curPosition = c3d_projection * curPosition;

			auto tbn = writer.declLocale( cuT( "tbn" )
				, transpose( mat3( vtx_tangent, vtx_bitangent, vtx_normal ) ) );
			vtx_tangentSpaceFragPosition = tbn * vtx_worldPosition;
			vtx_tangentSpaceViewPosition = tbn * c3d_cameraPosition.xyz();
			// Convert the jitter from non-homogeneous coordiantes to homogeneous
			// coordinates and add it:
			// (note that for providing the jitter in non-homogeneous projection space,
			//  pixel coordinates (screen space) need to multiplied by two in the C++
			//  code)
			curPosition.xy() -= c3d_jitter * curPosition.w();
			prvPosition.xy() -= c3d_jitter * prvPosition.w();
			out.gl_out.gl_Position = curPosition;

			vtx_curPosition = curPosition.xyw();
			vtx_prvPosition = prvPosition.xyw();
			// Positions in projection space are in [-1, 1] range, while texture
			// coordinates are in [0, 1] range. So, we divide by 2 to get velocities in
			// the scale (and flip the y axis):
			vtx_curPosition.xy() *= vec2( 0.5_f, -0.5 );
			vtx_prvPosition.xy() *= vec2( 0.5_f, -0.5 );
		};

		writer.implementFunction< sdw::Void >( cuT( "main" ), main );
		return std::make_unique< sdw::Shader >( std::move( writer.getShader() ) );
	}

	ShaderPtr ForwardRenderTechniquePass::doGetLegacyPixelShaderSource( PipelineFlags const & flags )const
	{
		using namespace sdw;
		FragmentWriter writer;

		// UBOs
		UBO_MATRIX( writer, MatrixUbo::BindingPoint, 0u );
		UBO_SCENE( writer, SceneUbo::BindingPoint, 0u );
		UBO_MODEL( writer, ModelUbo::BindingPoint, 0u );
		UBO_TEXTURES( writer, TexturesUbo::BindingPoint, 0u );

		// Fragment Intputs
		auto vtx_worldPosition = writer.declInput< Vec3 >( cuT( "vtx_worldPosition" )
			, RenderPass::VertexOutputs::WorldPositionLocation );
		auto vtx_viewPosition = writer.declInput< Vec3 >( cuT( "vtx_viewPosition" )
			, RenderPass::VertexOutputs::ViewPositionLocation );
		auto vtx_curPosition = writer.declInput< Vec3 >( cuT( "vtx_curPosition" )
			, RenderPass::VertexOutputs::CurPositionLocation );
		auto vtx_prvPosition = writer.declInput< Vec3 >( cuT( "vtx_prvPosition" )
			, RenderPass::VertexOutputs::PrvPositionLocation );
		auto vtx_tangentSpaceFragPosition = writer.declInput< Vec3 >( cuT( "vtx_tangentSpaceFragPosition" )
			, RenderPass::VertexOutputs::TangentSpaceFragPositionLocation );
		auto vtx_tangentSpaceViewPosition = writer.declInput< Vec3 >( cuT( "vtx_tangentSpaceViewPosition" )
			, RenderPass::VertexOutputs::TangentSpaceViewPositionLocation );
		auto vtx_normal = writer.declInput< Vec3 >( cuT( "vtx_normal" )
			, RenderPass::VertexOutputs::NormalLocation );
		auto vtx_tangent = writer.declInput< Vec3 >( cuT( "vtx_tangent" )
			, RenderPass::VertexOutputs::TangentLocation );
		auto vtx_bitangent = writer.declInput< Vec3 >( cuT( "vtx_bitangent" )
			, RenderPass::VertexOutputs::BitangentLocation );
		auto vtx_texture = writer.declInput< Vec3 >( cuT( "vtx_texture" )
			, RenderPass::VertexOutputs::TextureLocation );
		auto vtx_instance = writer.declInput< UInt >( cuT( "vtx_instance" )
			, RenderPass::VertexOutputs::InstanceLocation );
		auto vtx_material = writer.declInput< UInt >( cuT( "vtx_material" )
			, RenderPass::VertexOutputs::MaterialLocation );

		shader::LegacyMaterials materials{ writer };
		materials.declare( getEngine()->getRenderSystem()->getGpuInformations().hasShaderStorageBuffers() );
		auto c3d_sLights = writer.declSampledImage< FImgBufferRgba32 >( "c3d_sLights", LightBufferIndex, 0u );
		shader::TextureConfigurations textureConfigs{ writer };
		textureConfigs.declare( getEngine()->getRenderSystem()->getGpuInformations().hasShaderStorageBuffers() );

		auto index = MinTextureIndex;
		auto c3d_maps( writer.declSampledImageArray< FImg2DRgba32 >( "c3d_maps"
			, index
			, 1u
			, std::max( 1u, flags.texturesCount )
			, flags.texturesCount > 0u ) );
		index += flags.texturesCount;
		auto c3d_mapEnvironment( writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapEnvironment" )
			, ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) ) ? index++ : 0u
			, 1u
			, ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) ) ) );

		auto in = writer.getIn();

		shader::Fog fog{ getFogType( flags.sceneFlags ), writer };
		auto & renderSystem = *getEngine()->getRenderSystem();
		shader::Utils utils{ writer, renderSystem.isTopDown(), renderSystem.isZeroToOneDepth(), renderSystem.isInvertedNormals() };
		utils.declareInvertNormal();
		utils.declareApplyGamma();
		utils.declareRemoveGamma();
		utils.declareParallaxMappingFunc( flags );
		auto lighting = shader::PhongLightingModel::createModel( writer
			, utils
			, index
			, getCuller().getScene().getDirectionalShadowCascades() );
		shader::PhongReflectionModel reflections{ writer, utils };

		// Fragment Outputs
		auto pxl_fragColor( writer.declOutput< Vec4 >( cuT( "pxl_fragColor" ), 0 ) );

		writer.implementFunction< sdw::Void >( cuT( "main" ), [&]()
		{
			auto material = writer.declLocale( cuT( "material" )
				, materials.getMaterial( vtx_material ) );
			auto texCoord = writer.declLocale( cuT( "texCoord" )
				, vtx_texture );
			auto alpha = writer.declLocale( cuT( "alpha" )
				, material.m_opacity );
			auto gamma = writer.declLocale( cuT( "gamma" )
				, material.m_gamma );
			auto normal = writer.declLocale( cuT( "normal" )
				, normalize( vtx_normal ) );
			auto diffuse = writer.declLocale( cuT( "diffuse" )
				, utils.removeGamma( gamma, material.m_diffuse() ) );
			auto specular = writer.declLocale( cuT( "specular" )
				, material.m_specular );
			auto shininess = writer.declLocale( cuT( "shininess" )
				, material.m_shininess );
			auto emissive = writer.declLocale( cuT( "emissive" )
				, vec3( material.m_emissive ) );
			auto occlusion = writer.declLocale( cuT( "occlusion" )
				, 1.0_f );
			auto transmittance = writer.declLocale( cuT( "transmittance" )
				, 0.0_f );

			utils.computeHeightMapContribution( flags
				, textureConfigs
				, c3d_textureConfig
				, vtx_tangentSpaceViewPosition
				, vtx_tangentSpaceFragPosition
				, c3d_maps
				, texCoord );
			shader::PhongLightingModel::computeMapContributions( writer
				, utils
				, flags
				, gamma
				, textureConfigs
				, c3d_textureConfig
				, vtx_tangent
				, vtx_bitangent
				, c3d_maps
				, texCoord
				, normal
				, diffuse
				, specular
				, emissive
				, shininess
				, alpha
				, occlusion
				, transmittance );
			utils.applyAlphaFunc( flags.alphaFunc
				, alpha
				, material.m_alphaRef );
			emissive *= diffuse;
			auto worldEye = writer.declLocale( cuT( "worldEye" )
				, c3d_cameraPosition.xyz() );
			auto lightDiffuse = writer.declLocale( cuT( "lightDiffuse" )
				, vec3( 0.0_f ) );
			auto lightSpecular = writer.declLocale( cuT( "lightSpecular" )
				, vec3( 0.0_f ) );
			shader::OutputComponents output{ lightDiffuse, lightSpecular };
			lighting->computeCombined( worldEye
				, shininess
				, c3d_shadowReceiver
				, shader::FragmentInput( in.gl_FragCoord.xy(), vtx_viewPosition, vtx_worldPosition, normal )
				, output );

			auto ambient = writer.declLocale( cuT( "ambient" )
				, clamp( c3d_ambientLight.xyz() + material.m_ambient * material.m_diffuse()
					, vec3( 0.0_f )
					, vec3( 1.0_f ) ) );

			if ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) )
			{
				auto incident = writer.declLocale( cuT( "incident" )
					, reflections.computeIncident( vtx_worldPosition, c3d_cameraPosition.xyz() ) );

				if ( checkFlag( flags.textures, TextureFlag::eReflection )
					&& checkFlag( flags.textures, TextureFlag::eRefraction ) )
				{
					ambient = reflections.computeReflRefr( incident
						, normal
						, occlusion
						, c3d_mapEnvironment
						, material.m_refractionRatio
						, diffuse
						, shininess );
					diffuse = vec3( 0.0_f );
				}
				else if ( checkFlag( flags.textures, TextureFlag::eReflection ) )
				{
					diffuse *= reflections.computeRefl( incident
						, normal
						, occlusion
						, c3d_mapEnvironment );
					ambient = vec3( 0.0_f );
				}
				else
				{
					ambient = reflections.computeRefr( incident
						, normal
						, occlusion
						, c3d_mapEnvironment
						, material.m_refractionRatio
						, diffuse
						, shininess );
					diffuse = vec3( 0.0_f );
				}
			}
			else
			{
				ambient *= occlusion * diffuse;
				diffuse *= lightDiffuse;
			}

			pxl_fragColor = vec4( diffuse + lightSpecular + emissive + ambient, alpha );

			if ( getFogType( flags.sceneFlags ) != FogType::eDisabled )
			{
				pxl_fragColor = fog.apply( vec4( utils.removeGamma( gamma, c3d_backgroundColour.rgb() ), c3d_backgroundColour.a() )
					, pxl_fragColor
					, length( vtx_viewPosition )
					, vtx_viewPosition.y() );
			}
		} );

		return std::make_unique< sdw::Shader >( std::move( writer.getShader() ) );
	}

	ShaderPtr ForwardRenderTechniquePass::doGetPbrMRPixelShaderSource( PipelineFlags const & flags )const
	{
		using namespace sdw;
		FragmentWriter writer;

		// UBOs
		UBO_MATRIX( writer, MatrixUbo::BindingPoint, 0u );
		UBO_SCENE( writer, SceneUbo::BindingPoint, 0u );
		UBO_MODEL( writer, ModelUbo::BindingPoint, 0u );
		UBO_TEXTURES( writer, TexturesUbo::BindingPoint, 0u );

		// Fragment Intputs
		auto vtx_worldPosition = writer.declInput< Vec3 >( cuT( "vtx_worldPosition" )
			, RenderPass::VertexOutputs::WorldPositionLocation );
		auto vtx_viewPosition = writer.declInput< Vec3 >( cuT( "vtx_viewPosition" )
			, RenderPass::VertexOutputs::ViewPositionLocation );
		auto vtx_curPosition = writer.declInput< Vec3 >( cuT( "vtx_curPosition" )
			, RenderPass::VertexOutputs::CurPositionLocation );
		auto vtx_prvPosition = writer.declInput< Vec3 >( cuT( "vtx_prvPosition" )
			, RenderPass::VertexOutputs::PrvPositionLocation );
		auto vtx_tangentSpaceFragPosition = writer.declInput< Vec3 >( cuT( "vtx_tangentSpaceFragPosition" )
			, RenderPass::VertexOutputs::TangentSpaceFragPositionLocation );
		auto vtx_tangentSpaceViewPosition = writer.declInput< Vec3 >( cuT( "vtx_tangentSpaceViewPosition" )
			, RenderPass::VertexOutputs::TangentSpaceViewPositionLocation );
		auto vtx_normal = writer.declInput< Vec3 >( cuT( "vtx_normal" )
			, RenderPass::VertexOutputs::NormalLocation );
		auto vtx_tangent = writer.declInput< Vec3 >( cuT( "vtx_tangent" )
			, RenderPass::VertexOutputs::TangentLocation );
		auto vtx_bitangent = writer.declInput< Vec3 >( cuT( "vtx_bitangent" )
			, RenderPass::VertexOutputs::BitangentLocation );
		auto vtx_texture = writer.declInput< Vec3 >( cuT( "vtx_texture" )
			, RenderPass::VertexOutputs::TextureLocation );
		auto vtx_instance = writer.declInput< UInt >( cuT( "vtx_instance" )
			, RenderPass::VertexOutputs::InstanceLocation );
		auto vtx_material = writer.declInput< UInt >( cuT( "vtx_material" )
			, RenderPass::VertexOutputs::MaterialLocation );

		shader::PbrMRMaterials materials{ writer };
		materials.declare( getEngine()->getRenderSystem()->getGpuInformations().hasShaderStorageBuffers() );
		auto c3d_sLights = writer.declSampledImage< FImgBufferRgba32 >( "c3d_sLights", LightBufferIndex, 0u );
		shader::TextureConfigurations textureConfigs{ writer };
		textureConfigs.declare( getEngine()->getRenderSystem()->getGpuInformations().hasShaderStorageBuffers() );

		auto index = MinTextureIndex;
		auto c3d_maps( writer.declSampledImageArray< FImg2DRgba32 >( "c3d_maps"
			, index
			, 1u
			, std::max( 1u, flags.texturesCount )
			, flags.texturesCount > 0u ) );
		index += flags.texturesCount;
		auto c3d_mapEnvironment( writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapEnvironment" )
			, ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) ) ? index++ : 0u
			, 1u
			, ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) ) ) );
		auto c3d_mapIrradiance = writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapIrradiance" )
			, index++
			, 1u );
		auto c3d_mapPrefiltered = writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapPrefiltered" )
			, index++
			, 1u );
		auto c3d_mapBrdf = writer.declSampledImage< FImg2DRgba32 >( cuT( "c3d_mapBrdf" )
			, index++
			, 1u );

		auto in = writer.getIn();

		auto & renderSystem = *getEngine()->getRenderSystem();
		shader::Utils utils{ writer, renderSystem.isTopDown(), renderSystem.isZeroToOneDepth(), renderSystem.isInvertedNormals() };
		utils.declareInvertNormal();
		utils.declareApplyGamma();
		utils.declareRemoveGamma();
		utils.declareFresnelSchlick();
		utils.declareComputeIBL();
		utils.declareInvertVec3Y();
		utils.declareParallaxMappingFunc( flags );
		auto lighting = shader::MetallicBrdfLightingModel::createModel( writer
			, utils
			, index
			, getCuller().getScene().getDirectionalShadowCascades() );
		shader::MetallicPbrReflectionModel reflections{ writer, utils };
		shader::Fog fog{ getFogType( flags.sceneFlags ), writer };

		// Fragment Outputs
		auto pxl_fragColor( writer.declOutput< Vec4 >( cuT( "pxl_fragColor" ), 0 ) );

		writer.implementFunction< sdw::Void >( cuT( "main" ), [&]()
		{
			auto material = writer.declLocale( cuT( "material" )
				, materials.getMaterial( vtx_material ) );
			auto texCoord = writer.declLocale( cuT( "texCoord" )
				, vtx_texture );
			auto alpha = writer.declLocale( cuT( "alpha" )
				, material.m_opacity );
			auto normal = writer.declLocale( cuT( "normal" )
				, normalize( vtx_normal ) );
			auto ambient = writer.declLocale( cuT( "ambient" )
				, c3d_ambientLight.xyz() );
			auto metallic = writer.declLocale( cuT( "metallic" )
				, material.m_metallic );
			auto roughness = writer.declLocale( cuT( "roughness" )
				, material.m_roughness );
			auto gamma = writer.declLocale( cuT( "gamma" )
				, material.m_gamma );
			auto albedo = writer.declLocale( cuT( "albedo" )
				, utils.removeGamma( gamma, material.m_albedo ) );
			auto emissive = writer.declLocale( cuT( "emissive" )
				, vec3( material.m_emissive ) );
			auto worldEye = writer.declLocale( cuT( "worldEye" )
				, c3d_cameraPosition.xyz() );
			auto envAmbient = writer.declLocale( cuT( "envAmbient" )
				, vec3( 1.0_f ) );
			auto envDiffuse = writer.declLocale( cuT( "envDiffuse" )
				, vec3( 1.0_f ) );
			auto occlusion = writer.declLocale( cuT( "occlusion" )
				, 1.0_f );
			auto transmittance = writer.declLocale( cuT( "transmittance" )
				, 0.0_f );

			utils.computeHeightMapContribution( flags
				, textureConfigs
				, c3d_textureConfig
				, vtx_tangentSpaceViewPosition
				, vtx_tangentSpaceFragPosition
				, c3d_maps
				, texCoord );
			shader::MetallicBrdfLightingModel::computeMapContributions( writer
				, utils
				, flags
				, gamma
				, textureConfigs
				, c3d_textureConfig
				, vtx_tangent
				, vtx_bitangent
				, c3d_maps
				, texCoord
				, normal
				, albedo
				, metallic
				, emissive
				, roughness
				, alpha
				, occlusion
				, transmittance );

			if ( m_opaque )
			{
				utils.applyAlphaFunc( flags.alphaFunc
					, alpha
					, material.m_alphaRef );
			}

			emissive *= albedo;
			auto lightDiffuse = writer.declLocale( cuT( "lightDiffuse" )
				, vec3( 0.0_f ) );
			auto lightSpecular = writer.declLocale( cuT( "lightSpecular" )
				, vec3( 0.0_f ) );
			shader::OutputComponents output{ lightDiffuse, lightSpecular };
			lighting->computeCombined( worldEye
				, albedo
				, metallic
				, roughness
				, c3d_shadowReceiver
				, shader::FragmentInput( in.gl_FragCoord.xy(), vtx_viewPosition, vtx_worldPosition, normal )
				, output );

			if ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) )
			{
				auto incident = writer.declLocale( cuT( "incident" )
					, reflections.computeIncident( vtx_worldPosition, worldEye ) );
				auto ratio = writer.declLocale( cuT( "ratio" )
					, material.m_refractionRatio );

				if ( checkFlag( flags.textures, TextureFlag::eReflection ) )
				{
					// Reflection from environment map.
					ambient = reflections.computeRefl( incident
						, normal
						, occlusion
						, c3d_mapEnvironment
						, c3d_ambientLight.xyz()
						, albedo );
				}
				else
				{
					// Reflection from background skybox.
					ambient = c3d_ambientLight.xyz()
						* occlusion
						* utils.computeMetallicIBL( normal
							, vtx_worldPosition
							, albedo
							, metallic
							, roughness
							, c3d_cameraPosition.xyz()
							, c3d_mapIrradiance
							, c3d_mapPrefiltered
							, c3d_mapBrdf );
				}

				if ( checkFlag( flags.textures, TextureFlag::eRefraction ) )
				{
					// Refraction from environment map.
					ambient = reflections.computeRefrEnvMap( incident
						, normal
						, occlusion
						, c3d_mapEnvironment
						, ratio
						, ambient
						, albedo
						, roughness );
				}
				else
				{
					IF( writer, ratio != 0.0_f )
					{
						// Refraction from background skybox.
						ambient = reflections.computeRefrSkybox( incident
							, normal
							, occlusion
							, c3d_mapPrefiltered
							, material.m_refractionRatio
							, ambient
							, albedo
							, roughness );
					}
					FI;
				}
			}
			else
			{
				// Reflection from background skybox.
				ambient *= occlusion * utils.computeMetallicIBL( normal
					, vtx_worldPosition
					, albedo
					, metallic
					, roughness
					, c3d_cameraPosition.xyz()
					, c3d_mapIrradiance
					, c3d_mapPrefiltered
					, c3d_mapBrdf );
				auto ratio = writer.declLocale( cuT( "ratio" )
					, material.m_refractionRatio );

				IF( writer, ratio != 0.0_f )
				{
					// Refraction from background skybox.
					auto incident = writer.declLocale( cuT( "incident" )
						, reflections.computeIncident( vtx_worldPosition, worldEye ) );
					ambient = reflections.computeRefrSkybox( incident
						, normal
						, occlusion
						, c3d_mapPrefiltered
						, material.m_refractionRatio
						, ambient
						, albedo
						, roughness );
				}
				FI;
			}

			pxl_fragColor = vec4( sdw::fma( lightDiffuse
					, albedo
					, lightSpecular + emissive + ambient )
				, alpha );

			if ( getFogType( flags.sceneFlags ) != FogType::eDisabled )
			{
				pxl_fragColor = fog.apply( vec4( utils.removeGamma( gamma, c3d_backgroundColour.rgb() ), c3d_backgroundColour.a() )
					, pxl_fragColor
					, length( vtx_viewPosition )
					, vtx_viewPosition.y() );
			}
		} );

		return std::make_unique< sdw::Shader >( std::move( writer.getShader() ) );
	}

	ShaderPtr ForwardRenderTechniquePass::doGetPbrSGPixelShaderSource( PipelineFlags const & flags )const
	{
		using namespace sdw;
		FragmentWriter writer;

		// UBOs
		UBO_MATRIX( writer, MatrixUbo::BindingPoint, 0u );
		UBO_SCENE( writer, SceneUbo::BindingPoint, 0u );
		UBO_MODEL( writer, ModelUbo::BindingPoint, 0u );
		UBO_TEXTURES( writer, TexturesUbo::BindingPoint, 0u );

		// Fragment Intputs
		auto vtx_worldPosition = writer.declInput< Vec3 >( cuT( "vtx_worldPosition" )
			, RenderPass::VertexOutputs::WorldPositionLocation );
		auto vtx_viewPosition = writer.declInput< Vec3 >( cuT( "vtx_viewPosition" )
			, RenderPass::VertexOutputs::ViewPositionLocation );
		auto vtx_curPosition = writer.declInput< Vec3 >( cuT( "vtx_curPosition" )
			, RenderPass::VertexOutputs::CurPositionLocation );
		auto vtx_prvPosition = writer.declInput< Vec3 >( cuT( "vtx_prvPosition" )
			, RenderPass::VertexOutputs::PrvPositionLocation );
		auto vtx_tangentSpaceFragPosition = writer.declInput< Vec3 >( cuT( "vtx_tangentSpaceFragPosition" )
			, RenderPass::VertexOutputs::TangentSpaceFragPositionLocation );
		auto vtx_tangentSpaceViewPosition = writer.declInput< Vec3 >( cuT( "vtx_tangentSpaceViewPosition" )
			, RenderPass::VertexOutputs::TangentSpaceViewPositionLocation );
		auto vtx_normal = writer.declInput< Vec3 >( cuT( "vtx_normal" )
			, RenderPass::VertexOutputs::NormalLocation );
		auto vtx_tangent = writer.declInput< Vec3 >( cuT( "vtx_tangent" )
			, RenderPass::VertexOutputs::TangentLocation );
		auto vtx_bitangent = writer.declInput< Vec3 >( cuT( "vtx_bitangent" )
			, RenderPass::VertexOutputs::BitangentLocation );
		auto vtx_texture = writer.declInput< Vec3 >( cuT( "vtx_texture" )
			, RenderPass::VertexOutputs::TextureLocation );
		auto vtx_instance = writer.declInput< UInt >( cuT( "vtx_instance" )
			, RenderPass::VertexOutputs::InstanceLocation );
		auto vtx_material = writer.declInput< UInt >( cuT( "vtx_material" )
			, RenderPass::VertexOutputs::MaterialLocation );

		shader::PbrSGMaterials materials{ writer };
		materials.declare( getEngine()->getRenderSystem()->getGpuInformations().hasShaderStorageBuffers() );
		auto c3d_sLights = writer.declSampledImage< FImgBufferRgba32 >( "c3d_sLights", LightBufferIndex, 0u );
		shader::TextureConfigurations textureConfigs{ writer };
		textureConfigs.declare( getEngine()->getRenderSystem()->getGpuInformations().hasShaderStorageBuffers() );

		auto index = MinTextureIndex;
		auto c3d_maps( writer.declSampledImageArray< FImg2DRgba32 >( "c3d_maps"
			, index
			, 1u
			, std::max( 1u, flags.texturesCount )
			, flags.texturesCount > 0u ) );
		index += flags.texturesCount;
		auto c3d_mapEnvironment( writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapEnvironment" )
			, ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) ) ? index++ : 0u
			, 1u
			, ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) ) ) );
		auto c3d_mapIrradiance = writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapIrradiance" )
			, index++
			, 1u );
		auto c3d_mapPrefiltered = writer.declSampledImage< FImgCubeRgba32 >( cuT( "c3d_mapPrefiltered" )
			, index++
			, 1u );
		auto c3d_mapBrdf = writer.declSampledImage< FImg2DRgba32 >( cuT( "c3d_mapBrdf" )
			, index++
			, 1u );

		auto in = writer.getIn();

		shader::Fog fog{ getFogType( flags.sceneFlags ), writer };
		auto & renderSystem = *getEngine()->getRenderSystem();
		shader::Utils utils{ writer, renderSystem.isTopDown(), renderSystem.isZeroToOneDepth(), renderSystem.isInvertedNormals() };
		utils.declareInvertNormal();
		utils.declareApplyGamma();
		utils.declareRemoveGamma();
		utils.declareFresnelSchlick();
		utils.declareComputeIBL();
		utils.declareParallaxMappingFunc( flags );
		auto lighting = shader::SpecularBrdfLightingModel::createModel( writer
			, utils
			, index
			, getCuller().getScene().getDirectionalShadowCascades() );
		shader::SpecularPbrReflectionModel reflections{ writer, utils };

		// Fragment Outputs
		auto pxl_fragColor( writer.declOutput< Vec4 >( cuT( "pxl_fragColor" ), 0 ) );

		writer.implementFunction< sdw::Void >( cuT( "main" ), [&]()
		{
			auto material = writer.declLocale( cuT( "material" )
				, materials.getMaterial( vtx_material ) );
			auto texCoord = writer.declLocale( cuT( "texCoord" )
				, vtx_texture );
			auto alpha = writer.declLocale( cuT( "alpha" )
				, material.m_opacity );
			auto normal = writer.declLocale( cuT( "normal" )
				, normalize( vtx_normal ) );
			auto ambient = writer.declLocale( cuT( "ambient" )
				, c3d_ambientLight.xyz() );
			auto specular = writer.declLocale( cuT( "specular" )
				, material.m_specular );
			auto glossiness = writer.declLocale( cuT( "glossiness" )
				, material.m_glossiness );
			auto gamma = writer.declLocale( cuT( "gamma" )
				, material.m_gamma );
			auto albedo = writer.declLocale( cuT( "albedo" )
				, utils.removeGamma( gamma, material.m_diffuse() ) );
			auto emissive = writer.declLocale( cuT( "emissive" )
				, vec3( material.m_emissive ) );
			auto worldEye = writer.declLocale( cuT( "worldEye" )
				, c3d_cameraPosition.xyz() );
			auto envAmbient = writer.declLocale( cuT( "envAmbient" )
				, vec3( 1.0_f ) );
			auto envDiffuse = writer.declLocale( cuT( "envDiffuse" )
				, vec3( 1.0_f ) );
			auto occlusion = writer.declLocale( cuT( "occlusion" )
				, 1.0_f );
			auto transmittance = writer.declLocale( cuT( "transmittance" )
				, 0.0_f );

			utils.computeHeightMapContribution( flags
				, textureConfigs
				, c3d_textureConfig
				, vtx_tangentSpaceViewPosition
				, vtx_tangentSpaceFragPosition
				, c3d_maps
				, texCoord );
			shader::SpecularBrdfLightingModel::computeMapContributions( writer
				, utils
				, flags
				, gamma
				, textureConfigs
				, c3d_textureConfig
				, vtx_tangent
				, vtx_bitangent
				, c3d_maps
				, texCoord
				, normal
				, albedo
				, specular
				, emissive
				, glossiness
				, alpha
				, occlusion
				, transmittance );

			if ( m_opaque )
			{
				utils.applyAlphaFunc( flags.alphaFunc
					, alpha
					, material.m_alphaRef );
			}

			emissive *= albedo;
			auto lightDiffuse = writer.declLocale( cuT( "lightDiffuse" )
				, vec3( 0.0_f ) );
			auto lightSpecular = writer.declLocale( cuT( "lightSpecular" )
				, vec3( 0.0_f ) );
			shader::OutputComponents output{ lightDiffuse, lightSpecular };
			lighting->computeCombined( worldEye
				, albedo
				, specular
				, glossiness
				, c3d_shadowReceiver
				, shader::FragmentInput( in.gl_FragCoord.xy(), vtx_viewPosition, vtx_worldPosition, normal )
				, output );

			if ( checkFlag( flags.textures, TextureFlag::eReflection )
				|| checkFlag( flags.textures, TextureFlag::eRefraction ) )
			{
				auto incident = writer.declLocale( cuT( "incident" )
					, reflections.computeIncident( vtx_worldPosition, worldEye ) );
				auto ratio = writer.declLocale( cuT( "ratio" )
					, material.m_refractionRatio );

				if ( checkFlag( flags.textures, TextureFlag::eReflection ) )
				{
					// Reflection from environment map.
					ambient = reflections.computeRefl( incident
						, normal
						, occlusion
						, c3d_mapEnvironment
						, c3d_ambientLight.xyz()
						, albedo );
				}
				else
				{
					// Reflection from background skybox.
					ambient = c3d_ambientLight.xyz()
						* occlusion
						* utils.computeSpecularIBL( normal
							, vtx_worldPosition
							, albedo
							, specular
							, glossiness
							, c3d_cameraPosition.xyz()
							, c3d_mapIrradiance
							, c3d_mapPrefiltered
							, c3d_mapBrdf );
				}

				if ( checkFlag( flags.textures, TextureFlag::eRefraction ) )
				{
					// Refraction from environment map.
					ambient = reflections.computeRefrEnvMap( incident
						, normal
						, occlusion
						, c3d_mapEnvironment
						, ratio
						, ambient
						, albedo
						, glossiness );
				}
				else
				{
					IF( writer, ratio != 0.0_f )
					{
						// Refraction from background skybox.
						ambient = reflections.computeRefrSkybox( incident
							, normal
							, occlusion
							, c3d_mapPrefiltered
							, material.m_refractionRatio
							, ambient
							, albedo
							, glossiness );
					}
					FI;
				}
			}
			else
			{
				// Reflection from background skybox.
				ambient *= occlusion * utils.computeSpecularIBL( normal
					, vtx_worldPosition
					, albedo
					, specular
					, glossiness
					, c3d_cameraPosition.xyz()
					, c3d_mapIrradiance
					, c3d_mapPrefiltered
					, c3d_mapBrdf );
				auto ratio = writer.declLocale( cuT( "ratio" )
					, material.m_refractionRatio );

				IF( writer, ratio != 0.0_f )
				{
					// Refraction from background skybox.
					auto incident = writer.declLocale( cuT( "incident" )
						, reflections.computeIncident( vtx_worldPosition, worldEye ) );
					ambient = reflections.computeRefrSkybox( incident
						, normal
						, occlusion
						, c3d_mapPrefiltered
						, material.m_refractionRatio
						, ambient
						, albedo
						, glossiness );
				}
				FI;
			}

			pxl_fragColor = vec4( sdw::fma( lightDiffuse
					, albedo
					, lightSpecular + emissive + ambient )
				, alpha );

			if ( getFogType( flags.sceneFlags ) != FogType::eDisabled )
			{
				pxl_fragColor = fog.apply( vec4( utils.removeGamma( gamma, c3d_backgroundColour.rgb() ), c3d_backgroundColour.a() )
					, pxl_fragColor
					, length( vtx_viewPosition )
					, vtx_viewPosition.y() );
			}
		} );

		return std::make_unique< sdw::Shader >( std::move( writer.getShader() ) );
	}

	//*********************************************************************************************
}
