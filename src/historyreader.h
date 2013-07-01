#ifndef HISTORYREADER_H
#define HISTORYREADER_H

#include <QObject>

class HistoryReader : public QObject
{
    Q_OBJECT
public:
    explicit HistoryReader(QObject *parent = 0) : QObject(parent) {}
    virtual ~HistoryReader() {}
};

#endif
