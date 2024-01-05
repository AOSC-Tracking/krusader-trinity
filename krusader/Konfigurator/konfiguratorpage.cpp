/* **************************************************************************
                      konfiguratorpage.cpp  -  description
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

#include "konfiguratorpage.h"
#include <tqlayout.h>
#include "../krusader.h"
#include <tqwhatsthis.h>

KonfiguratorPage::KonfiguratorPage( bool firstTime, TQWidget* parent,  const char* name ) :
  TQFrame( parent, name ), firstCall( firstTime )
{
}

bool KonfiguratorPage::apply()
{
  bool restartNeeded = false;

  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    restartNeeded = item->apply() || restartNeeded;
    item = itemList.next();
  }

  krConfig->sync();  
  return restartNeeded;
}

void KonfiguratorPage::setDefaults()
{
  KonfiguratorExtension *item = itemList.first();
  int activePage = activeSubPage();

  while( item )
  {
    if( item->subPage() == activePage )
      item->setDefaults();
    item = itemList.next();
  }
}

void KonfiguratorPage::loadInitialValues()
{
  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    item->loadInitialValue();
    item = itemList.next();
  }
}

bool KonfiguratorPage::isChanged()
{
  KonfiguratorExtension *currentItem = itemList.current();  /* save the current pointer */
  bool isChanged = false;

  KonfiguratorExtension *item = itemList.first();

  while( item )
  {
    isChanged = isChanged || item->isChanged();
    item = itemList.next();
  }

  itemList.find( currentItem );  /* restore the current pointer */
  return isChanged;
}

KonfiguratorCheckBox* KonfiguratorPage::createCheckBox( TQString cls, TQString name,
    bool dflt, TQString text, TQWidget *parent, bool rst, TQString toolTip, int pg )
{
  KonfiguratorCheckBox *checkBox = new KonfiguratorCheckBox( cls, name, dflt, text,
                                 parent, TQString(cls + "/" + name).ascii(), rst, pg );
  if( !toolTip.isEmpty() )
    TQWhatsThis::add( checkBox, toolTip );
  
  registerObject( checkBox->extension() );
  return checkBox;
}

KonfiguratorSpinBox* KonfiguratorPage::createSpinBox(  TQString cls, TQString name,
    int dflt, int min, int max, TQWidget *parent, bool rst, int pg )
{
  KonfiguratorSpinBox *spinBox = new KonfiguratorSpinBox( cls, name, dflt, min, max,
                                 parent, TQString(cls + "/" + name).ascii(), rst, pg );

  registerObject( spinBox->extension() );
  return spinBox;
}

KonfiguratorEditBox* KonfiguratorPage::createEditBox(  TQString cls, TQString name,
    TQString dflt, TQWidget *parent, bool rst, int pg )
{
  KonfiguratorEditBox *editBox = new KonfiguratorEditBox( cls, name, dflt, parent,
                                        TQString(cls + "/" + name).ascii(), rst, pg );

  registerObject( editBox->extension() );
  return editBox;
}

KonfiguratorListBox* KonfiguratorPage::createListBox(  TQString cls, TQString name,
    TQStringList dflt, TQWidget *parent, bool rst, int pg )
{
  KonfiguratorListBox *listBox = new KonfiguratorListBox( cls, name, dflt, parent,
                                        TQString(cls + "/" + name).ascii(), rst, pg );

  registerObject( listBox->extension() );
  return listBox;
}

KonfiguratorURLRequester* KonfiguratorPage::createURLRequester(  TQString cls, TQString name,
    TQString dflt, TQWidget *parent, bool rst, int pg )
{
  KonfiguratorURLRequester *urlRequester = new KonfiguratorURLRequester( cls, name, dflt,
                                        parent, TQString(cls + "/" + name).ascii(), rst, pg );

  registerObject( urlRequester->extension() );
  return urlRequester;
}

TQGroupBox* KonfiguratorPage::createFrame( TQString text, TQWidget *parent,
                                          const char *widgetName )
{
  TQGroupBox *groupBox = new TQGroupBox( parent, widgetName );
  groupBox->setFrameShape( TQGroupBox::Box );
  groupBox->setFrameShadow( TQGroupBox::Sunken );
  if( !text.isNull() )
    groupBox->setTitle( text );
  groupBox->setColumnLayout(0, TQt::Vertical );
  groupBox->layout()->setSpacing( 0 );
  groupBox->layout()->setMargin( 0 );
  return groupBox;
}                                          

TQGridLayout* KonfiguratorPage::createGridLayout( TQLayout *parent )
{
  TQGridLayout *gridLayout = new TQGridLayout( parent );
  gridLayout->setAlignment( TQt::AlignTop );
  gridLayout->setSpacing( 6 );
  gridLayout->setMargin( 11 );
  return gridLayout;
}

TQLabel* KonfiguratorPage::addLabel( TQGridLayout *layout, int x, int y, TQString label,
                                    TQWidget *parent, const char *widgetName )
{
  TQLabel *lbl = new TQLabel( label, parent, widgetName );
  layout->addWidget( lbl, x, y );
  return lbl;
}

TQWidget* KonfiguratorPage::createSpacer( TQWidget *parent, const char *widgetName )
{
  TQWidget *widget = new TQWidget( parent, widgetName );
  TQHBoxLayout *hboxlayout = new TQHBoxLayout( widget );
  TQSpacerItem* spacer = new TQSpacerItem( 40, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum );
  hboxlayout->addItem( spacer );
  return widget;
}

