/***************************************************************************
                   krcolorcache.cpp
                  -------------------
copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
e-mail               : krusader@users.sourceforge.net
web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
Description
***************************************************************************

A

db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                          S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "krcolorcache.h"
#include "../krusader.h"
#include "../defaults.h"
#include <tdeglobalsettings.h> 
#include <tqfile.h> 


// Macro: set target = col, if col is valid
#define SETCOLOR(target, col) { if (col.isValid()) target = col; }

/*
Static class, which lists all allowed keywords for a quick access. Just call a method to initialize it. 
*/
class KrColorSettingNames
{
	static TQMap<TQString, bool> s_colorNames;
	static TQMap<TQString, bool> s_numNames;
	static TQMap<TQString, bool> s_boolNames;
	static void initialize();
public:
	static TQValueList<TQString> getColorNames();
	static bool isColorNameValid(const TQString & settingName);
	static TQValueList<TQString> getNumNames();
	static bool isNumNameValid(const TQString & settingName);
	static TQValueList<TQString> getBoolNames();
	static bool isBoolNameValid(const TQString & settingName);
} krColorSettingNames;

TQMap<TQString, bool> KrColorSettingNames::s_colorNames;
TQMap<TQString, bool> KrColorSettingNames::s_numNames;
TQMap<TQString, bool> KrColorSettingNames::s_boolNames;

void KrColorSettingNames::initialize()
{
	if (!s_colorNames.empty())
		return;
	s_colorNames["Foreground"] = true;
	s_colorNames["Inactive Foreground"] = true;
	s_colorNames["Directory Foreground"] = true;
	s_colorNames["Inactive Directory Foreground"] = true;
	s_colorNames["Executable Foreground"] = true;
	s_colorNames["Inactive Executable Foreground"] = true;
	s_colorNames["Symlink Foreground"] = true;
	s_colorNames["Inactive Symlink Foreground"] = true;
	s_colorNames["Invalid Symlink Foreground"] = true;
	s_colorNames["Inactive Invalid Symlink Foreground"] = true;
	s_colorNames["Marked Foreground"] = true;
	s_colorNames["Inactive Marked Foreground"] = true;
	s_colorNames["Marked Background"] = true;
	s_colorNames["Inactive Marked Background"] = true;
	s_colorNames["Current Foreground"] = true;
	s_colorNames["Inactive Current Foreground"] = true;
	s_colorNames["Current Background"] = true;
	s_colorNames["Inactive Current Background"] = true;
	s_colorNames["Marked Current Foreground"] = true;
	s_colorNames["Inactive Marked Current Foreground"] = true;
	s_colorNames["Alternate Background"] = true;
	s_colorNames["Inactive Alternate Background"] = true;
	s_colorNames["Background"] = true;
	s_colorNames["Inactive Background"] = true;
	s_colorNames["Alternate Marked Background"] = true;
	s_colorNames["Inactive Alternate Marked Background"] = true;
	s_colorNames["Dim Target Color"] = true;

	s_numNames["Dim Factor"] = true;

	s_boolNames["KDE Default"] = true;
	s_boolNames["Enable Alternate Background"] = true;
	s_boolNames["Show Current Item Always"] = true;
	s_boolNames["Dim Inactive Colors"] = true;
}

TQValueList<TQString> KrColorSettingNames::getColorNames()
{
	initialize();
	return s_colorNames.keys();
}

bool KrColorSettingNames::isColorNameValid(const TQString & settingName)
{
	initialize();
	return s_colorNames.contains(settingName);
}

TQValueList<TQString> KrColorSettingNames::getNumNames()
{
	initialize();
	return s_numNames.keys();
}

bool KrColorSettingNames::isNumNameValid(const TQString & settingName)
{
	initialize();
	return s_numNames.contains(settingName);
}

TQValueList<TQString> KrColorSettingNames::getBoolNames()
{
	initialize();
	return s_boolNames.keys();
}

bool KrColorSettingNames::isBoolNameValid(const TQString & settingName)
{
	initialize();
	return s_boolNames.contains(settingName);
}



