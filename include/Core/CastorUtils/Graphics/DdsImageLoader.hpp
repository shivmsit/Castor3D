/*
See LICENSE file in root folder
*/
#ifndef ___CU_DdsImageLoader_H___
#define ___CU_DdsImageLoader_H___

#include "CastorUtils/Graphics/ImageLoader.hpp"

namespace castor
{
	/**
	\~english
	\brief		DDS image loader.
	\~french
	\brief		Loader d'image DDS.
	*/
	class DdsImageLoader
		: public ImageLoaderImpl
	{
	public:
		CU_API static void registerLoader( ImageLoader & reg );
		CU_API static void unregisterLoader( ImageLoader & reg );
		/**
		 *\copydoc castor::ImageLoaderImpl::load
		 */
		CU_API PxBufferPtrArray load( String const & imageFormat
			, uint8_t const * data
			, uint32_t size )const override;
	};
}

#endif
