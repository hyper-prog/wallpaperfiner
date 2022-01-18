/*
  Wallpaper Finer

  Webpage: http://hyperprog.com
  (C) 2012-2022 Peter Deak  (hyper80@gmail.com)

  License: GPL v2.
*/

#include <QtCore>
#include <QtGui>

#include <stdio.h>

#include "wpfiner.h"
#include "ui_wpfinermw.h"

#ifdef __WIN32
#include <windows.h>
#endif

#ifdef Q_WS_PM
  #define INCL_WINWORKPLACE
  #include <os2.h>
  #include <os2emx.h>
#endif

QRectF minimizeToFit(QRectF outer,QRectF inner,double *ratio=NULL)
{
    double x,y,w,h;

    double r,rh,rw;

    x = outer.x();
    y = outer.y();
    rw = inner.width() / outer.width();
    rh = inner.height() / outer.height();

    r = rw > rh ? rw : rh;

    w = inner.width()  / r;
    h = inner.height() / r;
    x += (outer.width() - w) / 2;
    y += (outer.height() - h) / 2;

    if(ratio != NULL)
        *ratio = r;

    return QRectF(x,y,w,h);
}

WpFinerMainWidget::WpFinerMainWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WpFinerMainWidget)
{
    int idx;
    rh = 0; rw = 0;
    currentFileNamePath = "";
    scaledown = true;
    save_format = "JPEG";

    ui->setupUi(this);

    appsettings = new QSettings("hyperprog.com","WallpaperFiner");

    disp_res.push_back(DispRes(640,480));
    disp_res.push_back(DispRes(768,1024));
    disp_res.push_back(DispRes(800,600));
    disp_res.push_back(DispRes(1024,600));
    disp_res.push_back(DispRes(1024,768));
    disp_res.push_back(DispRes(1093,614));
    disp_res.push_back(DispRes(1152,864));
    disp_res.push_back(DispRes(1280,720));
    disp_res.push_back(DispRes(1280,768));
    disp_res.push_back(DispRes(1280,800));
    disp_res.push_back(DispRes(1280,960));
    disp_res.push_back(DispRes(1280,1024));
    disp_res.push_back(DispRes(1311,737));
    disp_res.push_back(DispRes(1360,768));
    disp_res.push_back(DispRes(1360,1024));
    disp_res.push_back(DispRes(1366,768));
    disp_res.push_back(DispRes(1400,1050));
    disp_res.push_back(DispRes(1440,900));
    disp_res.push_back(DispRes(1600,900));
    disp_res.push_back(DispRes(1600,1200));
    disp_res.push_back(DispRes(1680,1050));
    disp_res.push_back(DispRes(1920,1050));
    disp_res.push_back(DispRes(1920,1080));
    disp_res.push_back(DispRes(1920,1200));
    disp_res.push_back(DispRes(2048,1152));
    disp_res.push_back(DispRes(2560,1440));
    disp_res.push_back(DispRes(2560,1600));
    disp_res.push_back(DispRes(3440,1440));
    disp_res.push_back(DispRes(3840,2160));

    QList<DispRes>::iterator i=disp_res.begin();
    while(i != disp_res.end())
    {
        ui->cb->addItem(QString("%1 x %2 - (%3)").arg(i->w).arg(i->h).arg((double)i->w/(double)i->h));
        ++i;
    }

    connect(ui->frame,SIGNAL(fileLoaded(QString,int,int)),this,SLOT(slotLoaded(QString,int,int)));
    connect(ui->cb,SIGNAL(currentIndexChanged(int)),this,SLOT(slotResChanged(int)));
    connect(this,SIGNAL(signalResChanged(int,int)),ui->frame,SLOT(slotResChanged(int,int)));
    connect(ui->toWapplapierButton,SIGNAL(clicked()),this,SLOT(slotToDesktop()));
    connect(menuBar(),SIGNAL(triggered(QAction*)),this,SLOT(slotMenuAction(QAction*)));

    QRect screengeom;
    QList<QScreen *> screens = QGuiApplication::screens();
    if(screens.count() > 0)
        screengeom = screens[0]->geometry();

    dw = screengeom.width();
    dh = screengeom.height();
    for(idx=0;idx<disp_res.count();++idx)
    {
        if(disp_res[idx].w == dw && disp_res[idx].h == dh)
            ui->cb->setCurrentIndex(idx);
    }

    statuslabel = new QLabel(this);
    statusBar()->addWidget(statuslabel);

    QMenu *fm = menuBar()->addMenu(tr("File"));
    QMenu *sm = menuBar()->addMenu(tr("Settings"));
    QMenu *im = menuBar()->addMenu(tr("Info"));

    fm->addAction(QIcon(":/pixmaps/openicon.png"),tr("Open image"));
    fm->addAction(QIcon(":/pixmaps/saveicon.png"),tr("Save image"));
    fm->addSeparator();
    fm->addAction(QIcon(":/pixmaps/exiticon.png"),tr("Exit"),this,SLOT(close()));
    im->addAction(QIcon(":/pixmaps/infoicon.png"),tr("Author"));

    m_scaled = sm->addAction(tr("Scale down to desktop size"));
    m_jpg_f  = sm->addAction(tr("Save in JPEG format (Small size)"));
    m_bmp_f  = sm->addAction(tr("Save in BMP format (Win XP)"));
    m_png_f  = sm->addAction(tr("Save in PNG format (Lossless)"));

    m_scaled->setCheckable(true);
    m_jpg_f->setCheckable(true);
    m_bmp_f->setCheckable(true);
    m_png_f->setCheckable(true);

#ifdef __WIN32
    switch(QSysInfo::windowsVersion())
    {
        case QSysInfo::WV_NT:
        case QSysInfo::WV_2000:
        case QSysInfo::WV_XP:
        case QSysInfo::WV_2003:
                save_format = "BMP";
            break;
        case QSysInfo::WV_VISTA:
        case QSysInfo::WV_WINDOWS7:
        default:
                save_format = "JPEG";
            break;
    }
#endif

#ifdef Q_WS_PM
    save_format = "BMP";
#endif


    if(appsettings->contains("DownscaleImages"))
        scaledown = appsettings->value("DownscaleImages").toBool();
    if(appsettings->contains("ImageFormat"))
    {
        QString saved_f;
        saved_f = appsettings->value("ImageFormat").toString();
        if(saved_f == "JPEG" || saved_f == "BMP" || saved_f == "PNG")
            save_format = saved_f;
    }

    m_scaled->setChecked(scaledown);
    m_jpg_f->setChecked( save_format == "JPEG" ? true : false );
    m_bmp_f->setChecked( save_format == "BMP" ? true : false );
    m_png_f->setChecked( save_format == "PNG" ? true : false );

}

