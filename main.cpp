/*
  Wallpaper Finer

  Webpage: http://hyperprog.com
  (C) 2012-2022 Peter Deak (hyper80@gmail.com)

  License: GPL v2.
*/

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QApplication>

#include "wpfiner.h"

#define INSTALLED_LANGNUM  3
char langs[INSTALLED_LANGNUM][3][30] = {
    {"English"  ,"en",""},
    {"Hungarian","hu",":/wpfiner_hu.qm"},
    {"Polish"   ,"pl",":/wpfiner_pl.qm"}
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef _WIN32

    QTranslator qtTranslator;

    QString loadLang="";
    QSettings settings("hyperprog.com","WallpaperFiner");
    if(settings.contains("ProgramLanguage"))
    {
        int i;
        QString lang = settings.value("ProgramLanguage").toString();
        for(i = 0 ; i < INSTALLED_LANGNUM ; ++i)
        {
            if(QString(langs[i][1]) == lang)
                loadLang = langs[i][2];
        }
    }
    else
    {
        if(settings.isWritable())
        {
            int i;
            QString selected;

            QDialog *langSelectDialog = new QDialog(0);
            QComboBox *langSelect = new QComboBox(langSelectDialog);
            QPushButton *langOk = new QPushButton("Ok",langSelectDialog);
            QVBoxLayout *lay = new QVBoxLayout(langSelectDialog);
            lay->addWidget(langSelect);
            lay->addWidget(langOk);
            langSelectDialog->setWindowTitle("Select language");
            langSelectDialog->setWindowIcon(QIcon(":/pixmaps/wpfiner_small.png"));
            for(i = 0 ; i < INSTALLED_LANGNUM ; ++i)
                langSelect->addItem(langs[i][0]);
            langSelectDialog->connect(langOk,SIGNAL(clicked()),langSelectDialog,SLOT(accept()));
            langSelectDialog->resize(300,60);

            while(!langSelectDialog->exec());
            selected = langSelect->currentText();
            delete langSelectDialog;

            for(i = 0 ; i < INSTALLED_LANGNUM ; ++i)
                if(selected == QString(langs[i][0]))
                {
                        settings.setValue("ProgramLanguage",QString(langs[i][1]));
                        loadLang = langs[i][2];
                }
        }
    }

    //Load the translation
    if(!loadLang.isEmpty())
    {
        qtTranslator.load(loadLang);
        a.installTranslator(&qtTranslator);
    }

#else
    int i;
    QTranslator qtTranslator;
    for(i = 0 ; i < INSTALLED_LANGNUM ; ++i)
        if(QLocale::system().name().left(2).toLower() == QString(langs[i][1]))
            qtTranslator.load(langs[i][2]);
    a.installTranslator(&qtTranslator);
#endif

    WpFinerMainWidget w;
    w.show();
    
    return a.exec();
}