/*
KrColorSettings implementation. Contains all properties in TQMaps. loadFromConfig initializes them from krConfig.
*/
class KrColorSettingsImpl
{
	friend class KrColorSettings;
	TQMap<TQString, TQString> m_colorTextValues;
	TQMap<TQString, TQColor> m_colorValues;
	TQMap<TQString, int> m_numValues;
	TQMap<TQString, bool> m_boolValues;
	void loadFromConfig();
};

void KrColorSettingsImpl::loadFromConfig()
{
	krConfig->setGroup("Colors");
	TQValueList<TQString> names = KrColorSettingNames::getColorNames();
	for ( TQStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		m_colorTextValues[*it] = krConfig->readEntry(*it, "");
		m_colorValues[*it] = krConfig->readColorEntry(*it);
	}
	names = KrColorSettingNames::getNumNames();
	for ( TQStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		if (krConfig->readEntry(*it) != TQString())
			m_numValues[*it] = krConfig->readNumEntry(*it);
	}
	names = KrColorSettingNames::getBoolNames();
	for ( TQStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		if (krConfig->readEntry(*it) != TQString())
			m_boolValues[*it] = krConfig->readBoolEntry(*it);
	}
}



KrColorSettings::KrColorSettings()
{
	m_impl = new KrColorSettingsImpl();
	m_impl->loadFromConfig();
}

KrColorSettings::KrColorSettings(const KrColorSettings & src)
{
	m_impl = new KrColorSettingsImpl();
	operator =(src);
}

KrColorSettings::~KrColorSettings()
{
	delete m_impl;
}

const KrColorSettings & KrColorSettings::operator= (const KrColorSettings & src)
{
	if (this == & src)
		return * this;
	TQValueList<TQString> names = KrColorSettingNames::getColorNames();
	for ( TQStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		m_impl->m_colorTextValues[*it] = src.m_impl->m_colorTextValues[*it];
		m_impl->m_colorValues[*it] = src.m_impl->m_colorValues[*it];
	}
	for ( TQMap<TQString, int>::Iterator it = src.m_impl->m_numValues.begin(); it != src.m_impl->m_numValues.end(); ++it )
	{
		m_impl->m_numValues[it.key()] = it.data();
	}
	for ( TQMap<TQString, bool>::Iterator it = src.m_impl->m_boolValues.begin(); it != src.m_impl->m_boolValues.end(); ++it )
	{
		m_impl->m_boolValues[it.key()] = it.data();
	}
	return * this;
}

TQValueList<TQString> KrColorSettings::getColorNames()
{
	return KrColorSettingNames::getColorNames();
}

bool KrColorSettings::isColorNameValid(const TQString & settingName)
{
	return KrColorSettingNames::isColorNameValid(settingName);
}

bool KrColorSettings::setColorValue(const TQString & settingName, const TQColor & color)
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_colorValues[settingName] = color;
	return true;
}

TQColor KrColorSettings::getColorValue(const TQString & settingName) const
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return TQColor();
	}
	return m_impl->m_colorValues[settingName];
}

bool KrColorSettings::setColorTextValue(const TQString & settingName, const TQString & colorText)
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_colorTextValues[settingName] = colorText;
	return true;
}

TQString KrColorSettings::getColorTextValue(const TQString & settingName) const
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return TQString();
	}
	return m_impl->m_colorTextValues[settingName];
}

TQValueList<TQString> KrColorSettings::getNumNames()
{
	return KrColorSettingNames::getNumNames();
}

bool KrColorSettings::isNumNameValid(const TQString & settingName)
{
	return KrColorSettingNames::isNumNameValid(settingName);
}

