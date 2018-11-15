#include <QCoreApplication>
#include <QtDebug>
#include <QCommandLineParser>
#include <QDir>
#include <QProcess>
#include <QHash>

void mountPartitions();
void unmount();
void search(QString dir, QTextStream* stream, int* count);
void analyzeFile(QString file, QTextStream* stream, int* count);
bool fileEntropy(QFile* file);
QString fileLength(QFile* file);

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
        qDebug() << "Usage:\n-m\t--mount\t\t\tMount all partitions\n-d\t--dir\t--directory\tDirectory to search (\"~/dev\" by default)\n-s\t--search\t\tSearch for encrypted files\n-o\t--output\t\tOutput file (\"output.txt\" by default)";
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
    if(args.contains("s")  || args.contains("search"))
    {
        qDebug() << "Searching encrypted files...";
        QFile output(file);
        int count=0;
        output.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream stream(&output);
        search(dir, &stream, &count);
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

void search(QString dir, QTextStream* stream, int* count)
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
                analyzeFile(file.absoluteFilePath(), stream, count);
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d, stream, count);
    }
}

void analyzeFile(QString file, QTextStream* stream, int* count)
{
    QFile fileToCheck(file);
    fileToCheck.open(QIODevice::ReadOnly | QIODevice::Text);
    if(fileEntropy(&fileToCheck))
    {
        (*count)++;
        *stream << file << ": encrypted with " << fileLength(&fileToCheck) << " cypher." << endl;
        qDebug() << file << ": encrypted with " << fileLength(&fileToCheck) << " cypher.";
    }
}

bool fileEntropy(QFile* file)
{
    QTextStream stream(file);
    QHash<QString, int> count;
    for(int i = 65; i <= 90; i++)
    {
        count.insert(QByteArray::fromHex(QString::number(i).toLocal8Bit()), 0);
    }
    while(!stream.atEnd())
    {
        count[stream.read(1).toLower()]++;
    }
    QHashIterator<QString, int> i(count);
    int avg=0;
    int total=0;
    while (i.hasNext()) {
        i.next();
        avg+=i.value();
        total++;
    }
    avg/=total;
    i.toFront();
    int tooFar=0;
    while (i.hasNext())
    {
        i.next();
        if(i.value() < avg*0.5 || i.value() > avg*1.5)
        {
            tooFar++;
        }
    }
    if(tooFar > total/5)
    {
        return false;
    }
    return true;
}

QString fileLength(QFile* file)
{
   if(file->size()%256 == 0)
   {
       return "256 bit";
   }
   if(file->size()%192 == 0)
   {
       return "192 bit";
   }
   if(file->size()%128 == 0)
   {
       return "128 bit";
   }
   return "Unknown";
}
