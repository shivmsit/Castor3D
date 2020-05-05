/*
See LICENSE file in root folder
*/
#ifndef ___C3D_DeferredDirectionalLightPass_H___
#define ___C3D_DeferredDirectionalLightPass_H___

#include "OpaqueModule.hpp"
#include "Castor3D/Shader/Shaders/SdwModule.hpp"

#include "Castor3D/Render/Viewport.hpp"
#include "Castor3D/Render/Technique/Opaque/LightPass.hpp"

namespace castor3d
{
	class DirectionalLightPass
		: public LightPass
	{
	protected:
		/**
		\author		Sylvain DOREMUS
		\version	0.10.0
		\date		08/06/2017
		\~english
		\brief		Directional light pass program.
		\~french
		\brief		Programme de passe de lumière directionnelle.
		*/
		struct Program
			: public LightPass::Program
		{
		public:
			/**
			 *\~english
			 *\brief		Constructor.
			 *\param[in]	engine		The engine.
			 *\param[in]	pass		The light pass.
			 *\param[in]	vtx			The vertex shader source.
			 *\param[in]	pxl			The fragment shader source.
			 *\param[in]	hasShadows	Tells if this program uses shadow map.
			 *\~french
			 *\brief		Constructeur.
			 *\param[in]	engine		Le moteur.
			 *\param[in]	pass		La passe d'éclairage.
			 *\param[in]	vtx			Le source du vertex shader.
			 *\param[in]	pxl			Le source du fagment shader.
			 *\param[in]	hasShadows	Dit si ce programme utilise une shadow map.
			 */
			Program( Engine & engine
				, DirectionalLightPass & pass
				, ShaderModule const & vtx
				, ShaderModule const & pxl
				, bool hasShadows );
			/**
			 *\~english
			 *\brief		Destructor.
			 *\~french
			 *\brief		Destructeur.
			 */
			virtual ~Program();

		private:
			/**
			 *\copydoc		castor3d::LightPass::Program::doCreatePipeline
			 */
			ashes::GraphicsPipelinePtr doCreatePipeline( ashes::PipelineVertexInputStateCreateInfo const & vertexLayout
				, ashes::RenderPass const & renderPass
				, bool blend )override;
			/**
			 *\copydoc		castor3d::LightPass::Program::doBind
			 */
			void doBind( Light const & light )override;

		private:
			DirectionalLightPass & m_lightPass;
		};

	public:
		/**
		 *\~english
		 *\brief		Constructor.
		 *\param[in]	engine			The engine.
		 *\param[in]	depthView		The target depth view.
		 *\param[in]	diffuseView		The target diffuse view.
		 *\param[in]	specularView	The target specular view.
		 *\param[in]	gpInfoUbo		The geometry pass UBO.
		 *\param[in]	hasShadows		Tells if shadows are enabled for this light pass.
		 *\~french
		 *\brief		Constructeur.
		 *\param[in]	engine			Le moteur.
		 *\param[in]	depthView		La vue de profondeur cible.
		 *\param[in]	diffuseView		La vue de diffuse cible.
		 *\param[in]	specularView	La vue de spéculaire cible.
		 *\param[in]	gpInfoUbo		L'UBO de la geometry pass.
		 *\param[in]	hasShadows		Dit si les ombres sont activées pour cette passe d'éclairage.
		 */
		DirectionalLightPass( Engine & engine
			, ashes::ImageView const & depthView
			, ashes::ImageView const & diffuseView
			, ashes::ImageView const & specularView
			, GpInfoUbo & gpInfoUbo
			, bool hasShadows );
		/**
		 *\copydoc		castor3d::LightPass::initialise
		 */
		void initialise( Scene const & scene
			, GeometryPassResult const & gp
			, SceneUbo & sceneUbo
			, RenderPassTimer & timer )override;
		/**
		 *\copydoc		castor3d::LightPass::cleanup
		 */
		void cleanup()override;
		/**
		 *\copydoc		castor3d::RenderTechniquePass::accept
		 */
		void accept( RenderTechniqueVisitor & visitor )override;
		/**
		 *\~english
		 *\return		The number of primitives to draw.
		 *\~french
		 *\return		Le nombre de primitives à dessiner.
		 */
		uint32_t getCount()const override;

	protected:
		/**
		 *\copydoc		castor3d::LightPass::doUpdate
		 */
		void doUpdate( bool first
			, castor::Size const & size
			, Light const & light
			, Camera const & camera
			, ShadowMap const * shadowMap = nullptr
			, uint32_t shadowMapIndex = 0u )override;
		/**
		 *\copydoc		castor3d::LightPass::doGetVertexShaderSource
		 */
		ShaderPtr doGetVertexShaderSource( SceneFlags const & sceneFlags )const override;
		/**
		 *\copydoc		castor3d::LightPass::doCreateProgram
		 */
		LightPass::ProgramPtr doCreateProgram()override;

	private:
		struct Config
		{
			LightPass::Config base;
			//!\~english	The variable containing the light direction.
			//!\~french		La variable contenant la direction de la lumière.
			castor::Point4f direction;
			//!\~english	The variable containing the light's split depths.
			//!\~french		La variable contenant les profondeurs des cascades de la lumière.
			castor::Point4f splitDepths;
			//!\~english	The variable containing the light space transformation matrices.
			//!\~french		La variable contenant les matrices de transformations de la lumière.
			std::array< castor::Matrix4x4f, shader::DirectionalMaxCascadesCount > transform;
		};
		UniformBufferUPtr< Config > m_ubo;
		Viewport m_viewport;
	};
}

#endif
