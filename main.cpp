#include <QCoreApplication>
#include <QtDebug>
#include <QCommandLineParser>
#include <QDir>
#include <QProcess>
#include <QHash>

void mountPartitions();
void unmount();
void search(QString dir, QTextStream* stream, int* count, int fast);
void analyzeFile(QString file, QTextStream* stream, int* count, int fast);
bool fileEntropy(QFile* file, int* total, int fast);
QString fileLength(int total);

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption helpOption(QStringList() << "h" << "help",
                                          QCoreApplication::translate("main", "Usage Help"));
    parser.addOption(helpOption);
    QCommandLineOption mountOption(QStringList() << "m" << "mount",
                                          QCoreApplication::translate("main", "Mount all partitions"));
    parser.addOption(mountOption);
    QCommandLineOption searchOption(QStringList() << "s" << "search",
                                          QCoreApplication::translate("main", "Search for encrypted files"));
    parser.addOption(searchOption);
    QCommandLineOption fastOption(QStringList() << "f" << "fast",
                                          QCoreApplication::translate("main", "Read a maximum of 512 bytes from each file"));
    parser.addOption(fastOption);
    QCommandLineOption targetOutputOption(QStringList() << "o" << "output",
                QCoreApplication::translate("main", "Output file."),
                QCoreApplication::translate("main", "file"));
    parser.addOption(targetOutputOption);
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "dir" << "directory",
                QCoreApplication::translate("main", "Specify <directory>."),
                QCoreApplication::translate("main", "directory"));
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    QStringList args = parser.optionNames();
    if(args.contains("h") || args.contains("help"))
    {
        qDebug() << "Usage:\n-m\t--mount\t\t\tMount all partitions\n-d\t--dir\t--directory\tDirectory to search (\"~/dev\" by default)\n-s\t--search\t\tSearch for encrypted files\n-o\t--output\t\tOutput file (\"output.txt\" by default)\n-f\t--fast\t\t\tRead a maxiumum of 512 bytes from each file";
        return 0;
    }
    int mounted = 0;
    if(args.contains("m")  || args.contains("mount"))
    {
        qDebug() << "Mounting partitions...";
        mountPartitions();
        mounted = 1;
    }
    QString dir;
    if(args.contains("d")  || args.contains("dir") || args.contains("directory"))
    {
        dir=parser.value(targetDirectoryOption);
        QDir root(dir);
        if(!root.exists()){
            qDebug() << "Invalid directory: " << dir;
            return 0;
        }
    }else{
        dir = "/root/dev";
    }
    QString file;
    if(args.contains("o") || args.contains("output"))
    {
        file=parser.value(targetOutputOption);
    }
    else
    {
        file="output.txt";
    }
    int fast=0;
    if(args.contains("f") || args.contains("fast"))
    {
        qDebug() << "fast";
        fast=1;
    }
    if(args.contains("s")  || args.contains("search"))
    {
        qDebug() << "Searching encrypted files...";
        QFile output(file);
        int count=0;
        output.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream stream(&output);
        search(dir, &stream, &count, fast);
        qDebug() << "Found " << count << " encrypted files.";
    }

    if(mounted == 1)
    {
        qDebug() << "Unmounting partitions...";
        unmount();
    }

    return 0;//a.exec();
}

void mountPartitions()
{
    if (QProcess::execute("chmod +x script.sh") >= 0)
    {
        if (QProcess::execute(QString("/bin/sh") + " ./script.sh") < 0)
        {
            qDebug() << "Failed to run mount script.";
        }
        else
        {
            qDebug() << "Mounted at \"~/dev\".";
        }
    }
}

void unmount()
{
    if (QProcess::execute(QString("/bin/sh") + " ./umount.sh") < 0){
        qDebug() << "Failed to run unmount script.";
    }else{
        qDebug() << "Unmounted all partitions.";
    }
}

void search(QString dir, QTextStream* stream, int* count, int fast)
{
    QDir root(dir);
    QList<QString> dirs;
    foreach(QFileInfo file, root.entryInfoList())
    {
        if(!root.absolutePath().contains(file.absoluteFilePath()))
        {
            if(file.isDir())
            {
                dirs.append(file.absoluteFilePath());
            }
            else
            {
                analyzeFile(file.absoluteFilePath(), stream, count, fast);
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d, stream, count, fast);
    }
}

void analyzeFile(QString file, QTextStream* stream, int* count, int fast)
{
    QFile fileToCheck(file);
    fileToCheck.open(QIODevice::ReadOnly);
    int total=0;
    if(fileEntropy(&fileToCheck, &total, fast))
    {
        (*count)++;
        qDebug() << total;
        if(fast==0)
        {
            *stream << file + ": encrypted with " + fileLength(total) + " block size." << endl;
            qDebug() << file + ": encrypted with " + fileLength(total) + " block size.";
        }
        else
        {
            *stream << file << endl;
            qDebug() << file;
        }
    }
}

bool fileEntropy(QFile* file, int* total, int fast)
{
    if(file->size() < 128)
    {
        return false;
    }
    QHash<char, int> count;
    while(!file->atEnd())
    {
        QByteArray read = file->read(1);
        count[read[0]]++;
        (*total)++;
        if(fast == 1 && (*total) >= 512)
        {
            break;
        }
    }
    //Menos de 128 caracteres diferentes não interessa? Evitar falsos positivos
    if(count.size() < 128)
    {
        return false;
    }
    QHashIterator<char, int> i(count);
    float avg=0;
    while (i.hasNext()) {
        i.next();
        avg+=i.value();
    }
    avg/=(*total);
    i.toFront();
    int tooFar=0;
    while (i.hasNext())
    {
        i.next();
        //qDebug() << i.key() << ": " << i.value();
        if(i.value() < avg*0.5 || i.value() > avg*1.5)
        {
            tooFar++;
        }
    }
    if(tooFar > (*total)/2)
    {
        return false;
    }

    return true;
}

QString fileLength(int total)
{
   for(int i = 32; i <= 512; i+=32)
   {
       if(total%i == 0)
       {
           return QString::number(i) + " bit multiple";
       }
   }
   return "unknown";
}