int WpFinerMainWidget::slotMenuAction(QAction *a)
{
    if(a->text() == tr("Open image"))
        slotOpen();

    if(a->text() == tr("Save image"))
        slotSave();

    if(a->text() == tr("Author"))
        slotAbout();

    if(a->text() == tr("Save in JPEG format (Small size)"))
    {
        if(a->isChecked())
            save_format = "JPEG";
    }

    if(a->text() == tr("Save in BMP format (Win XP)"))
    {
        if(a->isChecked())
            save_format = "BMP";
    }

    if(a->text() == tr("Save in PNG format (Lossless)"))
    {
        if(a->isChecked())
            save_format = "PNG";
    }

    if(a->text() == tr("Scale down to desktop size"))
        scaledown = a->isChecked();

    m_scaled->setChecked(scaledown);
    m_jpg_f->setChecked( save_format == "JPEG" ? true : false );
    m_bmp_f->setChecked( save_format == "BMP" ? true : false );
    m_png_f->setChecked( save_format == "PNG" ? true : false );

    return 0;
}

int WpFinerMainWidget::slotAbout(void)
{
    QMessageBox::about(this,tr("Author"),
                       QString(tr("<strong>Wallpaper Finer</strong><br/><br/>Version: %1<br/>GPL v2<br/><br/>Author: P&eacute;ter De&aacute;k<br/>Webpage & Contact: <a href=\"http://hyperprog.com\">http://hyperprog.com</a>"))
                            .arg(VERSION));
    return 0;
}

