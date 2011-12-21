/***************************************************************************
                       diskusagegui.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#include "diskusagegui.h"
#include "../kicons.h"
#include "../krusader.h"
#include "../VFS/vfs.h"
#include "../Dialogs/krdialogs.h"

#include <tqtimer.h>
#include <tqhbox.h>
#include <klocale.h>
#include <tqtooltip.h>

DiskUsageGUI::DiskUsageGUI( KURL openDir, TQWidget* parent, const char *name ) 
  : TQDialog( parent, name, false, 0 ), exitAtFailure( true )
{  
  setCaption( i18n("Krusader::Disk Usage") );
  
  baseDirectory = openDir;
  if( !newSearch() )
    return;
  
  TQGridLayout *duGrid = new TQGridLayout( this );
  duGrid->setSpacing( 6 );
  duGrid->setMargin( 11 );
  
  TQHBox *duTools = new TQHBox( this, "duTools" );
  duTools->setSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed );
    
  btnNewSearch = new TQToolButton( duTools, "btnNewSearch" );
  btnNewSearch->setIconSet( TQIconSet(krLoader->loadIcon("fileopen",KIcon::Desktop)) );
  TQToolTip::add( btnNewSearch, i18n( "Start new disk usage search" ) );
  
  btnRefresh = new TQToolButton( duTools, "btnRefresh" );
  btnRefresh->setIconSet( TQIconSet(krLoader->loadIcon("reload",KIcon::Desktop)) );
  TQToolTip::add( btnRefresh, i18n( "Refresh" ) );

  btnDirUp = new TQToolButton( duTools, "btnDirUp" );
  btnDirUp->setIconSet( TQIconSet(krLoader->loadIcon("up",KIcon::Desktop)) );
  TQToolTip::add( btnDirUp, i18n( "Parent directory" ) );
  
  TQWidget * separatorWidget = new TQWidget( duTools, "separatorWidget" );
  separatorWidget->setMinimumWidth( 10 );
  
  btnLines = new TQToolButton( duTools, "btnLines" );
  btnLines->setIconSet( TQIconSet(krLoader->loadIcon("leftjust",KIcon::Desktop)) );
  btnLines->setToggleButton( true );
  TQToolTip::add( btnLines, i18n( "Line view" ) );

  btnDetailed = new TQToolButton( duTools, "btnDetailed" );
  btnDetailed->setIconSet( TQIconSet(krLoader->loadIcon("view_detailed",KIcon::Desktop)) );
  btnDetailed->setToggleButton( true );
  TQToolTip::add( btnDetailed, i18n( "Detailed view" ) );

  btnFilelight = new TQToolButton( duTools, "btnFilelight" );
  btnFilelight->setIconSet( TQIconSet(krLoader->loadIcon("kr_diskusage",KIcon::Desktop)) );
  btnFilelight->setToggleButton( true );
  TQToolTip::add( btnFilelight, i18n( "Filelight view" ) );
    
  TQWidget *spacerWidget = new TQWidget( duTools, "spacerWidget" );
  TQHBoxLayout *hboxlayout = new TQHBoxLayout( spacerWidget );
  TQSpacerItem* spacer = new TQSpacerItem( 0, 0, TQSizePolicy::Expanding, TQSizePolicy::Fixed );
  hboxlayout->addItem( spacer );
  
  duGrid->addWidget( duTools, 0, 0 );
  
  diskUsage = new DiskUsage( "DiskUsage", this );
  duGrid->addWidget( diskUsage, 1, 0 );
  
  status = new KSqueezedTextLabel( this );
  status->setFrameShape( TQLabel::StyledPanel );
  status->setFrameShadow( TQLabel::Sunken );  
  duGrid->addWidget( status, 2, 0 );
  
  connect( diskUsage, TQT_SIGNAL( status( TQString ) ), this, TQT_SLOT( setStatus( TQString ) ) );
  connect( diskUsage, TQT_SIGNAL( viewChanged( int ) ), this, TQT_SLOT( slotViewChanged( int ) ) );
  connect( diskUsage, TQT_SIGNAL( newSearch() ), this,  TQT_SLOT( newSearch() ) );
  connect( diskUsage, TQT_SIGNAL( loadFinished( bool ) ), this,  TQT_SLOT( slotLoadFinished( bool ) ) );
  connect( btnNewSearch, TQT_SIGNAL( clicked() ), this, TQT_SLOT( newSearch() ) );
  connect( btnRefresh, TQT_SIGNAL( clicked() ), this, TQT_SLOT( loadUsageInfo() ) );
  connect( btnDirUp, TQT_SIGNAL( clicked() ), diskUsage, TQT_SLOT( dirUp() ) );
  connect( btnLines, TQT_SIGNAL( clicked() ), this, TQT_SLOT( selectLinesView() ) );
  connect( btnDetailed, TQT_SIGNAL( clicked() ), this, TQT_SLOT( selectListView() ) );
  connect( btnFilelight, TQT_SIGNAL( clicked() ), this, TQT_SLOT( selectFilelightView() ) );  
  
  krConfig->setGroup( "DiskUsage" ); 
  
  int view = krConfig->readNumEntry( "View",  VIEW_LINES );
  if( view < VIEW_LINES || view > VIEW_FILELIGHT )
    view = VIEW_LINES;    
  diskUsage->setView( view );
  
  sizeX = krConfig->readNumEntry( "Window Width",  TQFontMetrics(font()).width("W") * 70 );
  sizeY = krConfig->readNumEntry( "Window Height", TQFontMetrics(font()).height() * 25 );    
  resize( sizeX, sizeY );
  
  if( krConfig->readBoolEntry( "Window Maximized",  false ) )
    showMaximized();
  else  
    show();

  exec();
}

DiskUsageGUI::~DiskUsageGUI()
{
}

void DiskUsageGUI::slotLoadFinished( bool result )
{
  if( exitAtFailure && !result )
    reject();
  else
    exitAtFailure = false;
}

void DiskUsageGUI::enableButtons( bool isOn )
{
  btnNewSearch->setEnabled( isOn );
  btnRefresh->setEnabled( isOn );
  btnDirUp->setEnabled( isOn );
  btnLines->setEnabled( isOn );
  btnDetailed->setEnabled( isOn );
  btnFilelight->setEnabled( isOn );
}

void DiskUsageGUI::resizeEvent( TQResizeEvent *e )
{   
  if( !isMaximized() )
  {
    sizeX = e->size().width();
    sizeY = e->size().height();
  }
  TQDialog::resizeEvent( e );
}

void DiskUsageGUI::reject()
{
  krConfig->setGroup( "DiskUsage" ); 
  krConfig->writeEntry("Window Width", sizeX );
  krConfig->writeEntry("Window Height", sizeY );
  krConfig->writeEntry("Window Maximized", isMaximized() );
  krConfig->writeEntry("View", diskUsage->getActiveView() );
  
  TQDialog::reject();
}

void DiskUsageGUI::loadUsageInfo()
{
  diskUsage->load( baseDirectory );
}

void DiskUsageGUI::setStatus( TQString stat )
{
  status->setText( stat );
}

void DiskUsageGUI::slotViewChanged( int view )
{
  if( view == VIEW_LOADER )
  {
    enableButtons( false );
    return;
  }
  enableButtons( true );

  btnLines->setOn( false );
  btnDetailed->setOn( false );
  btnFilelight->setOn( false );
  
  switch( view )
  {
  case VIEW_LINES:
    btnLines->setOn( true );
    break;
  case VIEW_DETAILED:
    btnDetailed->setOn( true );
    break;
  case VIEW_FILELIGHT:
    btnFilelight->setOn( true );
    break;
  case VIEW_LOADER:
    break;
  }
}

bool DiskUsageGUI::newSearch()
{ 
  // ask the user for the copy dest
  
  KURL tmp = KChooseDir::getDir(i18n( "Viewing the usage of directory:" ), baseDirectory, baseDirectory);
  if (tmp.isEmpty()) return false;
  baseDirectory = tmp;
  
  TQTimer::singleShot( 0, this, TQT_SLOT( loadUsageInfo() ) );
  return true;
}

#include "diskusagegui.moc"
