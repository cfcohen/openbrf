/* OpenBRF -- by marco tarini. Provided under GNU General Public License */


#include "tablemodel.h"
#include <QColor>
#include <QApplication>
#include <QPalette>
#include <QFont>

MyTableModel::MyTableModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

void MyTableModel::updateChanges(){
  int t=vec.size();
  emit(this->dataChanged(createIndex(0,0),createIndex(1,t+100)));
  emit(layoutChanged());
}

void MyTableModel::clear()
{
  int t=vec.size();
  vec.clear();
  vecUsed.clear();
  this->dataChanged(createIndex(0,0),createIndex(1,t));
}

int MyTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return vec.size();
}

int MyTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

Qt::DropActions MyTableModel::supportedDropActions() const{
    return Qt::CopyAction | Qt::MoveAction;
}

QVariant MyTableModel::data(const QModelIndex &index, int role) const
{
  static QFont alternate;
  static bool firstTime=true;
  if (firstTime) {
    alternate= QApplication::font();
    //alternate.setItalic(!alternate.italic());
    alternate.setBold(true);
    firstTime = false;
  }
  int i = index.row();
  if (i>=(int)vecUsed.size()) i = vecUsed.size()-1;
  if (i<0) {
    return QVariant();
  }
  if (role==Qt::DisplayRole)
    return vec[ i ];
  if (role==Qt::FontRole){
    return (vecUsed[ i ]!=1)?QApplication::font():alternate;
  }
  if (role==Qt::BackgroundColorRole) return QColor(255,255,255,255);
   // //return (index.row()%2==0)?QColor(128,128,128,255):QColor(0,0,0,255);
   // return (vecUsed[ index.row() ]!=0)?
    //    QApplication::palette().color (QPalette::Base):
    //    QApplication::palette().color (QPalette::AlternateBase);
  if (role==Qt::TextColorRole) {
    switch(vecUsed[ i ]){
    case 1: return QColor(0,0,150,255);
    case 0: return QColor(0,0,0,255);//QApplication::palette().color(QPalette::Text);
    case -1:return QColor(40,50,40,255);
    case -2:return QColor(140,150,140,255);
    //return QApplication::palette().color(QPalette::Text);
    }
  }
  return QVariant();
}

QVariant MyTableModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{
  return tr("HEADER");

  //  return QVariant();
}




