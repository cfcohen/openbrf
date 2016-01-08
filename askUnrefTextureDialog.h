/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKUNREFTEXTUREDIALOG_H
#define ASKUNREFTEXTUREDIALOG_H

#include <QDialog>

class QListWidgetItem;

namespace Ui {
    class AskUnrefTextureDialog;
}

class AskUnrefTextureDialog : public QDialog {
    Q_OBJECT
public:
    AskUnrefTextureDialog(QWidget *parent = 0);
    ~AskUnrefTextureDialog();
    void addFile(QString f);
    QString texturePath;

private slots:
    void refresh();
    void openTexture(QListWidgetItem* i);
    void moveAllToUnused();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AskUnrefTextureDialog *ui;
};

#endif // ASKUNREFTEXTUREDIALOG_H