bool KrColorSettings::setNumValue(const TQString & settingName, int value)
{
	if (!isNumNameValid(settingName))
	{
		krOut << "Invalid number setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_numValues[settingName] = value;
	return true;
}

int KrColorSettings::getNumValue(const TQString & settingName, int defaultValue) const
{
	if (!isNumNameValid(settingName))
	{
		krOut << "Invalid number setting name: " << settingName << endl;
		return 0;
	}
	if (!m_impl->m_numValues.contains(settingName))
		return defaultValue;
	return m_impl->m_numValues[settingName];
}

TQValueList<TQString> KrColorSettings::getBoolNames()
{
	return KrColorSettingNames::getBoolNames();
}

bool KrColorSettings::isBoolNameValid(const TQString & settingName)
{
	return KrColorSettingNames::isBoolNameValid(settingName);
}

bool KrColorSettings::setBoolValue(const TQString & settingName, bool value)
{
	if (!isBoolNameValid(settingName))
	{
		krOut << "Invalid bool setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_boolValues[settingName] = value;
	return true;
}

int KrColorSettings::getBoolValue(const TQString & settingName, bool defaultValue) const
{
	if (!isBoolNameValid(settingName))
	{
		krOut << "Invalid bool setting name: " << settingName << endl;
		return false;
	}
	if (!m_impl->m_boolValues.contains(settingName))
		return defaultValue;
	return m_impl->m_boolValues[settingName];
}



KrColorItemType::KrColorItemType()
{
	m_fileType = File;
	m_alternateBackgroundColor = false;
	m_activePanel = false;
	m_currentItem = false;
	m_selectedItem = false;
}

KrColorItemType::KrColorItemType(FileType type, bool alternateBackgroundColor, bool activePanel, bool currentItem, bool selectedItem)
{
	m_fileType = type;
	m_alternateBackgroundColor = alternateBackgroundColor;
	m_activePanel = activePanel;
	m_currentItem = currentItem;
	m_selectedItem = selectedItem;
}

KrColorItemType::KrColorItemType(const KrColorItemType & src)
{
	operator= (src);
}

const KrColorItemType & KrColorItemType::operator= (const KrColorItemType & src)
{
	if (this == & src)
		return * this;
	m_fileType = src.m_fileType;
	m_alternateBackgroundColor = src.m_alternateBackgroundColor;
	m_activePanel = src.m_activePanel;
	m_currentItem = src.m_currentItem;
	m_selectedItem = src.m_selectedItem;
	return * this;
}




/*
KrColorCache implementation. Contains the KrColorSettings used for teh calculation and the cache for the results.
getColors is the only method to call. All other are taken from the previous versions.
*/
class KrColorCacheImpl
{
	friend class KrColorCache;
	TQMap<TQString, TQColorGroup> m_cachedColors;
	KrColorSettings m_colorSettings;
	TQColorGroup getColors(const KrColorItemType & type) const;
	static const TQColor & setColorIfContrastIsSufficient(const TQColor & background, const TQColor & color1, const TQColor & color2);
	TQColor getForegroundColor(bool isActive) const;
	TQColor getSpecialForegroundColor(const TQString & type, bool isActive) const;
	TQColor getBackgroundColor(bool isActive) const;
	TQColor getAlternateBackgroundColor(bool isActive) const;
	TQColor getMarkedForegroundColor(bool isActive) const;
	TQColor getMarkedBackgroundColor(bool isActive) const;
	TQColor getAlternateMarkedBackgroundColor(bool isActive) const;
	TQColor getCurrentForegroundColor(bool isActive) const;
	TQColor getCurrentBackgroundColor(bool isActive) const;
	TQColor getCurrentMarkedForegroundColor(bool isActive) const;
	TQColor dimColor(TQColor color, bool isBackgroundColor) const;
};

TQColorGroup KrColorCacheImpl::getColors(const KrColorItemType & type) const
{
	TQColorGroup result;
	if (m_colorSettings.getBoolValue("KDE Default", _KDEDefaultColors))
	{
		// KDE default? Getcolors from TDEGlobalSettings.
		bool enableAlternateBackground = m_colorSettings.getBoolValue("Enable Alternate Background", _AlternateBackground);
		TQColor background = enableAlternateBackground && type.m_alternateBackgroundColor ? 
			TDEGlobalSettings::alternateBackgroundColor()
			: TDEGlobalSettings::baseColor();
		result.setColor(TQColorGroup::Base, background);
		result.setColor(TQColorGroup::Background, background);
		result.setColor(TQColorGroup::Text, TDEGlobalSettings::textColor());
		result.setColor(TQColorGroup::HighlightedText, TDEGlobalSettings::highlightedTextColor());
		result.setColor(TQColorGroup::Highlight, TDEGlobalSettings::highlightColor());
		return result;
	}
	bool markCurrentAlways = m_colorSettings.getBoolValue("Show Current Item Always", _ShowCurrentItemAlways);
	bool dimBackground = m_colorSettings.getBoolValue("Dim Inactive Colors", false);

	// cache m_activePanel flag. If color dimming is turned on, it is set to true, as the inactive colors
	// are calculated from the active ones at the end.
	bool isActive = type.m_activePanel;
	if (dimBackground)
		isActive = true;

	// First calculate fore- and background.
	TQColor background = type.m_alternateBackgroundColor ?
		getAlternateBackgroundColor(isActive) 
		: getBackgroundColor(isActive);
	TQColor foreground;
	switch(type.m_fileType)
	{
		case KrColorItemType::Directory :
			foreground = getSpecialForegroundColor("Directory", isActive);
			break;
		case KrColorItemType::Executable :
			foreground = getSpecialForegroundColor("Executable", isActive);
			break;
		case KrColorItemType::InvalidSymlink :
			foreground = getSpecialForegroundColor("Invalid Symlink", isActive);
			break;
		case KrColorItemType::Symlink :
			foreground = getSpecialForegroundColor("Symlink", isActive);
			break;
		default:
			foreground = getForegroundColor(isActive);
	}

	// set the background color
	result.setColor(TQColorGroup::Base, background);
	result.setColor(TQColorGroup::Background, background);
	
	// set the foreground color
	result.setColor(TQColorGroup::Text, foreground);

	// now the color of a marked item
	TQColor markedBackground = type.m_alternateBackgroundColor ?
		getAlternateMarkedBackgroundColor(isActive)
		: getMarkedBackgroundColor(isActive);
	TQColor markedForeground = getMarkedForegroundColor(isActive);
	if (!markedForeground.isValid()) // transparent
		// choose fore- or background, depending on its contrast compared to markedBackground
		markedForeground = setColorIfContrastIsSufficient(markedBackground, foreground, background);

	// set it in the color group (different group color than normal foreground!)
	result.setColor(TQColorGroup::HighlightedText, markedForeground);
	result.setColor(TQColorGroup::Highlight, markedBackground);

	// In case the current item is a selected one, set the fore- and background colors for the contrast calculation below
	if (type.m_selectedItem)
	{
		background = markedBackground;
		foreground = markedForeground;
	}
	
	// finally the current item
	if (type.m_currentItem && (markCurrentAlways || isActive))
	{
		// if this is the current item AND the panels has the focus OR the current should be marked always
		TQColor currentBackground = getCurrentBackgroundColor(isActive);

		if (!currentBackground.isValid()) // transparent
			currentBackground = background;
		
		// set the background
		result.setColor(TQColorGroup::Highlight, currentBackground);
		result.setColor(TQColorGroup::Base, currentBackground);
		result.setColor(TQColorGroup::Background, currentBackground);
		
		TQColor color;
		if (type.m_selectedItem)
			color = getCurrentMarkedForegroundColor(isActive);
		if (!color.isValid()) // not used
		{
			color = getCurrentForegroundColor(isActive);
			if (!color.isValid()) // transparent
				// choose fore- or background, depending on its contrast compared to markedBackground
				color = setColorIfContrastIsSufficient(currentBackground, foreground, background);
		}
		
		// set the foreground
		result.setColor(TQColorGroup::Text, color);
		result.setColor(TQColorGroup::HighlightedText, color);
	}

	if (dimBackground && !type.m_activePanel)
	{
		// if color dimming is choosen, dim the colors for the inactive panel 
		result.setColor(TQColorGroup::Base, dimColor(result.base(), true));
		result.setColor(TQColorGroup::Background, dimColor(result.base(), true));
		result.setColor(TQColorGroup::Text, dimColor(result.text(), false));
		result.setColor(TQColorGroup::HighlightedText, dimColor(result.highlightedText(), false));
		result.setColor(TQColorGroup::Highlight, dimColor(result.highlight(), true));
	}
	return result;
}

const TQColor & KrColorCacheImpl::setColorIfContrastIsSufficient(const TQColor & background, const TQColor & color1, const TQColor & color2)
{
   #define sqr(x) ((x)*(x))
   int contrast = sqr(color1.red() - background.red()) + sqr(color1.green() - background.green()) + sqr(color1.blue() - background.blue());

   // if the contrast between background and color1 is too small, take color2 instead.
   if (contrast < 1000)
      return color2;
   return color1;
}

TQColor KrColorCacheImpl::getForegroundColor(bool isActive) const
{
	TQColor color = TDEGlobalSettings::textColor();
	SETCOLOR(color, m_colorSettings.getColorValue("Foreground"));
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue("Inactive Foreground"));
	return color;
}

TQColor KrColorCacheImpl::getSpecialForegroundColor(const TQString & type, bool isActive) const
{
	TQString colorName = "Inactive " + type + " Foreground";
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "Inactive Foreground")
		return getForegroundColor(false);
	TQColor color = m_colorSettings.getColorValue(type + " Foreground");
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue(colorName));
	if (!color.isValid())
		return getForegroundColor(isActive);
	return color;
}

