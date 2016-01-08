/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "askCreaseDialog.h"
#include "ui_askCreaseDialog.h"

AskCreaseDialog::AskCreaseDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AskCreaseDialog)
{
    m_ui->setupUi(this);
    setModal(true);//Qt::WindowModal
}

AskCreaseDialog::~AskCreaseDialog()
{
    delete m_ui;
}

QSlider* AskCreaseDialog::slider() {return m_ui->slider; }
QCheckBox* AskCreaseDialog::checkbox() {return m_ui->seamsCB; }


void AskCreaseDialog::changeEvent(QEvent *e)
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
