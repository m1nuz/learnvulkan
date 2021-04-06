#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <vulkan/vulkan.h>

#include <optional>

#include "config.h"
#include "journal.hpp"

namespace Vulkan {

    constexpr char VK_TAG[] = "Vulkan";

#ifdef NDEBUG
    const bool validation_layers_enabled = false;
#else
    const bool validation_layers_enabled = true;
#endif

    static auto error_string( VkResult res ) {
        switch ( res ) {
        case VK_SUCCESS:
            return "Success";
        case VK_NOT_READY:
            return "Not ready";
        case VK_TIMEOUT:
            return "Timeout";
        case VK_EVENT_SET:
            return "Event set";
        case VK_EVENT_RESET:
            return "Event reset";
        case VK_INCOMPLETE:
            return "Incomplete";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "Error out of host memory";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "Error out of device memory";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "Error initialization failed";
        case VK_ERROR_DEVICE_LOST:
            return "Error device lost";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "Error memory map failed";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "Error layer not present";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "Error extension not present";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "Error feature not present";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "Error incompatible driver";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "Error too many objects";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "Error format not supported";
        case VK_ERROR_FRAGMENTED_POOL:
            return "Error fragmented pool";
        case VK_ERROR_UNKNOWN:
            return "Error unknown";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "Error out of pool memory";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "Error invalid external handle";
        case VK_ERROR_FRAGMENTATION:
            return "Error fragmentation";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "Error invalid opaque capture address";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "Error surface lost";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "Error native window in use";
        case VK_SUBOPTIMAL_KHR:
            return "Suboptimal";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "Error out of date";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "Error incompatible display";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "Error validation failed";
        case VK_ERROR_INVALID_SHADER_NV:
            return "Error invalid shader NV";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "Error invalid DRM format modifier plane layout";
        case VK_ERROR_NOT_PERMITTED_EXT:
            return "Error not permitted";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "Error full screen exclusive mode lost";
        }

        return "";
    }

    namespace Debugging {
#ifdef LUNAR_VALIDATION
        const std::vector<const char*> validation_layers = { "VK_LAYER_LUNARG_standard_validation" };
#else
        const std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };
#endif

