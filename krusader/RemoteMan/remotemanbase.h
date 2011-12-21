/****************************************************************************
** Form interface generated from reading ui file 'remotemanbase.ui'
**
** Created: Thu Jun 7 16:23:59 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef REMOTEMANBASE_H
#define REMOTEMANBASE_H

#include <tqvariant.h>
#include <tqdialog.h>
class TQVBoxLayout; 
class TQHBoxLayout; 
class TQGridLayout; 
class TQCheckBox;
class TQComboBox;
class TQLabel;
class TQLineEdit;
class TQListView;
class TQListViewItem;
class TQMultiLineEdit;
class TQPushButton;
class TQSpinBox;

class remoteManBase : public TQDialog
{ 
    Q_OBJECT
  TQ_OBJECT

public:
    remoteManBase( TQWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~remoteManBase();

    TQLabel* TextLabel1;
    TQLineEdit* sessionName;
    TQPushButton* moreBtn;
    TQPushButton* closeBtn;
    TQListView* sessions;
    TQLabel* TextLabel1_3_3;
    TQLineEdit* password;
    TQLabel* TextLabel1_3;
    TQLineEdit* userName;
    TQCheckBox* anonymous;
    TQLabel* TextLabel1_3_2;
    TQLineEdit* remoteDir;
    TQLabel* TextLabel1_3_2_2;
    TQMultiLineEdit* description;
    TQPushButton* removeBtn;
    TQPushButton* connectBtn;
    TQPushButton* newGroupBtn;
    TQPushButton* addBtn;
    TQLabel* TextLabel1_2;
    TQComboBox* protocol;
    TQLineEdit* hostName;
    TQSpinBox* portNum;
    TQLabel* TextLabel1_2_2;
    TQLabel* TextLabel1_4;

public slots:
    virtual void addSession();
    virtual void connection();
    virtual void moreInfo();
    virtual void addGroup();
    virtual void refreshData();
    virtual void removeSession();
    virtual void updateName(const TQString&);

protected:
    TQGridLayout* remoteManBaseLayout;
    TQVBoxLayout* Layout23;
    TQHBoxLayout* Layout12;
    TQVBoxLayout* Layout9;
    TQGridLayout* Layout10;
    TQVBoxLayout* Layout26;
    TQVBoxLayout* Layout27;
    TQGridLayout* layout;
    TQGridLayout* Layout11;
    bool event( TQEvent* );
};

#endif // REMOTEMANBASE_H
