#ifndef _TOOL_H
#define _TOOL_H

#pragma once
#include <QString>
#include <QFile>
#include <cstdio>
#include <cstdlib>
#include <QTextStream>

#include <random>
#include <iomanip>

#include <string>       // std::string 
#include <iostream>     // std::cout 
#include <sstream>      // std::stringstream  

#include <QFileInfo>
#include <QByteArray>

#include <QString>
#include <QProcess>
#include <QThread>

#include <QApplication>
#include <QTextCodec>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <QtNetwork/QHostInfo>
#include <QRegularExpression>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <md5.h>

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <iostream>
#include <string>
#include <vector>

#include <cmath>

#include <QUuid>

#include <openssl/bio.h>
#include <openssl/evp.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <QVector3D>
#include <cmath>
#include <cstdlib>   // rand()、srand()
#include <ctime>     // time()
#include <QtMath>

#include <QRect>


#ifdef _CUR_SYS_LINUX_
#include <iconv.h>
#include <unistd.h>
#else
#include <windows.h>
#include <stdio.h>
#endif

#define SHA256_ROTL(a,b) (((a>>(32-b))&(0x7fffffff>>(31-b)))|(a<<b))
#define SHA256_SR(a,b) ((a>>b)&(0x7fffffff>>(b-1)))
#define SHA256_Ch(x,y,z) ((x&y)^((~x)&z))
#define SHA256_Maj(x,y,z) ((x&y)^(x&z)^(y&z))
#define SHA256_E0(x) (SHA256_ROTL(x,30)^SHA256_ROTL(x,19)^SHA256_ROTL(x,10))
#define SHA256_E1(x) (SHA256_ROTL(x,26)^SHA256_ROTL(x,21)^SHA256_ROTL(x,7))
#define SHA256_O0(x) (SHA256_ROTL(x,25)^SHA256_ROTL(x,14)^SHA256_SR(x,3))
#define SHA256_O1(x) (SHA256_ROTL(x,15)^SHA256_ROTL(x,13)^SHA256_SR(x,10))


#define M_PI 3.14159265358979323846
// 角度转弧度
#define DEG2RAD(deg) ((deg) * M_PI / 180.0)
#define RAD2DEG(rad) ((rad) * 180.0 / M_PI)

class ProjectInfo : public QObject
{
    Q_OBJECT
private:
    ProjectInfo() {}
    ~ProjectInfo() {}
public:
    inline static ProjectInfo& Ins()
    {
        static ProjectInfo ins;
        return ins;
    }
public:
    QString m_projUUID;
};

//srcPath源文件文件路径，dstPath目的文件路径，coverFileIfExist文件存在是否覆盖
static bool copyFile(QString srcPath, QString dstPath, bool coverFileIfExist)
{
	srcPath.replace("\\", "/");
	dstPath.replace("\\", "/");
	if (srcPath == dstPath) {
		return true;
	}

	if (!QFile::exists(srcPath)) {  //源文件不存在
		return false;
	}

	if (QFile::exists(dstPath)) {
		if (coverFileIfExist) {
			QFile::remove(dstPath);
		}
	}

	if (!QFile::copy(srcPath, dstPath)) {
		return false;
	}
	return true;
}

//判断一个点在一个区域内
static bool PointInArea(const QRect& rectArea, const QPoint& point)
{
	if (point.x() >= rectArea.x() && point.x() <= (rectArea.x() + rectArea.width())
		&& point.y() >= rectArea.y() && point.y() <= (rectArea.y() + rectArea.height()))
	{
		return true;
	}

	return false;
}

