/*
  Wallpaper Finer

  Webpage: http://hyperprog.com
  (C) 2012-2022 Peter Deak (hyper80@gmail.com)

  License: GPL v2.
*/

#ifndef WPFINER_H
#define WPFINER_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#define VERSION "0.95"

namespace Ui {
class WpFinerMainWidget;
}

class DispRes
{
public:
    int w,h;
    DispRes(int w,int h) { this->w = w; this->h = h; }
};

class QLabel;
class WpFinerMainWidget : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit WpFinerMainWidget(QWidget *parent = 0);
    ~WpFinerMainWidget();

public slots:
    int slotLoaded(QString fnamepath,int w,int h);
    int slotResChanged(int idx);

    int slotToDesktop(void);
    int saveFinedImage(QString filename);

    void slotSetWmWallpaper(QString filepath);

    int slotMenuAction(QAction *a);
    int slotOpen(void);
    int slotSave(void);

    int slotAbout(void);

signals:
    void signalResChanged(int w,int h);

protected:
    void closeEvent(QCloseEvent *ce);

private:
    QSettings *appsettings;
    QString save_format;
    bool scaledown;
    QList<DispRes> disp_res;
    Ui::WpFinerMainWidget *ui;
    QString currentFileNamePath;
    int dw,dh; //desktop dimension
    int iw,ih; //image dimension
    int rw,rh; //requested dimension
    QLabel *statuslabel;

    QAction *m_scaled,*m_jpg_f,*m_bmp_f,*m_png_f; //adjustable menu actions
};

class MyDropFrame : public QFrame
{
    Q_OBJECT

public:
    MyDropFrame(QWidget *parent,Qt::WindowFlags f = Qt::WindowFlags());

private:
    double ratio,rdW,rdH;
    int oldX,oldY;
    int rpX,rpY;
    int desktopW,desktopH;
    QLabel *label;
    QImage *myimage;
    bool isHoover;

public:
    double getRatio(void) { return ratio; }
    int getRPX(void) { return rpX; }
    int getRPY(void) { return rpY; }
    double getRDW(void) { return rdW; }
    double getRDH(void) { return rdH; }

private:
    void drawClipper(QPainter *p,int x,int y,int w,int h);

signals:
    void fileLoaded(QString fnamepath,int w,int h);

public slots:
    int slotResChanged(int w,int h);
    int slotLoadFile(QString filename);

protected:
    void dragEnterEvent(QDragEnterEvent *ee);
    void dropEvent(QDropEvent *de);
    void paintEvent(QPaintEvent *pe);

    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void mousePressEvent(QMouseEvent *mpe);
    void mouseMoveEvent(QMouseEvent *mme);
    void mouseReleaseEvent(QMouseEvent *mre);

};

#endif // WPFINER_H
