#ifndef KRCOLORCACHE_H
#define KRCOLORCACHE_H

#include <tqobject.h>
#include <tqdict.h>

/*
Design goals: Color calculation is done on one place only. Configuration through krConfig OR through local settings.
Calculation must be fast through cacheing.

This implementation exposes 3 classes:

KrColorSettings: holds the color settings from krConfig, which can be changed locally
KrColorItemType: specifies the colors to be calculated
KrColorCache: perfomes the color calculation and caches the result. Uses KrColorSettings for the calculation
*/


/*
Copies all used color settings from krConfig into a local cache on creation. It contains 3 types of properties:
color, numeric (int) and boolean. Color properties can have string or color values. Property values can be
changed. These changes does not go into krConfig!

is*Valid checks, if a protery name is valid
get*Names returns a list of all allowed property names
set*Value overwrites a property with a new value
get*Value retunrs the current value

For colors teh value can be returned as text or as color. If a text representation is not a valid color, 
setColorValue(TQColor()) should be called.
*/
class KrColorSettings
{
	class KrColorSettingsImpl * m_impl;
public:
	KrColorSettings();
	KrColorSettings(const KrColorSettings &);
	~KrColorSettings();
	const KrColorSettings & operator= (const KrColorSettings &);

	static bool isColorNameValid(const TQString & settingName);
	static TQValueList<TQString> getColorNames();
	bool setColorValue(const TQString & settingName, const TQColor & color);
	TQColor getColorValue(const TQString & settingName) const;
	bool setColorTextValue(const TQString & settingName, const TQString & colorText);
	TQString getColorTextValue(const TQString & settingName) const;

	static bool isNumNameValid(const TQString & settingName);
	static TQValueList<TQString> getNumNames();
	bool setNumValue(const TQString & settingName, int value);
	int getNumValue(const TQString & settingName, int defaultValue = 0) const;

	static bool isBoolNameValid(const TQString & settingName);
	static TQValueList<TQString> getBoolNames();
	bool setBoolValue(const TQString & settingName, bool value);
	int getBoolValue(const TQString & settingName, bool defaultValue = false) const;
};

/*
A colletcion of properties which describe the color group to be calculated
*/
class KrColorItemType
{
public:
	enum FileType {File, InvalidSymlink, Symlink, Directory, Executable};
	FileType m_fileType;
	bool m_alternateBackgroundColor, m_activePanel, m_currentItem, m_selectedItem;
	KrColorItemType();
	KrColorItemType(FileType type, bool alternateBackgroundColor, bool activePanel, bool currentItem, bool selectedItem);
	KrColorItemType(const KrColorItemType &);
	const KrColorItemType & operator= (const KrColorItemType &);
};

/*
The color calculation. It bases on an internal KrColorSettings instance. Via setColors it can be changed.
getColors does the color calculation. It sets the colors Base, Background, Text, HighlightedText and Highlight.
All calculated values are cached. The cache is deleted on refreshColors and setColors, which also trigger
colorsRefreshed. getColorCache returns a statis color cached for painting the panels. On the color cache
setColors should NEVER be called!
*/
class KrColorCache : public TQObject
{
	TQ_OBJECT
  
	static KrColorCache * m_instance;
	class KrColorCacheImpl * m_impl;
	KrColorCache(const KrColorCache &);
	const KrColorCache & operator= (const KrColorCache &);
public:
	KrColorCache();
	~KrColorCache();
	static KrColorCache & getColorCache();
	void getColors(TQColorGroup & result, const KrColorItemType & type) const;
	static TQColor dimColor(const TQColor & color, int dim, const TQColor & targetColor);
public slots:
	void refreshColors();
	void setColors(const KrColorSettings &);
signals:
	void colorsRefreshed();
};

#endif
