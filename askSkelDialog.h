/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKSKELDIALOG_H
#define ASKSKELDIALOG_H

#include <QDialog>
#include <vector>

class BrfSkeleton;
namespace Ui {
    class AskSkelDialog;
}

class AskSkelDialog : public QDialog {
    Q_OBJECT
public:
    AskSkelDialog(QWidget *parent, const std::vector<BrfSkeleton> &sv, int fr, int to, int res, int meth);
    ~AskSkelDialog();
    int getSkelFrom() const;
    int getSkelTo() const;
    int getOutputType() const;
    int getMethodType() const;

private slots:
		//void onChangeSkelTo() const;
protected:
    void changeEvent(QEvent *e);

private:
    Ui::AskSkelDialog *m_ui;
};

#endif // ASKSKELDIALOG_H
