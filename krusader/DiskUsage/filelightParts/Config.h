
#ifndef Config_H
#define Config_H

#include <tqstringlist.h>

class TDEConfig;


namespace Filelight
{
    enum MapScheme { Rainbow, HighContrast, KDE, FileDensity, ModTime };

    class Config
    {
        static TDEConfig& tdeconfig();

    public:
        static void read();
        static void write();

        //keep everything positive, avoid using DON'T, NOT or NO

        static bool varyLabelFontSizes;
        static bool showSmallFiles;
        static uint contrast;
        static uint antiAliasFactor;
        static uint minFontPitch;
        static uint defaultRingDepth;

        static MapScheme scheme;
    };
}

using Filelight::Config;

#endif
