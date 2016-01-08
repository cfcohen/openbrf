/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "askSkelDialog.h"
#include "ui_askSkelDialog.h"

#include "brfSkeleton.h"

AskSkelDialog::AskSkelDialog(QWidget *parent, const std::vector<BrfSkeleton> &sv, int fr, int to, int out, int method) :
    QDialog(parent),
    m_ui(new Ui::AskSkelDialog)
{
  m_ui->setupUi(this);

  for (int i=0; i<(int)sv.size(); i++)
  {
    m_ui->cbSkelFrom->addItem( sv[i].name );
    m_ui->cbSkelTo->addItem( sv[i].name );
  }
	if (sv.size()>1) m_ui->cbSkelTo->addItem( tr("any other skeleton") );

  m_ui->radioButtonA0->setChecked(method==0);
  m_ui->radioButtonA1->setChecked(method==1);
  m_ui->radioButtonB0->setChecked(out==0);
  m_ui->radioButtonB1->setChecked(out==1);
  m_ui->radioButtonB2->setChecked(out==2);
  m_ui->cbSkelFrom->setCurrentIndex(fr);
  m_ui->cbSkelTo->setCurrentIndex(to);

}

int AskSkelDialog::getSkelFrom() const{
  return m_ui->cbSkelFrom->currentIndex();
}
int AskSkelDialog::getSkelTo() const{
  return m_ui->cbSkelTo->currentIndex();
}
int AskSkelDialog::getMethodType() const{
  return m_ui->radioButtonA1->isChecked();
}
int AskSkelDialog::getOutputType() const{
  return (m_ui->radioButtonB2->isChecked())?2:m_ui->radioButtonB1->isChecked();
}

AskSkelDialog::~AskSkelDialog()
{
    delete m_ui;
}

void AskSkelDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
