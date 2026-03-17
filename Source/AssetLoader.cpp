#include "AssetLoader.h"

#include <juce_core/juce_core.h>

AssetLoader::AssetLoader()
{
    assetsDir = findAssetsDir();
}

juce::Image AssetLoader::getImage(const juce::String& filename)
{
    std::lock_guard<std::mutex> lock(mutex);

    const std::string key = filename.toStdString();
    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;

    juce::Image img = loadFromDisk(filename);
    if (img.isValid())
        cache[key] = img;
    return img;
}

bool AssetLoader::hasImage(const juce::String& filename)
{
    return getImage(filename).isValid();
}

juce::File AssetLoader::findAssetsDir()
{
    const juce::String assetsSubpath = "assets" + juce::File::getSeparatorString() + "img";

    auto tryDir = [&assetsSubpath](const juce::File& base) -> juce::File
    {
        juce::File candidate = base.getChildFile(assetsSubpath);
        if (candidate.isDirectory())
            return candidate;
        return {};
    };

    juce::File cwd = juce::File::getCurrentWorkingDirectory();
    for (int i = 0; i < 12; ++i)
    {
        juce::File found = tryDir(cwd);
        if (found.exists())
            return found;
        cwd = cwd.getParentDirectory();
        if (!cwd.exists())
            break;
    }

    juce::File exeDir = juce::File::getSpecialLocation(
        juce::File::SpecialLocationType::currentExecutableFile).getParentDirectory();
    for (int i = 0; i < 12; ++i)
    {
        juce::File found = tryDir(exeDir);
        if (found.exists())
            return found;
        exeDir = exeDir.getParentDirectory();
        if (!exeDir.exists())
            break;
    }

    return {};
}

juce::Image AssetLoader::loadFromDisk(const juce::String& filename)
{
    if (!assetsDir.exists())
    {
        if (missingLogged.find(filename.toStdString()) == missingLogged.end())
        {
            missingLogged[filename.toStdString()] = true;
            DBG("AssetLoader: assets dir not found, cannot load " << filename);
        }
        return {};
    }

    juce::File file = assetsDir.getChildFile(filename);
    if (!file.existsAsFile())
    {
        if (missingLogged.find(filename.toStdString()) == missingLogged.end())
        {
            missingLogged[filename.toStdString()] = true;
            DBG("AssetLoader: asset not found " << file.getFullPathName());
        }
        return {};
    }

    juce::Image img = juce::ImageFileFormat::loadFrom(file);
    if (!img.isValid())
    {
        if (missingLogged.find(filename.toStdString()) == missingLogged.end())
        {
            missingLogged[filename.toStdString()] = true;
            DBG("AssetLoader: failed to decode " << filename);
        }
    }
    return img;
}
