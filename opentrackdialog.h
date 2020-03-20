#ifndef OPENTRACKDIALOG_H
#define OPENTRACKDIALOG_H

#include <QDialog>
#include <QSettings>

#include "exceptions.h"
#include "velodb.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OpenTrackDialog; }
QT_END_NAMESPACE

class OpenTrackDialog : public QDialog
{
  Q_OBJECT

public:
  explicit OpenTrackDialog(QWidget *parent = nullptr, VeloDb* productionDb = nullptr, VeloDb* betaDb = nullptr, VeloDb* customDb = nullptr) noexcept(false);
  ~OpenTrackDialog();

  Track getSelectedTrack() const;

private slots:
  void on_databaseComboBox_currentIndexChanged(const QString &database);

private:
  Ui::OpenTrackDialog *ui;
  VeloDb* productionDb;
  VeloDb* betaDb;
  VeloDb* customDb;
  void loadDatabase(VeloDb* database);
};

#endif // OPENTRACKDIALOG_H