void WpFinerMainWidget::closeEvent(QCloseEvent *ce)
{
    if(appsettings->isWritable())
    {
        appsettings->setValue("DownscaleImages",scaledown);
        appsettings->setValue("ImageFormat",save_format);
    }
    QMainWindow::closeEvent(ce);
}

WpFinerMainWidget::~WpFinerMainWidget()
{
    delete appsettings;
    delete ui;
}

int WpFinerMainWidget::slotResChanged(int idx)
{
    rw = disp_res[idx].w;
    rh = disp_res[idx].h;
    emit signalResChanged(rw,rh);
    return 0;
}

int WpFinerMainWidget::slotLoaded(QString fnamepath, int w, int h)
{
    currentFileNamePath = fnamepath;
    iw = w;
    ih = h;
    statuslabel->setText(
                    QString("%1 (%2 x %3 - %4) ")
                        .arg(fnamepath)
                        .arg(iw)
                        .arg(ih)
                        .arg((double)iw/(double)ih));
    return 0;
}

int WpFinerMainWidget::slotOpen(void)
{
    QString fname;
    fname = QFileDialog::getOpenFileName(this,tr("Image to fine"),"","Image (*.jpg *.jpeg *.png *.bmp)");
    if(!fname.isEmpty())
        ui->frame->slotLoadFile(fname);
    return 0;
}

int WpFinerMainWidget::slotSave(void)
{
    if(currentFileNamePath.isEmpty())
        return 0;

    QString ff="";
    QString fname="";

    if(save_format == "JPEG")
        ff = tr("Jpeg file (*.jpg)");
    if(save_format == "PNG")
        ff = tr("Png file (*.png)");
    if(save_format == "BMP")
        ff = tr("Bmp file (*.bmp)");

    fname = QFileDialog::getSaveFileName(this,tr("Fined wallpaier file to save"),"",ff);
    if(!fname.isEmpty())
        saveFinedImage(fname);
    return 0;
}


int WpFinerMainWidget::saveFinedImage(QString filename)
{
    if(currentFileNamePath.isEmpty())
        return 0;

    QFile finedFile;
    finedFile.setFileName(filename);
    if(finedFile.exists())
    {
        QMessageBox::warning(this,tr("Warning"),tr("The file has already existed!"));
        return 0;
    }

    QFile sourceFile(currentFileNamePath);
    QImage sourceImage;
    QImage trimmed,finedImage;
    if(sourceFile.open(QIODevice::ReadOnly))
    {
        sourceImage.loadFromData(sourceFile.readAll());
        sourceFile.close();
    }
    else
    {
        QMessageBox::critical(this,tr("Error"),QString(tr("I can't load the file: %1")).arg(currentFileNamePath));
        return 0;
    }

    //Let's do the essential work!
    int x,y,w,h;
    x = ui->frame->getRPX() * ui->frame->getRatio();
    y = ui->frame->getRPY() * ui->frame->getRatio();
    w = ui->frame->getRDW() * ui->frame->getRatio();
    h = ui->frame->getRDH() * ui->frame->getRatio();

    trimmed = sourceImage.copy(x,y,w,h);
    if(scaledown)
        finedImage = trimmed.scaled(rw,rh,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
    else
        finedImage = trimmed;

    //Save the file
    if(finedFile.open(QIODevice::WriteOnly))
    {
        finedImage.save(&finedFile,save_format.toLocal8Bit().data());
        finedFile.close();
    }
    else
    {
        QMessageBox::critical(this,tr("Error"),QString(tr("I can't write the file: %1")).arg(filename));
        return 0;
    }

    return 0;
}

int WpFinerMainWidget::slotToDesktop(void)
{
    if(currentFileNamePath.isEmpty())
        return 0;

    QDir homedir(QDir::homePath());
    QDir wpdir(QDir::homePath() + "/WpFiner/");
    if(!wpdir.exists())
        homedir.mkdir("WpFiner");
    if(!wpdir.exists())
    {
        QMessageBox::warning(this,tr("Error"),QString(tr("I can't create \"WpFiner\" directory in your home.")));
        return 0;
    }

    //Let's generate an unique filename
    QString finedPath;
    QFile wpfile;
    int seq=1;
    do
    {
        finedPath = wpdir.absolutePath() + "/" +
                    QString("WpFiner_%1_%2x%3.%4")
                            .arg(seq)
                            .arg(rw)
                            .arg(rh)
                            .arg(save_format.toLower().replace("jpeg","jpg"));
        wpfile.setFileName(finedPath);
        ++seq;
    }
    while(wpfile.exists());

    saveFinedImage(finedPath);
    slotSetWmWallpaper(finedPath);
    return 0;
}

void WpFinerMainWidget::slotSetWmWallpaper(QString filepath)
{
#ifdef __WIN32
    QSettings appSettings( "HKEY_CURRENT_USER\\Control Panel\\Desktop", QSettings::NativeFormat);
    //QString dispName = appSettings.value("Wallpaper").toString();

    filepath.replace("/","\\");
    //Set new background path
    appSettings.setValue("Wallpaper", filepath);
    appSettings.setValue("WallpaperStyle","0");
    appSettings.setValue("TileWallpaper","0");
    QByteArray ba = filepath.toLatin1();
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)ba.data(), SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);
#endif

