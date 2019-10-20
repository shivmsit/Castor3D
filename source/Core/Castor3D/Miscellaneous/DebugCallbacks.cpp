#include "Castor3D/Miscellaneous/DebugCallbacks.hpp"

#include <ashespp/Core/Instance.hpp>
#include <ashespp/Core/RendererList.hpp>

namespace castor3d
{
	namespace
	{
#if VK_EXT_debug_utils

		std::ostream & operator<<( std::ostream & stream, VkDebugUtilsObjectNameInfoEXT const & value )
		{
			stream << "(" << std::hex << value.objectHandle << ") " << ashes::getName( value.objectType );

			if ( value.pObjectName )
			{
				stream << " " << value.pObjectName;
			}

			return stream;
		}

		std::ostream & operator<<( std::ostream & stream, VkDebugUtilsLabelEXT const & value )
		{
			stream << "(" << value.color[0]
				<< ", " << value.color[1]
				<< ", " << value.color[2]
				<< ", " << value.color[3] << ")";

			if ( value.pLabelName )
			{
				stream << " " << value.pLabelName;
			}

			return stream;
		}

		template< typename ObjectT >
		void print( std::ostream & stream
			, std::string const & name
			, uint32_t count
			, ObjectT const * objects )
		{
			stream << "    " << name << ": " << count << "\n";

			for ( uint32_t i = 0u; i < count; ++i, ++objects )
			{
				stream << "      " << *objects << "\n";
			}
		}

		VkBool32 VKAPI_PTR debugMessageCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
			, VkDebugUtilsMessageTypeFlagsEXT messageTypes
			, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData
			, void * pUserData )
		{
			// Select prefix depending on flags passed to the callback
			// Note that multiple flags may be set for a single validation message
			std::locale loc{ "C" };
			std::stringstream stream;
			stream.imbue( loc );
			stream << "Vulkan ";

			// Error that may result in undefined behaviour
			switch ( messageSeverity )
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				stream << "Error";
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				stream << "Warning";
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				stream << "Info";
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				stream << "Verbose";
				break;
			}

			if ( ashes::checkFlag( messageTypes, VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT ) )
			{
				stream << " - General";
			}
			if ( ashes::checkFlag( messageTypes, VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ) )
			{
				stream << " - Validation";
			}
			if ( ashes::checkFlag( messageTypes, VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ) )
			{
				stream << " - Performance";
			}

			stream << ":\n";
			stream << "    Message ID: " << pCallbackData->pMessageIdName << "\n";
			stream << "    Code: 0x" << std::hex << pCallbackData->messageIdNumber << "\n";
			stream << "    Message: " << pCallbackData->pMessage << "\n";
			print( stream, "Objects", pCallbackData->objectCount, pCallbackData->pObjects );
			print( stream, "Queue Labels", pCallbackData->queueLabelCount, pCallbackData->pQueueLabels );
			print( stream, "CommmandBuffer Labels", pCallbackData->cmdBufLabelCount, pCallbackData->pCmdBufLabels );

			VkBool32 result = VK_FALSE;

			switch ( messageSeverity )
			{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				castor::Logger::logError( stream );
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				castor::Logger::logWarning( stream );
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				castor::Logger::logTrace( stream );
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				castor::Logger::logDebug( stream );
				break;
			}

			// The return value of this callback controls wether the Vulkan call that caused
			// the validation message will be aborted or not
			// Return VK_FALSE if we DON'T want Vulkan calls that cause a validation message 
			// (and return a VkResult) to abort
			// Return VK_TRUE if you instead want to have calls abort, and the function will 
			// return VK_ERROR_VALIDATION_FAILED_EXT 
			return result;
		}

		VkDebugUtilsMessengerEXT createDebugUtilsMessenger( ashes::Instance const & instance
			, void * userData )
		{
			VkDebugUtilsMessageSeverityFlagsEXT severityFlags = 0u
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
				//| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			VkDebugUtilsMessageTypeFlagsEXT typeFlags = 0u
				| VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			VkDebugUtilsMessengerCreateInfoEXT createInfo
			{
				VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				nullptr,
				0u,
				severityFlags,
				typeFlags,
				debugMessageCallback,
				userData
			};
			return instance.createDebugUtilsMessenger( createInfo );
		}

