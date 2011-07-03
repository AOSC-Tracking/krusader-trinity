/***************************************************************************
                               kcmdline.h
                            -------------------
   begin                : Thu May 4 2000
   copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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


#ifndef KCMDLINE_H
#define KCMDLINE_H

// QT includes
#include <tqwidget.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtoolbutton.h>

// KDE includes
#include <klineedit.h>
#include <kshellcompletion.h>
#include <kcombobox.h>

#include "../UserAction/kractionbase.h"

class KCMDModeButton;

class KrHistoryCombo: public KHistoryCombo {
  Q_OBJECT
  TQ_OBJECT

  public:
    KrHistoryCombo(TQWidget *tqparent): KHistoryCombo(tqparent) {}

  protected:
    void keyPressEvent( TQKeyEvent *e );

  signals:
    void returnToPanel();
};

class KCMDLine : public TQWidget, KrActionBase {
    Q_OBJECT
  TQ_OBJECT
  public:
    KCMDLine( TQWidget *tqparent = 0, const char *name = 0 );
    ~KCMDLine();
    void setCurrent( const TQString & );
    //virtual methods from KrActionBase
    void setText(TQString text);
    TQString command() const;
    ExecType execType() const;
    TQString startpath() const;
    TQString user() const;
    TQString text() const;
    bool acceptURLs() const;
    bool confirmExecution() const;
    bool doSubstitution() const;
  signals:
    void signalRun();		
  public slots:
    inline void setFocus() { cmdLine->setFocus(); } // overloaded for KCmdLine
    void slotReturnFocus(); // returns keyboard focus to panel
    void slotRun();
    void addPlaceholder();
    void addText( TQString text ) { cmdLine->setCurrentText( cmdLine->currentText() + text ); }
    void popup() { cmdLine->popup(); }


  private:
    TQLabel *path;
    KrHistoryCombo *cmdLine;
    KCMDModeButton *terminal;
    TQToolButton *buttonAddPlaceholder;
    KShellCompletion completion;
};

#endif