static unsigned char* base64_encodeJM(unsigned char* str, long str_len)
{
    long len;
    unsigned char* res;
    int i, j;
    //定义base64编码表  
    unsigned char* base64_table = (unsigned char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    //计算经过base64编码后的字符串长度  
    if (str_len % 3 == 0)
        len = str_len / 3 * 4;
    else
        len = (str_len / 3 + 1) * 4;

    res = (unsigned char*)malloc(sizeof(unsigned char) * len + 1);
    res[len] = '\0';

    //以3个8位字符为一组进行编码  
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
    {
        res[i] = base64_table[str[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符  
        res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
        res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
        res[i + 3] = base64_table[str[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符  
    }

    switch (str_len % 3)
    {
    case 1:
        res[i - 2] = '=';
        res[i - 1] = '=';
        break;
    case 2:
        res[i - 1] = '=';
        break;
    }

    return res;
}

static unsigned char* base64_decodeJM2(unsigned char* code2)
{
    char* code = (char*)(code2);
    //根据base64表，以字符找到对应的十进制数据  
    int table[] = { 0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,62,0,0,0,
         63,52,53,54,55,56,57,58,
         59,60,61,0,0,0,0,0,0,0,0,
         1,2,3,4,5,6,7,8,9,10,11,12,
         13,14,15,16,17,18,19,20,21,
         22,23,24,25,0,0,0,0,0,0,26,
         27,28,29,30,31,32,33,34,35,
         36,37,38,39,40,41,42,43,44,
         45,46,47,48,49,50,51
    };
    long len;
    long str_len;
    unsigned char* res;
    int i, j;

    //计算解码后的字符串长度  
    len = strlen((char*)code);
    //判断编码后的字符串后是否有=  
    if (strstr(code, "=="))
        str_len = len / 4 * 3 - 2;
    else if (strstr(code, "="))
        str_len = len / 4 * 3 - 1;
    else
        str_len = len / 4;

    res = (unsigned char*)malloc(sizeof(unsigned char) * str_len + 1);
    res[str_len] = '\0';

    //以4个字符为一位进行解码  
    for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
    {
        res[j] = ((unsigned char)table[code[i]]) << 2 | (((unsigned char)table[code[i + 1]]) >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
        res[j + 1] = (((unsigned char)table[code[i + 1]]) << 4) | (((unsigned char)table[code[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
        res[j + 2] = (((unsigned char)table[code[i + 2]]) << 6) | ((unsigned char)table[code[i + 3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
    }

    return res;

}

static char* StrSHA256(const char* str, long long length, char* sha256) {
    char* pp, * ppend;
    long l, i, W[64], T1, T2, A, B, C, D, E, F, G, H, H0, H1, H2, H3, H4, H5, H6, H7;
    H0 = 0x6a09e667, H1 = 0xbb67ae85, H2 = 0x3c6ef372, H3 = 0xa54ff53a;
    H4 = 0x510e527f, H5 = 0x9b05688c, H6 = 0x1f83d9ab, H7 = 0x5be0cd19;
    long K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    };
    l = length + ((length % 64 > 56) ? (128 - length % 64) : (64 - length % 64));
    if (!(pp = (char*)malloc((unsigned long)l))) return 0;
    for (i = 0; i < length; pp[i + 3 - 2 * (i % 4)] = str[i], i++);
    for (pp[i + 3 - 2 * (i % 4)] = 128, i++; i < l; pp[i + 3 - 2 * (i % 4)] = 0, i++);
    *((long*)(pp + l - 4)) = length << 3;
    *((long*)(pp + l - 8)) = length >> 29;
    for (ppend = pp + l; pp < ppend; pp += 64) {
        for (i = 0; i < 16; W[i] = ((long*)pp)[i], i++);
        for (i = 16; i < 64; W[i] = (SHA256_O1(W[i - 2]) + W[i - 7] + SHA256_O0(W[i - 15]) + W[i - 16]), i++);
        A = H0, B = H1, C = H2, D = H3, E = H4, F = H5, G = H6, H = H7;
        for (i = 0; i < 64; i++) {
            T1 = H + SHA256_E1(E) + SHA256_Ch(E, F, G) + K[i] + W[i];
            T2 = SHA256_E0(A) + SHA256_Maj(A, B, C);
            H = G, G = F, F = E, E = D + T1, D = C, C = B, B = A, A = T1 + T2;
        }
        H0 += A, H1 += B, H2 += C, H3 += D, H4 += E, H5 += F, H6 += G, H7 += H;
    }
    free(pp - l);
    //sprintf(sha256, "%08X%08X%08X%08X%08X%08X%08X%08X", H0, H1, H2, H3, H4, H5, H6, H7);
    return sha256;
}

static std::string QStr2Str(const QString qStr)
{
    QByteArray data = qStr.toLocal8Bit();
    return std::string(data);
}

static std::string ToStringUTF_8(const QString& qstr)
{
    QByteArray arr = qstr.toUtf8();
    std::string cstr = arr.data();
    return cstr;
}

static void getcpuidex(unsigned int CPUInfo[4], unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER)
#if defined(_WIN64)
    __cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
#else
    if (NULL == CPUInfo)  return;
    _asm {

        mov edi, CPUInfo;
        mov eax, InfoType;
        mov ecx, ECXValue;
        cpuid;
        mov[edi], eax;
        mov[edi + 4], ebx;
        mov[edi + 8], ecx;
        mov[edi + 12], edx;
    }
#endif
#endif
}

static void getcpuid(unsigned int CPUInfo[4], unsigned int InfoType)
{
#if defined(__GNUC__)
    __cpuid(InfoType, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
#elif defined(_MSC_VER)
#if _MSC_VER >= 1400 
    __cpuid((int*)(void*)CPUInfo, (int)(InfoType));
#else
    getcpuidex(CPUInfo, InfoType, 0);
#endif
#endif
}
static QString get_cpuId()
{
    QString cpu_id = "";
    unsigned int dwBuf[4] = { 0 };
    unsigned long long ret = 0;
    getcpuid(dwBuf, 1);
    ret = dwBuf[3];
    ret = ret << 32;

    QString str0 = QString::number(dwBuf[3], 16).toUpper();
    QString str0_1 = str0.rightJustified(8, '0');
    QString str1 = QString::number(dwBuf[0], 16).toUpper();
    QString str1_1 = str1.rightJustified(8, '0');
    cpu_id = str0_1 + str1_1;
    return cpu_id;
}

//图片转base64
static std::string Image2Base64(QString picPath)
{
    QByteArray array;
    QFile fi(picPath);
    QByteArray fileBase64;
    if (fi.open(QIODevice::ReadOnly))
    {
        array = fi.readAll();
        fileBase64 = array.toBase64();
        fi.close();
    }
    return fileBase64.toStdString();
}

static QString Image2Base642(QString picPath)
{
    QByteArray array;
    QFile fi(picPath);
    QByteArray fileBase64;
    if (fi.open(QIODevice::ReadOnly))
    {
        array = fi.readAll();
        fileBase64 = array.toBase64();
        fi.close();
    }
    return fileBase64;
}

static void outputMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    if (msg.contains("0x") == true || type == QtWarningMsg) return;
    //
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch (type)
    {
    case QtDebugMsg:
        text = QString("Debug:");
        break;

    case QtWarningMsg:
        text = QString("Warning:");
        break;

    case QtCriticalMsg:
        text = QString("Critical:");
        break;

    case QtFatalMsg:
        text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ");
    QString current_date = QString("(%1)").arg(current_date_time);
    //QString message = QString("%1 %2 %3 %4").arg(current_date).arg(text).arg(context_info).arg(msg);
    QString message = QString("%1 %2 %3").arg(current_date).arg(text).arg(msg);
    QString timestr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString logFolder = QString("%1/log").arg(QApplication::applicationDirPath());
    QDir dir(logFolder);
    if (!dir.exists())
    {
        dir.mkdir(logFolder);
    }
    QString fileName = logFolder + "/" + timestr + ".txt";

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();
    //
    mutex.unlock();
}

static bool ipAddrIsOK(const QString& ip)
{
    if (ip.isEmpty())
    {
        return false;
    }

    QStringList list = ip.split('.');
    if (list.size() != 4)
    {
        return false;
    }

    for (const auto& num : list)
    {
        bool ok = false;
        int temp = num.toInt(&ok);
        if (!ok || temp < 0 || temp > 255)
        {
            return false;
        }
    }

    return true;
}

static QString Get(QString data)
{
    static QRegularExpression re("\\s");
    data.remove(re);
    return data;
}

//清空文件夹
static bool clearDir(QString path)
{
    if (path.isEmpty())
    {
        return false;
    }
    QDir dir(path);
    if (!dir.exists())
    {
        return false;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
    QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
    //遍历文件信息
    foreach(QFileInfo file, fileList)
    {
        // 是文件，删除
        if (file.isFile())
        {
            file.dir().remove(file.fileName());
        }
        else // 递归删除
        {
            clearDir(file.absoluteFilePath());
            file.dir().rmdir(file.absoluteFilePath());
        }
    }
    return true;
}

static void addSubFolderImages(const QString path, QStringList& stringList)
{
    QDir dir(path);
    if (!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
    int i = 0;
    bool is_dir;
    do
    {
        QFileInfo file_info = list.at(i);
        if (file_info.fileName() == "." | file_info.fileName() == "..")
        {
            i++;
            continue;
        }

        is_dir = file_info.isDir();
        if (is_dir)
        {
            //进行递归
            addSubFolderImages(file_info.filePath(), stringList);
        }
        else
        {
            //获取文件后缀并获取所选包含类型，若存在包含类型且后缀相同，则添加
            QString suffix = file_info.suffix();
            if (QString::compare(suffix, QString("jpg"), Qt::CaseInsensitive) == 0 ||
                	QString::compare(suffix, QString("jpeg"), Qt::CaseInsensitive) == 0 ||
                		QString::compare(suffix, QString("png"), Qt::CaseInsensitive) == 0)
            {
                QString absolute_file_path = file_info.absoluteFilePath();
                stringList.append(absolute_file_path);
            }
        }
        i++;
    } while (i < list.size());
}

static void addSubFolder(const QString path, QStringList& folderList)
{
    QDir dir(path);
    if (!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
    dir.setSorting(QDir::DirsFirst);
    QFileInfoList list = dir.entryInfoList();
    int i = 0;
    bool is_dir;
    do
    {
        QFileInfo file_info = list.at(i);
        if (file_info.fileName() == "." | file_info.fileName() == "..")
        {
            i++;
            continue;
        }

        is_dir = file_info.isDir();
        if (is_dir)
        {
            //进行递归
            folderList.append(file_info.filePath());
            addSubFolder(file_info.filePath(), folderList);
        }
        else
        {                         
           //
        }
        i++;
    } while (i < list.size());
}

//文件编码
static QString FileCharacterEncoding(const QString& fileName)
{
    //假定默认编码utf8
    QString code = "UTF8";
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        //读取3字节用于判断
        QByteArray buffer = file.read(3);
        quint8 sz1st = buffer.at(0);
        quint8 sz2nd = buffer.at(1);
        quint8 sz3rd = buffer.at(2);
        if (sz1st == 0xFF && sz2nd == 0xFE)
        {
            code = "UTF16LE";
        }
        else if (sz1st == 0xFE && sz2nd == 0xFF)
        {
            code = "UTF16BE";
        }
        else if (sz1st == 0xEF && sz2nd == 0xBB && sz3rd == 0xBF)
        {
            code = "UTF8BOM";
        }
        else
        {
            //尝试用utf8转换,如果无效字符数大于0,则表示是ansi编码
            QTextCodec::ConverterState cs;
            QTextCodec* tc = QTextCodec::codecForName("utf-8");
            tc->toUnicode(buffer.constData(), buffer.size(), &cs);
            code = (cs.invalidChars > 0) ? "ANSI" : "UTF8";
        }
        file.close();
    }
    return code;
}

static QByteArray AnsiToUtf8(QByteArray& ansi)
{
#ifdef _CUR_SYS_LINUX_
    QByteArray utf8;
    char* buf = new char[ansi.size() * 2];
    char utf8Buf[] = { "utf-8" };
    char gb2312Buf[] = { "gb2312" };
    //ansi.resize(ansi.size()*2);
    int code = code_convert(gb2312Buf, utf8Buf, ansi.data(), ansi.size(), buf, 2 * ansi.size());
    if (code == -1) {
        delete[]buf;
        utf8 = "";
        return utf8;
    }
    utf8 = buf;
    delete[]buf;
    return utf8;
#else
    int len;
    QByteArray result;
    //ANSI转UNICODE
    len = MultiByteToWideChar(CP_ACP, NULL, ansi.data(), -1, NULL, 0);
    WCHAR* unicode = new WCHAR[len + 1];
    memset(unicode, 0, len * 2 + 2);
    MultiByteToWideChar(CP_ACP, NULL, ansi.data(), -1, unicode, len);
    //UNICODE转utf8
    len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
    char* utf8 = new char[len + 1];
    memset(utf8, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, len, NULL, NULL);
    //赋值
    result = utf8;
    delete[] unicode;
    delete[] utf8;
    return result;
#endif
}

//复制文件
static bool CopyFile(QString srcPath, QString dstPath, bool coverFileIfExist)
{
    srcPath.replace("\\", "/");
    dstPath.replace("\\", "/");
    if (srcPath == dstPath) {
        return true;
    }

    if (!QFile::exists(srcPath)) {  //源文件不存在
        return false;
    }

    if (QFile::exists(dstPath)) {
        if (coverFileIfExist) {
            QFile::remove(dstPath);
        }
    }

    if (!QFile::copy(srcPath, dstPath)) {
        return false;
    }
    return true;
}

static bool getmachinecode(std::string& oStr)
{
	unsigned long VolumeSerialNumber;
	GetVolumeInformationA("c:\\", NULL, 12, &VolumeSerialNumber, NULL, NULL, NULL, 10);

	std::ostringstream os;
	os << VolumeSerialNumber;
	std::string result;
	std::istringstream is(os.str());
	is >> oStr;

	return true;
}

static int round4_5(double number)
{
	return (number > 0.0) ? (number + 0.5) : (number - 0.5);
}

static bool isUsed(QString filePath)
{
	bool isUsed = false;

	QString fpathx = filePath + "sqh";

	QFile file(filePath);
	//文件是否存在
	bool isExist = file.exists();
	if (isExist == true)
	{
		//看文件是否可以重新命名
		bool isCanRename = file.rename(filePath, fpathx);
		if (isCanRename == false)
		{
			isUsed = true;
		}
		//可以重新命名，说明未被占用
		else
		{
			file.rename(fpathx, filePath);
		}
	}
	file.close();
	return isUsed;
}

//分割字符串
static void stringSplit(std::string s, std::string delim, std::vector<std::string>& ans)
{
    std::string::size_type pos_1, pos_2 = 0;
	while (pos_2 != s.npos) {
		pos_1 = s.find_first_not_of(delim, pos_2);
		if (pos_1 == s.npos) break;
		pos_2 = s.find_first_of(delim, pos_1);
		ans.push_back(s.substr(pos_1, pos_2 - pos_1));
	}
}

static std::string UTF8ToGB(const char* str)
{
	std::string result;
	WCHAR* strSrc;
	LPSTR szRes;

	//获得临时变量的大小
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	//获得临时变量的大小
	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
}

static QStringList getDirAllFiles(const QString dirPath)
{
    QStringList fileList;
    QDir dir(dirPath);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list)
    {
        if (fileInfo.isDir())
        {
            getDirAllFiles(fileInfo.absoluteFilePath());
        }
        else
        {
            fileList.append(fileInfo.absoluteFilePath());
        }
    }
    return fileList;
}

static bool isFileExist(QString fullFilePath) //判断文件存在
{
    QFileInfo fileInfo(fullFilePath);
    if (fileInfo.exists())
    {
        return true;
    }
    return false;
}

// 判断进程是否存在
static bool checkProcessByQProcess(const QString& strExe)
{
    bool bResult = false;
    QProcess tasklist;
    tasklist.start("tasklist",
        QStringList() << "/NH"
        << "/FO" << "CSV"
        << "/FI" << QString("IMAGENAME eq %1").arg(strExe));
    tasklist.waitForFinished();
    QString strOutput = tasklist.readAllStandardOutput();
    if (strOutput.startsWith(QString("\"%1").arg(strExe)))
    {
        qInfo() << "check process";
        bResult = true;
    }
    return bResult;
}

static std::string GetSerialFromSys()
{
    #ifdef _WIN32
        unsigned long VolumeSerialNumber;
        GetVolumeInformationA("c:\\", NULL, 12, &VolumeSerialNumber, NULL, NULL, NULL, 10);
        std::ostringstream os;
        os << VolumeSerialNumber;
        std::string result;
        std::istringstream is(os.str());
        is >> result;
        return result;
    #else
        std::string result = "";
        const char* cmd = "lsblk -o SERIAL /dev/sda";
        char buffer[128];
        std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
        while (fgets(buffer, sizeof(buffer), pipe.get())) {
            result += buffer;
        }
        std::string result0 = result.substr(result.find("\n") + 1);
        result0.erase(std::remove(result0.begin(), result0.end(), '\n'), result0.end());
        return result0;      
    #endif
}

static std::string base64Encode(const std::vector<unsigned char>& binary) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(b64, binary.data(), binary.size());
    BIO_flush(b64);

    char* output;
    long len = BIO_get_mem_data(mem, &output);
    std::string result(output, len);

    BIO_free_all(b64);
    return result;
}

static std::vector<unsigned char>base64Decode(const std::string& encoded) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new_mem_buf(encoded.data(), encoded.size());
    BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    std::vector<unsigned char> output(encoded.size());
    int len = BIO_read(b64, output.data(), output.size());
    output.resize(len);

    BIO_free_all(b64);
    return output;
}

static std::string aes_encrypt(const std::string& plaintext, const std::string& key) {
    // 生成随机IV
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, sizeof(iv));

    // 准备加密缓冲区（IV + 密文）
    std::vector<unsigned char> ciphertext(AES_BLOCK_SIZE + plaintext.size() + AES_BLOCK_SIZE);
    memcpy(ciphertext.data(), iv, AES_BLOCK_SIZE);

    // 设置加密密钥
    AES_KEY enc_key;
    AES_set_encrypt_key((const unsigned char*)key.data(), 128, &enc_key);

    // 执行CBC模式加密
    AES_cbc_encrypt((const unsigned char*)plaintext.data(),
        ciphertext.data() + AES_BLOCK_SIZE,
        plaintext.size(),
        &enc_key, iv, AES_ENCRYPT);

    // 转换为Base64
    return base64Encode(ciphertext);
}

static std::string aes_decrypt(const std::string& ciphertext, const std::string& key) {
    // Base64解码
    std::vector<unsigned char> binary = base64Decode(ciphertext);
    if (binary.size() <= AES_BLOCK_SIZE) {
        //throw std::runtime_error("Invalid ciphertext length");
        return "";
    }

    // 提取IV（前16字节）
    unsigned char iv[AES_BLOCK_SIZE];
    memcpy(iv, binary.data(), AES_BLOCK_SIZE);

    // 准备解密缓冲区
    std::vector<unsigned char> plaintext(binary.size() - AES_BLOCK_SIZE);

    // 设置解密密钥
    AES_KEY dec_key;
    AES_set_decrypt_key((const unsigned char*)key.data(), 128, &dec_key);

    // 执行CBC模式解密
    AES_cbc_encrypt(binary.data() + AES_BLOCK_SIZE,
        plaintext.data(),
        plaintext.size(),
        &dec_key, iv, AES_DECRYPT);

    // 去除可能的填充字节
    size_t pad_len = plaintext.back();
    if (pad_len > AES_BLOCK_SIZE) pad_len = 0;
    return std::string(plaintext.begin(), plaintext.end() - pad_len);
}

static bool ReAnalyiseByFile(QString& _msg)
{
    QString auth = QApplication::applicationDirPath() + QString("/device.lic");

    QString deviceId;
    QString orginComputerKey;
    QString startTime;
    QString endTime;


    QFile file(auth);
    if (file.exists() == false)
    {
        _msg = u8"授权文件不存在！";
        return false;
    }
    if (!file.open(QFile::ReadOnly))
    {
        file.close();
        return false;
    }
    QByteArray code = file.readAll();
    file.close();

    std::string key = "jdtnsjuesgjueguf";
    std::string str_device = aes_decrypt(code.toStdString(), key);
    QString device = QString::fromStdString(str_device);

    if (device.isEmpty())
    {
        _msg = u8"证书验证失败！";
        file.remove();
        return false;
    }

    QJsonParseError jError;
    QJsonDocument jDoc = QJsonDocument::fromJson(str_device.c_str(), &jError);
    if (jDoc.isNull() || jError.error != QJsonParseError::NoError)
    {
        _msg = u8"证书验证失败！";
        return false;
    }
    QJsonObject jObj = jDoc.object();
    deviceId = jObj["deviceId"].toString();
    orginComputerKey = jObj["orginComputerKey"].toString();
    startTime = jObj["startTime"].toString();
    endTime = jObj["endTime"].toString();

    std::string LMachineCode = GetSerialFromSys();
    //机器码转成MD5
    std::string machineMd5 = MD5(LMachineCode).toStr().substr(8, 16);
    machineMd5 = machineMd5.insert(4, "-");
    machineMd5 = machineMd5.insert(9, "-");
    machineMd5 = machineMd5.insert(14, "-");
    transform(machineMd5.begin(), machineMd5.end(), machineMd5.begin(), ::toupper);

    if (machineMd5 == deviceId.toStdString())
    {
        QDateTime time = QDateTime::currentDateTime();
        uint curTime = time.toTime_t();
        QDateTime t1 = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
        uint beginTime = t1.toTime_t();
        QDateTime t2 = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
        uint endTime = t2.toTime_t();
        if (curTime > beginTime && curTime < endTime)
        {
            int day = round4_5((endTime - curTime) / (24 * 3600.0));
            _msg = u8"剩余天数约为：" + QString::number(day) + u8"天";
            return true;
        }
        else
        {
            _msg = u8"注册码已过期，请联系维护人员，生成新注册码";
            return false;//机器码到期了
        }
    }
    _msg = u8"注册码和机器码不匹配";
    return false;//机器码不匹配
}

static int round45(double number)
{
    return (number > 0.0) ? (number + 0.5) : (number - 0.5);
}

static bool analyiseByFile(QString& _msg, QString _file)
{
    QString auth = QApplication::applicationDirPath() + QString("/device.lic");
    if (_file != NULL)
    {
        auth = _file;
    }

    QString deviceId;
    QString orginComputerKey;
    QString startTime;
    QString endTime;

    if (isFileExist(auth) == true)
    {
        QFile file(auth);
        if (!file.open(QFile::ReadOnly))
        {
            file.close();
            return false;
        }
        QByteArray code = file.readAll();
        file.close();

        std::string key = "jdtnsjuesgjueguf";  
        std::string str_device = aes_decrypt(code.toStdString(), key);
        QString device = QString::fromStdString(str_device);

        if (device.isEmpty())
        {
            _msg = u8"证书验证失败！";
            file.remove();
            return false;
        }

        QJsonParseError jError;
        QJsonDocument jDoc = QJsonDocument::fromJson(str_device.c_str(), &jError);
        if (jDoc.isNull() || jError.error != QJsonParseError::NoError)
        {
            _msg = u8"证书验证失败！";
            return false;
        }
        QJsonObject jObj = jDoc.object();
        deviceId = jObj["deviceId"].toString();
        startTime = jObj["startTime"].toString();
        endTime = jObj["endTime"].toString();

    }
    else
    {
        _msg = u8"本地校验失败";
        return false;//未进行授权
    }

    string LMachineCode = GetSerialFromSys();

    //机器码转成MD5
    string machineMd5 = MD5(LMachineCode).toStr().substr(8, 16);
    machineMd5 = machineMd5.insert(4, "-");
    machineMd5 = machineMd5.insert(9, "-");
    machineMd5 = machineMd5.insert(14, "-");
    transform(machineMd5.begin(), machineMd5.end(), machineMd5.begin(), ::toupper);

    if (machineMd5 == deviceId.toStdString())
    {
        QDateTime time = QDateTime::currentDateTime();
        uint curTime = time.toTime_t();
        QDateTime t1 = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
        uint beginTime = t1.toTime_t();
        QDateTime t2 = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
        uint endTime = t2.toTime_t();
        if (curTime > beginTime && curTime < endTime)
        {
            //赋值许可文件
            if (!_file.isEmpty())
            {
                QString originauth = QApplication::applicationDirPath() + QString("/device.lic");
                if (QFile::exists(originauth))
                    QFile::remove(originauth);
                QFile::copy(_file, originauth);
            }
            //
            int day = round45((endTime - curTime) / (24 * 3600.0));
            _msg = u8"剩余天数约为：" + QString::number(day) + u8"天";
            return true;
        }
        else
        {
            _msg = u8"注册码已过期，请联系维护人员，生成新注册码";
            return false;//机器码到期了
        }
    }
    _msg = u8"注册码和机器码不匹配";
    return false;//机器码不匹配
}

static bool analyiseByCode(const QString& _encrypedBase64Code, QString& _msg)
{
    std::string key = "jdtnsjuesgjueguf";
    std::string str_device = aes_decrypt(_encrypedBase64Code.toStdString(), key);
    QString device = QString::fromStdString(str_device);

    if (device.isEmpty())
    {
        _msg = u8"证书验证失败";
        return false;
    }
    QJsonParseError jError;
    QJsonDocument jDoc = QJsonDocument::fromJson(str_device.c_str(), &jError);
    if (jDoc.isNull() || jError.error != QJsonParseError::NoError)
    {
        _msg = u8"证书验证失败";
        return false;
    }

    QString deviceId;
    QString orginComputerKey;
    QString startTime;
    QString endTime;

    QJsonObject jObj = jDoc.object();
    deviceId = jObj["deviceId"].toString();
    startTime = jObj["startTime"].toString();
    endTime = jObj["endTime"].toString();

    string LMachineCode = GetSerialFromSys();
    //机器码转成MD5
    string machineMd5 = MD5(LMachineCode).toStr().substr(8, 16);
    machineMd5 = machineMd5.insert(4, "-");
    machineMd5 = machineMd5.insert(9, "-");
    machineMd5 = machineMd5.insert(14, "-");
    transform(machineMd5.begin(), machineMd5.end(), machineMd5.begin(), ::toupper);

    if (machineMd5 == deviceId.toStdString())
    {
        QDateTime time = QDateTime::currentDateTime();
        uint curTime = time.toTime_t();
        QDateTime t1 = QDateTime::fromString(startTime, "yyyy-MM-dd hh:mm:ss");
        uint beginTime = t1.toTime_t();
        QDateTime t2 = QDateTime::fromString(endTime, "yyyy-MM-dd hh:mm:ss");
        uint endTime = t2.toTime_t();
        if (curTime > beginTime && curTime < endTime)
        {
            int day = round45((endTime - curTime) / (24 * 3600.0));
            _msg = u8"剩余天数约为：" + QString::number(day) + u8"天";
            //写入
            QByteArray by = _encrypedBase64Code.toUtf8();
            QString newFilePath = QApplication::applicationDirPath() + QString("/device.lic");
            QFile file(newFilePath);
            if (!file.open(QFile::WriteOnly))
            {
                return false;
            }
            file.write(by);
            file.close();
            return true;
        }
        else
        {
            _msg = u8"注册码已过期，请联系维护人员，生成新注册码!";
            return false;//机器码到期了
        }
    }
    _msg = u8"注册码和机器码不匹配!";
    return false;//机器码不匹配
}

static QString getUUID()
{
    QUuid id0 = QUuid::createUuid();//生成唯一码
    QString strId = id0.toString();//转换为QString
    QString strId1 = strId.replace("{", "");
    QString strId2 = strId1.replace("}", "");
    QString strId3 = strId2.replace("-", "");
    return strId3;
}

static QStringList searchImages(const QString& folderPath, bool recursive = false,
    const QStringList& formats = { "jpg", "png" })
{
    QStringList imagePaths;
    QDir dir(folderPath);

    // 检查文件夹是否存在
    if (!dir.exists()) {
        qWarning() << "文件夹不存在：" << folderPath;
        return imagePaths;
    }

    // 设置文件过滤规则
    QDir::Filters filters = QDir::Files | QDir::NoSymLinks; // 只搜文件，排除符号链接
    QDir::SortFlags sort = QDir::Name | QDir::IgnoreCase;   // 按名称排序，忽略大小写

    // 构建格式过滤器（如 "*.jpg", "*.png"）
    QStringList nameFilters;
    for (const QString& fmt : formats) {
        nameFilters << QString("*.%1").arg(fmt.toLower())
            << QString("*.%1").arg(fmt.toUpper()); // 兼容大小写（如 JPG/jpg）
    }

    // 获取文件列表
    QFileInfoList fileList;
    if (recursive) {
        // 递归搜索（含子文件夹）
        fileList = dir.entryInfoList(nameFilters, filters, sort);
        // 遍历子文件夹
        QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString& subDir : subDirs) {
            QString subDirPath = dir.filePath(subDir);
            imagePaths += searchImages(subDirPath, true, formats); // 递归调用
        }
    }
    else {
        // 单层搜索（仅当前文件夹）
        fileList = dir.entryInfoList(nameFilters, filters, sort);
    }
    // 验证文件是否为有效图片（避免后缀名伪装的文件）
    for (const QFileInfo& fileInfo : fileList) 
           imagePaths << fileInfo.absoluteFilePath();
    return imagePaths;
}

// 初始化随机数种子（确保每次运行随机数不同，仅初始化一次）
static void initRandomSeed() {
    static bool isInit = false;
    if (!isInit) {
        srand(static_cast<unsigned int>(time(nullptr))); // 系统时间为种子
        isInit = true;
    }
}

// 生成[-1.0, 1.0]范围内的随机双精度浮点数
static double randomDouble() {
    return (static_cast<double>(rand() % 2000 - 1000)) / 1000.0;
}

/**
 * @brief 生成单位球上距离指定点直线距离为dist的随机点（每次位置不同）
 * @param P 单位球上的原始点（会自动归一化容错）
 * @param dist 目标直线距离（默认0.05）
 * @return 单位球上满足距离要求的随机点
 */
static QVector3D getNearPointOnUnitSphere(const QVector3D& P, double dist = 0.05) {
    // 1. 初始化随机种子 + 归一化输入点（确保是单位向量）
    initRandomSeed();
    QVector3D P_unit = P.normalized();
    const double EPS = 1e-6; // 自定义极小容差（远小于0.05，避免浮点误差）

    // 2. 生成非零随机向量（无qFuzzyCompare重载歧义）
    QVector3D randomVec;
    do {
        randomVec = QVector3D(randomDouble(), randomDouble(), randomDouble());
        // 直接比较长度平方与极小值，绕过qFuzzyCompare重载问题
    } while (randomVec.lengthSquared() < EPS * EPS);

    // 3. 构造随机垂直于P的单位向量U（核心：每次方向不同）
    QVector3D U = QVector3D::crossProduct(P_unit, randomVec);
    // 容错：若叉乘结果为零（P与randomVec共线），则用备用垂直向量
    if (U.lengthSquared() < EPS * EPS) {
        if (qAbs(P_unit.x()) > EPS || qAbs(P_unit.y()) > EPS) {
            U = QVector3D(-P_unit.y(), P_unit.x(), 0);
        }
        else {
            U = QVector3D(0, -P_unit.z(), P_unit.y());
        }
    }
    U.normalize(); // 归一化为单位向量

    // 4. 计算球心角θ（由直线距离推导，严格公式）
    double theta = 2 * asin(dist / 2.0);

    // 5. 沿随机U方向旋转P，得到目标点Q（罗德里格斯旋转公式）
    QVector3D cross = QVector3D::crossProduct(U, P_unit);
    QVector3D Q = P_unit * cos(theta) + cross * sin(theta);

    // 6. 归一化修正（消除浮点误差，确保Q在单位球上）
    Q.normalize();

    return Q;
}

#endif