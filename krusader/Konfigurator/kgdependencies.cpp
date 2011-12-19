/***************************************************************************
                       kgdependencies.cpp  -  description
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

#include "kgdependencies.h"
#include "../krservices.h"
#include "../krusader.h"
#include <tqtabwidget.h>
#include <klocale.h>
#include <tqhbox.h>
#include <kmessagebox.h>

#define PAGE_GENERAL   0
#define PAGE_PACKERS   1
#define PAGE_CHECKSUM  2

KgDependencies::KgDependencies( bool first, TQWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  TQGridLayout *kgDependenciesLayout = new TQGridLayout( parent );
  kgDependenciesLayout->setSpacing( 6 );

  //  ---------------------------- GENERAL TAB -------------------------------------
  tabWidget = new TQTabWidget( parent, "tabWidget" );

  TQWidget *general_tab = new TQWidget( tabWidget, "tab" );
  tabWidget->insertTab( general_tab, i18n( "General" ) );

  TQGridLayout *pathsGrid = new TQGridLayout( general_tab );
  pathsGrid->setSpacing( 6 );
  pathsGrid->setMargin( 11 );
  pathsGrid->setAlignment( TQt::AlignTop );
  
  addApplication( "df",       pathsGrid, 0, general_tab, PAGE_GENERAL );
  addApplication( "eject",    pathsGrid, 1, general_tab, PAGE_GENERAL );
  addApplication( "kdesu",    pathsGrid, 2, general_tab, PAGE_GENERAL );
  addApplication( "kget",     pathsGrid, 3, general_tab, PAGE_GENERAL );
  addApplication( "kmail",    pathsGrid, 4, general_tab, PAGE_GENERAL );
  addApplication( "diff utility",  pathsGrid, 5, general_tab, PAGE_GENERAL );
  addApplication( "krename",  pathsGrid, 6, general_tab, PAGE_GENERAL );
  addApplication( "krusader", pathsGrid, 7, general_tab, PAGE_GENERAL );
  addApplication( "locate",   pathsGrid, 8, general_tab, PAGE_GENERAL );
  addApplication( "mount",    pathsGrid, 9, general_tab, PAGE_GENERAL );
  addApplication( "umount",   pathsGrid,10, general_tab, PAGE_GENERAL );
  addApplication( "updatedb", pathsGrid,11, general_tab, PAGE_GENERAL );

  //  ---------------------------- PACKERS TAB -------------------------------------
  TQWidget *packers_tab = new TQWidget( tabWidget, "tab_3" );
  tabWidget->insertTab( packers_tab, i18n( "Packers" ) );

  TQGridLayout *archGrid1 = new TQGridLayout( packers_tab );
  archGrid1->setSpacing( 6 );
  archGrid1->setMargin( 11 );
  archGrid1->setAlignment( TQt::AlignTop );

  addApplication( "7z",    archGrid1, 0, packers_tab, PAGE_PACKERS, "7za" );
  addApplication( "arj",   archGrid1, 1, packers_tab, PAGE_PACKERS );
  addApplication( "bzip2", archGrid1, 2, packers_tab, PAGE_PACKERS );
  addApplication( "cpio",  archGrid1, 3, packers_tab, PAGE_PACKERS );
  addApplication( "dpkg",  archGrid1, 4, packers_tab, PAGE_PACKERS );
  addApplication( "gzip",  archGrid1, 5, packers_tab, PAGE_PACKERS );
  addApplication( "lha",   archGrid1, 6, packers_tab, PAGE_PACKERS );
  addApplication( "rar",   archGrid1, 7, packers_tab, PAGE_PACKERS );
  addApplication( "tar",   archGrid1, 8, packers_tab, PAGE_PACKERS );
  addApplication( "unace", archGrid1, 9, packers_tab, PAGE_PACKERS );
  addApplication( "unarj", archGrid1,10, packers_tab, PAGE_PACKERS );
  addApplication( "unrar", archGrid1,11, packers_tab, PAGE_PACKERS );
  addApplication( "unzip", archGrid1,12, packers_tab, PAGE_PACKERS );
  addApplication( "zip",   archGrid1,13, packers_tab, PAGE_PACKERS );

  //  ---------------------------- CHECKSUM TAB -------------------------------------
  TQWidget *checksum_tab = new TQWidget( tabWidget, "tab_4" );
  tabWidget->insertTab( checksum_tab, i18n( "Checksum Utilities" ) );

  TQGridLayout *archGrid2 = new TQGridLayout( checksum_tab );
  archGrid2->setSpacing( 6 );
  archGrid2->setMargin( 11 );
  archGrid2->setAlignment( TQt::AlignTop );

  addApplication( "md5sum",         archGrid2, 0, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha1sum",        archGrid2, 1, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha224sum",      archGrid2, 2, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha256sum",      archGrid2, 3, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha384sum",      archGrid2, 4, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha512sum",      archGrid2, 5, checksum_tab, PAGE_CHECKSUM );
  addApplication( "md5deep",        archGrid2, 6, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha1deep",       archGrid2, 7, checksum_tab, PAGE_CHECKSUM );
  addApplication( "sha256deep",     archGrid2, 8, checksum_tab, PAGE_CHECKSUM );
  addApplication( "tigerdeep",      archGrid2, 9, checksum_tab, PAGE_CHECKSUM );
  addApplication( "whirlpooldeep",  archGrid2, 10, checksum_tab, PAGE_CHECKSUM );
  addApplication( "cfv",            archGrid2, 11, checksum_tab, PAGE_CHECKSUM );

  kgDependenciesLayout->addWidget( tabWidget, 0, 0 );
}

void KgDependencies::addApplication( TQString name, TQGridLayout *grid, int row, TQWidget *parent, int page, TQString additionalList )
{
  TQString dflt = KrServices::fullPathName( name ); /* try to autodetect the full path name */

  if( dflt.isEmpty() ) {
    TQStringList list = TQStringList::split( ',', additionalList );
    for( unsigned i=0; i != list.count(); i++ )
      if( !KrServices::fullPathName( list[ i ] ).isEmpty() ) {
        dflt = KrServices::fullPathName( list[ i ] );
        break;
      }
  }

  addLabel( grid, row, 0, name, parent, (TQString( "label:" )+name).ascii() );

  KonfiguratorURLRequester *fullPath = createURLRequester( "Dependencies", name, dflt, parent, false, page );
  connect( fullPath->extension(), TQT_SIGNAL( applyManually( TQObject *, TQString, TQString ) ),
           this, TQT_SLOT( slotApply( TQObject *, TQString, TQString ) ) );
  grid->addWidget( fullPath, row, 1 );
}

void KgDependencies::slotApply( TQObject *obj, TQString cls, TQString name )
{
  KonfiguratorURLRequester *urlRequester = (KonfiguratorURLRequester *) obj;

  krConfig->setGroup( cls );
  krConfig->writeEntry( name, urlRequester->url() );

  TQString usedPath = KrServices::fullPathName( name );

  if( urlRequester->url() != usedPath )
  {
    krConfig->writeEntry( name, usedPath );
    if( usedPath.isEmpty() )
      KMessageBox::error( this, i18n( "The %1 path is incorrect, no valid path found." )
                          .tqarg( urlRequester->url() ) );
    else
      KMessageBox::error( this, i18n( "The %1 path is incorrect, %2 used instead." )
                          .tqarg( urlRequester->url() ).tqarg( usedPath ) );
    urlRequester->setURL( usedPath );
  }
}

int KgDependencies::activeSubPage() {
  return tabWidget->currentPageIndex();
}

#include "kgdependencies.moc"
