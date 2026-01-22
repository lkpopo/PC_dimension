#pragma once
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include <QApplication>
#include <QString>
#include <QSettings>
#include <QTextCodec>

#include <QProcess>
#include <QStandardPaths>
#include <QtNetwork/QHostInfo>
#include "CommonTool.h"

static int linkDB_SQLite()
{
    QSqlDatabase db;
    //检测已连接的方式 - 默认连接名
    if (QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db = QSqlDatabase::addDatabase("QSQLITE");
    //设置数据库路径，不存在则创建
    QString dbPath = QApplication::applicationDirPath() + "/DataBase/QSQLITE.db";
    db.setDatabaseName(dbPath);
    //打开数据库
    if (db.open()) 
    {
        qDebug() << "open success";   
        //关闭数据库
       //db.close();
        return 0;
    }  
    else
        return -1;
}

static int linkDBSQLite()
{
	//通过addDatabase添加数据库类型
	QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
	//sqlite3只要设置一个数据库名 
    QString dbPath = QApplication::applicationDirPath() + "/DataBase/QSQLITE.db";
	db.setDatabaseName(dbPath);
	//打开数据库
	if (db.open() == false)
	{
        return -1;
	}
    return 0;
}





