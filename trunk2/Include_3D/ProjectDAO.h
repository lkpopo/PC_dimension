#pragma once
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

// 资源项数据结构
struct AssetRecord {
  QString filePath;
  double lon = 0.0;
  double lat = 0.0;
  double alt = 0.0;
};

// 项目主信息结构
struct ProjectRecord {
  QString projectName;
  QString createTime;
  QString modifyTime;
  QList<AssetRecord> assets;
};

class ProjectDataManager {
 public:
  // 初始化数据库连接并创建表
  static bool initDatabase();

  // 检查项目名称是否存在
  static bool isProjectExists(const QString& pName);

  // 保存项目（内部处理：新建插入、编辑更新、清空旧资产、插入新资产）
  static bool saveProject(const QString& pName,
                          const QList<AssetRecord>& assets, bool isUpdate);

  // 获取项目的所有资源项
  static QList<AssetRecord> getProjectAssets(const QString& pName);

  // 删除项目（级联删除资产）
  static bool deleteProject(const QString& pName);

  // 获取最近一次错误描述
  static QString lastError();

 private:
  static QString m_lastError;
};