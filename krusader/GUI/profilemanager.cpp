/***************************************************************************
                      profilemanager.cpp  -  description
                             -------------------
    copyright            : (C) 2004 + by Csaba Karai
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

#include "../krusader.h"
#include "profilemanager.h"

#include <tdelocale.h>
#include <tqtooltip.h>
#include <tdepopupmenu.h>
#include <tqcursor.h>
#include <kinputdialog.h>
#include <kiconloader.h>

ProfileManager::ProfileManager( TQString profileType, TQWidget * parent, const char * name ) 
  : TQPushButton( parent, name )
{
  setText( "" );
  TDEIconLoader *iconLoader = new TDEIconLoader();
  setPixmap( iconLoader->loadIcon( "kr_profile", TDEIcon::Toolbar, 16 ) );
  TQToolTip::add( this, i18n( "Profiles" ) );
  
  this->profileType = profileType;
  
  connect( this, TQ_SIGNAL( clicked() ), this, TQ_SLOT( profilePopup() ) );

  krConfig->setGroup("Private");
  profileList = krConfig->readListEntry( profileType );
}

void ProfileManager::profilePopup()
{
  // profile menu identifiers
  #define ADD_NEW_ENTRY_ID    1000
  #define LOAD_ENTRY_ID       2000
  #define REMOVE_ENTRY_ID     3000
  #define OVERWRITE_ENTRY_ID  4000
  
  // create the menu
  TDEPopupMenu popup, removePopup, overwritePopup;
  popup.insertTitle(i18n("Profiles"));
  
  for( unsigned i=0; i != profileList.count() ; i++ )
  {
    krConfig->setGroup( profileType + " - " + profileList[i] ); 
    TQString name = krConfig->readEntry( "Name" );
    popup.insertItem( name, LOAD_ENTRY_ID + i );
    removePopup.insertItem( name, REMOVE_ENTRY_ID + i );
    overwritePopup.insertItem( name, OVERWRITE_ENTRY_ID + i );
  }

  popup.insertSeparator();
  
  if( profileList.count() )
  {
    popup.insertItem( i18n("Remove entry"), &removePopup );
    popup.insertItem( i18n("Overwrite entry"), &overwritePopup );
  }
  
  popup.insertItem(i18n("Add new entry"),ADD_NEW_ENTRY_ID);

  unsigned result=popup.exec(TQCursor::pos());

  // check out the user's selection
  if( result == ADD_NEW_ENTRY_ID )
    newProfile();
  else if( result >= LOAD_ENTRY_ID && result < LOAD_ENTRY_ID + profileList.count() )
  {
    emit loadFromProfile( profileType + " - " + profileList[ result - LOAD_ENTRY_ID ] );
  }else if( result >= REMOVE_ENTRY_ID && result < REMOVE_ENTRY_ID + profileList.count() )
  { 
    krConfig->deleteGroup( profileType + " - " + profileList[ result - REMOVE_ENTRY_ID ] );    
    profileList.remove( profileList[ result - REMOVE_ENTRY_ID ] );
  
    krConfig->setGroup("Private");
    krConfig->writeEntry( profileType, profileList );
    krConfig->sync();
  }else if( result >= OVERWRITE_ENTRY_ID && result < OVERWRITE_ENTRY_ID + profileList.count() )
  {
    emit saveToProfile( profileType + " - " + profileList[ result - OVERWRITE_ENTRY_ID ] );
  }  
}

void ProfileManager::newProfile( TQString defaultName )
{
  TQString profile = KInputDialog::getText( i18n( "Krusader::ProfileManager" ), i18n( "Enter the profile name:" ), 
                                           defaultName, 0, this );  
  if( !profile.isEmpty() )
  {
    int profileNum = 1;
    while( profileList.contains( TQString( "%1" ).arg( profileNum ) ) )
      profileNum++;

    TQString profileString = TQString( "%1" ).arg( profileNum );
    TQString profileName = profileType + " - " + profileString;
    profileList.append( TQString( "%1" ).arg( profileString ) );
  
    krConfig->setGroup("Private");
    krConfig->writeEntry( profileType, profileList );
      
    krConfig->setGroup( profileName );
    krConfig->writeEntry( "Name", profile );
    emit saveToProfile( profileName );
    krConfig->sync();
  }
}
  
void ProfileManager::deleteProfile( TQString name )
{
  for( unsigned i=0; i != profileList.count() ; i++ )
  {
    krConfig->setGroup( profileType + " - " + profileList[ i ] ); 
    TQString currentName = krConfig->readEntry( "Name" );
    
    if( name == currentName )
    {
      krConfig->deleteGroup( profileType + " - " + profileList[ i ] );    
      profileList.remove( profileList[ i ] );
  
      krConfig->setGroup("Private");
      krConfig->writeEntry( profileType, profileList );
      krConfig->sync();
      return;
    }
  }
}
  
void ProfileManager::overwriteProfile( TQString name )
{
  for( unsigned i=0; i != profileList.count() ; i++ )
  {
    krConfig->setGroup( profileType + " - " + profileList[ i ] ); 
    TQString currentName = krConfig->readEntry( "Name" );
    
    if( name == currentName )
    {
      emit saveToProfile( profileType + " - " + profileList[ i ] );
      return;
    }
  }
}

bool ProfileManager::loadProfile( TQString name )
{
  for( unsigned i=0; i != profileList.count() ; i++ )
  {
    krConfig->setGroup( profileType + " - " + profileList[i] ); 
    TQString currentName = krConfig->readEntry( "Name" );
    
    if( name == currentName )
    {
      emit loadFromProfile( profileType + " - " + profileList[ i ] );
      return true;
    }
  }
  return false;    
}

TQStringList ProfileManager::availableProfiles( TQString profileType ) {
  krConfig->setGroup("Private");
  TQStringList profiles = krConfig->readListEntry( profileType );
  TQStringList profileNames;

  for( unsigned i=0; i != profiles.count() ; i++ ) {
    krConfig->setGroup( profileType + " - " + profiles[ i ] ); 
    profileNames.append( krConfig->readEntry("Name") );
  }

  return profileNames;
}

#include "profilemanager.moc"
