#include "LightStreaksPostEffect/KawaseUbo.hpp"

#include "LightStreaksPostEffect/LightStreaksPostEffect.hpp"

#include <Castor3D/Engine.hpp>

using namespace castor;
using namespace castor3d;

namespace light_streaks
{
	String const KawaseUbo::Name = cuT( "Kawase" );
	String const KawaseUbo::PixelSize = cuT( "c3d_pixelSize" );
	String const KawaseUbo::Direction = cuT( "c3d_direction" );
	String const KawaseUbo::Samples = cuT( "c3d_samples" );
	String const KawaseUbo::Attenuation = cuT( "c3d_attenuation" );
	String const KawaseUbo::Pass = cuT( "c3d_passCount" );

	KawaseUbo::KawaseUbo( Engine & engine )
		: m_engine{ engine }
	{
	}

	void KawaseUbo::initialise()
	{
		m_ubo = castor3d::makeUniformBuffer< Configuration >( *m_engine.getRenderSystem()
			, PostEffect::Count * 3u
			, 0u
			, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			, "KawaseCfg" );
	}

	void KawaseUbo::cleanup()
	{
		m_ubo.reset();
	}

	void KawaseUbo::update( uint32_t index
		, VkExtent2D const & size
		, castor::Point2f const & direction
		, uint32_t pass )
	{
		Point2f pixelSize{ 1.0f / size.width, 1.0f / size.height };
		auto & data = m_ubo->getData( index );
		data.samples = 4;
		data.attenuation = 0.9f;
		data.pixelSize = pixelSize;
		data.direction = direction;
		data.pass = int( pass );
	}

	void KawaseUbo::upload()
	{
		m_ubo->upload( 0u, uint32_t( m_ubo->getDatas().size() ) );
	}

	//************************************************************************************************
}
