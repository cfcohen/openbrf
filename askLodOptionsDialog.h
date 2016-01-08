/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKLODOPTIONSDIALOG_H
#define ASKLODOPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
    class AskLodOptionsDialog;
}

class QAbstractButton;

class AskLodOptionsDialog : public QDialog {
    Q_OBJECT
public:
    AskLodOptionsDialog(QWidget *parent = 0);
    ~AskLodOptionsDialog();
    void getData(float* perc, bool* yesno, bool *overRide) const;
    void setData(const float* perc, const bool* yesno, bool overRide);

protected:
    void changeEvent(QEvent *e);
protected slots:
    void restoreDefaults();


private:
    Ui::AskLodOptionsDialog *ui;
};

#endif // ASKLODOPTIONSDIALOG_H
