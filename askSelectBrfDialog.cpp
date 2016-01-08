/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "askSelectBrfDialog.h"
#include "ui_askSelectBrfDialog.h"

#include <QModelIndex>


AskSelectBRFDialog::AskSelectBRFDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskSelectBRFDialog)
{
    ui->setupUi(this);

    connect(ui->countUsed,SIGNAL(clicked()),this,SLOT(countUsed()));
    connect(ui->refresh,SIGNAL(clicked()),this,SLOT(refresh()));

    connect(ui->listMod  ,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(clickedOnList(QModelIndex)));
    connect(ui->listUnref,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(clickedOnList(QModelIndex)));
    connect(ui->listComm ,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(clickedOnList(QModelIndex)));
}

int AskSelectBRFDialog::doExec(){
  ui->listMod->addItems(name[0]);
  ui->listMod->setProperty("id",0);
  ui->listUnref->addItems(name[1]);
  ui->listUnref->setProperty("id",1);
  ui->listComm->addItems(name[2]);
  ui->listComm->setProperty("id",2);
  loadMe.clear();
  setResult(0);
  return QDialog::exec();
}

AskSelectBRFDialog::~AskSelectBRFDialog()
{
    delete ui;
}

void AskSelectBRFDialog::countUsed(){
  setResult(1);
  loadMe = QString("???1");
  close();
}

void AskSelectBRFDialog::refresh(){
  setResult(1);
  loadMe = QString("???2");
  close();
}

QPushButton* AskSelectBRFDialog::openModuleIniButton(){
  return ui->openModuleIni;
}

void AskSelectBRFDialog::clickedOnList(QModelIndex i){
  QObject* s = sender();
  if (s) {
    QVariant v = s->property("id");
    if (v.isValid()) {
      int k = v.toInt();
      if (k>=0 && k<3) {
        loadMe = path[k][i.row()];
        setResult(0);
        close();
      }
    }
  }
}

void AskSelectBRFDialog::changeEvent(QEvent *e)
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

void AskSelectBRFDialog::addName(int k, QString _name, QString _path){
  name[k].append(_name);
  path[k].append(_path);

}