TQColor KrColorCacheImpl::getBackgroundColor(bool isActive) const
{
	TQColor color = TDEGlobalSettings::baseColor();
	SETCOLOR(color, m_colorSettings.getColorValue("Background"));
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue("Inactive Background"));
	return color;
}

TQColor KrColorCacheImpl::getAlternateBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Alternate Background") == "Background")
		return getBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Background") == "")
		return getAlternateBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Background") == "Inactive Background")
		return getBackgroundColor(false);
	TQColor color = isActive ? 
		m_colorSettings.getColorValue("Alternate Background") 
		: m_colorSettings.getColorValue("Inactive Alternate Background");
	if (!color.isValid())
		color = TDEGlobalSettings::alternateBackgroundColor();
	if (!color.isValid())
		color = TDEGlobalSettings::baseColor();
	return color;
}

TQColor KrColorCacheImpl::getMarkedForegroundColor(bool isActive) const
{
	TQString colorName = isActive?"Marked Foreground":"Inactive Marked Foreground";
	if (m_colorSettings.getColorTextValue(colorName) == "transparent")
		return TQColor();
	if (isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return TDEGlobalSettings::highlightedTextColor();
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return getMarkedForegroundColor(true);
	return m_colorSettings.getColorValue(colorName);
}

TQColor KrColorCacheImpl::getMarkedBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Marked Background") == "")
		return TDEGlobalSettings::highlightColor();
	if (isActive && m_colorSettings.getColorTextValue("Marked Background") == "Background")
		return getBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Marked Background") == "")
		return getMarkedBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Marked Background") == "Inactive Background")
		return getBackgroundColor(false);
	return isActive ?
		m_colorSettings.getColorValue("Marked Background")
		: m_colorSettings.getColorValue("Inactive Marked Background");
}

