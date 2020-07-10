#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QStyleFactory>
#include "sqlite3.h"

int main(int argc, char* argv[])
{
  //QCoreApplication::setOrganizationName("VeloTrackEditToolbox");
  //QCoreApplication::setApplicationName("VeloTrackEditToolbox");
  //QSettings::setDefaultFormat(QSettings::IniFormat);
  QDir settingsDir;
  //QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, "settings.ini");

  QApplication app(argc, argv);

#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
    if(settings.value("AppsUseLightTheme") == 0){
        app.setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        QColor darkColor = QColor(45,45,45);
        QColor disabledColor = QColor(127,127,127);
        darkPalette.setColor(QPalette::Window, darkColor);
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(18,18,18));
        darkPalette.setColor(QPalette::AlternateBase, darkColor);
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        darkPalette.setColor(QPalette::Button, darkColor);
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

        darkPalette.setColor(QPalette::Highlight, QColor(226, 128, 53));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);

        app.setPalette(darkPalette);
        app.setStyleSheet("QToolTip { color: #000000; background-color: #fecb89; border: 1px solid black; }");
    }
#endif

  MainWindow mainWindow;
  mainWindow.show();

  return app.exec();
}
