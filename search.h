#ifndef SEARCH_H
#define SEARCH_H

#include <QObject>
#include <QtCore>
#include "thread.h"

class Search : public QObject
{
    Q_OBJECT
public:
    explicit Search(QObject *parent = nullptr);
    void setStuff(QString dir, QString file, int fast, int bytes);
    void search();
private:
    QString dir;
    QString file;
    int totalThreads=0;
    int endedThreads=0;
    int count=0;
    int fast;
    int bytes;
signals:

public slots:
    void ended(int count);
};

#endif // SEARCH_H
