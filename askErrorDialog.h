#ifndef ASKERRORDIALOG_H
#define ASKERRORDIALOG_H

#include <QDialog>
#include <QUrl>

class IniData;
class ErrListModel;
class QTextBrowser;

class AskErrorDialog : public QDialog
{
    Q_OBJECT
public:
    AskErrorDialog(QWidget* parent, IniData &i);
    ErrListModel *model;
    ~AskErrorDialog();

    int i, j, kind; // file i, object j, of kind kind

    IniData *inidata;
    QTextBrowser *te;
protected slots:
    void linkClicked(const QUrl &l);
    void setup();
};

#endif // ASKERRORDIALOG_H
