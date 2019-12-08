/*
See LICENSE file in root folder
*/
#ifndef ___C3DSMAA_BlendingWeightCalculation_H___
#define ___C3DSMAA_BlendingWeightCalculation_H___

#include "SmaaPostEffect/SmaaConfig.hpp"

#include <Castor3D/PostEffect/PostEffect.hpp>
#include <Castor3D/PostEffect/PostEffectSurface.hpp>
#include <Castor3D/RenderToTexture/RenderQuad.hpp>

#include <ShaderWriter/Shader.hpp>

namespace smaa
{
	class BlendingWeightCalculation
		: public castor3d::RenderQuad
	{
	public:
		BlendingWeightCalculation( castor3d::RenderTarget & renderTarget
			, ashes::ImageView const & edgeDetectionView
			, castor3d::TextureLayoutSPtr depthView
			, SmaaConfig const & config );
		castor3d::CommandsSemaphore prepareCommands( castor3d::RenderPassTimer const & timer
			, uint32_t passIndex );
		void update( castor::Point4i const & subsampleIndices );
		void accept( castor3d::PipelineVisitorBase & visitor );

		inline castor3d::TextureLayoutSPtr getSurface()const
		{
			return m_surface.colourTexture;
		}

	private:
		void doFillDescriptorSet( ashes::DescriptorSetLayout & descriptorSetLayout
			, ashes::DescriptorSet & descriptorSet )override;

	private:
		ashes::ImageView const & m_edgeDetectionView;
		ashes::RenderPassPtr m_renderPass;
		castor3d::PostEffectSurface m_surface;
		ashes::SamplerPtr m_pointSampler;
		castor3d::UniformBufferUPtr< castor::Point4i > m_ubo;
		castor3d::TextureLayoutSPtr m_areaTex;
		castor3d::TextureLayoutSPtr m_searchTex;
		castor3d::ShaderModule m_vertexShader;
		castor3d::ShaderModule m_pixelShader;
	};
}

#endif