/***************************************************************************
                     konfiguratoritems.cpp  -  description
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konfiguratoritems.h"
#include "../krusader.h"
#include <klocale.h>
#include <tqpainter.h>
#include <tqpen.h>
#include <tqcolordialog.h>
#include <kiconloader.h>

KonfiguratorExtension::KonfiguratorExtension( TQObject *obj, TQString cfgClass, TQString cfgName, bool rst, int pg) :
      TQObject(), objectPtr( obj ), applyConnected( false ), setDefaultsConnected( false ),
      changed( false ), restartNeeded( rst ), subpage(pg), configClass( cfgClass ), configName( cfgName )
{
}

void KonfiguratorExtension::connectNotify( const char *signal )
{
  TQString signalString    = TQString( signal ).replace( " ", "" );
  TQString applyString     = TQString( TQT_SIGNAL( applyManually(TQObject *,TQString, TQString) ) ).replace( " ", "" );
  TQString defaultsString  = TQString( TQT_SIGNAL( setDefaultsManually(TQObject *) ) ).replace( " ", "" );

  if( signalString == applyString )
    applyConnected = true;
  else if ( signalString == defaultsString )
    setDefaultsConnected = true;

  TQObject::connectNotify( signal );
}

bool KonfiguratorExtension::apply()
{
  if( !changed )
    return false;

  if( applyConnected )
    emit applyManually( objectPtr, configClass, configName );
  else
    emit applyAuto( objectPtr, configClass, configName );

  setChanged( false );
  return restartNeeded;
}

void KonfiguratorExtension::setDefaults()
{
  if( setDefaultsConnected )
    emit setDefaultsManually( objectPtr );
  else
    emit setDefaultsAuto( objectPtr );
}

void KonfiguratorExtension::loadInitialValue()
{
  emit setInitialValue( objectPtr );
}

bool KonfiguratorExtension::isChanged()
{
  return changed;
}

// KonfiguratorCheckBox class
///////////////////////////////

KonfiguratorCheckBox::KonfiguratorCheckBox( TQString cls, TQString name, bool dflt, TQString text,
    TQWidget *tqparent, const char *widgetName, bool rst, int pg ) : TQCheckBox( text, tqparent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  connect( TQT_TQOBJECT(this), TQT_SIGNAL( stateChanged( int ) ), ext, TQT_SLOT( setChanged() ) );
  loadInitialValue();
}

KonfiguratorCheckBox::~KonfiguratorCheckBox()
{
  delete ext;
}

void KonfiguratorCheckBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setChecked( krConfig->readBoolEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorCheckBox::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, isChecked() );
}

void KonfiguratorCheckBox::slotSetDefaults(TQObject *)
{
  if( isChecked() != defaultValue )
    setChecked( defaultValue );
}

// KonfiguratorSpinBox class
///////////////////////////////

KonfiguratorSpinBox::KonfiguratorSpinBox( TQString cls, TQString name, int dflt, int min, int max,
    TQWidget *tqparent, const char *widgetName, bool rst, int pg ) : TQSpinBox( tqparent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  connect( TQT_TQOBJECT(this), TQT_SIGNAL( valueChanged(int) ), ext, TQT_SLOT( setChanged() ) );

  setMinValue( min );
  setMaxValue( max );

  loadInitialValue();
}

KonfiguratorSpinBox::~KonfiguratorSpinBox()
{
  delete ext;
}

void KonfiguratorSpinBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setValue( krConfig->readNumEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorSpinBox::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, value() );
}

void KonfiguratorSpinBox::slotSetDefaults(TQObject *)
{
  if( value() != defaultValue )
    setValue( defaultValue );
}

// KonfiguratorCheckBoxGroup class
///////////////////////////////

void KonfiguratorCheckBoxGroup::add( KonfiguratorCheckBox *checkBox )
{
  checkBoxList.append( checkBox );
}

KonfiguratorCheckBox * KonfiguratorCheckBoxGroup::find( int index )
{
  return checkBoxList.at( index );
}

KonfiguratorCheckBox * KonfiguratorCheckBoxGroup::find( TQString name )
{
  KonfiguratorCheckBox *checkBox = checkBoxList.first();

  while( checkBox )
  {
    if( checkBox->extension()->getCfgName() == name )
      return checkBox;
    checkBox = checkBoxList.next();
  }

  return 0;
}


// KonfiguratorRadioButtons class
///////////////////////////////

KonfiguratorRadioButtons::KonfiguratorRadioButtons( TQString cls, TQString name,
    TQString dflt, TQWidget *tqparent, const char *widgetName, bool rst, int pg ) :
    TQButtonGroup( tqparent, widgetName ), defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );
}

KonfiguratorRadioButtons::~KonfiguratorRadioButtons()
{
  delete ext;
}

void KonfiguratorRadioButtons::addRadioButton( TQRadioButton *radioWidget, TQString name, TQString value )
{
  radioButtons.append( radioWidget );
  radioNames.push_back( name );
  radioValues.push_back( value );

  connect( radioWidget, TQT_SIGNAL( stateChanged(int) ), ext, TQT_SLOT( setChanged() ) );
}

TQRadioButton * KonfiguratorRadioButtons::find( int index )
{
  return radioButtons.at( index );
}

TQRadioButton * KonfiguratorRadioButtons::find( TQString name )
{
  int index = radioNames.findIndex( name );
  if( index == -1 )
    return 0;

  return radioButtons.at( index );
}

void KonfiguratorRadioButtons::selectButton( TQString value )
{
  int cnt = 0;
  TQRadioButton *btn  = radioButtons.first();

  while( btn )
  {
    if( value == radioValues[ cnt ] )
    {
      btn->setChecked( true );
      return;
    }

    btn = radioButtons.next();
    cnt++;
  }

  if( radioButtons.first() )
    radioButtons.first()->setChecked( true );
}

void KonfiguratorRadioButtons::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  TQString initValue = krConfig->readEntry( ext->getCfgName(), defaultValue );

  selectButton( initValue );
  ext->setChanged( false );
}

void KonfiguratorRadioButtons::slotApply(TQObject *,TQString cls, TQString name)
{
  TQRadioButton *btn  = radioButtons.first();
  int cnt = 0;

  while( btn )
  {
    if( btn->isChecked() )
    {
      krConfig->setGroup( cls );
      krConfig->writeEntry( name, radioValues[ cnt ] );
      break;
    }

    btn = radioButtons.next();
    cnt++;
  }
}

void KonfiguratorRadioButtons::slotSetDefaults(TQObject *)
{
  selectButton( defaultValue );
}

// KonfiguratorEditBox class
///////////////////////////////

KonfiguratorEditBox::KonfiguratorEditBox( TQString cls, TQString name, TQString dflt,
    TQWidget *tqparent, const char *widgetName, bool rst, int pg ) : TQLineEdit( tqparent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  connect( TQT_TQOBJECT(this), TQT_SIGNAL( textChanged(const TQString &) ), ext, TQT_SLOT( setChanged() ) );

  loadInitialValue();
}

KonfiguratorEditBox::~KonfiguratorEditBox()
{
  delete ext;
}

void KonfiguratorEditBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setText( krConfig->readEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorEditBox::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, text() );
}

void KonfiguratorEditBox::slotSetDefaults(TQObject *)
{
  if( text() != defaultValue )
    setText( defaultValue );
}


// KonfiguratorURLRequester class
///////////////////////////////

KonfiguratorURLRequester::KonfiguratorURLRequester( TQString cls, TQString name, TQString dflt,
    TQWidget *tqparent, const char *widgetName, bool rst, int pg ) : KURLRequester( tqparent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  connect( TQT_TQOBJECT(this), TQT_SIGNAL( textChanged(const TQString &) ), ext, TQT_SLOT( setChanged() ) );

  button()->setIconSet( SmallIcon( "fileopen" ) );
  loadInitialValue();
}

KonfiguratorURLRequester::~KonfiguratorURLRequester()
{
  delete ext;
}

void KonfiguratorURLRequester::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setURL( krConfig->readEntry( ext->getCfgName(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorURLRequester::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, url() );
}

void KonfiguratorURLRequester::slotSetDefaults(TQObject *)
{
  if( url() != defaultValue )
    setURL( defaultValue );
}

// KonfiguratorFontChooser class
///////////////////////////////

KonfiguratorFontChooser::KonfiguratorFontChooser( TQString cls, TQString name, TQFont *dflt,
  TQWidget *tqparent, const char *widgetName, bool rst, int pg ) : TQHBox ( tqparent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  pLabel = new TQLabel( this );
  pLabel->setMinimumWidth( 150 );
  pToolButton = new TQToolButton( this );

  connect( pToolButton, TQT_SIGNAL( clicked() ), TQT_TQOBJECT(this), TQT_SLOT( slotBrowseFont() ) );

  pToolButton->setIconSet( SmallIcon( "fileopen" ) );

  loadInitialValue();
}

KonfiguratorFontChooser::~KonfiguratorFontChooser()
{
  delete ext;
}

void KonfiguratorFontChooser::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  font = krConfig->readFontEntry( ext->getCfgName(), defaultValue );
  ext->setChanged( false );
  setFont();
}

void KonfiguratorFontChooser::setFont()
{
  pLabel->setFont( font );
  pLabel->setText( font.family()+TQString(", %1").tqarg(font.pointSize()) );
}

void KonfiguratorFontChooser::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, font );
}

void KonfiguratorFontChooser::slotSetDefaults(TQObject *)
{
  font = *defaultValue;
  ext->setChanged();
  setFont();
}

void KonfiguratorFontChooser::slotBrowseFont()
{
  int ok=KFontDialog::getFont( font );
  if (ok!=1) return;  // cancelled by the user
  ext->setChanged();
  setFont();
}

// KonfiguratorComboBox class
///////////////////////////////

KonfiguratorComboBox::KonfiguratorComboBox( TQString cls, TQString name, TQString dflt,
    KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen, TQWidget *tqparent,
    const char *widgetName, bool rst, bool editable, int pg ) : TQComboBox ( tqparent, widgetName ),
    defaultValue( dflt ), listLen( listInLen )
{
  list = new KONFIGURATOR_NAME_VALUE_PAIR[ listInLen ];

  for( int i=0; i != listLen; i++ )
  {
    list[i] = listIn[i];
    insertItem( list[i].text );
  }

  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

//  connect( TQT_TQOBJECT(this), TQT_SIGNAL( highlighted(int) ), ext, TQT_SLOT( setChanged() ) ); /* Removed because of startup combo failure */
  connect( TQT_TQOBJECT(this), TQT_SIGNAL( activated(int) ), ext, TQT_SLOT( setChanged() ) );
  connect( TQT_TQOBJECT(this), TQT_SIGNAL( textChanged ( const TQString & ) ), ext, TQT_SLOT( setChanged() ) );

  setEditable( editable );
  loadInitialValue();
}

