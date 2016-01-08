/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKHUESATBRIDIALOG_H
#define ASKHUESATBRIDIALOG_H

#include <QDialog>

namespace Ui {
    class Dialog;
}

class AskHueSatBriDialog : public QDialog {
    Q_OBJECT
public:
    AskHueSatBriDialog(QWidget *parent);
    ~AskHueSatBriDialog();

signals:
    void anySliderMoved(int, int, int, int);

private slots:
    void onAnySliderMove(int);

private:
    Ui::Dialog *ui;
};

#endif // ASKHUESATBRIDIALOG_H
