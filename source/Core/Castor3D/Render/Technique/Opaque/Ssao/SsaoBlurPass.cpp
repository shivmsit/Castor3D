#include "Castor3D/Render/Technique/Opaque/Ssao/SsaoBlurPass.hpp"

#include "Castor3D/Engine.hpp"
#include "Castor3D/Cache/SamplerCache.hpp"
#include "Castor3D/Buffer/UniformBuffer.hpp"
#include "Castor3D/Material/Texture/Sampler.hpp"
#include "Castor3D/Material/Texture/TextureLayout.hpp"
#include "Castor3D/Material/Texture/TextureUnit.hpp"
#include "Castor3D/Render/RenderPassTimer.hpp"
#include "Castor3D/Render/RenderPipeline.hpp"
#include "Castor3D/Render/RenderSystem.hpp"
#include "Castor3D/Render/Technique/Opaque/Ssao/SsaoConfig.hpp"
#include "Castor3D/Render/Technique/Opaque/Ssao/SsaoConfigUbo.hpp"
#include "Castor3D/Shader/Program.hpp"
#include "Castor3D/Shader/Shaders/GlslLight.hpp"
#include "Castor3D/Shader/Shaders/GlslShadow.hpp"
#include "Castor3D/Shader/Shaders/GlslUtils.hpp"
#include "Castor3D/Shader/Ubos/GpInfoUbo.hpp"
#include "Castor3D/Shader/Ubos/MatrixUbo.hpp"

#include <CastorUtils/Graphics/RgbaColour.hpp>

#include <ashespp/Image/Image.hpp>
#include <ashespp/Pipeline/GraphicsPipelineCreateInfo.hpp>
#include <ashespp/RenderPass/RenderPassCreateInfo.hpp>

#include <ShaderWriter/Source.hpp>

using namespace castor;

namespace castor3d
{
	namespace
	{
		static uint32_t constexpr SsaoCfgUboIdx = 0u;
		static uint32_t constexpr GpInfoUboIdx = 1u;
		static uint32_t constexpr BlurCfgUboIdx = 2u;
		static uint32_t constexpr NmlImgIdx = 3u;
		static uint32_t constexpr InpImgIdx = 4u;
		static uint32_t constexpr BntImgIdx = 5u;

		ShaderPtr doGetVertexProgram( Engine & engine )
		{
			using namespace sdw;
			VertexWriter writer;

			// Shader inputs
			auto position = writer.declInput< Vec2 >( "position", 0u );

			// Shader outputs
			auto out = writer.getOut();

			writer.implementFunction< Void >( "main"
				, [&]()
				{
					out.vtx.position = vec4( position, 0.0_f, 1.0_f );
				} );
			return std::make_unique< ast::Shader >( std::move( writer.getShader() ) );
		}
		
