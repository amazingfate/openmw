#ifndef OPENMW_COMPONENTS_MISC_TIMECONVERT_H
#define OPENMW_COMPONENTS_MISC_TIMECONVERT_H

#include <cerrno>
#include <chrono>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <system_error>

namespace Misc
{
    inline std::time_t toTimeT(std::filesystem::file_time_type tp)
    {
        using namespace std::chrono;
#if false // __cpp_lib_chrono >= 201907
        const auto systemTime = clock_cast<system_clock>(tp); // it maybe throw exception
#else
        auto systemTime = time_point_cast<system_clock::duration>(
            tp - std::filesystem::file_time_type::clock::now() + system_clock::now());
#endif
        return system_clock::to_time_t(systemTime);
    }

    inline std::string timeTToString(const std::time_t tp, const char* fmt)
    {
        tm time_info{};
#ifdef _WIN32
        if (const errno_t error = localtime_s(&time_info, &tp); error != 0)
            throw std::system_error(error, std::generic_category());
#else
        if (localtime_r(&tp, &time_info) == nullptr)
            throw std::system_error(errno, std::generic_category());
#endif
        std::stringstream out;
        out << std::put_time(&time_info, fmt);
        return out.str();
    }

    inline std::string fileTimeToString(const std::filesystem::file_time_type& tp, const char* fmt)
    {
        return timeTToString(toTimeT(tp), fmt);
    }

    inline std::string timeToString(const std::chrono::system_clock::time_point& tp, const char* fmt)
    {
        return timeTToString(std::chrono::system_clock::to_time_t(tp), fmt);
    }
} // namespace Misc

#endif
