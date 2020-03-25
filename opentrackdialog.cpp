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

  ui->databaseLabel->setVisible(ui->databaseComboBox->count() > 1);
  ui->databaseComboBox->setVisible(ui->databaseComboBox->count() > 1);
}

OpenTrackDialog::~OpenTrackDialog()
{
  delete ui;
}

void OpenTrackDialog::on_databaseComboBox_currentIndexChanged(const QString &arg1)
{
  ui->trackListTreeWidget->clear();

  VeloDb* selectedDb = nullptr;
  if (arg1 == tr("Production")) {
    selectedDb = productionDb;
  } else if (arg1 == tr("Beta")) {
    selectedDb = betaDb;
  } else if (arg1 == tr("Custom")) {
    selectedDb = customDb;
  }

  loadDatabase(selectedDb);
}

TrackData OpenTrackDialog::getSelectedTrack() const
{
  TrackData track;
  QList<QTreeWidgetItem*> selectedTableItems = ui->trackListTreeWidget->selectedItems();
  if (selectedTableItems.size() > 0)
    track = selectedTableItems.first()->data(TrackTreeColumns::NameColumn, Qt::UserRole).value<TrackData>();

  return track;
}

void OpenTrackDialog::loadDatabase(VeloDb* database)
{
  if (database == nullptr)
    return;

  database->queryAll();

  int row = 0;
  foreach(TrackData track, *database->getTracks()) {
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();
    trackItem->setText(0, track.name);

    for (QVector<SceneData>::iterator i = database->getScenes()->begin(); i != database->getScenes()->end(); ++i) {
      if (i->id == track.sceneId) {
        trackItem->setText(TrackTreeColumns::SceneColumn, i->title);
        break;
      }
    }

    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    ui->trackListTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->trackListTreeWidget->setHeaderLabels(labels);
  ui->trackListTreeWidget->header()->setSectionResizeMode(TrackTreeColumns::NameColumn, QHeaderView::ResizeToContents);
}