KonfiguratorComboBox::~KonfiguratorComboBox()
{
  delete []list;
  delete ext;
}

void KonfiguratorComboBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  TQString select = krConfig->readEntry( ext->getCfgName(), defaultValue );
  selectEntry( select );
  ext->setChanged( false );
}

void KonfiguratorComboBox::slotApply(TQObject *,TQString cls, TQString name)
{
  TQString text = editable() ? lineEdit()->text() : currentText();
  TQString value = text;
  
  for( int i=0; i != listLen; i++ )
    if( list[i].text == text ) {
      value = list[i].value;
      break;
    }
  
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, value );
}

void KonfiguratorComboBox::selectEntry( TQString entry )
{
  for( int i=0; i != listLen; i++ )
    if( list[i].value == entry )
    {
      setCurrentItem( i );
      return;
    }

  if( editable() )
    lineEdit()->setText( entry );
  else
    setCurrentItem( 0 );
}

void KonfiguratorComboBox::slotSetDefaults(TQObject *)
{
  selectEntry( defaultValue );
}


// KonfiguratorColorChooser class
///////////////////////////////

KonfiguratorColorChooser::KonfiguratorColorChooser( TQString cls, TQString name, TQColor dflt,
    TQWidget *tqparent, const char *widgetName, bool rst, ADDITIONAL_COLOR *addColPtr,
    int addColNum, int pg ) : TQComboBox ( tqparent, widgetName ),
    defaultValue( dflt ), disableColorChooser( true )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );

  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  addColor( i18n("Custom color" ),  TQColor( 255, 255, 255 ) );
  addColor( i18n("Default" ),       defaultValue );

  for( int i=0; i != addColNum; i++ )
  {
    additionalColors.push_back( addColPtr[i] );
    addColor( addColPtr[i].name, addColPtr[i].color );
  }

  addColor( i18n("Red" ),           TQt::red );
  addColor( i18n("Green" ),         TQt::green );
  addColor( i18n("Blue" ),          TQt::blue );
  addColor( i18n("Cyan" ),          TQt::cyan );
  addColor( i18n("Magenta" ),       TQt::magenta );
  addColor( i18n("Yellow" ),        TQt::yellow );
  addColor( i18n("Dark Red" ),      TQt::darkRed );
  addColor( i18n("Dark Green" ),    TQt::darkGreen );
  addColor( i18n("Dark Blue" ),     TQt::darkBlue );
  addColor( i18n("Dark Cyan" ),     TQt::darkCyan );
  addColor( i18n("Dark Magenta" ),  TQt::darkMagenta );
  addColor( i18n("Dark Yellow" ),   TQt::darkYellow );
  addColor( i18n("White" ),         TQt::white );
  addColor( i18n("Light Gray" ),    TQt::lightGray );
  addColor( i18n("Gray" ),          TQt::gray );
  addColor( i18n("Dark Gray" ),     TQt::darkGray );
  addColor( i18n("Black" ),         TQt::black );

  connect( TQT_TQOBJECT(this), TQT_SIGNAL( activated(int) ),   TQT_TQOBJECT(this), TQT_SLOT( slotCurrentChanged( int ) ) );

  loadInitialValue();
}