		ShaderPtr doGetPixelProgram( Engine & engine
			, bool useNormalsBuffer )
		{
			using namespace sdw;
			FragmentWriter writer;

			UBO_SSAO_CONFIG( writer, SsaoCfgUboIdx, 0u );
			UBO_GPINFO( writer, GpInfoUboIdx, 0u );
			Ubo configuration{ writer, "BlurConfiguration", BlurCfgUboIdx, 0u };
			/** (1, 0) or (0, 1)*/
			auto c3d_axis = configuration.declMember< IVec2 >( "c3d_axis" );
			auto c3d_dummy = configuration.declMember< IVec2 >( "c3d_dummy" );
			auto c3d_gaussian = configuration.declMember< Vec4 >( "c3d_gaussian", 2u );
			configuration.end();
			auto c3d_mapNormal = writer.declSampledImage< FImg2DRgba32 >( "c3d_mapNormal", NmlImgIdx, 0u, useNormalsBuffer );
			auto c3d_mapInput = writer.declSampledImage< FImg2DRgba32 >( "c3d_mapInput", InpImgIdx, 0u );
			auto c3d_mapBentInput = writer.declSampledImage< FImg2DRgba32 >( "c3d_mapBentInput", BntImgIdx, 0u );

			/** Same size as result buffer, do not offset by guard band when reading from it */
			auto c3d_readMultiplyFirst = writer.declConstant( "c3d_readMultiplyFirst", vec3( 2.0_f ) );
			auto c3d_readAddSecond = writer.declConstant( "c3d_readAddSecond", vec3( 1.0_f ) );

			auto in = writer.getIn();

			// Shader outputs
			auto pxl_fragColor = writer.declOutput< Vec3 >( "pxl_fragColor", 0u );
			auto pxl_bentNormal = writer.declOutput< Vec3 >( "pxl_bentNormal", 1u );
#define  result         pxl_fragColor.r()
#define  keyPassThrough pxl_fragColor.g()

			/** Returns a number on (0, 1) */
			auto unpackKey = writer.implementFunction< Float >( "unpackKey"
				, [&]( Float const & p )
				{
					writer.returnStmt( p );
				}
				, InFloat{ writer, "p" } );

			// Reconstruct camera-space P.xyz from screen-space S = (x, y) in
			// pixels and camera-space z < 0.  Assumes that the upper-left pixel center
			// is at (0.5, 0.5) [but that need not be the location at which the sample tap
			// was placed!]
			// Costs 3 MADD.  Error is on the order of 10^3 at the far plane, partly due to z precision.
			auto reconstructCSPosition = writer.implementFunction< Vec3 >( "reconstructCSPosition"
				, [&]( Vec2 const & S
					, Float const & z
					, Vec4 const & projInfo )
				{
					writer.returnStmt( vec3( sdw::fma( S.xy(), projInfo.xy(), projInfo.zw() ) * z, z ) );
				}
				, InVec2{ writer, "S" }
				, InFloat{ writer, "z" }
				, InVec4{ writer, "projInfo" } );

			auto positionFromKey = writer.implementFunction< Vec3 >( "positionFromKey"
				, [&]( Float const & key
					, IVec2 const & ssCenter
					, Vec4 const & projInfo )
				{
					auto z = writer.declLocale( "z"
						, key * c3d_farPlaneZ );
					auto position = writer.declLocale( "position"
						, reconstructCSPosition( vec2( ssCenter ) + vec2( 0.5_f )
							, z
							, projInfo ) );
					writer.returnStmt( position );
				}
				, InFloat{ writer, "key" }
				, InIVec2{ writer, "ssCenter" }
				, InVec4{ writer, "projInfo" } );

			auto getTapInformation = writer.implementFunction< Vec3 >( "getTapInformation"
				, [&]( IVec2 const & tapLoc
					, Float tapKey
					, Float value
					, Vec3 bent )
				{
					auto temp = writer.declLocale( "temp"
						, texelFetch( c3d_mapInput, tapLoc, 0_i ).rgb() );
					bent = texelFetch( c3d_mapBentInput, tapLoc, 0_i ).xyz();
					bent = normalize( sdw::fma( bent, c3d_readMultiplyFirst.xyz(), c3d_readAddSecond.xyz() ) );
					tapKey = unpackKey( temp.g() );
					value = temp.r();

					if ( useNormalsBuffer )
					{
						temp = texelFetch( c3d_mapNormal, tapLoc, 0_i ).xyz();
						temp = normalize( sdw::fma( temp, c3d_readMultiplyFirst.xyz(), c3d_readAddSecond.xyz() ) );
					}
					else
					{
						temp = vec3( 0.0_f );
					}

					writer.returnStmt( temp );
				}
				, InIVec2{ writer, "tapLoc" }
				, OutFloat{ writer, "tapKey" }
				, OutFloat{ writer, "value" }
				, OutVec3{ writer, "bent" } );

			auto square = writer.implementFunction< Float >( "square"
				, [&]( Float const & x )
				{
					writer.returnStmt( x * x );
				}
				, InFloat{ writer, "x" } );

			auto calculateBilateralWeight = writer.implementFunction< Float >( "calculateBilateralWeight"
				, [&]( Float const & key
					, Float const & tapKey
					, IVec2 const & tapLoc
					, Vec3 const & normal
					, Vec3 const & tapNormal
					, Vec3 const & position )
				{
					auto scale = writer.declLocale( "scale"
						, 1.5_f * c3d_invRadius );

					// The "bilateral" weight. As depth difference increases, decrease weight.
					// The key values are in scene-specific scale. To make them scale-invariant, factor in
					// the AO radius, which should be based on the scene scale.
					auto depthWeight = writer.declLocale( "depthWeight"
						, max( 0.0_f, 1.0_f - ( c3d_edgeSharpness * 2000.0_f ) * abs( tapKey - key ) * scale ) );
					auto k_normal = writer.declLocale( "k_normal"
						, 1.0_f );
					auto k_plane = writer.declLocale( "k_plane"
						, 1.0_f );

					// Prevents blending over creases. 
					auto normalWeight = writer.declLocale( "normalWeight"
						, 1.0_f );
					auto planeWeight = writer.declLocale( "planeWeight"
						, 1.0_f );

					if ( useNormalsBuffer )
					{
						auto normalCloseness = writer.declLocale( "normalCloseness"
							, dot( tapNormal, normal ) );

						IF( writer, c3d_blurHighQuality == 0_i )
						{
							normalCloseness = normalCloseness * normalCloseness;
							normalCloseness = normalCloseness * normalCloseness;
							k_normal = 4.0_f;
						}
						FI;

						auto normalError = writer.declLocale( "normalError"
							, ( 1.0_f - normalCloseness ) * k_normal );
						normalWeight = max( 1.0_f - c3d_edgeSharpness * normalError, 0.0_f );

						IF( writer, c3d_blurHighQuality )
						{
							auto lowDistanceThreshold2 = writer.declLocale( "lowDistanceThreshold2"
								, 0.001_f );

							auto tapPosition = writer.declLocale( "tapPosition"
								, positionFromKey( tapKey, tapLoc, c3d_projInfo ) );

							// Change position in camera space
							auto dq = writer.declLocale( "dq"
								, position - tapPosition );

							// How far away is this point from the original sample
							// in camera space? (Max value is unbounded)
							auto distance2 = writer.declLocale( "distance2"
								, dot( dq, dq ) );

							// How far off the expected plane (on the perpendicular) is this point?  Max value is unbounded.
							auto planeError = writer.declLocale( "planeError"
								, max( abs( dot( dq, tapNormal ) ), abs( dot( dq, normal ) ) ) );

							// Minimum distance threshold must be scale-invariant, so factor in the radius
							planeWeight = writer.ternary( distance2 * square( scale ) < lowDistanceThreshold2
								, 1.0_f
								, Float{ pow( max( 0.0_f
										, 1.0_f - c3d_edgeSharpness * 2.0 * k_plane * planeError / sqrt( distance2 ) )
									, 2.0_f ) } );
						}
						FI;
					}

					writer.returnStmt( depthWeight * normalWeight * planeWeight );
				}
				, InFloat{ writer, "key" }
				, InFloat{ writer, "tapKey" }
				, InIVec2{ writer, "tapLoc" }
				, InVec3{ writer, "normal" }
				, InVec3{ writer, "tapNormal" }
				, InVec3{ writer, "position" } );

			writer.implementFunction< Void >( "main"
				, [&]()
				{
					auto ssCenter = writer.declLocale( "ssCenter"
						, ivec2( in.fragCoord.xy() ) );

					auto temp = writer.declLocale( "temp"
						, texelFetch( c3d_mapInput, ssCenter, 0_i ) );
					auto sum = writer.declLocale( "sum"
						, temp.r() );
					auto raySum = writer.declLocale( "raySum"
						, texelFetch( c3d_mapBentInput, ssCenter, 0_i ).xyz() );
					raySum = normalize( sdw::fma( raySum, c3d_readMultiplyFirst.xyz(), c3d_readAddSecond.xyz() ) );

					keyPassThrough = temp.g();
					auto key = writer.declLocale( "key"
						, unpackKey( keyPassThrough ) );
					auto normal = writer.declLocale( "normal"
						, vec3( 0.0_f ) );

					if ( useNormalsBuffer )
					{
						normal = texelFetch( c3d_mapNormal, ssCenter, 0_i ).xyz();
						normal = -normalize( ( c3d_mtxInvView * vec4( normal, 1.0 ) ).xyz() );
					}

					IF( writer, key == 1.0_f )
					{
						// Sky pixel (if you aren't using depth keying, disable this test)
						result = sum;
						pxl_bentNormal = normalize( ( raySum - c3d_readAddSecond ) / c3d_readMultiplyFirst );
						writer.returnStmt();
					}
					FI;

					// Base weight for depth falloff.  Increase this for more blurriness,
					// decrease it for better edge discrimination
					auto BASE = writer.declLocale( "BASE"
						, c3d_gaussian[0_u][0_u] );
					auto totalWeight = writer.declLocale( "totalWeight"
						, BASE );
					sum *= totalWeight;
					raySum *= totalWeight;

					auto position = writer.declLocale( "position"
						, positionFromKey( key, ssCenter, c3d_projInfo ) );

					FOR( writer, Int, r, -c3d_blurRadius, r <= c3d_blurRadius, ++r )
					{
						// We already handled the zero case above.  This loop should be unrolled and the static branch optimized out,
						// so the IF statement has no runtime cost
						IF( writer, r != 0_i )
						{
							auto tapLoc = writer.declLocale( "tapLoc"
								, ssCenter + c3d_axis * ( r * c3d_blurStepSize ) );

							// spatial domain: offset gaussian tap
							auto absR = writer.declLocale( "absR"
								, writer.cast< UInt >( abs( r ) ) );
							auto weight = writer.declLocale( "weight"
								, 0.3_f + c3d_gaussian[absR % 2_u][absR / 2_u] );

							auto tapKey = writer.declLocale< Float >( "tapKey" );
							auto value = writer.declLocale< Float >( "value" );
							auto bent = writer.declLocale< Vec3 >( "bent" );
							auto tapNormal = writer.declLocale( "tapNormal"
								, getTapInformation( tapLoc, tapKey, value, bent ) );

							auto bilateralWeight = writer.declLocale( "bilateralWeight"
								, calculateBilateralWeight( key
									, tapKey
									, tapLoc
									, normal
									, tapNormal
									, position ) );

							weight *= bilateralWeight;
							sum += value * weight;
							raySum += bent * weight;
							totalWeight += weight;
						}
						FI;
					}
					ROF;

					auto const epsilon = writer.declLocale( "epsilon"
						, 0.0001_f );
					result = sum / ( totalWeight + epsilon );
					pxl_bentNormal = normalize( ( raySum - c3d_readAddSecond ) / c3d_readMultiplyFirst );
				} );
			return std::make_unique< ast::Shader >( std::move( writer.getShader() ) );

#undef result
#undef keyPassThrough
		}