KonfiguratorCheckBoxGroup* KonfiguratorPage::createCheckBoxGroup( int sizex, int sizey,
    KONFIGURATOR_CHECKBOX_PARAM *params, int paramNum, TQWidget *parent,
    const char *widgetName, int pg )
{
  KonfiguratorCheckBoxGroup *groupWidget = new KonfiguratorCheckBoxGroup( parent, widgetName );
  TQGridLayout *layout = new TQGridLayout( groupWidget );
  layout->setSpacing( 6 );
  layout->setMargin( 0 );
  
  int x = 0, y = 0;
  
  for( int i=0; i != paramNum; i++ )
  {
    KonfiguratorCheckBox *checkBox = createCheckBox( params[i].configClass,
      params[i].configName, params[i].defaultValue, params[i].text, groupWidget,
      params[i].restart, params[i].toolTip, pg );

    groupWidget->add( checkBox );
    layout->addWidget( checkBox, y, x );

    if( sizex )
    {
      if( ++x == sizex )
        x = 0, y++;
    }
    else
    {
      if( ++y == sizey )
        y = 0, x++;
    }
  }
  
  return groupWidget;
}

KonfiguratorRadioButtons* KonfiguratorPage::createRadioButtonGroup( TQString cls,
    TQString name, TQString dflt, int sizex, int sizey, KONFIGURATOR_NAME_VALUE_TIP *params,
    int paramNum, TQWidget *parent, const char *widgetName, bool rst, int pg )
{
  KonfiguratorRadioButtons *radioWidget = new KonfiguratorRadioButtons( cls, name, dflt, parent, widgetName, rst, pg );
  radioWidget->setFrameShape( TQButtonGroup::NoFrame );
  radioWidget->setFrameShadow( TQButtonGroup::Sunken );
  radioWidget->setTitle( "" );
  radioWidget->setExclusive( true );
  radioWidget->setRadioButtonExclusive( true );
  radioWidget->setColumnLayout(0, TQt::Vertical );

  TQGridLayout *layout = new TQGridLayout( radioWidget->layout() );
  layout->setAlignment( TQt::AlignTop );
  layout->setSpacing( 6 );
  layout->setMargin( 0 );

  int x = 0, y = 0;

  for( int i=0; i != paramNum; i++ )
  {
    TQRadioButton *radBtn = new TQRadioButton( params[i].text, radioWidget,
                        TQString( cls + "/" + name + "/" + params[i].value ).ascii() );

    if( !params[i].tooltip.isEmpty() )
      TQWhatsThis::add( radBtn, params[i].tooltip );

    layout->addWidget( radBtn, y, x );

    radioWidget->addRadioButton( radBtn, params[i].text, params[i].value );

    if( sizex )
    {
      if( ++x == sizex )
        x = 0, y++;
    }
    else
    {
      if( ++y == sizey )
        y = 0, x++;
    }
  }

  radioWidget->loadInitialValue();
  registerObject( radioWidget->extension() );  
  return radioWidget;
}

KonfiguratorFontChooser *KonfiguratorPage::createFontChooser( TQString cls, TQString name,
  TQFont *dflt, TQWidget *parent, bool rst, int pg )
{
  KonfiguratorFontChooser *fontChooser = new KonfiguratorFontChooser( cls, name, dflt, parent,
                                        TQString(cls + "/" + name).ascii(), rst, pg );

  registerObject( fontChooser->extension() );
  return fontChooser;
}

KonfiguratorComboBox *KonfiguratorPage::createComboBox(  TQString cls, TQString name, TQString dflt,
    KONFIGURATOR_NAME_VALUE_PAIR *params, int paramNum, TQWidget *parent, bool rst, bool editable, int pg )
{
  KonfiguratorComboBox *comboBox = new KonfiguratorComboBox( cls, name, dflt, params,
                                        paramNum, parent, TQString(cls + "/" + name).ascii(),
                                        rst, editable, pg );

  registerObject( comboBox->extension() );
  return comboBox;
}

TQFrame* KonfiguratorPage::createLine( TQWidget *parent, const char *widgetName, bool vertical )
{
  TQFrame *line = new TQFrame( parent, widgetName );
  line->setFrameStyle( ( vertical ? TQFrame::VLine : TQFrame::HLine ) | TQFrame::Sunken );
  return line;
}

void KonfiguratorPage::registerObject( KonfiguratorExtension *item )
{
  KonfiguratorExtension *currentItem = itemList.current();
  
  itemList.append( item );
  connect( item, TQ_SIGNAL( sigChanged( bool ) ), this, TQ_SIGNAL ( sigChanged( ) ) );

  itemList.find( currentItem );
}

void KonfiguratorPage::removeObject( KonfiguratorExtension *item )
{
  if( item == itemList.current() )
  {
    itemList.remove();
    if( itemList.current() != itemList.getFirst() )
      itemList.prev();
  }
  else
    itemList.removeRef( item );
}

KonfiguratorColorChooser *KonfiguratorPage::createColorChooser( TQString cls, TQString name, TQColor dflt,
                                                                TQWidget *parent, bool rst,
                                                                ADDITIONAL_COLOR *addColPtr, int addColNum, int pg )
{
  KonfiguratorColorChooser *colorChooser = new KonfiguratorColorChooser( cls, name, dflt,  parent,
                                        TQString(cls + "/" + name).ascii(), rst, addColPtr, addColNum, pg );

  registerObject( colorChooser->extension() );
  return colorChooser;
}

#include "konfiguratorpage.moc"
