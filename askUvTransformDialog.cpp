#include "askUvTransformDialog.h"
#include "ui_askUvTransformDialog.h"

AskUvTransformDialog::AskUvTransformDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskUvTransformDialog)
{
    ui->setupUi(this);

    connect(ui->invertY,SIGNAL(clicked()),this,SLOT(setInvertY()));
    connect(ui->Reset,SIGNAL(clicked()),this,SLOT(reset()));
    connect(ui->traU,SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
    connect(ui->traV,SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
    connect(ui->scaleU,SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
    connect(ui->scaleV,SIGNAL(valueChanged(double)), this, SIGNAL(changed()));
}

void AskUvTransformDialog::getData(float &su, float &sv, float &tu, float &tv){
  su = ui->scaleU->value()*0.01;
  sv = ui->scaleV->value()*0.01;
  tu = ui->traU->value();
  tv = ui->traV->value();

}

void AskUvTransformDialog::reset(){
  ui->scaleU->setValue(100);
  ui->scaleV->setValue(100);
  ui->traU->setValue(0);
  ui->traV->setValue(0);
}

void AskUvTransformDialog::setInvertY(){
  ui->scaleV->setValue(-100);
  ui->traV->setValue(1);
}

AskUvTransformDialog::~AskUvTransformDialog()
{
    delete ui;
}
