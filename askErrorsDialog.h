#ifndef ASKERRORSDIALOG_H
#define ASKERRORSDIALOG_H

#include <QtGui/QDialog>
#include <QUrl>

class IniData;
class ErrListModel;
class QTextBrowser;

namespace Ui {
    class AskErrorsDialog;
}

class AskErrorsDialog : public QDialog {
    Q_OBJECT
public:
    AskErrorsDialog(QWidget *parent , IniData &i);
    ~AskErrorsDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AskErrorsDialog *m_ui;
    ErrListModel *model;
    QTextBrowser *te;

protected slots:
    void linkClicked(const QUrl&l);
};

#endif // ASKERRORSDIALOG_H