#ifdef Q_WS_X11
    //On ubuntu gnome...
    QString program="gsettings";
    QStringList arguments;
    arguments << "set" << "org.gnome.desktop.background" << "picture-uri" << "file://"+filepath;
    QProcess *setBGProcess = new QProcess(this);
    setBGProcess->start(program,arguments);
    if(!setBGProcess->waitForFinished(-1))
        QMessageBox::information(this,tr("Your wallpaier"),QString(tr("Sorry, I can\'t set the desktop wallpaier.\nI saved the file here: %1")).arg(filepath));
    delete setBGProcess;

#endif

#ifdef Q_WS_PM
    QFileInfo bgfile(filepath);

    HOBJECT hobj;
    const char *objName = "<WP_DESKTOP>";
    if (hobj = WinQueryObject(reinterpret_cast< const unsigned char * > (objName)))
        WinSetObjectData(hobj,reinterpret_cast< const unsigned char * > ((QString("BACKGROUND=%1,S,1,I;").arg(QDir::toNativeSeparators(bgfile.absoluteFilePath()))).toAscii().constData()));
#endif

}

// ///////////////////////////////////////////////////////////////////////////////////////

MyDropFrame::MyDropFrame(QWidget *parent,Qt::WindowFlags f)
    :QFrame(parent,f)
{
    myimage = NULL;

    setAcceptDrops(true);
    label = new QLabel(this);
    label->setText(tr("Drop your image here to fit as desktop wallpaper..."));
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->addStretch();
    lay->addWidget(label);
    lay->addStretch();
    rpX = 0;
    rpY = 0;
    ratio = 1.0;
    isHoover = false;
}

void MyDropFrame::dragEnterEvent(QDragEnterEvent *ee)
{
   if(ee->mimeData()->hasUrls())
             ee->acceptProposedAction();
}

void MyDropFrame::dropEvent(QDropEvent *de)
{
    if(de->mimeData()->hasUrls())
    {
        QString path;
        de->acceptProposedAction();

        path = QUrl::fromPercentEncoding(de->mimeData()->data("text/uri-list"));
        path = path.replace("file://","").trimmed();
        //QMessageBox::information(this,"The dropped file",path);

#ifdef __WIN32
        if(path.startsWith("/"))
            path = path.remove(0,1);
#endif
        slotLoadFile(path);
    }
}

int MyDropFrame::slotLoadFile(QString filename)
{
    QFile f(filename);
    if(f.open(QIODevice::ReadOnly))
    {
        if(myimage != NULL)
        {
            delete myimage;
            myimage = NULL;
        }

        QImageReader imgreader;
        imgreader.setDevice(&f);
        if(!imgreader.canRead())
        {
            f.close();
            QMessageBox::warning(this,tr("Error"),
                                 QString(tr("Sorry, It seems I can't read this file (not supported format?): %1\n%2"))
                                    .arg(filename)
                                    .arg(imgreader.errorString()));
            return 0;
        }

        myimage = new QImage(imgreader.read());
        if(myimage->isNull())
        {
            QMessageBox::warning(this,tr("Error"),
                                 QString(tr("Sorry, Image reading was failed: %1"))
                                    .arg(imgreader.errorString()));
            f.close();
            return 0;
        }

        f.close();

        label->setText("");
        if(isHoover)
            setCursor(Qt::OpenHandCursor);
        emit fileLoaded(filename,myimage->width(),myimage->height());
    }
    else
        QMessageBox::warning(this,tr("Error"),QString(tr("I can't load the file: %1")).arg(filename));

    update();
    return 0;
}

