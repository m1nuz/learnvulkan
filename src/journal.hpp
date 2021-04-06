#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <string>
#include <string_view>

namespace journal {

    namespace v3 {

        template <typename... Args> inline auto critical( const std::string_view tag, Args&&... args ) -> void {
            const auto str = fmt::format( std::forward<Args>( args )... );
            const auto t = std::time( nullptr );
            const auto style = bg( fmt::terminal_color::red ) | fmt::emphasis::bold;
            fmt::print( style, "{:%Y-%m-%d %H-%M-%S} CRITICAL [{}] {}\n", fmt::localtime( t ), tag, str );
        }

        template <typename... Args> inline auto error( const std::string_view tag, Args&&... args ) -> void {
            const auto str = fmt::format( std::forward<Args>( args )... );
            const auto t = std::time( nullptr );
            const auto style = fg( fmt::terminal_color::bright_red );
            fmt::print( style, "{:%Y-%m-%d %H-%M-%S} ERROR [{}] {}\n", fmt::localtime( t ), tag, str );
        }

        template <typename... Args> inline auto warning( const std::string_view tag, Args&&... args ) -> void {
            const auto str = fmt::format( std::forward<Args>( args )... );
            const auto t = std::time( nullptr );
            const auto style = fg( fmt::terminal_color::bright_yellow );
            fmt::print( style, "{:%Y-%m-%d %H-%M-%S} WARNING [{}] {}\n", fmt::localtime( t ), tag, str );
        }

        template <typename... Args> inline auto info( const std::string_view tag, Args&&... args ) -> void {
            const auto str = fmt::format( std::forward<Args>( args )... );
            const auto t = std::time( nullptr );
            const auto style = fg( fmt::terminal_color::white );
            fmt::print( style, "{:%Y-%m-%d %H-%M-%S} INFO [{}] {}\n", fmt::localtime( t ), tag, str );
        }

        template <typename... Args> inline auto debug( const std::string_view tag, Args&&... args ) -> void {
            const auto str = fmt::format( std::forward<Args>( args )... );
            const auto t = std::time( nullptr );
            const auto style = fg( fmt::terminal_color::cyan );
            fmt::print( style, "{:%Y-%m-%d %H-%M-%S} DEBUG [{}] {}\n", fmt::localtime( t ), tag, str );
        }

        template <typename... Args> inline auto verbose( const std::string_view tag, Args&&... args ) -> void {
            const auto str = fmt::format( std::forward<Args>( args )... );
            const auto t = std::time( nullptr );
            const auto style = fg( fmt::terminal_color::blue );
            fmt::print( style, "{:%Y-%m-%d %H-%M-%S} VERBOSE [{}] {}\n", fmt::localtime( t ), tag, str );
        }

    } // namespace v3

} // namespace journal

namespace journal {

    using namespace v3;

} // namespace journal