		ashes::PipelineShaderStageCreateInfoArray doGetProgram( Engine & engine
			, SsaoConfig const & config
			, ShaderModule & vertexShader
			, ShaderModule & pixelShader )
		{
			vertexShader.shader = doGetVertexProgram( engine );
			pixelShader.shader = doGetPixelProgram( engine, config.useNormalsBuffer );
			auto & device = getCurrentRenderDevice( engine );
			ashes::PipelineShaderStageCreateInfoArray result
			{
				makeShaderState( device, vertexShader ),
				makeShaderState( device, pixelShader ),
			};
			return result;
		}

		SamplerSPtr doCreateSampler( Engine & engine
			, String const & name
			, VkSamplerAddressMode mode )
		{
			SamplerSPtr sampler;

			if ( engine.getSamplerCache().has( name ) )
			{
				sampler = engine.getSamplerCache().find( name );
			}
			else
			{
				sampler = engine.getSamplerCache().add( name );
				sampler->setMinFilter( VK_FILTER_NEAREST );
				sampler->setMagFilter( VK_FILTER_NEAREST );
				sampler->setWrapS( mode );
				sampler->setWrapT( mode );
			}

			return sampler;
		}

		TextureUnit doCreateTexture( Engine & engine
			, String const & name
			, VkFormat format
			, VkExtent2D const & size )
		{
			auto & renderSystem = *engine.getRenderSystem();
			auto sampler = doCreateSampler( engine, name, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE );
			
			ashes::ImageCreateInfo image
			{
				0u,
				VK_IMAGE_TYPE_2D,
				format,
				{ size.width, size.height, 1u },
				1u,
				1u,
				VK_SAMPLE_COUNT_1_BIT,
				VK_IMAGE_TILING_OPTIMAL,
				( VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_SAMPLED_BIT ),
			};
			auto ssaoResult = std::make_shared< TextureLayout >( renderSystem
				, image
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
				, name );
			TextureUnit result{ engine };
			result.setTexture( ssaoResult );
			result.setSampler( sampler );
			result.initialise();
			return result;
		}

