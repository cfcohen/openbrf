/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <QPushButton>
#include "askLodOptionsDialog.h"
#include "ui_askLodOptionsDialog.h"

AskLodOptionsDialog::AskLodOptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskLodOptionsDialog)
{
    ui->setupUi(this);
    QPushButton* b =  this->ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(
      b,SIGNAL(clicked()),
      this,SLOT(restoreDefaults())
    );
    restoreDefaults();
}

void AskLodOptionsDialog::getData(float* perc, bool* yesno, bool *overRide) const{
  yesno[0]=ui->checkBox_1->isChecked();
  yesno[1]=ui->checkBox_2->isChecked();
  yesno[2]=ui->checkBox_3->isChecked();
  yesno[3]=ui->checkBox_4->isChecked();
  perc[0]=(float)ui->doubleSpinBox_1->value();
  perc[1]=(float)ui->doubleSpinBox_2->value();
  perc[2]=(float)ui->doubleSpinBox_3->value();
  perc[3]=(float)ui->doubleSpinBox_4->value();
  *overRide = ui->overrideYes->isChecked();
}

void AskLodOptionsDialog::setData(const float* perc, const bool* yesno, bool overRide){
  ui->checkBox_1->setChecked(yesno[0]);
  ui->checkBox_2->setChecked(yesno[1]);
  ui->checkBox_3->setChecked(yesno[2]);
  ui->checkBox_4->setChecked(yesno[3]);
  ui->doubleSpinBox_1->setValue(perc[0]);
  ui->doubleSpinBox_2->setValue(perc[1]);
  ui->doubleSpinBox_3->setValue(perc[2]);
  ui->doubleSpinBox_4->setValue(perc[3]);
  ui->overrideNo->setChecked(!overRide);
  ui->overrideYes->setChecked(overRide);
}

void AskLodOptionsDialog::restoreDefaults(){
  ui->checkBox_1->setChecked(true);
  ui->checkBox_2->setChecked(true);
  ui->checkBox_3->setChecked(true);
  ui->checkBox_4->setChecked(true);
  ui->doubleSpinBox_1->setValue(50);
  ui->doubleSpinBox_2->setValue(25);
  ui->doubleSpinBox_3->setValue(12.5);
  ui->doubleSpinBox_4->setValue(6.25);
  ui->overrideNo->setChecked(false);
  ui->overrideYes->setChecked(true);
}

AskLodOptionsDialog::~AskLodOptionsDialog()
{
    delete ui;
}

void AskLodOptionsDialog::changeEvent(QEvent *e)
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
