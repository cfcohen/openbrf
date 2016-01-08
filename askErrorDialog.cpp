#include <QListView.h>
#include <QTextBrowser.h>
#include <QLabel.h>


#include "askErrorDialog.h"
#include "brfData.h"
#include "iniData.h"

class ErrListModel: public QAbstractListModel{
public:

  ErrListModel():QAbstractListModel(0){}
  int rowCount(const QModelIndex& i) const {return 5;}
  QVariant data(const QModelIndex& i, int role)const{
    if (role == Qt::DisplayRole) {
      return QString("<b>Cazz</b>!");
    }
    if (role == Qt::DecorationRole) {
      return QVariant();
    }
    return QVariant();
  }


};


void AskErrorDialog::setup(){
  setBaseSize(300,240);
  //QListView* lv= new QListView(this);
  //model = new ErrListModel();
  //setWindowTitle("BrfEdit - errors in module set");
  //lv->setModel(model);

  //lv->setGeometry(0,0,100,100);
  //this->layout();
  te = new QTextBrowser(this);
  te->setReadOnly(true);
  te->setGeometry(20,20,280,200);
  //QLabel *label = new QLabel(tr("porcamadonna"),this);
  //te->openLinks(true);
  inidata->findErrors();
  te->setText(inidata->errorList.join("<p>"));

  connect(te,SIGNAL( anchorClicked(QUrl)),
          this, SLOT(linkClicked(QUrl)) );

  i = j = kind = -1;
  resize(320, 240);
}

AskErrorDialog::AskErrorDialog(QWidget* parent, IniData &i):QDialog(parent)
{
  inidata = &i;
  setup();
}

void AskErrorDialog::linkClicked(const QUrl&l){
  QString s = l.toString();
  sscanf(s.toAscii().data(), "#%d.%d.%d",&i, &j, &kind);
  accept();
}

AskErrorDialog::~AskErrorDialog(){
 //delete model;
}
