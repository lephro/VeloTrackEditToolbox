#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <QObject>
#include <QMessageBox>
#include <QString>

class VeloToolkitException
{
public:
  VeloToolkitException(const QString description = "") : description(description) {}
  ~VeloToolkitException() = default;

  virtual void Message() const
  {
    QMessageBox::critical(nullptr, "An error occured", description, QMessageBox::StandardButton::Ok);
  }

  operator QString() const
  {
    return description;
  }

protected:
  const QString description;
};

class InvalidTrackException : public VeloToolkitException
{
public:
  InvalidTrackException() :
    VeloToolkitException("The track is invalid") {}
};

class NoDatabasesFileNameException : public VeloToolkitException
{
public:
  NoDatabasesFileNameException() :
    VeloToolkitException("Cannot access the database! No filename was set.") {}
};

class NoValidDatabasesFoundException : public VeloToolkitException
{
public:
  NoValidDatabasesFoundException() :
    VeloToolkitException("No valid databases found!") {}
};

class ProtectedTrackException : public VeloToolkitException
{
public:
  ProtectedTrackException() :
    VeloToolkitException("The track is protected!") {}
};

class SQLErrorException : public VeloToolkitException
{
public:
  SQLErrorException(int errorCode, QString externalDesc = "") :
    VeloToolkitException(getErrorCodeDescription(errorCode)),
    externalDescription(externalDesc) {}

  void Message() const override { QMessageBox::critical(nullptr, "An error occured", description + "\nMessage: " + getExternalErrorCodeDescription(), QMessageBox::StandardButton::Ok); }

  QString getExternalErrorCodeDescription() const { return externalDescription; }
  int getSQLErrorCode() const { return errorCode; }

private:
  int errorCode;
  QString getErrorCodeDescription(int errorCode);
  QString externalDescription;
};

class TrackDoesNotBelongToDatabaseException : public VeloToolkitException
{
public:
  TrackDoesNotBelongToDatabaseException() :
    VeloToolkitException("The track does not belong to the database") { }
};

class PrefabNotEditableException : public VeloToolkitException
{
public:
  PrefabNotEditableException(const uint prefabId, const QString prefabName) :
    VeloToolkitException(QString("The prefab %1 (Id: %1) is not editable").arg(prefabId).arg(prefabName)) {}
};


#endif // EXCEPTIONS_H
