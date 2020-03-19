#include "opentrackdialog.h"
#include "ui_opentrackdialog.h"

OpenTrackDialog::OpenTrackDialog(QWidget *parent, VeloDb* productionDb, VeloDb* betaDb, VeloDb* customDb) :
  QDialog(parent),
  ui(new Ui::OpenTrackDialog)
{
  this->productionDb = productionDb;
  this->betaDb = betaDb;
  this->customDb = customDb;

  if ((!productionDb->isValid()) && (!betaDb->isValid()) && (!customDb->isValid())) {
    throw "ToDo: Invalid Confiugration Exception";
  }

  ui->setupUi(this);

  if (productionDb->isValid())
    ui->databaseComboBox->insertItem(0, tr("Production"));

  if (betaDb->isValid())
    ui->databaseComboBox->insertItem(1, tr("Beta"));

  if (customDb->isValid())
    ui->databaseComboBox->insertItem(2, tr("Custom"));

  ui->databaseComboBox->setVisible(ui->databaseComboBox->count() > 1); 
}

OpenTrackDialog::~OpenTrackDialog()
{
  delete ui;
}

void OpenTrackDialog::on_databaseComboBox_currentIndexChanged(const QString &database)
{
  ui->trackListWidget->clear();

  VeloDb* selectedDb = nullptr;
  if (database == tr("Production")) {
    selectedDb = productionDb;
  } else if (database == tr("Beta")) {
    selectedDb = betaDb;
  } else if (database == tr("Custom")) {
    selectedDb = customDb;
  }

  if (selectedDb == nullptr)
    return;

  selectedDb->queryTracks();
  foreach(Track track, *selectedDb->getTracks()) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(track.name);

    QVariant var;
    var.setValue(track);
    item->setData(Qt::UserRole, var);

    ui->trackListWidget->addItem(item);
  }
}

Track OpenTrackDialog::getSelectedTrack() const
{
  Track track;
  QList<QListWidgetItem*> selectedItems = ui->trackListWidget->selectedItems();
  if (selectedItems.size() > 0)
    track = selectedItems.value(0)->data(Qt::UserRole).value<Track>();

  return track;
}

void OpenTrackDialog::loadDatabase(VeloDb* database)
{
  database->queryTracks();
  foreach(Track track, *database->getTracks()) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(track.name);

    QVariant var;
    var.setValue(track);
    item->setData(Qt::UserRole, var);

    ui->trackListWidget->addItem(item);
  }
}
