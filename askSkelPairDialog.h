#ifndef ASKSKELPAIRDIALOG_H
#define ASKSKELPAIRDIALOG_H

#include <QDialog>

namespace Ui {
    class askSkelPairDialog;
}

class AskSkelPairDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AskSkelPairDialog(QWidget *parent = 0);
    ~AskSkelPairDialog();

    void addSkeleton(QString name, int nbone, int index);

    int exec();

    int skelFrom() const;
    int skelTo() const;

    int numBoneInAni;

private slots:
    void update();

private:
    std::vector<int> nbones;
    Ui::askSkelPairDialog *ui;
};

#endif // ASKSKELPAIRDIALOG_H
