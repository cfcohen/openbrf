/* OpenBRF -- by marco tarini. Provided under GNU General Public License */


#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <vector>


#include <QApplication>
#include <QDoubleSpinBox>

class MyDoubleSpinBox : public QDoubleSpinBox{
public:
  MyDoubleSpinBox(QWidget * parent = 0):QDoubleSpinBox(parent){}

  void stepBy(int steps){
    double oldStep = singleStep();
    if (QApplication::keyboardModifiers()&(Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier))
    setSingleStep( oldStep/4.0 );
    QDoubleSpinBox::stepBy(steps);
    setSingleStep( oldStep );
  }
};


//! [0]
class MyTableModel : public QAbstractListModel
{
    Q_OBJECT
    
public:
    MyTableModel(QObject *parent=0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    Qt::DropActions supportedDropActions() const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    std::vector<QString> vec;
    std::vector<int> vecUsed;
    void clear();
    int size() const {return (int)vec.size();}
    QModelIndex pleaseCreateIndex(int a, int b){
      return createIndex(a,b);
    }
    void updateChanges();
private:
};
//! [0]

#endif