KonfiguratorColorChooser::~KonfiguratorColorChooser()
{
  delete ext;
}

TQPixmap KonfiguratorColorChooser::createPixmap( TQColor color )
{
  TQPainter painter;
  TQPen pen;
  int size = TQFontMetrics(font()).height()*3/4;
  TQRect rect( 0, 0, size, size );
  TQPixmap pixmap( rect.width(), rect.height() );

  pen.setColor( TQt::black );

  painter.begin( &pixmap );
  TQBrush brush( color );
  painter.fillRect( rect, brush );
  painter.setPen( pen );
  painter.drawRect( rect );
  painter.end();

  pixmap.detach();
  return pixmap;
}

void KonfiguratorColorChooser::addColor( TQString text, TQColor color )
{
  insertItem( createPixmap(color), text );
  palette.push_back( color );
}

void KonfiguratorColorChooser::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  TQString selected = krConfig->readEntry( ext->getCfgName(), "" );
  setValue( selected );
  ext->setChanged( false );
}

void KonfiguratorColorChooser::setDefaultColor( TQColor dflt )
{
  defaultValue = dflt;
  palette[1] = defaultValue;
  changeItem( createPixmap( defaultValue ), text( 1 ), 1 );

  if( currentItem() == 1 )
    emit colorChanged();
}

