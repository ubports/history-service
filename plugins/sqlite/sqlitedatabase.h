#ifndef SQLITEDATABASE_H
#define SQLITEDATABASE_H

#include <QObject>
#include <QSqlDatabase>

class SQLiteDatabase : public QObject
{
    Q_OBJECT
public:
    static SQLiteDatabase *instance();

    bool initializeDatabase();
    QSqlDatabase database() const;

protected:
    bool createDatabase();

private:
    explicit SQLiteDatabase(QObject *parent = 0);
    QString mDatabasePath;
    QSqlDatabase mDatabase;
    
};

#endif // SQLITEDATABASE_H
