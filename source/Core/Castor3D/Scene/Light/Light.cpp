#include "Castor3D/Scene/Light/Light.hpp"

#include "Castor3D/Engine.hpp"

#include "Castor3D/Scene/Light/DirectionalLight.hpp"
#include "Castor3D/Scene/Light/LightFactory.hpp"
#include "Castor3D/Scene/Light/PointLight.hpp"
#include "Castor3D/Scene/Light/SpotLight.hpp"
#include "Castor3D/Scene/Scene.hpp"

using namespace castor;

namespace castor3d
{
	//*************************************************************************************************

	Light::TextWriter::TextWriter( String const & tabs )
		: castor::TextWriter< Light >{ tabs }
	{
	}

	bool Light::TextWriter::operator()( Light const & object, TextFile & file )
	{
		return object.m_category->createTextWriter( m_tabs )->writeInto( file );
	}

	//*************************************************************************************************

	Light::Light( String const & name
		, Scene & scene
		, SceneNodeSPtr node
		, LightFactory & factory
		, LightType lightType )
		: MovableObject{ name, scene, MovableType::eLight, node }
	{
		m_category = factory.create( lightType, std::ref( *this ) );

		if ( node )
		{
			m_notifyIndex = node->onChanged.connect( [this]( SceneNode const & node )
				{
					onNodeChanged( node );
				} );
			onNodeChanged( *node );
		}
	}

	Light::~Light()
	{
	}

	void Light::update()
	{
		m_category->update();
	}

	void Light::bind( Point4f * buffer )const
	{
		m_category->bind( buffer );
	}

	void Light::attachTo( SceneNodeSPtr node )
	{
		MovableObject::attachTo( node );
		auto parent = getParent();

		if ( parent )
		{
			m_notifyIndex = node->onChanged.connect( [this]( SceneNode const & node )
				{
					onNodeChanged( node );
				} );
			onNodeChanged( *parent );
		}
	}

	DirectionalLightSPtr Light::getDirectionalLight()const
	{
		CU_Require( m_category->getLightType() == LightType::eDirectional );
		return std::static_pointer_cast< DirectionalLight >( m_category );
	}

	PointLightSPtr Light::getPointLight()const
	{
		CU_Require( m_category->getLightType() == LightType::ePoint );
		return std::static_pointer_cast< PointLight >( m_category );
	}

	SpotLightSPtr Light::getSpotLight()const
	{
		CU_Require( m_category->getLightType() == LightType::eSpot );
		return std::static_pointer_cast< SpotLight >( m_category );
	}

	void Light::onNodeChanged( SceneNode const & node )
	{
		m_category->updateNode( node );
	}
}