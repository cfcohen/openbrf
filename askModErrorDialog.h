/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKMODERRORDIALOG_H
#define ASKMODERRORDIALOG_H

#include <QtGui/QDialog>
#include <QUrl>
class IniData;
class ErrListModel;
class QTextBrowser;

namespace Ui {
    class askModErrorDialog;
}

class AskModErrorDialog : public QDialog {
    Q_OBJECT
public:
    AskModErrorDialog(QWidget *parent,  IniData &i, bool search, QString searchSt);
    ~AskModErrorDialog();

    ErrListModel *model;
    int i, j, kind; // file i, object j, of kind kind
    int maxErr;
    IniData *inidata;
    QTextBrowser *te;
    QString searchString;
    bool searchCommonRes;
    int searchToken;
    void setOptions(bool, int, QString);
    void getOptions(bool *, int *, QString*);
public slots:
    void setup();


protected slots:
    void linkClicked(const QUrl &l);
    void performSearch();
    void performErrorSearch();
    void refresh();
    void moreErrors();
protected:
    void changeEvent(QEvent *e);
    void paintEvent ( QPaintEvent * event );

private:
    Ui::askModErrorDialog *m_ui;
    bool isSearch;
    void getIniDataReady();
    bool iniDataReady, firstPaintDone;
};

#endif // ASKMODERRORDIALOG_H
