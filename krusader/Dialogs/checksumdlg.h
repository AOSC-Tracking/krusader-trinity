#ifndef CHECKSUMDLG_H
#define CHECKSUMDLG_H

#include <kdialogbase.h>
#include <tqvaluelist.h>

class KTempFile;
extern void initChecksumModule();

class CreateChecksumDlg: public KDialogBase {
public:
	CreateChecksumDlg(const TQStringList& files, bool containFolders, const TQString& path);

private:
	KTempFile *tmpOut, *tmpErr;
};


class MatchChecksumDlg: public KDialogBase {
public:
	MatchChecksumDlg(const TQStringList& files, bool containFolders, 
		const TQString& path, const TQString& checksumFile=TQString());

	static TQString checksumTypesFilter;

protected:
	bool verifyChecksumFile(TQString path, TQString& extension);

private:
	KTempFile *tmpOut, *tmpErr;
};


class ChecksumResultsDlg: public KDialogBase {
public:
	ChecksumResultsDlg(const TQStringList& stdOut, const TQStringList& stdErr,
		const TQString& suggestedFilename, const TQString& binary, const TQString& type,
		bool standardFormat);

protected:
	bool saveChecksum(const TQStringList& data, TQString filename);
	void savePerFile(const TQStringList& data, const TQString& type);
	
private:
	TQString _binary;
};


class VerifyResultDlg: public KDialogBase {
public:
	VerifyResultDlg(const TQStringList& failed);
};

#endif // CHECKSUMDLG_H