void KonfiguratorColorChooser::changeAdditionalColor( unsigned int num, TQColor color )
{
  if( num < additionalColors.size() )
  {
    palette[2+num] = color;
    additionalColors[num].color = color;
    changeItem( createPixmap( color ), text( 2+num ), 2+num );

    if( (unsigned int)currentItem() == 2+num )
      emit colorChanged();
  }
}

void KonfiguratorColorChooser::setDefaultText( TQString text )
{
  changeItem( createPixmap( defaultValue ), text, 1 );
}

void KonfiguratorColorChooser::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, getValue() );
}

void KonfiguratorColorChooser::setValue( TQString value )
{
  disableColorChooser = true;

  if( value.isEmpty() )
  {
    setCurrentItem( 1 );
    customValue = defaultValue;
  }
  else
  {
    bool found = false;

    for( unsigned j=0; j != additionalColors.size(); j++ )
      if( additionalColors[j].value == value )
      {
        setCurrentItem( 2 + j );
        found = true;
        break;
      }

    if( ! found )
    {
      krConfig->setGroup( ext->getCfgClass() );
      krConfig->writeEntry( "TmpColor", value );
      TQColor color = krConfig->readColorEntry( "TmpColor", &defaultValue );
      customValue = color;
      krConfig->deleteEntry( "TmpColor" );

      setCurrentItem( 0 );
      for( unsigned i= 2+additionalColors.size(); i != palette.size(); i++ )
        if( palette[i] == color )
        {
          setCurrentItem( i );
          break;
        }
    }
  }

  palette[0] = customValue;
  changeItem( createPixmap( customValue ), text( 0 ), 0 );

  ext->setChanged();
  emit colorChanged();
  disableColorChooser = false;
}

