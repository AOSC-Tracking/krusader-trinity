#include "checksumdlg.h"
#include "../krusader.h"
#include <klocale.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <klineedit.h>
#include <klistview.h>
#include <tqpixmap.h>
#include <kcursor.h>
#include <kmessagebox.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <kfiledialog.h>
#include <tqframe.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <tqfileinfo.h>
#include <kurlrequester.h>
#include "../krservices.h"
#include <tqptrlist.h>
#include <tqmap.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

class CS_Tool; // forward
typedef void PREPARE_PROC_FUNC(KProcess& proc, CS_Tool *self, const TQStringList& files, 
	const TQString checksumFile, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName,	const TQString& type=TQString());
typedef TQStringList GET_FAILED_FUNC(const TQStringList& stdOut, const TQStringList& stdErr);

class CS_Tool {
public:
	enum Type {
          MD5=0, SHA1, SHA256, TIGER, WHIRLPOOL, SFV, CRC,
          SHA224, SHA384, SHA512,
	  NumOfTypes
	};
	
	Type type;
	TQString binary;
	bool recursive;
	bool standardFormat;
	PREPARE_PROC_FUNC *create, *verify;
	GET_FAILED_FUNC *failed;
};

class CS_ToolByType {
public:
	TQPtrList<CS_Tool> tools, r_tools; // normal and recursive tools	
};

// handles md5sum and sha1sum
void sumCreateFunc(KProcess& proc, CS_Tool *self, const TQStringList& files, 
	const TQString, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName, const TQString&) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	Q_ASSERT(!recursive); 
	proc << files << "1>" << stdoutFileName << "2>" << stderrFileName;	
}

void sumVerifyFunc(KProcess& proc, CS_Tool *self, const TQStringList& /* files */, 
	const TQString checksumFile, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName, const TQString& /* type */) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	Q_ASSERT(!recursive);
	proc << "-c" << checksumFile << "1>" << stdoutFileName << "2>" << stderrFileName;
}

TQStringList sumFailedFunc(const TQStringList& stdOut, const TQStringList& stdErr) {
	// md5sum and sha1sum print "...: FAILED" for failed files and display
	// the number of failures to stderr. so if stderr is empty, we'll assume all is ok
	TQStringList result;
	if (stdErr.size()==0) return result;
	result += stdErr;
	// grep for the ":FAILED" substring
	const TQString tmp = TQString(": FAILED").local8Bit();
	for (uint i=0; i<stdOut.size();++i) {
		if (stdOut[i].find(tmp) != -1)
			result += stdOut[i];
	}
	
	return result;
}

// handles *deep binaries
void deepCreateFunc(KProcess& proc, CS_Tool *self, const TQStringList& files, 
	const TQString, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName, const TQString&) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	if (recursive) proc << "-r";
	proc << "-l" << files << "1>" << stdoutFileName << "2>" << stderrFileName;
}

void deepVerifyFunc(KProcess& proc, CS_Tool *self, const TQStringList& files, 
	const TQString checksumFile, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName, const TQString&) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary );
	if (recursive) proc << "-r";
	proc << "-x" << checksumFile << files << "1>" << stdoutFileName << "2>" << stderrFileName;
}

TQStringList deepFailedFunc(const TQStringList& stdOut, const TQStringList&/* stdErr */) {
	// *deep dumps (via -x) all failed hashes to stdout
	return stdOut;
}

// handles cfv binary
void cfvCreateFunc(KProcess& proc, CS_Tool *self, const TQStringList& files, 
	const TQString, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName, const TQString& type) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary ) << "-C" << "-VV";
	if (recursive) proc << "-rr";
	proc << "-t" << type << "-f-" << "-U" << files << "1>" << stdoutFileName << "2>" << stderrFileName;	
}

void cfvVerifyFunc(KProcess& proc, CS_Tool *self, const TQStringList& /* files */, 
	const TQString checksumFile, bool recursive, const TQString& stdoutFileName, 
	const TQString& stderrFileName, const TQString&type) {
	proc.setUseShell(true, "/bin/bash");
	proc << KrServices::fullPathName( self->binary ) << "-M";
	if (recursive) proc << "-rr";
	proc << "-U" << "-VV" << "-t" << type << "-f" << checksumFile << "1>" << stdoutFileName << "2>" << stderrFileName;// << files;
}

