/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKINTERVALDIALOG_H
#define ASKINTERVALDIALOG_H

#include <QDialog>

namespace Ui {
    class AskIntervalDialog;
}

class AskIntervalDialog : public QDialog {
    Q_OBJECT
public:
    AskIntervalDialog(QWidget *parent, QString title, int a, int b);
    ~AskIntervalDialog();
    int getA();
    int getB();

protected:
    void changeEvent(QEvent *e);
private slots:
    void validate();

private:
    Ui::AskIntervalDialog *ui;
};

#endif // ASKINTERVALDIALOG_H
