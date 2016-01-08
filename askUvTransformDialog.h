#ifndef ASKUVTRANSFORMDIALOG_H
#define ASKUVTRANSFORMDIALOG_H

#include <QDialog>

namespace Ui {
    class AskUvTransformDialog;
}

class AskUvTransformDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AskUvTransformDialog(QWidget *parent = 0);
    ~AskUvTransformDialog();
    void getData(float& su, float& sy, float& tu, float& tv);
public slots:
    void setInvertY();
    void reset();
signals:
    void changed();
private:
    Ui::AskUvTransformDialog *ui;

};

#endif // ASKUVTRANSFORMDIALOG_H
