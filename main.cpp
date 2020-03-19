#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include "sqlite3.h"

int main(int argc, char* argv[])
{
  //QCoreApplication::setOrganizationName("VeloTrackEditToolbox");
  //QCoreApplication::setApplicationName("VeloTrackEditToolbox");
  //QSettings::setDefaultFormat(QSettings::IniFormat);
  QDir settingsDir;
  //QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "settings.ini");

  QApplication a(argc, argv);
  MainWindow w;

  w.show();

  return a.exec();
}