int MyDropFrame::slotResChanged(int w,int h)
{
    desktopW = w;
    desktopH = h;
    update();
    return 0;
}

void MyDropFrame::drawClipper(QPainter *p,int x,int y,int w,int h)
{
    p->setPen(QColor(255,0,0));
    p->drawLine(x,y,x+w,y);
    p->drawLine(x+w,y,x+w,y+h);
    p->drawLine(x+w,y+h,x,y+h);
    p->drawLine(x,y+h,x,y);
    p->setPen(QColor(255,255,255));
    x++; y++; w-=2; h-=2;
    p->drawLine(x,y,x+w,y);
    p->drawLine(x+w,y,x+w,y+h);
    p->drawLine(x+w,y+h,x,y+h);
    p->drawLine(x,y+h,x,y);
}

void MyDropFrame::paintEvent(QPaintEvent *pe)
{
    double diffw,diffh;

    QPainter p(this);
    QRectF desktop(0,0,desktopW,desktopH);
    QRectF drawarea(1,1,width()-2,height()-2);
    p.setPen(Qt::SolidLine);
    p.setPen(QColor(0,0,0));
    p.drawRect(drawarea);

    if(myimage != NULL)
    {
        QRectF picsarea(2,2,width()-3,height()-3);
        QRectF source(0,0,myimage->width(),myimage->height());

        QRectF pics = minimizeToFit(picsarea,source,&ratio);

        p.drawImage(pics,*myimage,source);

        QRectF d = minimizeToFit(pics,desktop);

        diffh = pics.height() - d.height();
        if(rpY < 0)
            rpY = 0;
        if(rpY > diffh)
            rpY = diffh;

        diffw = pics.width() - d.width();
        if(rpX < 0)
            rpX = 0;
        if(rpX > diffw)
            rpX = diffw;

        rdW = d.width();
        rdH = d.height();

        drawClipper(&p,pics.x()+rpX , pics.y()+rpY , d.width() , d.height() );

        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(5,5,5,170),Qt::SolidPattern));

        p.drawRect(pics.x(),pics.y(),rpX,pics.height());
        p.drawRect(pics.x(),pics.y(),pics.width(),rpY);
        p.drawRect(pics.x()+rdW+rpX+1,pics.y(),diffw-rpX,pics.height());
        p.drawRect(pics.x(),pics.y()+rdH+rpY+1,pics.width(),diffh-rpY);
    }
    p.end();
    QFrame::paintEvent(pe);
}

void MyDropFrame::enterEvent(QEvent *e)
{
    isHoover = true;
    if(myimage != NULL)
        setCursor(Qt::OpenHandCursor);
    QFrame::enterEvent(e);
}

void MyDropFrame::leaveEvent(QEvent *e)
{
    isHoover = false;
    setCursor(Qt::ArrowCursor);
    QFrame::leaveEvent(e);
}

void MyDropFrame::mousePressEvent(QMouseEvent *mpe)
{
    oldX = mpe->x();
    oldY = mpe->y();

    setCursor(Qt::ClosedHandCursor);
}

void MyDropFrame::mouseMoveEvent(QMouseEvent *mme)
{
    rpX -= oldX-mme->x();
    rpY -= oldY-mme->y();
    oldX = mme->x();
    oldY = mme->y();

    update();
}

void MyDropFrame::mouseReleaseEvent(QMouseEvent *mre)
{
    rpX -= oldX-mre->x();
    rpY -= oldY-mre->y();
    oldX = mre->x();
    oldY = mre->y();

    setCursor(Qt::OpenHandCursor);
    update();
}

// end code.
