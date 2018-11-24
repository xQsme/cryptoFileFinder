#include "thread.h"

Thread::Thread(int thread, int totalThreads, QString dir, QTextStream* stream)
{
    this->thread=thread;
    this->totalThreads=totalThreads;
    this->dir=dir;
    this->stream=stream;
    currentFile=0;
    count=0;
}

void Thread::run()
{
    search(dir);
    emit ended(count);
}

void Thread::search(QString dir)
{
    QDir root(dir);
    QList<QString> dirs;
    foreach(QFileInfo file, root.entryInfoList())
    {
        if(!root.absolutePath().contains(file.absoluteFilePath()))
        {
            if(file.isDir() && file.absoluteFilePath().left(2)!="/.")
            {
                dirs.append(file.absoluteFilePath());
            }
            else
            {
                currentFile++;
                if(currentFile % totalThreads == thread)
                {
                    analyzeFile(file.absoluteFilePath());
                }
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d);
    }
}

void Thread::analyzeFile(QString file)
{
    QFile fileToCheck(file);
    fileToCheck.open(QIODevice::ReadOnly);
    if(fileToCheck.size() < 32)
    {
        return;
    }
    total=0;
    QHash<char, long> data;
    while(!fileToCheck.atEnd())
    {
        QByteArray read = fileToCheck.read(1);
        data[read[0]]++;
        total++;
    }
    float entropy = fileEntropy(data);
    if(entropy > 7)
    {
        float chi2 = calculateChi2(data);
        if(chi2 < 350)
        {
            float piError = approximatePi(&fileToCheck);
            if(piError < 0.18 && piError > 0.13)
            {
                QString command = fileCommand(file);
                /*if(command.contains("enc'd") || command.contains("encrypted") || (command.contains("data") && !command.contains("image") %% !command.contains("archive"))
                {*/
                    count++;
                    qDebug() << "Size:" << fileToCheck.size();
                    *stream << fileToCheck.size() << ";";
                    qDebug() << "Entropy:" << entropy;
                    *stream << entropy << ";";
                    qDebug() << "Chi^2:" << chi2;
                    *stream << chi2 << ";";
                    qDebug() << "Pi error: " << piError;
                    *stream << piError << ";";
                    qDebug() << "Command: " << command;
                    *stream << command << ";";
                //}
            }
        }
    }
    fileToCheck.close();
}

float Thread::fileEntropy(QHash<char, long> data)
{

    QHashIterator<char, long> i(data);
    float entropy=0;
    int max=0;
    while (i.hasNext())
    {
        i.next();
        float current = 1.0*i.value()/total;
        entropy -= current*log2(current);
        if(i.value() > max){
            max = i.value();
        }
    }
    return entropy;
}

float Thread::calculateChi2(QHash<char, long> data){
    float avg = total/data.keys().length();
    QHashIterator<char, long> i(data);
    float chi2=0;
    while (i.hasNext())
    {
        i.next();
        chi2+=(i.value()-avg)*(i.value()-avg)/avg;
    }
    return chi2;
}

float Thread::approximatePi(QFile* file)
{
    file->reset();
    int value;
    int iterations=3;
    int nSuccess=0;
    int count=0;
    while(!file->atEnd())
    {
        value=0;
        for(int i = 0; i < iterations; i++)
        {
            if(file->atEnd())
            {
                break;
            }
            value += file->read(1)[0];
        }
        count++;
        if(value <= 128 && value >= -128){
            nSuccess++;
        }
    }
    return abs((1.0*nSuccess/count-M_PI_4)/M_PI_4);
}

QString Thread::fileCommand(QString file)
{
    QProcess process;
    QStringList args;
    args << file;
    process.start("file", args);
    process.waitForFinished();
    QString output(process.readAllStandardOutput());
    return output;
}
