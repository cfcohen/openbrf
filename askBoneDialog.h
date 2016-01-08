/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKBONEDIALOG_H
#define ASKBONEDIALOG_H

#include "brfSkeleton.h"
#include "carryPosition.h"

#include <QtGui/QDialog>
#include <vector>

namespace Ui {
    class AskBoneDialog;
}

class BrfSkeleton;
class AskBoneDialog : public QDialog {
    Q_OBJECT
public:
    AskBoneDialog(QWidget *parent, const std::vector<BrfSkeleton> &skel
		              ,const std::vector<CarryPosition> &cp);
    ~AskBoneDialog();
    void setSkeleton(const BrfSkeleton &s);
    int getSkel() const;
    int getBone() const;
		int getCarryPos() const;
    bool pieceAtOrigin() const;
    void sayNotRigged(bool say);

protected:
    void changeEvent(QEvent *e);
    //const std::vector<BrfSkeleton> &sv;
    std::vector<BrfSkeleton> sv;
		std::vector<CarryPosition> carrypos;

protected slots:
    void onSelectSkel(int i);
		void onSelectCarryPos(int i);
		void onSelectBone(int i);

private:
    Ui::AskBoneDialog *ui;
};

#endif // ASKBONEDIALOG_H
