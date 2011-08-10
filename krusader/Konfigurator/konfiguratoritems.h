/***************************************************************************
                      konfiguratoritems.h  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#ifndef __KONFIGURATOR_ITEMS_H__
#define __KONFIGURATOR_ITEMS_H__

#include <tqobject.h>
#include <tqstring.h>
#include <tqcheckbox.h>
#include <tqspinbox.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqptrlist.h>
#include <tqvaluelist.h>
#include <tqvaluevector.h>
#include <tqbuttongroup.h>
#include <tqlineedit.h>
#include <kurlrequester.h>
#include <tqhbox.h>
#include <kfontdialog.h>
#include <tqlabel.h>
#include <tqfont.h>
#include <tqtoolbutton.h>
#include <tqcombobox.h>

#define FIRST_PAGE        0

class KonfiguratorExtension : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorExtension(TQObject *obj, TQString cfgClass, TQString cfgName, bool rst = false, int pg=FIRST_PAGE );

  virtual void    loadInitialValue();
  virtual bool    apply();
  virtual void    setDefaults();
  virtual bool    isChanged();
  virtual void    setSubPage(int pg) {subpage = pg;}
  virtual int     subPage()          {return subpage;}

  inline TQObject *object()           {return objectPtr;}

  inline TQString  getCfgClass()      {return configClass;}
  inline TQString  getCfgName()       {return configName;}

public slots:
  void    setChanged()               {emit sigChanged( changed = true);}
  void    setChanged( bool chg )     {emit sigChanged( changed = chg);}

signals:
  void    applyManually(TQObject *,TQString, TQString);
  void    applyAuto(TQObject *,TQString, TQString);
  void    setDefaultsManually(TQObject *);
  void    setDefaultsAuto(TQObject *);
  void    setInitialValue(TQObject *);
  void    sigChanged( bool );

protected:
  TQObject *objectPtr;

  bool    applyConnected;
  bool    setDefaultsConnected;
  bool    changed;
  bool    restartNeeded;
  int     subpage;

  TQString configClass;
  TQString configName;

  virtual void connectNotify( const char *signal );
};


// KonfiguratorCheckBox class
///////////////////////////////

class KonfiguratorCheckBox : public TQCheckBox
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorCheckBox( TQString cls, TQString name, bool dflt, TQString text,
                        TQWidget *tqparent=0, const char *widgetName=0, bool rst=false, int pg=FIRST_PAGE );
  ~KonfiguratorCheckBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  bool  defaultValue;
  KonfiguratorExtension *ext;
};

// KonfiguratorSpinBox class
///////////////////////////////

class KonfiguratorSpinBox : public TQSpinBox
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorSpinBox( TQString cls, TQString name, int dflt, int min, int max,
                       TQWidget *tqparent=0, const char *widgetName=0, bool rst=false, int pg=FIRST_PAGE );
  ~KonfiguratorSpinBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  int  defaultValue;
  KonfiguratorExtension *ext;
};

// KonfiguratorCheckBoxGroup class
///////////////////////////////

class KonfiguratorCheckBoxGroup : public TQWidget
{
public:
  KonfiguratorCheckBoxGroup( TQWidget * tqparent = 0, const char * name = 0 ) :
    TQWidget( tqparent, name ) {};

  void                    add( KonfiguratorCheckBox * );
  KonfiguratorCheckBox *  find( int index );
  KonfiguratorCheckBox *  find( TQString name );

private:
  TQPtrList<KonfiguratorCheckBox>  checkBoxList;
};

// KonfiguratorRadioButtons class
///////////////////////////////

class KonfiguratorRadioButtons : public TQButtonGroup
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorRadioButtons( TQString cls, TQString name, TQString dflt, TQWidget *tqparent=0,
                            const char *widgetName=0, bool rst=false, int pg=FIRST_PAGE );
  ~KonfiguratorRadioButtons();

  inline KonfiguratorExtension *extension()   {return ext;}

  void  addRadioButton( TQRadioButton *radioWidget, TQString name, TQString value );

  void            selectButton( TQString value );

  TQRadioButton*   find( int index );
  TQRadioButton*   find( TQString name );

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  TQPtrList<TQRadioButton>  radioButtons;
  TQValueList<TQString>   radioValues;
  TQValueList<TQString>   radioNames;

  TQString         defaultValue;
  TQButtonGroup    *buttonGroup;

  KonfiguratorExtension *ext;
};

// KonfiguratorEditBox class
///////////////////////////////

class KonfiguratorEditBox : public TQLineEdit
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorEditBox( TQString cls, TQString name, TQString dflt, TQWidget *tqparent=0,
                       const char *widgetName=0, bool rst=false, int pg=FIRST_PAGE );
  ~KonfiguratorEditBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  TQString  defaultValue;
  KonfiguratorExtension *ext;
};


// KonfiguratorURLRequester class
///////////////////////////////

class KonfiguratorURLRequester : public KURLRequester
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorURLRequester( TQString cls, TQString name, TQString dflt, TQWidget *tqparent=0,
                       const char *widgetName=0, bool rst=false, int pg=FIRST_PAGE );
  ~KonfiguratorURLRequester();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  TQString  defaultValue;
  KonfiguratorExtension *ext;
};

// KonfiguratorFontChooser class
///////////////////////////////

class KonfiguratorFontChooser : public TQHBox
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorFontChooser( TQString cls, TQString name, TQFont *dflt, TQWidget *tqparent=0,
                            const char *widgetName=0, bool rst=false, int pg=FIRST_PAGE );
  ~KonfiguratorFontChooser();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void    loadInitialValue();
  void            slotApply(TQObject *,TQString, TQString);
  void            slotSetDefaults(TQObject *);
  void            slotBrowseFont();

protected:
  TQFont         * defaultValue;
  TQFont           font;
  KonfiguratorExtension *ext;

  TQLabel        * pLabel;
  TQToolButton   * pToolButton;

  void            setFont();
};

// KONFIGURATOR_NAME_VALUE_PAIR structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_PAIR
{
  TQString text;
  TQString value;
};

// KONFIGURATOR_NAME_VALUE_TIP structure
///////////////////////////////

struct KONFIGURATOR_NAME_VALUE_TIP
{
  TQString text;
  TQString value;
  TQString tooltip;
};

// KonfiguratorComboBox class
///////////////////////////////

class KonfiguratorComboBox : public TQComboBox
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorComboBox( TQString cls, TQString name, TQString dflt,
                        KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen,
                        TQWidget *tqparent=0, const char *widgetName=0,
                        bool rst=false,  bool editable=false, int pg=FIRST_PAGE );
  ~KonfiguratorComboBox();

  inline KonfiguratorExtension *extension()   {return ext;}

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  TQString                       defaultValue;
  KONFIGURATOR_NAME_VALUE_PAIR *list;
  int                           listLen;
  KonfiguratorExtension        *ext;

  void                          selectEntry( TQString entry );
};


// KonfiguratorColorChooser class
///////////////////////////////

typedef struct
{
  TQString name;
  TQColor  color;
  TQString value;
} ADDITIONAL_COLOR;

class KonfiguratorColorChooser : public TQComboBox
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorColorChooser( TQString cls, TQString name, TQColor dflt,
                            TQWidget *tqparent=0, const char *widgetName=0, bool rst=false,
                            ADDITIONAL_COLOR *addColPtr = 0, int addColNum = 0, int pg=FIRST_PAGE );
  ~KonfiguratorColorChooser();

  inline KonfiguratorExtension *extension()   {return ext;}

  void          setDefaultColor( TQColor dflt );
  void          setDefaultText( TQString text );
  TQColor        getColor();
  void          changeAdditionalColor( unsigned int num, TQColor color );
  TQString       getValue();
  bool          isValueRGB();
  void          setValue( TQString value );

public slots:
  virtual void  loadInitialValue();
  void          slotApply(TQObject *,TQString, TQString);
  void          slotSetDefaults(TQObject *);
  void          slotCurrentChanged( int number );

signals:
  void          colorChanged();

private:
  void          addColor( TQString text, TQColor color );
  TQPixmap       createPixmap( TQColor color );

protected:
  TQColor                          defaultValue;
  TQColor                          customValue;
  TQValueVector<TQColor>            palette;
  TQValueVector<ADDITIONAL_COLOR>  additionalColors;
  KonfiguratorExtension          *ext;
  bool                            disableColorChooser;
};

// KonfiguratorListBox class
///////////////////////////////

class KonfiguratorListBox : public TQListBox
{
  Q_OBJECT
  TQ_OBJECT

public:
  KonfiguratorListBox( TQString cls, TQString name, TQStringList dflt,
                        TQWidget *tqparent=0, const char *widgetName=0,
                        bool rst=false,  int pg=FIRST_PAGE );
  ~KonfiguratorListBox();

  inline KonfiguratorExtension *extension()   {return ext;}

  void addItem( const TQString & );
  void removeItem( const TQString & );

public slots:
  virtual void loadInitialValue();
  void slotApply(TQObject *,TQString, TQString);
  void slotSetDefaults(TQObject *);

protected:
  TQStringList                   list();
  void                          setList( TQStringList );

  TQStringList                   defaultValue;
  KonfiguratorExtension        *ext;
};

#endif /* __KONFIGURATOR_ITEMS_H__ */

