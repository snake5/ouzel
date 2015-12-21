// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "CompileConfig.h"
#include "FileSystem.h"

#ifdef OUZEL_PLATFORM_OSX
#include <CoreFoundation/CoreFoundation.h>
#include <sys/stat.h>
#endif

namespace ouzel
{
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
        CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFStringRef urlString = CFURLCopyPath(appUrlRef);
        
        char temporaryCString[1024] = "";
        
        CFStringGetCString(urlString, temporaryCString, sizeof(temporaryCString), kCFStringEncodingUTF8);
        
        CFRelease(appUrlRef);
        CFRelease(urlString);
        
        std::string str = std::string(temporaryCString) + "Contents/Resources/" + filename;
        
        if (fileExists(str))
        {
            return str;
        }
        else
        {
            for (const std::string& path : _resourcePaths)
            {
                str = std::string(temporaryCString) + "Contents/Resources/" + path + "/" + filename;
                
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