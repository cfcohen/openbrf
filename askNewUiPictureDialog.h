/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKNEWUIPICTUREDIALOG_H
#define ASKNEWUIPICTUREDIALOG_H

#include <QDialog>

namespace Ui {
    class AskNewUiPictureDialog;
}

class AskNewUiPictureDialog : public QDialog {
    Q_OBJECT
public:
    AskNewUiPictureDialog(QWidget *parent = 0);
    ~AskNewUiPictureDialog();
    void setBrowsable(QString s);


public slots:
    void resetFullScreen();
    void resetNE(); void resetN(); void resetNW();
    void resetE();  void resetC(); void resetW();
    void resetSE(); void resetS(); void resetSW();


    void resetActualPixels();
    void updateMeasure();
    void browse();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::AskNewUiPictureDialog *ui;

    void onUiChanged();
    int toPixX(float x);
    int toPixY(float x);
    float toPerX(int x);
    float toPerY(int x);
    static bool measurePix;

    void accept();
    void showSizeAndPos();
    void center(float relposX, float relposY);

    QString path;

public:
    int pixSx, pixSy;
    QString ext;
    static int overlayMode;
    static float sx, sy, px, py;
    static int mode;
    static char name[255];
    static bool replace;
};

#endif // ASKNEWUIPICTUREDIALOG_H
