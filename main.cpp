/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <QtGui/QApplication>
#include "mainwindow.h"

static void showUsage(){
  printf("usage:\n");
  printf("OpenBRF\n");
  printf("  ... startsGUI\n");
  printf("OpenBRF <file.brf>\n");
  printf("  ... starts GUI, opens file.brf\n");
  printf("OpenBRF --dump <module_path> <file.txt>\n");
  printf("  ... shell only, dumps objects names into file.txt\n");
}

extern char* applVersion;

int main(int argc, char** argv)
{

  Q_INIT_RESOURCE(resource);

  QString nextTranslator;

  QApplication app(argc,argv); //argc, argv);
  QStringList arguments = QCoreApplication::arguments();
  app.setApplicationVersion(applVersion);
  app.setApplicationName("OpenBrf");
  app.setOrganizationName("Marco Tarini");
  app.setOrganizationDomain("Marco Tarini");


  bool useAlphaC = false;

  if ((arguments.size()>1)&&(arguments[1].startsWith("-"))) {
    if ((arguments[1] == "--dump")&&(arguments.size()==4)) {
      switch (MainWindow().loadModAndDump(arguments[2],arguments[3])) {
      case -1: printf("OpenBRF: invalid module folder\n"); break;
      case -2: printf("OpenBRF: error scanning brf data or ini file\n"); break;
      case -3: printf("OpenBRF: error writing output file\n"); break;
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

