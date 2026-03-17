#pragma once

#include <juce_graphics/juce_graphics.h>
#include <unordered_map>
#include <mutex>

/** Caches PNG images loaded from assets/img at runtime. Resolves paths relative to
 *  executable and working directory. Returns null Image on missing assets.
 */
class AssetLoader
{
public:
    AssetLoader();
    ~AssetLoader() = default;

    /** Returns a valid Image if the asset was loaded, otherwise an Image with
     *  isNull() true. Logs missing assets once.
     */
    juce::Image getImage(const juce::String& filename);

    /** Returns true if the given asset was successfully loaded. */
    bool hasImage(const juce::String& filename);

private:
    juce::File findAssetsDir();
    juce::Image loadFromDisk(const juce::String& filename);

    juce::File assetsDir;
    std::unordered_map<std::string, juce::Image> cache;
    std::unordered_map<std::string, bool> missingLogged;
    std::mutex mutex;
};
