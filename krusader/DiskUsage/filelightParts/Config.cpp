
#include "Config.h"
#include <tdeconfig.h>
#include <tdeglobal.h>


bool Config::varyLabelFontSizes = true;
bool Config::showSmallFiles = false;
uint Config::contrast = 50;
uint Config::antiAliasFactor = 2;
uint Config::minFontPitch = 10;
uint Config::defaultRingDepth = 4;
Filelight::MapScheme Config::scheme;


inline TDEConfig&
Filelight::Config::tdeconfig()
{
    TDEConfig *config = TDEGlobal::config();
    config->setGroup( "DiskUsage" );
    return *config;
}

void
Filelight::Config::read()
{
    const TDEConfig &config = tdeconfig();

    varyLabelFontSizes = config.readBoolEntry( "varyLabelFontSizes", true );
    showSmallFiles     = config.readBoolEntry( "showSmallFiles", false );
    contrast           = config.readNumEntry( "contrast", 50 );
    antiAliasFactor    = config.readNumEntry( "antiAliasFactor", 2 );
    minFontPitch       = config.readNumEntry( "minFontPitch", TQFont().pointSize() - 3);
    scheme = (MapScheme) config.readNumEntry( "scheme", 0 );

    defaultRingDepth   = 4;
}

void
Filelight::Config::write()
{
    TDEConfig &config = tdeconfig();

    config.writeEntry( "varyLabelFontSizes", varyLabelFontSizes );
    config.writeEntry( "showSmallFiles", showSmallFiles);
    config.writeEntry( "contrast", contrast );
    config.writeEntry( "antiAliasFactor", antiAliasFactor );
    config.writeEntry( "minFontPitch", minFontPitch );
    config.writeEntry( "scheme", scheme );
}
