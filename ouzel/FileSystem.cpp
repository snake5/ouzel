// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <sys/stat.h>
#include "CompileConfig.h"
#include "FileSystem.h"

#if defined(OUZEL_PLATFORM_OSX) || defined(OUZEL_PLATFORM_IOS)
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined(OUZEL_PLATFORM_WINDOWS)
#include <Windows.h>
#endif

namespace ouzel
{
#ifdef OUZEL_PLATFORM_WINDOWS
	const std::string FileSystem::DIRECTORY_SEPARATOR = "\\";
#else
	const std::string FileSystem::DIRECTORY_SEPARATOR = "/";
#endif
    
	FileSystem::FileSystem()
    {
        
    }
    
    FileSystem::~FileSystem()
    {
        
    }
    
    bool FileSystem::fileExists(const std::string& filename)
    {
        struct stat buf;
        if (stat(filename.c_str(), &buf) != -1)
        {
            return true;
        }
        return false;
    }
    
    std::string FileSystem::getPath(const std::string& filename)
    {
		std::string appPath;

#if defined(OUZEL_PLATFORM_OSX)
        CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFStringRef urlString = CFURLCopyPath(appUrlRef);
        
        char temporaryCString[256];
        
        CFStringGetCString(urlString, temporaryCString, sizeof(temporaryCString), kCFStringEncodingUTF8);
        
        CFRelease(appUrlRef);
        CFRelease(urlString);

		appPath = std::string(temporaryCString) + "Contents/Resources/";
#endif

#if defined(OUZEL_PLATFORM_WINDOWS)
        wchar_t szBuffer[256];
        GetCurrentDirectoryW(256, szBuffer);

        char temporaryCString[256];

        WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, temporaryCString, sizeof(temporaryCString), nullptr, nullptr);

        appPath = std::string(temporaryCString) + DIRECTORY_SEPARATOR;
#endif

        std::string str = appPath + filename;
        
        if (fileExists(str))
        {
            return str;
        }
        else
        {
            for (const std::string& path : _resourcePaths)
            {
                str = appPath + DIRECTORY_SEPARATOR + path + DIRECTORY_SEPARATOR + filename;
                
                if (fileExists(str))
                {
                    return str;
                }
            }
        }
        
        return "";
    }
    
    void FileSystem::addResourcePath(const std::string& path)
    {
        std::vector<std::string>::iterator i = std::find(_resourcePaths.begin(), _resourcePaths.end(), path);
        
        if (i == _resourcePaths.end())
        {
            _resourcePaths.push_back(path);
        }
    }
}