        static VkDebugUtilsMessengerEXT debug_utils_messenger;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData ) {

            if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ) {
                journal::verbose( VK_TAG, "{}", pCallbackData->pMessage );
            } else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ) {
                journal::info( VK_TAG, "{}", pCallbackData->pMessage );
            } else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ) {
                journal::warning( VK_TAG, "{}", pCallbackData->pMessage );
            } else if ( messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT ) {
                journal::error( VK_TAG, "{}", pCallbackData->pMessage );
            }

            return VK_FALSE;
        }

        static auto CreateDebugUtilsMessengerEXT( VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
            const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback ) noexcept {
            auto vkCreateDebugUtilsMessenger_fp
                = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
            if ( vkCreateDebugUtilsMessenger_fp )
                return vkCreateDebugUtilsMessenger_fp( instance, pCreateInfo, pAllocator, pCallback );

            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator ) noexcept {
            auto vkDestroyDebugUtilsMessenger_fp
                = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
            if ( vkDestroyDebugUtilsMessenger_fp ) {
                vkDestroyDebugUtilsMessenger_fp( instance, callback, pAllocator );
            }
        }

        [[nodiscard]] static auto setup( const VkInstance instance ) noexcept {
            VkDebugUtilsMessengerCreateInfoEXT create_info = { };
            create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            create_info.pfnUserCallback = debugCallback;
            create_info.pUserData = nullptr;

            if ( const auto res = CreateDebugUtilsMessengerEXT( instance, &create_info, nullptr, &debug_utils_messenger );
                 res != VK_SUCCESS ) {
                journal::error( VK_TAG, "{}", error_string( res ) );
                return false;
            }

            return true;
        }

        static auto cleanup( const VkInstance instance ) noexcept {
            DestroyDebugUtilsMessengerEXT( instance, debug_utils_messenger, nullptr );
        }

    } // namespace Debugging

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    struct QueueParameters {
        VkQueue handle = VK_NULL_HANDLE;
        uint32_t family_index = 0;
    };

    [[nodiscard]] static auto check_validation_layer_support( const std::vector<const char*>& validation_layers ) {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties( &layer_count, nullptr );

        std::vector<VkLayerProperties> available_layers( layer_count );
        vkEnumerateInstanceLayerProperties( &layer_count, std::data( available_layers ) );

        for ( const char* layer_name : validation_layers ) {
            bool layer_found = false;

            for ( const auto& layerProperties : available_layers ) {
                if ( strcmp( layer_name, layerProperties.layerName ) == 0 ) {
                    layer_found = true;
                    break;
                }
            }

            if ( !layer_found ) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] static auto required_extensions( ) -> std::vector<const char*> {
        uint32_t glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions( &glfw_extension_count );

        std::vector<const char*> extensions( glfw_extensions, glfw_extensions + glfw_extension_count );

        if ( validation_layers_enabled ) {
            extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
        }

        return extensions;
    }

    [[nodiscard]] static auto create_instance( ) -> std::optional<VkInstance> {
        if ( validation_layers_enabled && !check_validation_layer_support( Debugging::validation_layers ) ) {
            journal::error( VK_TAG, "{}", "Validation layers not supported!" );
            return { };
        }

        VkApplicationInfo app_info = { };
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = APP_NAME;
        app_info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
        app_info.apiVersion = VK_API_VERSION_1_0;

        auto extensions = required_extensions( );

        VkInstanceCreateInfo create_info = { };
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = std::size( extensions );
        create_info.ppEnabledExtensionNames = std::data( extensions );
        if ( validation_layers_enabled ) {
            create_info.enabledLayerCount = static_cast<uint32_t>( std::size( Debugging::validation_layers ) );
            create_info.ppEnabledLayerNames = std::data( Debugging::validation_layers );
        } else {
            create_info.enabledLayerCount = 0;
            create_info.pNext = nullptr;
        }

        VkInstance instance;
        if ( const auto res = vkCreateInstance( &create_info, nullptr, &instance ); res != VK_SUCCESS ) {
            journal::error( VK_TAG, "{}", error_string( res ) );
            return { };
        }

        if ( validation_layers_enabled ) {
            if ( !Debugging::setup( instance ) ) {
                journal::warning( VK_TAG, "Failed to setup debug messager!" );
            }
        }

        uint32_t supported_extension_count = 0;
        if ( const auto res = vkEnumerateInstanceExtensionProperties( nullptr, &supported_extension_count, nullptr ); res != VK_SUCCESS ) {
            journal::error( VK_TAG, "{}", error_string( res ) );
            return { };
        }

        std::vector<VkExtensionProperties> supported_extensions;
        supported_extensions.resize( supported_extension_count );
        if ( const auto res
             = vkEnumerateInstanceExtensionProperties( nullptr, &supported_extension_count, std::data( supported_extensions ) );
             res != VK_SUCCESS ) {
            journal::error( VK_TAG, "{}", error_string( res ) );
            return { };
        }

        for ( const auto ext : supported_extensions ) {
            journal::verbose( VK_TAG, "{} : {}", ext.extensionName, ext.specVersion );
        }

        return { instance };
    }

    static auto destroy_instance( VkInstance instance ) {
        if ( validation_layers_enabled ) {
            Debugging::cleanup( instance );
        }

        vkDestroyInstance( instance, nullptr );
    }

    [[nodiscard]] static auto enumerate_physical_devices( const VkInstance instance ) -> std::vector<VkPhysicalDevice> {
        uint32_t gpu_count = 0;
        if ( const auto res = vkEnumeratePhysicalDevices( instance, &gpu_count, nullptr ); res != VK_SUCCESS ) {
            journal::error( VK_TAG, "{}", error_string( res ) );
            return { };
        }

        std::vector<VkPhysicalDevice> devices;
        devices.resize( gpu_count );

        if ( const auto res = vkEnumeratePhysicalDevices( instance, &gpu_count, std::data( devices ) ); res != VK_SUCCESS ) {
            journal::error( VK_TAG, "{}", error_string( res ) );
            return { };
        }

        return devices;
    }

    [[nodiscard]] static auto is_device_suitable( VkPhysicalDevice physical_device, VkSurfaceKHR presentation_surface,
        uint32_t& selected_graphics_queue_family_index, uint32_t& selected_present_queue_family_index ) {

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        VkPhysicalDeviceMemoryProperties device_memory_properties;

        vkGetPhysicalDeviceProperties( physical_device, &device_properties );
        vkGetPhysicalDeviceFeatures( physical_device, &device_features );
        vkGetPhysicalDeviceMemoryProperties( physical_device, &device_memory_properties );

        const auto major_version = VK_VERSION_MAJOR( device_properties.apiVersion );

        if ( ( major_version < 1 ) || ( device_properties.limits.maxImageDimension2D < 4096 ) ) {
            journal::error( VK_TAG, "Physical device {}:{} doesn't support required parameters!", device_properties.deviceID,
                device_properties.deviceName );
            return false;
        }

        if ( device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || !device_features.geometryShader ) {
            journal::error( VK_TAG, "Physical device {}:{} doesn't support required parameters!", device_properties.deviceID,
                device_properties.deviceName );
            return false;
        }

        uint32_t queue_families_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_families_count, nullptr );
        if ( queue_families_count == 0 ) {
            journal::error( VK_TAG, "Physical device {}:{} doesn't have any queue families! ", device_properties.deviceID,
                device_properties.deviceName );
            return false;
        }

        std::vector<VkQueueFamilyProperties> queue_family_properties;
        queue_family_properties.resize( queue_families_count );
        std::vector<VkBool32> queue_present_support;
        queue_present_support.resize( queue_families_count );

        auto graphics_queue_family_index = UINT32_MAX;
        auto present_queue_family_index = UINT32_MAX;

        vkGetPhysicalDeviceQueueFamilyProperties( physical_device, &queue_families_count, std::data( queue_family_properties ) );

        auto idx = 0;
        for ( const auto& property : queue_family_properties ) {
            vkGetPhysicalDeviceSurfaceSupportKHR( physical_device, idx, presentation_surface, &queue_present_support[idx] );

            // Select first queue that supports graphics
            if ( property.queueCount > 0 && property.queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
                if ( graphics_queue_family_index == UINT32_MAX ) {
                    graphics_queue_family_index = idx;
                }
            }

            // If there is queue that supports both graphics and present - prefer it
            if ( queue_present_support[idx] ) {
                selected_graphics_queue_family_index = selected_present_queue_family_index = idx;
                return true;
            }

            idx++;
        }

        // We don't have queue that supports both graphics and present so we have to use separate queues
        for ( uint32_t i = 0; i < queue_families_count; ++i ) {
            if ( queue_present_support[i] ) {
                present_queue_family_index = i;
                break;
            }
        }

        if ( ( graphics_queue_family_index == UINT32_MAX ) || ( present_queue_family_index == UINT32_MAX ) ) {
            journal::error( VK_TAG, "Could not find queue families with required properties on physical device {}:{}! ",
                device_properties.deviceID, device_properties.deviceName );
            return false;
        }

        selected_graphics_queue_family_index = graphics_queue_family_index;
        selected_present_queue_family_index = present_queue_family_index;

        return true;
    }

    [[nodiscard]] static auto pick_physical_device( const VkInstance instance, VkSurfaceKHR presentation_surface,
        uint32_t& selected_graphics_queue_family_index, uint32_t& selected_present_queue_family_index ) -> std::optional<VkPhysicalDevice> {

        auto physical_devices = enumerate_physical_devices( instance );
        if ( physical_devices.empty( ) )
            return { };

        auto selected_device_index = 0u;
        for ( const auto& device : physical_devices ) {
            if ( is_device_suitable(
                     device, presentation_surface, selected_graphics_queue_family_index, selected_present_queue_family_index ) )
                break;

            selected_device_index++;
        }

        if ( selected_device_index >= physical_devices.size( ) ) {
            journal::error( VK_TAG, "Invalid device index" );
            return { };
        }

        const auto physical_device = physical_devices[selected_device_index];

        return { physical_device };
    }

    [[nodiscard]] auto create_logical_device( VkPhysicalDevice physical_device, const uint32_t selected_graphics_queue_family_index,
        const uint32_t selected_present_queue_family_index ) -> std::optional<VkDevice> {
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        const float queue_priorities[] = { 1.0f };

        if ( selected_graphics_queue_family_index != UINT32_MAX ) {
            VkDeviceQueueCreateInfo queue_create_info = { };
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = selected_graphics_queue_family_index;
            queue_create_info.queueCount = std::size( queue_priorities );
            queue_create_info.pQueuePriorities = std::data( queue_priorities );

            queue_create_infos.push_back( queue_create_info );
        }

        if ( selected_graphics_queue_family_index != selected_present_queue_family_index ) {
            VkDeviceQueueCreateInfo queue_create_info = { };
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = selected_present_queue_family_index;
            queue_create_info.queueCount = std::size( queue_priorities );
            queue_create_info.pQueuePriorities = std::data( queue_priorities );

            queue_create_infos.push_back( queue_create_info );
        }

        const std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo device_create_info = { };
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_create_info.pQueueCreateInfos = std::data( queue_create_infos );
        device_create_info.queueCreateInfoCount = std::size( queue_create_infos );
        device_create_info.ppEnabledExtensionNames = std::data( extensions );
        device_create_info.enabledExtensionCount = std::size( extensions );

        VkDevice device;
        if ( vkCreateDevice( physical_device, &device_create_info, nullptr, &device ) != VK_SUCCESS ) {
            journal::error( VK_TAG, "Could not create vulkan device!" );
            return { };
        }

        return { device };
    }

    auto destroy_logical_device( VkDevice device ) noexcept {
        if ( const auto res = vkDeviceWaitIdle( device ); res != VK_SUCCESS ) {
            journal::error( VK_TAG, "Couldn't wait device %1", error_string( res ) );
        }

        vkDestroyDevice( device, nullptr );
    }

} // namespace Vulkan

