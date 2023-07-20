/****************************************************************************
** Form interface generated from reading ui file 'newftpgui.ui'
**
** Created: Fri Oct 27 23:47:08 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef NEWFTPGUI_H
#define NEWFTPGUI_H

#include <tqvariant.h>
#include <tqdialog.h>
class TQVBoxLayout; 
class TQHBoxLayout; 
class TQGridLayout; 
class TQLabel;
class TQLineEdit;
class TQPushButton;
class TQSpinBox;
class KComboBox;
class KHistoryCombo;

class newFTPGUI : public TQDialog {
    TQ_OBJECT
  
public:
    newFTPGUI( TQWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~newFTPGUI();

    TQLabel* TextLabel1;
    KComboBox* prefix;
    TQLabel* TextLabel1_2_2;
	 TQLabel* TextLabel1_22;
    TQLabel* TextLabel1_2;
    TQLabel* TextLabel1_3;
    TQSpinBox* port;
    TQLineEdit* password;
    TQPushButton* connectBtn;
    TQPushButton* saveBtn;
    TQPushButton* cancelBtn;
    TQLabel* PixmapLabel1;
    TQLabel* TextLabel3;
    TQLineEdit* username;
    KHistoryCombo* url;

public slots:
    void slotTextChanged(const TQString& string);

protected:
    TQHBoxLayout* hbox;
    bool event( TQEvent* );
};

#endif // NEWFTPGUI_H