TQString KonfiguratorColorChooser::getValue()
{
  TQColor color = palette[ currentItem() ];
  if( currentItem() == 1 )    /* it's the default value? */
    return "";
  else if( currentItem() >= 2 && (unsigned)currentItem() < 2 + additionalColors.size() )
    return additionalColors[ currentItem() - 2 ].value;
  else
    return TQString( "%1,%2,%3" ).tqarg( color.red() ).tqarg( color.green() ).tqarg( color.blue() );
}

bool KonfiguratorColorChooser::isValueRGB()
{
  return !( currentItem() >= 1 && (unsigned)currentItem() < 2 + additionalColors.size() );
}

void KonfiguratorColorChooser::slotSetDefaults(TQObject *)
{
  ext->setChanged();
  setCurrentItem( 1 );
  emit colorChanged();
}

void KonfiguratorColorChooser::slotCurrentChanged( int number )
{
  ext->setChanged();
  if( number == 0 && !disableColorChooser )
  {
    TQColor color = TQColorDialog::getColor ( customValue, this, "ColorDialog" );
    if( color.isValid() )
    {
      disableColorChooser = true;
      customValue = color;
      palette[0] = customValue;
      changeItem( createPixmap( customValue ), text( 0 ), 0 );
      disableColorChooser = false;
    }
  }

  emit colorChanged();
}

TQColor KonfiguratorColorChooser::getColor()
{
  return palette[ currentItem() ];
}

// KonfiguratorListBox class
///////////////////////////////

KonfiguratorListBox::KonfiguratorListBox( TQString cls, TQString name, TQStringList dflt,
    TQWidget *tqparent, const char *widgetName, bool rst, int pg ) : TQListBox( tqparent, widgetName ),
    defaultValue( dflt )
{
  ext = new KonfiguratorExtension( TQT_TQOBJECT(this), cls, name, rst, pg );
  connect( ext, TQT_SIGNAL( applyAuto(TQObject *,TQString, TQString) ), TQT_TQOBJECT(this), TQT_SLOT( slotApply(TQObject *,TQString, TQString) ) );
  connect( ext, TQT_SIGNAL( setDefaultsAuto(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( slotSetDefaults(TQObject *) ) );
  connect( ext, TQT_SIGNAL( setInitialValue(TQObject *) ), TQT_TQOBJECT(this), TQT_SLOT( loadInitialValue() ) );

  loadInitialValue();
}

KonfiguratorListBox::~KonfiguratorListBox()
{
  delete ext;
}

void KonfiguratorListBox::loadInitialValue()
{
  krConfig->setGroup( ext->getCfgClass() );
  setList( krConfig->readListEntry( ext->getCfgName().ascii(), defaultValue ) );
  ext->setChanged( false );
}

void KonfiguratorListBox::slotApply(TQObject *,TQString cls, TQString name)
{
  krConfig->setGroup( cls );
  krConfig->writeEntry( name, list() );
}

void KonfiguratorListBox::slotSetDefaults(TQObject *)
{
  if( list() != defaultValue )
  {
    ext->setChanged();
    setList( defaultValue );
  }
}

void KonfiguratorListBox::setList( TQStringList list )
{
  clear();
  insertStringList( list );
}

TQStringList KonfiguratorListBox::list()
{
  TQStringList lst;
  
  for( unsigned i=0; i != count(); i++ )
    lst += text( i );

  return lst;
}

void KonfiguratorListBox::addItem( const TQString & item )
{
  if( !list().contains( item ) )
  {
    insertItem( item );
    ext->setChanged();
  }
}

void KonfiguratorListBox::removeItem( const TQString & item )
{
  TQListBoxItem * listItem = findItem( item );
  if( listItem != 0 )
  {
    takeItem( listItem );
    ext->setChanged();
  }
}

#include "konfiguratoritems.moc"
