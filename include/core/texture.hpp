#pragma once

namespace scTracer::Core
{
    class Texture
    {
    public:
        Texture() : mWidth(0), mHeight(0), mChannel(0) {};
        Texture(std::string texName, unsigned char *data, int w, int h, int c);
        ~Texture() {}

        bool LoadTexture(const std::string &filename);

        int mWidth;
        int mHeight;
        int mChannel;
        std::vector<unsigned char> texData;
        std::string name;
    };
}