TQStringList cfvFailedFunc(const TQStringList& /* stdOut */, const TQStringList& stdErr) {
	// cfv dumps all failed hashes to stderr
	return stdErr;
}

// important: this table should be ordered like so that all md5 tools should be
// one after another, and then all sha1 and so on and so forth. they tools must be grouped,
// since the code in getTools() counts on it!
CS_Tool cs_tools[] = {
	// type              binary            recursive   stdFmt           create_func       verify_func      failed_func
	{CS_Tool::MD5,       "md5sum",         false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::MD5,       "md5deep",        true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::MD5,       "cfv",            true,       true,            cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
	{CS_Tool::SHA1,      "sha1sum",        false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA1,      "sha1deep",       true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::SHA1,      "cfv",            true,       true,            cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
	{CS_Tool::SHA224,    "sha224sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA256,    "sha256sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA256,    "sha256deep",     true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::SHA384,    "sha384sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::SHA512,    "sha512sum",      false,      true,            sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
	{CS_Tool::TIGER,     "tigerdeep",      true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::WHIRLPOOL, "whirlpooldeep",  true,       true,            deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
	{CS_Tool::SFV,       "cfv",            true,       false,           cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
	{CS_Tool::CRC,       "cfv",            true,       false,           cfvCreateFunc,    cfvVerifyFunc,   cfvFailedFunc},
};

TQMap<TQString, CS_Tool::Type> cs_textToType;
TQMap<CS_Tool::Type, TQString> cs_typeToText;

void initChecksumModule() {
	// prepare the dictionaries - pity it has to be manually
	cs_textToType["md5"]=CS_Tool::MD5;
	cs_textToType["sha1"]=CS_Tool::SHA1;
	cs_textToType["sha256"]=CS_Tool::SHA256;
	cs_textToType["sha224"]=CS_Tool::SHA224;
	cs_textToType["sha384"]=CS_Tool::SHA384;
	cs_textToType["sha512"]=CS_Tool::SHA512;
	cs_textToType["tiger"]=CS_Tool::TIGER;
	cs_textToType["whirlpool"]=CS_Tool::WHIRLPOOL;
	cs_textToType["sfv"]=CS_Tool::SFV;
	cs_textToType["crc"]=CS_Tool::CRC;

	cs_typeToText[CS_Tool::MD5]="md5";
	cs_typeToText[CS_Tool::SHA1]="sha1";
	cs_typeToText[CS_Tool::SHA256]="sha256";
	cs_typeToText[CS_Tool::SHA224]="sha224";
	cs_typeToText[CS_Tool::SHA384]="sha384";
	cs_typeToText[CS_Tool::SHA512]="sha512";
	cs_typeToText[CS_Tool::TIGER]="tiger";
	cs_typeToText[CS_Tool::WHIRLPOOL]="whirlpool";
	cs_typeToText[CS_Tool::SFV]="sfv";
	cs_typeToText[CS_Tool::CRC]="crc";

	// build the checksumFilter (for usage in KRQuery)
	TQMap<TQString, CS_Tool::Type>::Iterator it;
	for (it=cs_textToType.begin(); it!=cs_textToType.end(); ++it)
		MatchChecksumDlg::checksumTypesFilter += ("*."+it.key()+" ");
}

// --------------------------------------------------

// returns a list of tools which can work with recursive or non-recursive mode and are installed
// note: only 1 tool from each type is suggested
static TQPtrList<CS_Tool> getTools(bool folders) {
	TQPtrList<CS_Tool> result;
	uint i;
	for (i=0; i < sizeof(cs_tools)/sizeof(CS_Tool); ++i) {
		if (result.last() && result.last()->type == cs_tools[i].type) continue; // 1 from each type please
		if (folders && !cs_tools[i].recursive) continue;
		if (KrServices::cmdExist(cs_tools[i].binary))
			result.append(&cs_tools[i]);
	}
	
	return result;
}

// ------------- CreateChecksumDlg

CreateChecksumDlg::CreateChecksumDlg(const TQStringList& files, bool containFolders, const TQString& path):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp) {
	
	TQPtrList<CS_Tool> tools = getTools(containFolders);

	if (tools.count() == 0) { // nothing was suggested?!
		TQString error = i18n("<qt>Can't calculate checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.</qt>");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep, tigerdeep and cfv</i></qt>");
		KMessageBox::error(0, error);
		return;
	}
	
	TQGridLayout *tqlayout = new TQGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	TQHBoxLayout *htqlayout = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
	TQLabel *p = new TQLabel(plainPage());
	p->setPixmap(krLoader->loadIcon("binary", KIcon::Desktop, 32));
	htqlayout->addWidget(p);
	TQLabel *l1 = new TQLabel(i18n("About to calculate checksum for the following files") + 
		(containFolders ? i18n(" and folders:") : ":"), plainPage());
	htqlayout->addWidget(l1);
	tqlayout->addMultiCellLayout(htqlayout, row, row, 0, 1, TQt::AlignLeft); 
	++row;
	
	// file list
	KListBox *lb = new KListBox(plainPage());
	lb->insertStringList(files);
	tqlayout->addMultiCellWidget(lb, row, row, 0, 1);
	++row;

	// checksum method
	TQHBoxLayout *htqlayout2 = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
	TQLabel *l2 = new TQLabel(i18n("Select the checksum method:"), plainPage());
	htqlayout2->addWidget(l2);
	KComboBox *method = new KComboBox(plainPage());
	// -- fill the combo with available methods
	uint i;
	for ( i=0; i<tools.count(); ++i )
		method->insertItem( cs_typeToText[tools.at(i)->type], i);
	method->setFocus();
	htqlayout2->addWidget(method);	
	tqlayout->addMultiCellLayout(htqlayout2, row, row, 0, 1, TQt::AlignLeft);
	++row;

	if (exec() != Accepted) return;
	// else implied: run the process
	tmpOut = new KTempFile(locateLocal("tmp", "krusader"), ".stdout" );
	tmpErr = new KTempFile(locateLocal("tmp", "krusader"), ".stderr" );
	KProcess proc;
	CS_Tool *mytool = tools.at(method->currentItem());
	mytool->create(proc, mytool, KrServices::quote(files), TQString(), containFolders, 
		tmpOut->name(), tmpErr->name(), method->currentText());
	
	krApp->startWaiting(i18n("Calculating checksums ..."), 0, true);	
	TQApplication::setOverrideCursor( KCursor::waitCursor() );
	bool r = proc.start(KProcess::NotifyOnExit, KProcess::AllOutput);
	if (r) while ( proc.isRunning() ) {
		usleep( 500 );
		tqApp->processEvents();
		if (krApp->wasWaitingCancelled()) { // user cancelled
			proc.kill();
			TQApplication::restoreOverrideCursor();
			return;
		}
   };
	krApp->stopWait();
	TQApplication::restoreOverrideCursor();
	if (!r || !proc.normalExit()) {	
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>").arg(mytool->binary));
		return;
	}
	
	// suggest a filename
	TQString suggestedFilename = path + '/';
	if (files.count() > 1) suggestedFilename += ("checksum." + cs_typeToText[mytool->type]);
	else suggestedFilename += (files[0] + '.' + cs_typeToText[mytool->type]);
	// send both stdout and stderr
	TQStringList stdOut, stdErr;
	if (!KrServices::fileToStringList(tmpOut->textStream(), stdOut) || 
			!KrServices::fileToStringList(tmpErr->textStream(), stdErr)) {
		KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
		return;
	}

	ChecksumResultsDlg dlg( stdOut, stdErr, suggestedFilename, mytool->binary, cs_typeToText[mytool->type], mytool->standardFormat);
	tmpOut->unlink(); delete tmpOut;
	tmpErr->unlink(); delete tmpErr;
}

// ------------- MatchChecksumDlg

TQString MatchChecksumDlg::checksumTypesFilter;

MatchChecksumDlg::MatchChecksumDlg(const TQStringList& files, bool containFolders, 
	const TQString& path, const TQString& checksumFile):
	KDialogBase(Plain, i18n("Verify Checksum"), Ok | Cancel, Ok, krApp) {
	
	TQPtrList<CS_Tool> tools = getTools(containFolders);

	if (tools.count() == 0) { // nothing was suggested?!
		TQString error = i18n("<qt>Can't verify checksum since no supported tool was found. "
			"Please check the <b>Dependencies</b> page in Krusader's settings.</qt>");
		if (containFolders) 
			error += i18n("<qt><b>Note</b>: you've selected directories, and probably have no recursive checksum tool installed."
			" Krusader currently supports <i>md5deep, sha1deep, sha256deep, tigerdeep and cfv</i></qt>");
		KMessageBox::error(0, error);
		return;
	}
	
	TQGridLayout *tqlayout = new TQGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());
	
	int row=0;
		
	// title (icon+text)	
	TQHBoxLayout *htqlayout = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
	TQLabel *p = new TQLabel(plainPage());
	p->setPixmap(krLoader->loadIcon("binary", KIcon::Desktop, 32));
	htqlayout->addWidget(p);
	TQLabel *l1 = new TQLabel(i18n("About to verify checksum for the following files") +
		(containFolders ? i18n(" and folders:") : ":"), plainPage());
	htqlayout->addWidget(l1);
	tqlayout->addMultiCellLayout(htqlayout, row, row, 0, 1, TQt::AlignLeft); 
	++row;
	
	// file list
	KListBox *lb = new KListBox(plainPage());
	lb->insertStringList(files);
	tqlayout->addMultiCellWidget(lb, row, row, 0, 1);
	++row;

	// checksum file
	TQHBoxLayout *htqlayout2 = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
	TQLabel *l2 = new TQLabel(i18n("Checksum file:"), plainPage());
	htqlayout2->addWidget(l2);
	KURLRequester *checksumFileReq = new KURLRequester( plainPage() );
	if (!checksumFile.isEmpty())
		checksumFileReq->setURL(checksumFile);
	checksumFileReq->fileDialog()->setURL(path);
	checksumFileReq->setFocus();
	htqlayout2->addWidget(checksumFileReq);
	tqlayout->addMultiCellLayout(htqlayout2, row, row, 0, 1, TQt::AlignLeft);

	if (exec() != Accepted) return;
	TQString file = checksumFileReq->url();
	TQString extension;
	if (!verifyChecksumFile(file, extension)) {
		KMessageBox::error(0, i18n("<qt>Error reading checksum file <i>%1</i>.<br />Please specify a valid checksum file.</qt>").arg(file));
		return;
	}
	
	// do we have a tool for that extension?
	uint i;
	CS_Tool *mytool = 0;
	for ( i=0; i < tools.count(); ++i )
		if (cs_typeToText[tools.at(i)->type] == extension.lower()) {
			mytool = tools.at(i);
			break;
		}
	if (!mytool) {
		KMessageBox::error(0, i18n("<qt>Krusader can't find a checksum tool that handles %1 on your system. Please check the <b>Dependencies</b> page in Krusader's settings.</qt>").arg(extension));
		return;
	}
	
	// else implied: run the process
	tmpOut = new KTempFile(locateLocal("tmp", "krusader"), ".stdout" );
	tmpErr = new KTempFile(locateLocal("tmp", "krusader"), ".stderr" );
	KProcess proc;
	mytool->verify(proc, mytool, KrServices::quote(files), KrServices::quote(file), containFolders, tmpOut->name(), tmpErr->name(), extension);
	krApp->startWaiting(i18n("Verifying checksums ..."), 0, true);	
	TQApplication::setOverrideCursor( KCursor::waitCursor() );
	bool r = proc.start(KProcess::NotifyOnExit, KProcess::AllOutput);
	if (r) while ( proc.isRunning() ) {
		usleep( 500 );
  		tqApp->processEvents();
		if (krApp->wasWaitingCancelled()) { // user cancelled
			proc.kill();
			TQApplication::restoreOverrideCursor();
			return;
		}
   };
	if (!r || !proc.normalExit()) {	
		KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>").arg(mytool->binary));
		return;
	}
	TQApplication::restoreOverrideCursor();
	krApp->stopWait();
	// send both stdout and stderr
	TQStringList stdOut,stdErr;
	if (!KrServices::fileToStringList(tmpOut->textStream(), stdOut) || 
			!KrServices::fileToStringList(tmpErr->textStream(), stdErr)) {
		KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
		return;
	}
	VerifyResultDlg dlg(mytool->failed(stdOut, stdErr));
	tmpOut->unlink(); delete tmpOut;
	tmpErr->unlink(); delete tmpErr;
}

bool MatchChecksumDlg::verifyChecksumFile(TQString path,  TQString& extension) {
	TQFileInfo f(path);
	if (!f.exists() || f.isDir()) return false;
	// find the extension
	extension = path.mid(path.findRev(".")+1);
	
	// TODO: do we know the extension? if not, ask the user for one
	
	
	return true;
}

// ------------- VerifyResultDlg
VerifyResultDlg::VerifyResultDlg(const TQStringList& failed):
	KDialogBase(Plain, i18n("Verify Checksum"), Close, Close, krApp) {
	TQGridLayout *tqlayout = new TQGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());

	bool errors = failed.size()>0;
	int row = 0;
	
	// create the icon and title
	TQHBoxLayout *htqlayout = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
	TQLabel p(plainPage());
	p.setPixmap(krLoader->loadIcon(errors ? "messagebox_critical" : "messagebox_info", KIcon::Desktop, 32));
	htqlayout->addWidget(&p);
	
	TQLabel *l1 = new TQLabel((errors ? i18n("Errors were detected while verifying the checksums") :
		i18n("Checksums were verified successfully")), plainPage());
	htqlayout->addWidget(l1);
	tqlayout->addMultiCellLayout(htqlayout,row,row,0,1, TQt::AlignLeft);
	++row;

	if (errors) { 
		TQLabel *l3 = new TQLabel(i18n("The following files have failed:"), plainPage());
		tqlayout->addMultiCellWidget(l3, row, row, 0, 1);
		++row;
		KListBox *lb2 = new KListBox(plainPage());
		lb2->insertStringList(failed);
		tqlayout->addMultiCellWidget(lb2, row, row, 0, 1);
		++row;
	}
		
	exec();
}

// ------------- ChecksumResultsDlg

ChecksumResultsDlg::ChecksumResultsDlg(const TQStringList& stdOut, const TQStringList& stdErr,
	const TQString& suggestedFilename, const TQString& binary, const TQString& /* type */, bool standardFormat):
	KDialogBase(Plain, i18n("Create Checksum"), Ok | Cancel, Ok, krApp), _binary(binary) {
	TQGridLayout *tqlayout = new TQGridLayout( plainPage(), 1, 1,
		KDialogBase::marginHint(), KDialogBase::spacingHint());

	// md5 tools display errors into stderr, so we'll use that to determine the result of the job
	bool errors = stdErr.size()>0;
	bool successes = stdOut.size()>0;
	int row = 0;
	
	// create the icon and title
	TQHBoxLayout *htqlayout = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
	TQLabel p(plainPage());
	p.setPixmap(krLoader->loadIcon(errors ? "messagebox_critical" : "messagebox_info", KIcon::Desktop, 32));
	htqlayout->addWidget(&p);
	
	TQLabel *l1 = new TQLabel((errors ? i18n("Errors were detected while creating the checksums") :
		i18n("Checksums were created successfully")), plainPage());
	htqlayout->addWidget(l1);
	tqlayout->addMultiCellLayout(htqlayout,row,row,0,1, TQt::AlignLeft);
	++row;

	if (successes) {
		if (errors) {
			TQLabel *l2 = new TQLabel(i18n("Here are the calculated checksums:"), plainPage());
			tqlayout->addMultiCellWidget(l2, row, row, 0, 1);
			++row;
		}
		KListView *lv = new KListView(plainPage());
		if(standardFormat){
			lv->addColumn(i18n("Hash"));
			lv->addColumn(i18n("File"));
			lv->setAllColumnsShowFocus(true);
		} else {
			lv->addColumn(i18n("File and hash"));
		}
		for ( TQStringList::ConstIterator it = stdOut.begin(); it != stdOut.end(); ++it ) {
			TQString line = (*it);
			if(standardFormat) {
				int space = line.find(' ');
				new KListViewItem(lv, line.left(space), line.mid(space+2));
			} else {
				new KListViewItem(lv, line);
			}	
		}
		tqlayout->addMultiCellWidget(lv, row, row, 0, 1);
		++row;
	}

	if (errors) {
		TQFrame *line1 = new TQFrame( plainPage() );
		line1->setGeometry( TQRect( 60, 210, 501, 20 ) );
		line1->setFrameShape( TQFrame::HLine );
		line1->setFrameShadow( TQFrame::Sunken );
		tqlayout->addMultiCellWidget(line1, row, row, 0, 1);
		++row;
    
		TQLabel *l3 = new TQLabel(i18n("Here are the errors received:"), plainPage());
		tqlayout->addMultiCellWidget(l3, row, row, 0, 1);
		++row;
		KListBox *lb = new KListBox(plainPage());
		lb->insertStringList(stdErr);
		tqlayout->addMultiCellWidget(lb, row, row, 0, 1);
		++row;
	}

	// save checksum to disk, if any hashes are found
	KURLRequester *checksumFile=0;
	TQCheckBox *saveFileCb=0;
	if (successes) {
		TQHBoxLayout *htqlayout2 = new TQHBoxLayout(tqlayout, KDialogBase::spacingHint());
		saveFileCb = new TQCheckBox(i18n("Save checksum to file:"), plainPage());
		saveFileCb->setChecked(true);
		htqlayout2->addWidget(saveFileCb);

		checksumFile = new KURLRequester( suggestedFilename, plainPage() );
		htqlayout2->addWidget(checksumFile, TQt::AlignLeft);
		tqlayout->addMultiCellLayout(htqlayout2, row, row,0,1, TQt::AlignLeft);
		++row;
		connect(saveFileCb, TQT_SIGNAL(toggled(bool)), checksumFile, TQT_SLOT(setEnabled(bool)));
		checksumFile->setFocus();
	}
	
	TQCheckBox *onePerFile=0;
	if (stdOut.size() > 1 && standardFormat) {
		onePerFile = new TQCheckBox(i18n("Checksum file for each source file"), plainPage());
		onePerFile->setChecked(false);
		// clicking this, disables the 'save as' part
		connect(onePerFile, TQT_SIGNAL(toggled(bool)), saveFileCb, TQT_SLOT(toggle()));
		connect(onePerFile, TQT_SIGNAL(toggled(bool)), saveFileCb, TQT_SLOT(setDisabled(bool)));
		connect(onePerFile, TQT_SIGNAL(toggled(bool)), checksumFile, TQT_SLOT(setDisabled(bool)));
		tqlayout->addMultiCellWidget(onePerFile, row, row,0,1, TQt::AlignLeft);
		++row;
	}
	
	if (exec() == Accepted && successes) {
		if (stdOut.size()>1 && standardFormat && onePerFile->isChecked()) {
			savePerFile(stdOut, suggestedFilename.mid(suggestedFilename.findRev('.')));
		} else if (saveFileCb->isEnabled() && saveFileCb->isChecked() && !checksumFile->url().simplifyWhiteSpace().isEmpty()) {
			saveChecksum(stdOut, checksumFile->url());
		}
	}
}

bool ChecksumResultsDlg::saveChecksum(const TQStringList& data, TQString filename) {
	if (TQFile::exists(filename) &&
		KMessageBox::warningContinueCancel(this,
		i18n("File %1 already exists.\nAre you sure you want to overwrite it?").arg(filename),
		i18n("Warning"), i18n("Overwrite")) != KMessageBox::Continue) {
		// find a better name to save to
		filename = KFileDialog::getSaveFileName(TQString(), "*", 0, i18n("Select a file to save to"));
		if (filename.simplifyWhiteSpace().isEmpty()) return false;
	} 
	TQFile file(filename);
	if (!file.open(IO_WriteOnly)) {
		KMessageBox::detailedError(0, i18n("Error saving file %1").arg(filename),
			file.errorString());
		return false;
	}
	TQTextStream stream(&file);
	for ( TQStringList::ConstIterator it = data.constBegin(); it != data.constEnd(); ++it)
		stream << *it << "\n";
	file.close();
	return true;
}

void ChecksumResultsDlg::savePerFile(const TQStringList& data, const TQString& type) {
	krApp->startWaiting(i18n("Saving checksum files..."), 0);
	for ( TQStringList::ConstIterator it = data.begin(); it != data.end(); ++it ) {
			TQString line = (*it);
			TQString filename = line.mid(line.find(' ')+2)+type;
			if (!saveChecksum(*it, filename)) {
				KMessageBox::error(0, i18n("Errors occured while saving multiple checksums. Stopping"));
				krApp->stopWait();
				return;
			}
	}
	krApp->stopWait();
}
