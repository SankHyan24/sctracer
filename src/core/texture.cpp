#include <core/texture.hpp>
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
namespace scTracer::Core
{
    Texture::Texture(std::string texName, unsigned char *texData, int w, int h, int c) : name(texName), mWidth(w), mHeight(h), mChannel(c)
    {
        data.resize(mWidth * mHeight * mChannel);
        std::copy(texData, texData + mWidth * mHeight * mChannel, data.begin());
    }
    bool Texture::LoadTexture(const std::string &filename)
    {
        name = filename;
        mChannel = 4;
        unsigned char *texData = stbi_load(filename.c_str(), &mWidth, &mHeight, NULL, mChannel);
        if (texData == nullptr)
            return false;
        data.resize(mWidth * mHeight * mChannel);
        std::copy(texData, texData + mWidth * mHeight * mChannel, data.begin());
        stbi_image_free(texData);
        return true;
    }
}
