/*
See LICENSE file in root folder
*/
#ifndef ___C3D_DepthPass_H___
#define ___C3D_DepthPass_H___

#include "TechniqueModule.hpp"

#include "Castor3D/Render/Technique/RenderTechniquePass.hpp"

namespace castor3d
{
	class DepthPass
		: public RenderTechniquePass
	{
	public:
		/**
		 *\~english
		 *\brief		Constructor for opaque nodes.
		 *\param[in]	name		This pass name.
		 *\param[in]	matrixUbo	The scene matrix UBO.
		 *\param[in]	culler		The render pass culler.
		 *\param[in]	ssaoConfig	The SSAO configuration.
		 *\param[in]	depthBuffer	The target depth buffer.
		 *\~french
		 *\brief		Constructeur pour les noeuds opaques.
		 *\param[in]	name		Le nom de cette passe.
		 *\param[in]	matrixUbo	L'UBO de matrices de la scène.
		 *\param[in]	culler		Le culler pour cette passe.
		 *\param[in]	ssaoConfig	La configuration du SSAO.
		 *\param[in]	depthBuffer	Le tampon de profondeur cible.
		 */
		DepthPass( castor::String const & name
			, MatrixUbo & matrixUbo
			, SceneCuller & culler
			, SsaoConfig const & ssaoConfig
			, TextureLayoutSPtr depthBuffer );
		/**
		 *\~english
		 *\brief		Destructor
		 *\~french
		 *\brief		Destructeur
		 */
		~DepthPass();
		/**
		 *\copydoc		castor3d::RenderTechniquePass::update
		 */
		void update( RenderInfo & info
			, castor::Point2f const & jitter )override;

	private:
		/**
		 *\copydoc		castor3d::RenderPass::doUpdate
		 */
		void doUpdate( RenderQueueArray & queues )override;
		/**
		*\copydoc		castor3d::RenderPass::doCreateUboBindings
		*/
		ashes::VkDescriptorSetLayoutBindingArray doCreateUboBindings( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doUpdateFlags
		 */
		void doUpdateFlags( PipelineFlags & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doUpdatePipeline
		 */
		C3D_API void doUpdatePipeline( RenderPipeline & pipeline)const override;
		/**
		 *\copydoc		castor3d::RenderPass::doCreateDepthStencilState
		 */
		ashes::PipelineDepthStencilStateCreateInfo doCreateDepthStencilState( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doCreateBlendState
		 */
		ashes::PipelineColorBlendStateCreateInfo doCreateBlendState( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doGetVertexShaderSource
		 */
		C3D_API ShaderPtr doGetVertexShaderSource( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doGetGeometryShaderSource
		 */
		C3D_API ShaderPtr doGetGeometryShaderSource( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doGetPhongPixelShaderSource
		 */
		C3D_API ShaderPtr doGetPhongPixelShaderSource( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doGetPbrMRPixelShaderSource
		 */
		C3D_API ShaderPtr doGetPbrMRPixelShaderSource( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doGetPbrSGPixelShaderSource
		 */
		ShaderPtr doGetPbrSGPixelShaderSource( PipelineFlags const & flags )const override;
		/**
		 *\copydoc		castor3d::RenderPass::doGetPixelShaderSource
		 */
		C3D_API ShaderPtr doGetPixelShaderSource( PipelineFlags const & flags )const;

	private:
		ashes::RenderPassPtr m_renderPass;
		ashes::FrameBufferPtr m_frameBuffer;
	};
}

#endif
