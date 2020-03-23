#include "opentrackdialog.h"
#include "ui_opentrackdialog.h"

OpenTrackDialog::OpenTrackDialog(QWidget *parent, VeloDb* productionDb, VeloDb* betaDb, VeloDb* customDb) noexcept(false) :
  QDialog(parent),
  ui(new Ui::OpenTrackDialog)
{
  this->productionDb = productionDb;
  this->betaDb = betaDb;
  this->customDb = customDb;

  ui->setupUi(this);

  if (productionDb->isValid())
    ui->databaseComboBox->insertItem(0, tr("Production"));

  if (betaDb->isValid())
    ui->databaseComboBox->insertItem(1, tr("Beta"));

  if (customDb->isValid())
    ui->databaseComboBox->insertItem(2, tr("Custom"));

  if (ui->databaseComboBox->count() == 0)
    throw NoValidDatabasesFoundException();

  ui->databaseComboBox->setVisible(ui->databaseComboBox->count() > 1); 
}

OpenTrackDialog::~OpenTrackDialog()
{
  delete ui;
}

void OpenTrackDialog::on_databaseComboBox_currentIndexChanged(const QString &arg1)
{
  ui->trackListWidget->clear();

  VeloDb* selectedDb = nullptr;
  if (arg1 == tr("Production")) {
    selectedDb = productionDb;
  } else if (arg1 == tr("Beta")) {
    selectedDb = betaDb;
  } else if (arg1 == tr("Custom")) {
    selectedDb = customDb;
  }

  if (selectedDb == nullptr)
    return;

  selectedDb->queryAll();
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
    track = selectedItems.first()->data(Qt::UserRole).value<Track>();

  return track;
}

void OpenTrackDialog::loadDatabase(VeloDb* database)
{
  database->queryAll();
  foreach(Track track, *database->getTracks()) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(track.name);

    QVariant var;
    var.setValue(track);
    item->setData(Qt::UserRole, var);

    ui->trackListWidget->addItem(item);
  }
}



