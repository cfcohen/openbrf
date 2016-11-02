/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKTEXTURENAMEDIALOG_H
#define ASKTEXTURENAMEDIALOG_H

#include <QDialog>

namespace Ui {
    class askTexturenameDialog;
}

class AskTexturenameDialog : public QDialog {
    Q_OBJECT
public:
    AskTexturenameDialog(QWidget *parent , QString letAlsoAdd);
    ~AskTexturenameDialog();

    void setDef(QString s);
    void setLabel(QString s);
    void setBrowsable(QString s);
    void setRes(QStringList &);
    QStringList getRes() const;
    bool alsoAdd();
protected:
    QString path;
    void changeEvent(QEvent *e);
    static bool lastAlsoAdd;

private:
    Ui::askTexturenameDialog *m_ui;

private slots:
    void on_pushButton_clicked();
};

#endif // ASKTEXTURENAMEDIALOG_H
