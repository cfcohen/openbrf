#include "askErrorsDialog.h"
#include "ui_askErrorsDialog.h"

AskErrorsDialog::AskErrorsDialog(QWidget *parent, IniData &i) :
    QDialog(parent),
    m_ui(new Ui::AskErrorsDialog)
{
  m_ui->setupUi(this);


  setBaseSize(300,240);
  //QListView* lv= new QListView(this);
  //model = new ErrListModel();
  setWindowTitle("BrfEdit - errors in module set");
  //lv->setModel(model);

  //lv->setGeometry(0,0,100,100);
  //this->layout();
  te = m_ui->textBrowser;
  te->setReadOnly(true);
  te->setText("<b>Bold</b> and a <a href=\"#1.1.1\">link</a>");
  //te->setGeometry(20,20,280,200);
  //QLabel *label = new QLabel(tr("porcamadonna"),this);
  //te->openLinks(true);

  connect(te,SIGNAL( anchorClicked(QUrl)),
          this, SLOT(linkClicked(QUrl)) );

  //resize(320, 240);

}

AskErrorsDialog::~AskErrorsDialog()
{
    delete m_ui;
}

void AskErrorsDialog::changeEvent(QEvent *e)
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

void AskErrorsDialog::linkClicked(const QUrl&l){
  accept();
}

