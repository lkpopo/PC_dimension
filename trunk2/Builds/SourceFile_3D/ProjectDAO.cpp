#include "ProjectDAO.h"

QString ProjectDataManager::m_lastError = "";

bool ProjectDataManager::initDatabase() {

  QSqlQuery query;
  // 1. 创建项目主表
  QString createProjectTable =
      "CREATE TABLE IF NOT EXISTS t_project ("
      "p_name TEXT PRIMARY KEY, "
      "create_time DATETIME, "
      "modify_time DATETIME)";

  // 2. 创建资源路径表
  QString createAssetsTable =
      "CREATE TABLE IF NOT EXISTS t_project_assets ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "p_name TEXT, "
      "file_path TEXT, "
      "lon REAL DEFAULT 0, "
      "lat REAL DEFAULT 0, "
      "alt REAL DEFAULT 0, "
      "FOREIGN KEY(p_name) REFERENCES t_project(p_name) ON DELETE CASCADE)";

  if (!query.exec(createProjectTable) || !query.exec(createAssetsTable)) {
    m_lastError = query.lastError().text();
    return false;
  }
  return true;
}

bool ProjectDataManager::isProjectExists(const QString& pName) {
  QSqlQuery query;
  query.prepare("SELECT p_name FROM t_project WHERE p_name = ?");
  query.addBindValue(pName);
  return query.exec() && query.next();
}

bool ProjectDataManager::saveProject(const QString& pName,
                                     const QList<AssetRecord>& assets,
                                     bool isUpdate) {
  QSqlDatabase db = QSqlDatabase::database();
  if (!db.transaction()) return false;

  QSqlQuery query;
  QString currentTime =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

  if (!isUpdate) {
    // A1. 新建项目
    query.prepare(
        "INSERT INTO t_project (p_name, create_time, modify_time) VALUES (?, "
        "?, ?)");
    query.addBindValue(pName);
    query.addBindValue(currentTime);
    query.addBindValue(currentTime);
  } else {
    // A2. 编辑项目：更新时间并清空旧资产
    query.prepare("UPDATE t_project SET modify_time = ? WHERE p_name = ?");
    query.addBindValue(currentTime);
    query.addBindValue(pName);
    if (!query.exec()) {
      db.rollback();
      return false;
    }

    query.prepare("DELETE FROM t_project_assets WHERE p_name = ?");
    query.addBindValue(pName);
  }

  if (!query.exec()) {
    m_lastError = query.lastError().text();
    db.rollback();
    return false;
  }

  // B. 批量插入新资产
  query.prepare(
      "INSERT INTO t_project_assets (p_name, file_path, lon, lat, alt) VALUES "
      "(?, ?, ?, ?, ?)");
  for (const auto& asset : assets) {
    query.addBindValue(pName);
    query.addBindValue(asset.filePath);
    query.addBindValue(asset.lon);
    query.addBindValue(asset.lat);
    query.addBindValue(asset.alt);
    if (!query.exec()) {
      m_lastError = query.lastError().text();
      db.rollback();
      return false;
    }
  }

  return db.commit();
}

bool ProjectDataManager::deleteProject(const QString& pName) {
  QSqlDatabase db = QSqlDatabase::database();
  if (!db.transaction()) return false;
  QSqlQuery query;
  // A. 删除子表记录
  query.prepare("DELETE FROM t_project_assets WHERE p_name = ?");
  query.addBindValue(pName);
  if (!query.exec()) {
    db.rollback();
    return false;
  }
  // B. 删除主表记录
  query.prepare("DELETE FROM t_project WHERE p_name = ?");
  query.addBindValue(pName);
  if (query.exec()) {
    db.commit();
    return true;
  } else {
    db.rollback();
    return false;
  }
}

QList<AssetRecord> ProjectDataManager::getProjectAssets(const QString& pName) {
  QList<AssetRecord> list;
  QSqlQuery query;
  query.prepare(
      "SELECT file_path, lon, lat, alt FROM t_project_assets WHERE p_name = ?");
  query.addBindValue(pName);

  if (query.exec()) {
    while (query.next()) {
      AssetRecord rec;
      rec.filePath = query.value(0).toString();
      rec.lon = query.value(1).toDouble();
      rec.lat = query.value(2).toDouble();
      rec.alt = query.value(3).toDouble();
      list.append(rec);
    }
  }
  return list;
}

QString ProjectDataManager::lastError() { return m_lastError; }