		ashes::RenderPassPtr doCreateRenderPass( Engine & engine
			, String const & name )
		{
			ashes::VkAttachmentDescriptionArray attaches
			{
				{
					0u,
					SsaoBlurPass::ResultFormat,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
				{
					0u,
					VK_FORMAT_R32G32B32A32_SFLOAT,
					VK_SAMPLE_COUNT_1_BIT,
					VK_ATTACHMENT_LOAD_OP_CLEAR,
					VK_ATTACHMENT_STORE_OP_STORE,
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
					VK_ATTACHMENT_STORE_OP_DONT_CARE,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				},
			};
			ashes::SubpassDescriptionArray subpasses;
			subpasses.emplace_back( ashes::SubpassDescription
				{
					0u,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					{},
					{
						{ 0u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
						{ 1u, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL },
					},
					{},
					ashes::nullopt,
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
				},
			};
			ashes::RenderPassCreateInfo createInfo
			{
				0u,
				std::move( attaches ),
				std::move( subpasses ),
				std::move( dependencies ),
			};
			auto & renderSystem = *engine.getRenderSystem();
			auto & device = getCurrentRenderDevice( renderSystem );
			auto result = device->createRenderPass( name
				, std::move( createInfo ) );
			return result;
		}

		ashes::FrameBufferPtr doCreateFrameBuffer( Engine & engine
			, String const & name
			, ashes::RenderPass const & renderPass
			, TextureUnit const & texture
			, TextureUnit const & bent )
		{
			ashes::ImageViewCRefArray attaches;
			attaches.emplace_back( texture.getTexture()->getDefaultView() );
			attaches.emplace_back( bent.getTexture()->getDefaultView() );
			auto size = texture.getTexture()->getDimensions();
			auto result = renderPass.createFrameBuffer( name
				, VkExtent2D{ size.width, size.height }
				, std::move( attaches ) );
			return result;
		}
	}

	//*********************************************************************************************

	SsaoBlurPass::SsaoBlurPass( Engine & engine
		, String const & prefix
		, VkExtent2D const & size
		, SsaoConfig const & config
		, SsaoConfigUbo & ssaoConfigUbo
		, GpInfoUbo const & gpInfoUbo
		, Point2i const & axis
		, TextureUnit const & input
		, TextureUnit const & bentInput
		, ashes::ImageView const & normals )
		: RenderQuad{ *engine.getRenderSystem(), VK_FILTER_NEAREST }
		, Named{ prefix + cuT( "SsaoBlur" ) }
		, m_engine{ engine }
		, m_ssaoConfigUbo{ ssaoConfigUbo }
		, m_gpInfoUbo{ gpInfoUbo }
		, m_input{ input }
		, m_bentInput{ bentInput }
		, m_normals{ normals }
		, m_vertexShader{ VK_SHADER_STAGE_VERTEX_BIT, getName() }
		, m_pixelShader{ VK_SHADER_STAGE_FRAGMENT_BIT, getName() }
		, m_config{ config }
		, m_program{ doGetProgram( m_engine, m_config, m_vertexShader, m_pixelShader ) }
		, m_size{ size }
		, m_result{ doCreateTexture( m_engine, getName() + cuT( "Result" ), SsaoBlurPass::ResultFormat, m_size ) }
		, m_bentResult{ doCreateTexture( m_engine, getName() + cuT( "BentNormals" ), m_bentInput.getTexture()->getPixelFormat(), m_size ) }
		, m_renderPass{ doCreateRenderPass( m_engine, getName() ) }
		, m_fbo{ doCreateFrameBuffer( m_engine, getName(), *m_renderPass, m_result, m_bentResult ) }
		, m_timer{ std::make_shared< RenderPassTimer >( m_engine, cuT( "SSAO" ), cuT( "Blur" ) ) }
		, m_finished{ getCurrentRenderDevice( m_engine )->createSemaphore( getName() ) }
		, m_configurationUbo{ makeUniformBuffer< Configuration >( *m_engine.getRenderSystem()
			, 1u
			, 0u
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, getName() + cuT( "Cfg" ) ) }
	{
		auto & device = getCurrentRenderDevice( m_renderSystem );
		auto & configuration = m_configurationUbo->getData();
		configuration.axis = axis;

		ashes::VkDescriptorSetLayoutBindingArray bindings
		{
			makeDescriptorSetLayoutBinding( SsaoCfgUboIdx
				, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, VK_SHADER_STAGE_FRAGMENT_BIT ),
			makeDescriptorSetLayoutBinding( GpInfoUboIdx
				, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, VK_SHADER_STAGE_FRAGMENT_BIT ),
			makeDescriptorSetLayoutBinding( BlurCfgUboIdx
				, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				, VK_SHADER_STAGE_FRAGMENT_BIT ),
			makeDescriptorSetLayoutBinding( NmlImgIdx
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT ),
			makeDescriptorSetLayoutBinding( InpImgIdx
				, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
				, VK_SHADER_STAGE_FRAGMENT_BIT ),
		};
		createPipeline( m_size
			, Position{}
			, m_program
			, m_bentInput.getTexture()->getDefaultView()
			, *m_renderPass
			, std::move( bindings )
			, {} );
		m_commandBuffer = device.graphicsCommandPool->createCommandBuffer( getName() );

		m_commandBuffer->begin( VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT );
		m_commandBuffer->beginDebugBlock(
			{
				"SSAO - " + getName(),
				makeFloatArray( m_engine.getNextRainbowColour() ),
			} );
		m_timer->beginPass( *m_commandBuffer );
		m_commandBuffer->beginRenderPass( *m_renderPass
			, *m_fbo
			, { opaqueWhiteClearColor }
			, VK_SUBPASS_CONTENTS_INLINE );
		registerFrame( *m_commandBuffer );
		m_timer->endPass( *m_commandBuffer );
		m_commandBuffer->endRenderPass();
		m_commandBuffer->endDebugBlock();
		m_commandBuffer->end();
	}

	SsaoBlurPass::~SsaoBlurPass()
	{
		m_timer.reset();
		m_fbo.reset();
		m_renderPass.reset();
		m_result.cleanup();
	}

	void SsaoBlurPass::update()const
	{
		if ( m_config.blurRadius.isDirty() )
		{
			auto & configuration = m_configurationUbo->getData();

			switch ( m_config.blurRadius.value().value() )
			{
			case 1u:
				configuration.gaussian[0][0] = 0.5f;
				configuration.gaussian[0][1] = 0.25f;
				break;
			case 2u:
				configuration.gaussian[0][0] = 0.153170f;
				configuration.gaussian[0][1] = 0.144893f;
				configuration.gaussian[0][2] = 0.122649f;
				break;
			case 3u:
				configuration.gaussian[0][0] = 0.153170f;
				configuration.gaussian[0][1] = 0.144893f;
				configuration.gaussian[0][2] = 0.122649f;
				configuration.gaussian[0][3] = 0.092902f;
				break;
			case 4u:
				configuration.gaussian[0][0] = 0.153170f;
				configuration.gaussian[0][1] = 0.144893f;
				configuration.gaussian[0][2] = 0.122649f;
				configuration.gaussian[0][3] = 0.092902f;
				configuration.gaussian[1][0] = 0.062970f;
				break;
			case 5u:
				configuration.gaussian[0][0] = 0.111220f;
				configuration.gaussian[0][1] = 0.107798f;
				configuration.gaussian[0][2] = 0.098151f;
				configuration.gaussian[0][3] = 0.083953f;
				configuration.gaussian[1][0] = 0.067458f;
				configuration.gaussian[1][1] = 0.050920f;
				break;
			default:
				configuration.gaussian[0][0] = 0.111220f;
				configuration.gaussian[0][1] = 0.107798f;
				configuration.gaussian[0][2] = 0.098151f;
				configuration.gaussian[0][3] = 0.083953f;
				configuration.gaussian[1][0] = 0.067458f;
				configuration.gaussian[1][1] = 0.050920f;
				configuration.gaussian[1][2] = 0.036108f;
				break;
			}

			m_configurationUbo->upload();
		}
	}

	ashes::Semaphore const & SsaoBlurPass::blur( ashes::Semaphore const & toWait )const
	{
		auto timerBlock = m_timer->start();
		m_timer->notifyPassRender();
		auto * result = &toWait;

		auto & device = getCurrentRenderDevice( m_renderSystem );
		device.graphicsQueue->submit( *m_commandBuffer
			, *result
			, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			, *m_finished
			, nullptr );
		result = m_finished.get();

		return *result;
	}

	void SsaoBlurPass::accept( bool horizontal
		, SsaoConfig & config
		, RenderTechniqueVisitor & visitor )
	{
		if ( horizontal )
		{
			visitor.visit( "SSAO HBlurred AO", getResult().getTexture()->getDefaultView() );
			visitor.visit( "HBlurred Bent Normals", getBentResult().getTexture()->getDefaultView() );
		}
		else
		{
			visitor.visit( "SSAO Blurred AO", getResult().getTexture()->getDefaultView() );
			visitor.visit( "Blurred Bent Normals", getBentResult().getTexture()->getDefaultView() );
		}

		visitor.visit( m_vertexShader );
		visitor.visit( m_pixelShader );
		config.accept( m_vertexShader.name, visitor );
	}

	void SsaoBlurPass::doFillDescriptorSet( ashes::DescriptorSetLayout & descriptorSetLayout
		, ashes::DescriptorSet & descriptorSet )
	{
		descriptorSet.createSizedBinding( descriptorSetLayout.getBinding( SsaoCfgUboIdx )
			, m_ssaoConfigUbo.getUbo().getBuffer()
			, 0u
			, 1u );
		descriptorSet.createSizedBinding( descriptorSetLayout.getBinding( GpInfoUboIdx )
			, m_gpInfoUbo.getUbo().getBuffer()
			, 0u
			, 1u );
		descriptorSet.createSizedBinding( descriptorSetLayout.getBinding( BlurCfgUboIdx )
			, m_configurationUbo->getBuffer()
			, 0u
			, 1u );
		descriptorSet.createBinding( descriptorSetLayout.getBinding( NmlImgIdx )
			, m_normals
			, m_sampler->getSampler() );
		descriptorSet.createBinding( descriptorSetLayout.getBinding( InpImgIdx )
			, m_input.getTexture()->getDefaultView()
			, m_input.getSampler()->getSampler() );
	}

	void SsaoBlurPass::doRegisterFrame( ashes::CommandBuffer & commandBuffer )const
	{
	}
}