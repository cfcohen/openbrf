/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "askIntervalDialog.h"
#include "ui_askIntervalDialog.h"
#include <QPushButton>

AskIntervalDialog::AskIntervalDialog(QWidget *parent, QString title, int a, int b) :
    QDialog(parent),
    ui(new Ui::AskIntervalDialog)
{
  ui->setupUi(this);
  ui->title->setText(title);
  ui->from->setText(QString::number(a));
  ui->to->setText(QString::number(b));
  validate();
  connect(ui->from, SIGNAL(textEdited(QString)), this, SLOT(validate()));
  connect(ui->to, SIGNAL(textEdited(QString)), this, SLOT(validate()));

}

void AskIntervalDialog::validate(){
  int a = ui->from->text().toInt();
  int b = ui->to->text().toInt();
  ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(a<b);
}

int AskIntervalDialog::getA(){
  return ui->from->text().toInt();
}

int AskIntervalDialog::getB(){
  return ui->to->text().toInt();
}

AskIntervalDialog::~AskIntervalDialog()
{
    delete ui;
}

void AskIntervalDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
