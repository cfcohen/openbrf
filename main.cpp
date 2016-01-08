/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <QtGui/QApplication>
#include "mainwindow.h"

static void showUsage(){
  system(
    "echo off&"
    "echo off&"
    "echo.usages: &"
    "echo.&"
    "echo.  OpenBRF &"
    "echo.     ...starts GUI&"
    "echo.&"
    "echo.  OpenBRF ^<file.brf^> &"
    "echo.     ...starts GUI, opens file.brf&"
    "echo.&"
    "echo.  OpenBRF --dump ^<module_path^> ^<file.txt^>&"
    "echo.     ...shell only, dumps objects names into file.txt&"
    "echo.&"
    "echo.&"
    "pause"
  );

}

extern char* applVersion;

int main(int argc, char* argv[])
{

  Q_INIT_RESOURCE(resource);

  QString nextTranslator;


  char* argv_2[]={"OpenBrf"}; int argc_2=1;

  QApplication app(argc_2,argv_2); //argc, argv);
  QStringList arguments = QCoreApplication::arguments();
  app.setApplicationVersion(applVersion);
  app.setApplicationName("OpenBrf");
  app.setOrganizationName("Marco Tarini");
  app.setOrganizationDomain("Marco Tarini");


  bool useAlphaC = false;

  if ((arguments.size()>1)&&(arguments[1].startsWith("-"))) {
    if ((arguments[1] == "--dump")&&(arguments.size()==4)) {
      switch (MainWindow().loadModAndDump(arguments[2],arguments[3])) {
      case -1: system("echo OpenBRF: invalid module folder & pause"); break;
      case -2: system("echo OpenBRF: error scanning brf data or ini file & pause"); break;
      case -3: system("echo OpenBRF: error writing output file & pause"); break;
      default: return 0;
      }
      return -1;
    } else if ((arguments[1] == "--useAlphaCommands")&&(arguments.size()==2))  {
      useAlphaC = true;
      arguments.clear();
    } else {
      showUsage();
      return -1;
    }

  }

  while (1){
    QTranslator translator;
    QTranslator qtTranslator;

    if (nextTranslator.isEmpty()){
      QString loc;
      switch (MainWindow::getLanguageOption()) {
      default: loc = QLocale::system().name(); break;
      case 1: loc = QString("en");break;
      case 2: loc = QString("zh_CN");break;
      case 3: loc = QString("es");break;
      case 4: loc = QString("de");break;
      }
      translator.load(QString(":/translations/openbrf_%1.qm").arg(loc));

      qtTranslator.load(QString(":/translations/qt_%1.qm").arg(loc));
    } else {
      translator.load(nextTranslator);
    }
    app.installTranslator(&translator);
    app.installTranslator(&qtTranslator);

    MainWindow w;
    w.setUseAlphaCommands(useAlphaC);
    w.show();

    if (arguments.size()>1) w.loadFile(arguments[1]); arguments.clear();
    if (app.exec()==101) {
      nextTranslator = w.getNextTranslatorFilename();
      continue; // just changed language! another run
    }
    break;
  }

  return 0;
}