#endif
#if VK_EXT_debug_report

		VkBool32 VKAPI_PTR debugReportCallback( VkDebugReportFlagsEXT flags
			, VkDebugReportObjectTypeEXT objectType
			, uint64_t object
			, size_t location
			, int32_t messageCode
			, const char * pLayerPrefix
			, const char * pMessage
			, void * pUserData )
		{
			// Select prefix depending on flags passed to the callback
			// Note that multiple flags may be set for a single validation message
			std::locale loc{ "C" };
			std::stringstream stream;
			stream.imbue( loc );
			stream << "Vulkan ";

			// Error that may result in undefined behaviour
			if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_ERROR_BIT_EXT ) )
			{
				stream << "Error:\n";
			};
			// Warnings may hint at unexpected / non-spec API usage
			if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_WARNING_BIT_EXT ) )
			{
				stream << "Warning:\n";
			};
			// May indicate sub-optimal usage of the API
			if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT ) )
			{
				stream << "Performance:\n";
			};
			// Informal messages that may become handy during debugging
			if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_INFORMATION_BIT_EXT ) )
			{
				stream << "Info:\n";
			}
			// Diagnostic info from the Vulkan loader and layers
			// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
			if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_DEBUG_BIT_EXT ) )
			{
				stream << "Debug:\n";
			}

			// Display message to default output (console/logcat)
			stream << "    Layer: " << pLayerPrefix << "\n";
			stream << "    Code: 0x" << std::hex << messageCode << "\n";
			stream << "    Object: (" << std::hex << object << ") " << ashes::getName( objectType ) << "\n";
			stream << "    Message: " << pMessage;

			VkBool32 result = VK_FALSE;

			if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_ERROR_BIT_EXT ) )
			{
				castor::Logger::logError( stream );
			}
			else if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_WARNING_BIT_EXT )
				|| ashes::checkFlag( flags, VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT ) )
			{
				castor::Logger::logWarning( stream );
			}
			else if ( ashes::checkFlag( flags, VK_DEBUG_REPORT_INFORMATION_BIT_EXT ) )
			{
				castor::Logger::logTrace( stream );
			}
			else
			{
				castor::Logger::logDebug( stream );
			}

			// The return value of this callback controls wether the Vulkan call that caused
			// the validation message will be aborted or not
			// Return VK_FALSE if we DON'T want Vulkan calls that cause a validation message 
			// (and return a VkResult) to abort
			// Return VK_TRUE if you instead want to have calls abort, and the function will 
			// return VK_ERROR_VALIDATION_FAILED_EXT 
			return result;
		}

		VkDebugReportCallbackEXT createDebugReportCallback( ashes::Instance const & instance
			, void * userData )
		{
			VkDebugReportFlagsEXT debugReportFlags = 0u
				| VK_DEBUG_REPORT_DEBUG_BIT_EXT
				//| VK_DEBUG_REPORT_INFORMATION_BIT_EXT
				| VK_DEBUG_REPORT_WARNING_BIT_EXT
				| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
				| VK_DEBUG_REPORT_ERROR_BIT_EXT;
			VkDebugReportCallbackCreateInfoEXT createInfo
			{
				VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
				nullptr,
				debugReportFlags,
				debugReportCallback,
				userData
			};
			return instance.createDebugReportCallback( createInfo );
		}

#endif
	}

	//*************************************************************************

	DebugCallbacks::DebugCallbacks( ashes::Instance const & instance
		, void * userData )
		: m_instance{ instance }
		, m_userData{ userData }
	{
#if VK_EXT_debug_utils

		m_messenger = createDebugUtilsMessenger( instance, userData );

		if ( m_messenger == VK_NULL_HANDLE )
#endif
		{

#if VK_EXT_debug_report

			m_callback = createDebugReportCallback( instance, userData );

#endif
		}
	}

	DebugCallbacks::~DebugCallbacks()
	{
#if VK_EXT_debug_report

		if ( m_callback != VK_NULL_HANDLE )
		{
			m_instance.vkDestroyDebugReportCallbackEXT( m_instance, m_callback, nullptr );
		}

#endif
#if VK_EXT_debug_utils

		if ( m_messenger != VK_NULL_HANDLE )
		{
			m_instance.vkDestroyDebugUtilsMessengerEXT( m_instance, m_messenger, nullptr );
		}

#endif
	}

	//*************************************************************************
}