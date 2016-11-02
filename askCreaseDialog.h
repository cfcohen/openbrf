/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKCREASEDIALOG_H
#define ASKCREASEDIALOG_H

#include <QDialog>

namespace Ui {
    class AskCreaseDialog;
}
class QSlider;
class QCheckBox;

class AskCreaseDialog : public QDialog {
    Q_OBJECT
public:
    AskCreaseDialog(QWidget *parent = 0);
    ~AskCreaseDialog();
    QSlider* slider();
    QCheckBox* checkbox();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AskCreaseDialog *m_ui;
};

#endif // ASKCREASEDIALOG_H