namespace Application {

    constexpr uint32_t WINDOW_WIDTH = 1920;
    constexpr uint32_t WINDOW_HEIGHT = 1080;
    constexpr char WINDOW_TITLE[] = "Learn Vulkan Window";

    constexpr char APP_TAG[] = "App";

    struct ApplicationContext {
        GLFWwindow* window = nullptr;
        VkInstance instance;
        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice logical_device = VK_NULL_HANDLE;
        VkSurfaceKHR presentation_surface = VK_NULL_HANDLE;
        Vulkan::QueueParameters graphics_queue = { };
        Vulkan::QueueParameters present_queue = { };
    };

    static ApplicationContext app_context;

    static void center_window( GLFWwindow* window, GLFWmonitor* monitor ) {
        if ( !monitor )
            return;

        const GLFWvidmode* mode = glfwGetVideoMode( monitor );
        if ( !mode )
            return;

        int monitor_x = 0, monitor_y = 0;
        glfwGetMonitorPos( monitor, &monitor_x, &monitor_y );

        int width = 0, height = 0;
        glfwGetWindowSize( window, &width, &height );

        glfwSetWindowPos( window, monitor_x + ( mode->width - width ) / 2, monitor_y + ( mode->height - height ) / 2 );
    }

    [[nodiscard]] static auto create_window( ) -> std::optional<GLFWwindow*> {

        glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
        glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );
        glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE );

        auto window = glfwCreateWindow( WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr );
        if ( !window ) {
            const char* description = nullptr;
            const auto err = glfwGetError( &description );
            journal::error( APP_TAG, "{}", description );
            return { };
        }

        center_window( window, glfwGetPrimaryMonitor( ) );
        glfwSetCursorPos( window, WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f );

        glfwShowWindow( window );

        return { window };
    }

    static auto destroy_window( GLFWwindow* window ) {
        glfwDestroyWindow( window );
    }

    static auto startup( ) {
        glfwInit( );
        if ( auto window = create_window( ); window ) {
            app_context.window = *window;
        } else {
            journal::critical( APP_TAG, "Failed to create Window" );
            exit( EXIT_FAILURE );
        }

        if ( auto instance = Vulkan::create_instance( ); instance ) {
            app_context.instance = *instance;
        } else {
            journal::critical( APP_TAG, "Failed to create Vulkan instance" );
            exit( EXIT_FAILURE );
        }

        if ( const auto res
             = glfwCreateWindowSurface( app_context.instance, app_context.window, nullptr, &app_context.presentation_surface );
             res != VK_SUCCESS ) {
            journal::critical( APP_TAG, "Could not create presentation surface!" );
            exit( EXIT_FAILURE );
        }

        auto selected_graphics_queue_family_index = UINT32_MAX;
        auto selected_present_queue_family_index = UINT32_MAX;
        auto selected_physical_device = Vulkan::pick_physical_device( app_context.instance, app_context.presentation_surface,
            selected_graphics_queue_family_index, selected_present_queue_family_index );

        if ( !selected_physical_device ) {
            journal::critical( APP_TAG, "Could not select physical device based on the chosen properties!" );
            exit( EXIT_FAILURE );
        }

        app_context.physical_device = *selected_physical_device;

        auto logical_device = Vulkan::create_logical_device(
            *selected_physical_device, selected_graphics_queue_family_index, selected_present_queue_family_index );
        if ( !logical_device ) {
            journal::critical( APP_TAG, "Couldn't create logical device!" );
            exit( EXIT_FAILURE );
        }

        app_context.logical_device = *logical_device;
        app_context.graphics_queue.family_index = selected_graphics_queue_family_index;
        app_context.present_queue.family_index = selected_present_queue_family_index;

        vkGetDeviceQueue( app_context.logical_device, app_context.graphics_queue.family_index, 0, &app_context.graphics_queue.handle );
        vkGetDeviceQueue( app_context.logical_device, app_context.present_queue.family_index, 0, &app_context.present_queue.handle );
    }

    static auto shutdown( ) {
        vkDestroySurfaceKHR( app_context.instance, app_context.presentation_surface, nullptr );
        Vulkan::destroy_logical_device( app_context.logical_device );
        Vulkan::destroy_instance( app_context.instance );
        destroy_window( app_context.window );
        glfwTerminate( );
    }

    static auto mainloop( ) {
        while ( !glfwWindowShouldClose( app_context.window ) ) {
            glfwPollEvents( );
        }
    }

    auto run( ) -> int {
        startup( );
        mainloop( );
        shutdown( );
        return EXIT_SUCCESS;
    }

} // namespace Application

int main( [[maybe_unused]] int argc, [[maybe_unused]] char* argv[] ) {
    return Application::run( );
}