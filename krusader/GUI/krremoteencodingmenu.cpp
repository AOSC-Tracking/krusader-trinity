/***************************************************************************
                   krremoteencodingmenu.cpp  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
    based on             : KRemoteEncodingPlugin from Dawit Alemayehu
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

#include <tdepopupmenu.h>
#include <kcharsets.h>
#include <tdeio/slaveconfig.h>
#include <dcopclient.h>

#include "../krusader.h"
#include "../krusaderview.h"
#include "../Panel/listpanel.h"
#include "../Panel/panelfunc.h"

#include "krremoteencodingmenu.h"

#define DATA_KEY    TQString::fromLatin1("Charset")

KrRemoteEncodingMenu::KrRemoteEncodingMenu(const TQString &text, const TQString &icon, TQObject *parent, const char *name) :
  TDEActionMenu( text, icon, parent, name ), settingsLoaded( false )
{
  connect(popupMenu(), TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(slotAboutToShow()));
}

void KrRemoteEncodingMenu::slotAboutToShow()
{
  if (!settingsLoaded)
    loadSettings();

  // uncheck everything
  for (unsigned i =  0; i < popupMenu()->count(); i++)
    popupMenu()->setItemChecked(popupMenu()->idAt(i), false);

  KURL currentURL = ACTIVE_PANEL->virtualPath();

  TQString charset = TDEIO::SlaveConfig::self()->configData(currentURL.protocol(), currentURL.host(), DATA_KEY);
  if (!charset.isEmpty())
  {
    int id = 1;
    TQStringList::Iterator it;
    for (it = encodingNames.begin(); it != encodingNames.end(); ++it, ++id)
      if ((*it).find(charset) != -1)
        break;

//     kdDebug() << k_funcinfo << "URL=" << currentURL << " charset=" << charset << endl;

    if (it == encodingNames.end())
      kdWarning() << k_funcinfo << "could not find entry for charset=" << charset << endl;
    else
      popupMenu()->setItemChecked(id, true);
  }
  else
    popupMenu()->setItemChecked(defaultID, true);
}

void KrRemoteEncodingMenu::loadSettings()
{
  settingsLoaded = true;
  encodingNames = TDEGlobal::charsets()->descriptiveEncodingNames();

  TDEPopupMenu *menu = popupMenu();
  menu->clear();

  TQStringList::ConstIterator it;
  int count = 0;
  for (it = encodingNames.begin(); it != encodingNames.end(); ++it)
    menu->insertItem(*it, this, TQ_SLOT(slotItemSelected(int)), 0, ++count);
  menu->insertSeparator();

  menu->insertItem(i18n("Reload"), this, TQ_SLOT(slotReload()), 0, ++count);
  menu->insertItem(i18n("Default"), this, TQ_SLOT(slotDefault()), 0, ++count);
  defaultID = count;
}

int KrRemoteEncodingMenu::plug( TQWidget *widget, int index )
{
  if( widget->inherits( "TQPopupMenu" ) )
  {
    connect( widget, TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(slotCheckEnabled()));
    slotCheckEnabled();
  }

  return TDEActionMenu::plug( widget, index );
}

void KrRemoteEncodingMenu::slotCheckEnabled()
{
  KURL currentURL = ACTIVE_PANEL->virtualPath();
  setEnabled( currentURL.protocol() == "ftp" || currentURL.protocol() == "sftp" || currentURL.protocol() == "fish" );
}

void KrRemoteEncodingMenu::slotItemSelected(int id)
{
  KURL currentURL = ACTIVE_PANEL->virtualPath();

  TDEConfig config(("tdeio_" + currentURL.protocol() + "rc").latin1());
  TQString host = currentURL.host();

  if (!popupMenu()->isItemChecked(id))
  {
    TQString charset = TDEGlobal::charsets()->encodingForName( encodingNames[id - 1] );

    config.setGroup(host);
    config.writeEntry(DATA_KEY, charset);
    config.sync();

    // Update the io-slaves...
    updateKIOSlaves();
  }
}

void KrRemoteEncodingMenu::slotReload()
{
  loadSettings();
}

void KrRemoteEncodingMenu::slotDefault()
{
  KURL currentURL = ACTIVE_PANEL->virtualPath();

  // We have no choice but delete all higher domain level
  // settings here since it affects what will be matched.
  TDEConfig config(("tdeio_" + currentURL.protocol() + "rc").latin1());

  TQStringList partList = TQStringList::split('.', currentURL.host(), false);
  if (!partList.isEmpty())
  {
    partList.remove(partList.begin());

    TQStringList domains;
    // Remove the exact name match...
    domains << currentURL.host();

    while (partList.count())
    {
      if (partList.count() == 2)
        if (partList[0].length() <= 2 && partList[1].length() == 2)
          break;

      if (partList.count() == 1)
        break;

      domains << partList.join(".");
      partList.remove(partList.begin());
    }

    for (TQStringList::Iterator it = domains.begin(); it != domains.end(); it++)
    {
//    kdDebug() << k_funcinfo << "Domain to remove: " << *it << endl;
      if (config.hasGroup(*it))
        config.deleteGroup(*it);
      else if (config.hasKey(*it))
        config.deleteEntry(*it);
    }
  }
  config.sync();

  updateKIOSlaves();
}


void KrRemoteEncodingMenu::updateKIOSlaves()
{
  // Inform running io-slaves about the change...
  DCOPClient *client = new DCOPClient();

  if (!client->attach())
    kdDebug() << "Can't connect with DCOP server." << endl;

  TQByteArray data;
  TQDataStream stream(data, IO_WriteOnly);
  stream << TQString();
  client->send("*", "TDEIO::Scheduler", "reparseSlaveConfiguration(TQString)", data);
  delete client;

  // Reload the page with the new charset
  TQTimer::singleShot( 500, ACTIVE_FUNC, TQ_SLOT( refresh() ) );
}

#include "krremoteencodingmenu.moc"
