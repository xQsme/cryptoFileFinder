#include <QCoreApplication>
#include <QtDebug>
#include <QCommandLineParser>
#include <QDir>
#include <QProcess>

void mountPartitions();
void search(QString dir);
void analyzeFile(QString file);

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
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "dir",
                QCoreApplication::translate("main", "Specify <directory>."),
                QCoreApplication::translate("main", "directory"));
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    QStringList args = parser.optionNames();
    if(args.contains("h") || args.contains("help")){
        qDebug() << "Usage:\n-m\t--mount\t\t\tMount all partitions\n-d\t--dir\t--directory\tDirectory to search\n-s\t--search\t\tSearch for encrypted files";
        return 0;
    }
    if(args.contains("m")  || args.contains("mount")){
        qDebug() << "Mounting partitions...";
        mountPartitions();
    }
    QString dir;
    if(args.contains("d")  || args.contains("dir") || args.contains("directory")){
        dir=parser.value(targetDirectoryOption);
        QDir root(dir);
        if(!root.exists()){
            qDebug() << "Invalid directory: " << dir;
            return 0;
        }
    }else{
        dir = "/root/dev";
    }
    if(args.contains("s")  || args.contains("search")){
        qDebug() << "Searching encrypted files...";
        search(dir);
    }

    return 0;//a.exec();
}

void mountPartitions()
{
    if (QProcess::execute(QString("/bin/sh") + " ./script.sh") < 0){
        qDebug() << "Failed to run";
    }else{
        qDebug() << "Mounted";
    }
}

void search(QString dir)
{
    qDebug() << "searching: " << dir;
    QDir root(dir);
    QList<QString> dirs;
    foreach(QFileInfo file, root.entryInfoList())
    {
        if(!dir.contains(file.absoluteFilePath()))
        {
            if(file.isDir())
            {
                dirs.append(file.absoluteFilePath());
            }
            else
            {
                analyzeFile(file.absoluteFilePath());
            }
        }
    }
    foreach(QString d, dirs)
    {
        search(d);
    }
}

void analyzeFile(QString file)
{
    qDebug() << "Analyzing file: " << file;
}
