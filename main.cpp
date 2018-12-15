#include <QtCore>
#include "thread.h"
#include "search.h"

void mountPartitions();
int unmount();
QString fileLength();
void help();
int analyzeFile(QString file);

int main(int argc, char *argv[])
{
    int match=0;
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    QCommandLineOption helpOption(QStringList() << "h" << "help",
                QCoreApplication::translate("main", "Usage Help"));
    parser.addOption(helpOption);

    QCommandLineOption mountOption(QStringList() << "m" << "mount",
                QCoreApplication::translate("main", "Mount all partitions"));
    parser.addOption(mountOption);

    QCommandLineOption unmountOption(QStringList() << "u" << "umount" << "unmount",
                QCoreApplication::translate("main", "Mount all partitions"));
    parser.addOption(unmountOption);

    QCommandLineOption testOption(QStringList() << "t" << "test" << "testing",
                QCoreApplication::translate("main", "Generate test data"));
    parser.addOption(testOption);

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

    QCommandLineOption fileOption(QStringList() << "f" << "file",
                QCoreApplication::translate("main", "File to analyze"),
                QCoreApplication::translate("main", "file"));
    parser.addOption(fileOption);
    parser.process(a);

    QStringList args = parser.optionNames();
    if(args.contains("h") || args.contains("help"))
    {
        help();
        return 0;
    }
    if(args.contains("f") || args.contains("file"))
    {
        return analyzeFile(parser.value(fileOption));
    }
    if(args.contains("m")  || args.contains("mount"))
    {
        match=1;
        qDebug() << "Mounting partitions...";
        mountPartitions();
    }
    QString dir;
    if(args.contains("d")  || args.contains("dir") || args.contains("directory"))
    {
        dir=parser.value(targetDirectoryOption);
        QDir root(dir);
        if(!root.exists()){
            qDebug() << "Invalid directory: " << dir;
            return -1;
        }
    }else{
        dir = "/root/dev";
    }
    QString output;
    if(args.contains("o") || args.contains("output"))
    {
        output=parser.value(targetOutputOption);
    }
    else
    {
        if(args.contains("t") || args.contains("test") || args.contains("testing"))
        {
            output="output.csv";
        }
        else
        {
            output="output.txt";
        }
    }
    int testing=0;
    if(args.contains("t") || args.contains("test") || args.contains("testing"))
    {
        testing=1;
    }
    if(args.contains("s")  || args.contains("search"))
    {
        int toUnmount=0;
        if(args.contains("u") || args.contains("umount") || args.contains("umount"))
        {
            toUnmount=1;
        }
        match=1;
        Search s;
        s.setStuff(dir, output, toUnmount, &a, testing);
        return s.search();
    }
    else
    {
        if(args.contains("u") || args.contains("umount") || args.contains("umount"))
        {
            qDebug() << "Unmounting partitions...";
            return unmount();
        }
    }
    if(!match){
        help();
    }
    return 0;
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

int unmount()
{
    if (QProcess::execute(QString("/bin/sh") + " ./umount.sh") < 0){
        qDebug() << "Failed to run unmount script.";
        return -1;
    }
    qDebug() << "Done unmounting.";
    return 0;
}

void help()
{
    qDebug() << "Usage:\n-f   --file\t\t\tSingle file to analyze" <<
                "\n-m   --mount\t\t\tMount all partitions at \"~/dev\"" <<
                "\n-u   --umount\t--unmount\tUnmount all partitions at \"~/dev\"" <<
                "\n-d   --dir\t--directory\tDirectory to search (\"~/dev\" by default)" <<
                "\n-s   --search\t\t\tSearch for encrypted files recursively" <<
                "\n-o   --output\t\t\tOutput file (\"output.txt\" by default)" <<
                "\n-t   --test\t--testing\tGenerate csv test data (statistical values for every file)";
}

int analyzeFile(QString file)
{
    if(!QFile::exists(file)){
        qDebug() << "Supplied file name does not exist";
        return -1;
    }
    QFile fileToCheck(file);
    if(fileToCheck.size() < 32)
    {
        qDebug() << "File too small to analyze";
        return 0;
    }
    fileToCheck.open(QIODevice::ReadOnly);
    int total=0;
    QHash<char, long> data;
    while(!fileToCheck.atEnd())
    {
        QByteArray read = fileToCheck.read(1);
        data[read[0]]++;
        total++;
    }
    QHashIterator<char, long> i(data);
    double entropy=0;
    while (i.hasNext())
    {
        i.next();
        double current = 1.0*i.value()/total;
        entropy -= current*log2(current);
    }
    if(entropy > 7.3)
    {
        double avg = total/data.size();
        i.toFront();
        double chi2=0;
        while (i.hasNext())
        {
            i.next();
            chi2+=(i.value()-avg)*(i.value()-avg)/avg;
        }
        double limit= 22017.84 + (374.6088 - 22017.84)/(1.0 + pow((1.0*fileToCheck.size()/2269952000), 0.8129303));
        if(chi2 < limit)
        {
            fileToCheck.reset();
            total=0;
            QHash<QByteArray, long> dataNGram;
            while(!fileToCheck.atEnd())
            {
                dataNGram[fileToCheck.read(3)]++;
                total++;
                if(!fileToCheck.atEnd())
                {
                    fileToCheck.seek(fileToCheck.pos()-2);
                }
            }
            double avg = total/dataNGram.size();
            QHashIterator<QByteArray, long> iterator(dataNGram);
            double nGramChi2=0;
            while (iterator.hasNext())
            {
                iterator.next();
                nGramChi2+=(iterator.value()-avg)*(iterator.value()-avg)/avg;
            }
            double limit2;
            if(fileToCheck.size() < 1000000)
            {
                limit2 = 1.335831 + 0.0004732094*fileToCheck.size() + 0.00000003104824*fileToCheck.size() * fileToCheck.size();
            }
            else
            {
                limit2 = 18715610 + (-20651.77 - 18715610)/(1.0 + pow((1.0*fileToCheck.size()/25331810), 1.824073));
            }
            if(nGramChi2 < limit2)
            {
                qDebug() << "Encrypted";
                fileToCheck.close();
                return 0;
            }
        }
    }
    fileToCheck.close();
    qDebug() << "Not Encrypted";
    return 0;
}
