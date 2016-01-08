#include <vector>

#include "askSkelPairDialog.h"
#include "ui_askSkelPairDialog.h"

AskSkelPairDialog::AskSkelPairDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::askSkelPairDialog)
{

  ui->setupUi(this);

  connect(ui->skelTo,SIGNAL(currentIndexChanged(int)),this,SLOT(update()));

}

int AskSkelPairDialog::exec(){
  //ui->skelTo->setCurrentIndex(1);
  update();
  return QDialog::exec();
}

void AskSkelPairDialog::update()
{

  ui->nboneFrom->display(numBoneInAni);

  int a=ui->skelTo->currentIndex();

  QVariant ud = ui->skelTo->itemData ( a,  Qt::UserRole );

  ui->nboneTo->display(ud.toSize().width());

}


int AskSkelPairDialog::skelFrom() const{
  QVariant ud = ui->skelFrom->itemData( ui->skelFrom->currentIndex() ,  Qt::UserRole );
  return ud.toSize().height();
}

int AskSkelPairDialog::skelTo() const{
  QVariant ud = ui->skelTo->itemData( ui->skelTo->currentIndex() ,  Qt::UserRole );
  return ud.toSize().height();
}

void AskSkelPairDialog::addSkeleton(QString name, int nbone, int index){
  QVariant data(QSize(nbone,index));
  if (nbone==numBoneInAni) ui->skelFrom->addItem(name,data);
  ui->skelTo->addItem(name,data);

}

AskSkelPairDialog::~AskSkelPairDialog()
{
    delete ui;
}
