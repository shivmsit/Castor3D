#include "Castor3D/RenderToTexture/TextureProjectionToCube.hpp"

#include "Castor3D/Miscellaneous/DebugName.hpp"
#include "Castor3D/Miscellaneous/makeVkType.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Shader/Program.hpp"
#include "Castor3D/Shader/Shaders/GlslUtils.hpp"
#include "Castor3D/Texture/TextureLayout.hpp"

#include <ashespp/Buffer/StagingBuffer.hpp>
#include <ashespp/Command/Queue.hpp>
#include <ashespp/Core/Device.hpp>
#include <ashespp/Image/Image.hpp>
#include <ashespp/Pipeline/PipelineDepthStencilStateCreateInfo.hpp>
#include <ashespp/Pipeline/PipelineInputAssemblyStateCreateInfo.hpp>
#include <ashespp/Pipeline/PipelineMultisampleStateCreateInfo.hpp>
#include <ashespp/Pipeline/PipelineVertexInputStateCreateInfo.hpp>
#include <ashespp/Pipeline/PipelineViewportStateCreateInfo.hpp>
#include <ashespp/RenderPass/RenderPassCreateInfo.hpp>
#include <ashespp/Sync/Fence.hpp>

#include <ShaderWriter/Source.hpp>

#include <CastorUtils/Math/TransformationMatrix.hpp>

using namespace castor;
using namespace sdw;

namespace castor3d
{
	//*********************************************************************************************

	namespace
	{
		ashes::PipelineShaderStageCreateInfoArray doCreateProgram( RenderDevice const & device
			, VkFormat format )
		{
			ShaderModule vtx{ VK_SHADER_STAGE_VERTEX_BIT, "TextureProjToCube" };
			{
				VertexWriter writer;

				// Inputs
				auto position = writer.declInput< Vec4 >( cuT( "position" ), 0u );
				auto matrixUbo = sdw::Ubo{ writer, cuT( "Matrix" ), 0u, 0u };
				auto mtxViewProjection = matrixUbo.declMember< Mat4 >( cuT( "mtxViewProjection" ) );
				matrixUbo.end();

				// Outputs
				auto vtx_position = writer.declOutput< Vec3 >( cuT( "vtx_position" ), 0u );
				auto out = writer.getOut();

				std::function< void() > main = [&]()
				{
					vtx_position = position.xyz();
					out.gl_out.gl_Position = mtxViewProjection * position;
				};

				writer.implementFunction< sdw::Void >( cuT( "main" ), main );
				vtx.shader = std::make_unique< sdw::Shader >( std::move( writer.getShader() ) );
			}

			ShaderModule pxl{ VK_SHADER_STAGE_FRAGMENT_BIT, "TextureProjToCube" };
			{
				FragmentWriter writer;

				// Inputs
				auto mapColour = writer.declSampledImage< FImg2DRgba32 >( cuT( "mapColour" ), 1u, 0u );
				auto vtx_position = writer.declInput< Vec3 >( cuT( "vtx_position" ), 0u );

				// Outputs
				auto pxl_colour = writer.declOutput< Vec4 >( cuT( "pxl_colour" ), 0u );

				shader::Utils utils{ writer, device.renderSystem.isTopDown() };
				utils.declareInvertVec2Y();
				
				auto sampleSphericalMap = writer.implementFunction< Vec2 >( cuT( "sampleSphericalMap" )
					, [&]( Vec3 const & v )
					{
						auto uv = writer.declLocale( cuT( "uv" ), vec2( atan( v.z() / v.x() ), asin( v.y() ) ) );
						uv *= vec2( 0.1591_f, 0.3183_f );
						uv += 0.5_f;
						writer.returnStmt( uv );
					}
					, InVec3{ writer, cuT( "v" ) } );

				std::function< void() > main = [&]()
				{
					auto uv = writer.declLocale( cuT( "uv" ), sampleSphericalMap( normalize( vtx_position ) ) );
					pxl_colour = vec4( texture( mapColour, utils.bottomUpToTopDown( uv ) ).rgb(), 1.0 );
				};

				writer.implementFunction< sdw::Void >( cuT( "main" ), main );
				pxl.shader = std::make_unique< sdw::Shader >( std::move( writer.getShader() ) );
			}

			return ashes::PipelineShaderStageCreateInfoArray
			{
				makeShaderState( device, vtx ),
				makeShaderState( device, pxl ),
			};
		}

		ashes::RenderPassPtr doCreateRenderPass( RenderDevice const & device
			, VkFormat format )
		{
			ashes::VkAttachmentDescriptionArray attaches
			{
				{
					0u,
					format,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				}
			};
			ashes::SubpassDescriptionArray subpasses;
			subpasses.emplace_back( ashes::SubpassDescription
				{
					0u,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					{},
					{ { 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL } },
					{},
					std::nullopt,
					{},
				} );
			ashes::VkSubpassDependencyArray dependencies
			{
				{
					VK_SUBPASS_EXTERNAL,
					0u,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
					VK_DEPENDENCY_BY_REGION_BIT,
				},
				{
					0u,
					VK_SUBPASS_EXTERNAL,
					VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_ACCESS_SHADER_READ_BIT,
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
			auto result = device->createRenderPass( std::move( createInfo ) );
			setDebugObjectName( device, *result, "TextureProjectionToCubeRenderPass" );
			return result;
		}
	}

	//*********************************************************************************************

	TextureProjectionToCube::TextureProjectionToCube( TextureLayout const & equiRectangular
		, RenderDevice const & device
		, TextureLayout const & target )
		: RenderCube{ device, false }
		, m_commandBuffer{ m_device.graphicsCommandPool->createCommandBuffer() }
		, m_view{ equiRectangular.getDefaultView() }
		, m_renderPass{ doCreateRenderPass( m_device, target.getPixelFormat() ) }
	{
		auto size = VkExtent2D{ target.getWidth(), target.getHeight() };
		auto program = doCreateProgram( m_device, equiRectangular.getPixelFormat() );
		uint32_t face = 0u;

		for ( auto & facePipeline : m_frameBuffers )
		{
			ashes::ImageViewCRefArray attaches;
			facePipeline.view = target.getTexture().createView( VK_IMAGE_VIEW_TYPE_2D
				, target.getPixelFormat()
				, 0u
				, 1u
				, face
				, 1u );
			attaches.emplace_back( facePipeline.view );
			facePipeline.frameBuffer = m_renderPass->createFrameBuffer( size, std::move( attaches ) );
			++face;
		}

		createPipelines( size
			, Position{}
			, program
			, m_view
			, *m_renderPass
			, {} );
	}

	void TextureProjectionToCube::render()
	{
		uint32_t face = 0u;

		for ( auto & frameBuffer : m_frameBuffers )
		{
			m_commandBuffer->begin( VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT );

			if ( face == 0u )
			{
				m_commandBuffer->memoryBarrier( VK_PIPELINE_STAGE_TRANSFER_BIT
					, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
					, m_view.makeShaderInputResource( VK_IMAGE_LAYOUT_UNDEFINED ) );
			}

			m_commandBuffer->beginRenderPass( *m_renderPass
				, *frameBuffer.frameBuffer
				, { ashes::makeClearValue( VkClearColorValue{ 0, 0, 0, 0 } ) }
				, VK_SUBPASS_CONTENTS_INLINE );
			registerFrame( *m_commandBuffer, face );
			m_commandBuffer->endRenderPass();
			m_commandBuffer->end();

			m_device.graphicsQueue->submit( *m_commandBuffer, nullptr );
			m_device.graphicsQueue->waitIdle();

			++face;
		}
	}

	//*********************************************************************************************
}