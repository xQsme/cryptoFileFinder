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
    void setStuff(QString dir, QString file, int toUnmount);
    void search();
private:
    QString dir;
    QString file;
    QFile* output;
    QCoreApplication* app;
    int totalThreads=0;
    int endedThreads=0;
    int count=0;
    int toUnmount;
signals:

public slots:
    void ended(int count);
};

#endif // SEARCH_H