TQColor KrColorCacheImpl::getAlternateMarkedBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Alternate Marked Background") == "Alternate Background")
		return getAlternateBackgroundColor(true);
	if (isActive && m_colorSettings.getColorTextValue("Alternate Marked Background") == "")
		return getMarkedBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Marked Background") == "")
		return getAlternateMarkedBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Marked Background") == "Inactive Alternate Background")
		return getAlternateBackgroundColor(false);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Marked Background") == "Inactive Marked Background")
		return getMarkedBackgroundColor(false);
	return isActive ? 
		m_colorSettings.getColorValue("Alternate Marked Background")
		: m_colorSettings.getColorValue("Inactive Alternate Marked Background");
}

TQColor KrColorCacheImpl::getCurrentForegroundColor(bool isActive) const
{
	TQColor color = m_colorSettings.getColorValue("Current Foreground");
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue("Inactive Current Foreground"));
	return color;
}

TQColor KrColorCacheImpl::getCurrentBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Current Background") == "")
		return TQColor();
	if (isActive && m_colorSettings.getColorTextValue("Current Background") == "Background")
		return getBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Current Background") == "")
		return getCurrentBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Current Background") == "Inactive Background")
		return getBackgroundColor(false);
	return isActive ? 
		m_colorSettings.getColorValue("Current Background")
		: m_colorSettings.getColorValue("Inactive Current Background");
}

