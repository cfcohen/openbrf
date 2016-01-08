/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKSELECTBRFDIALOG_H
#define ASKSELECTBRFDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
    class AskSelectBRFDialog;
}

class QModelIndex;
class QPushButton;
class AskSelectBRFDialog : public QDialog {
    Q_OBJECT
public:
    AskSelectBRFDialog(QWidget *parent = 0);
    ~AskSelectBRFDialog();
    void addName(int k, QString name, QString path);
    QString loadMe;
    int doExec();
    QPushButton* openModuleIniButton();

private slots:
    void clickedOnList(QModelIndex q);
    void countUsed();
    void refresh();
protected:
    void changeEvent(QEvent *e);
    QStringList name[3];
    QStringList path[3];

private:
    Ui::AskSelectBRFDialog *ui;
};

#endif // ASKSELECTBRFDIALOG_H
