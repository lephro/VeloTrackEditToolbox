#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <QObject>
#include <QMessageBox>
#include <QString>

class VeloToolkitException
{
public:
  VeloToolkitException(QString description = "") : description(description) {}
  ~VeloToolkitException() = default;

  void Message() const
  {
    QMessageBox::critical(nullptr, "Error:", description, QMessageBox::StandardButton::Ok);
  }

  operator QString() const
  {
    return description;
  }

private:
  const QString description;
};

class NoValidDatabasesFoundException : public VeloToolkitException
{
public:
  NoValidDatabasesFoundException() :
    VeloToolkitException("No valid databases found!") {}
};

class SQLErrorException : public VeloToolkitException
{
public:
  SQLErrorException(int errorCode) :
    VeloToolkitException(getErrorCodeDescription(errorCode)) {}

  int getSQLErrorCode() { return errorCode; }

private:
  QString getErrorCodeDescription(int errorCode);
  int errorCode;
};

#endif // EXCEPTIONS_H