TQColor KrColorCacheImpl::getCurrentMarkedForegroundColor(bool isActive) const
{
	TQString colorName = isActive?"Marked Current Foreground":"Inactive Marked Current Foreground";
	if (isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return TQColor();
	if (isActive && m_colorSettings.getColorTextValue(colorName) == "Marked Foreground")
		return getMarkedForegroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return getCurrentMarkedForegroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "Inactive Marked Foreground")
		return getMarkedForegroundColor(false);
	return m_colorSettings.getColorValue(colorName);
}

TQColor KrColorCacheImpl::dimColor(TQColor color, bool /* isBackgroundColor */) const
{
	krConfig->setGroup("Colors");
	int dimFactor = m_colorSettings.getNumValue("Dim Factor", 100);
	TQColor targetColor = m_colorSettings.getColorValue("Dim Target Color");
	if (!targetColor.isValid())
		targetColor = TQColor(255, 255, 255);
	bool dimBackground = m_colorSettings.getBoolValue("Dim Inactive Colors", false);
	bool dim = dimFactor >= 0 && dimFactor < 100 && dimBackground;
	if (dim)
		color = KrColorCache::dimColor(color, dimFactor, targetColor);
	return color;
}






KrColorCache * KrColorCache::m_instance = 0;

KrColorCache::KrColorCache()
{
	m_impl = new KrColorCacheImpl;
}

KrColorCache::~KrColorCache()
{
	delete m_impl;
}

KrColorCache & KrColorCache::getColorCache()
{
	if (!m_instance)
	{
		m_instance = new KrColorCache;
		m_instance->refreshColors();
	}
	return * m_instance;
}

void KrColorCache::getColors(TQColorGroup  & result, const KrColorItemType & type) const
{
	// for the cache lookup: calculate a unique key from the type
	char hashKey[128];
	switch(type.m_fileType)
	{
		case KrColorItemType::Directory :
		 	strcpy(hashKey, "Directory");
			break;
		case KrColorItemType::Executable :
		 	strcpy(hashKey, "Executable");
			break;
		case KrColorItemType::InvalidSymlink :
		 	strcpy(hashKey, "InvalidSymlink");
			break;
		case KrColorItemType::Symlink :
		 	strcpy(hashKey, "Symlink");
			break;
		default:
		 	strcpy(hashKey, "File");
	}
	if (type.m_activePanel)
		strcat(hashKey, "-Active");
	if (type.m_alternateBackgroundColor)
		strcat(hashKey, "-Alternate");
	if (type.m_currentItem)
		strcat(hashKey, "-Current");
	if (type.m_selectedItem)
		strcat(hashKey, "-Selected");

	// lookup in cache
	if (!m_impl->m_cachedColors.contains(hashKey))
		// not found: calculate color group and store it in cache
		m_impl->m_cachedColors[hashKey] = m_impl->getColors(type);

	// get color group from cache
	const TQColorGroup & col = m_impl->m_cachedColors[hashKey];

	// copy colors in question to result color group
	result.setColor(TQColorGroup::Base, col.base());
	result.setColor(TQColorGroup::Background, col.base());
	result.setColor(TQColorGroup::Text, col.text());
	result.setColor(TQColorGroup::HighlightedText, col.highlightedText());
	result.setColor(TQColorGroup::Highlight, col.highlight());
}

TQColor KrColorCache::dimColor(const TQColor & color, int dim, const TQColor & targetColor)
{
   return TQColor((targetColor.red() * (100 - dim) + color.red() * dim) / 100, 
		(targetColor.green() * (100 - dim) + color.green() * dim) / 100, 
		(targetColor.blue() * (100 - dim) + color.blue() * dim) / 100);
}

void KrColorCache::refreshColors()
{
	m_impl->m_cachedColors.clear();
	m_impl->m_colorSettings = KrColorSettings();
	colorsRefreshed();
}

void KrColorCache::setColors(const KrColorSettings & colorSettings)
{
	m_impl->m_cachedColors.clear();
	m_impl->m_colorSettings = colorSettings;
	colorsRefreshed();
}

#include "krcolorcache.moc"
