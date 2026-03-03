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

  // 2. 创建资源路径表 (存放项目中用到的所有原始资源)
  QString createAssetsTable =
      "CREATE TABLE IF NOT EXISTS t_project_assets ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "p_name TEXT, "
      "file_path TEXT, "  // 存放文件路径，导线可存为 "INTERNAL_LINE"
      "lon REAL DEFAULT 0, "  // 原有字段，可保留用于兼容旧的模型点坐标
      "lat REAL DEFAULT 0, "
      "alt REAL DEFAULT 0, "
      "start_x REAL DEFAULT 0, "  // 导线起点 X (或经度)
      "start_y REAL DEFAULT 0, "  // 导线起点 Y (或纬度)
      "start_z REAL DEFAULT 0, "  // 导线起点 Z (或高度)
      "end_x REAL DEFAULT 0, "    // 导线终点 X
      "end_y REAL DEFAULT 0, "    // 导线终点 Y
      "end_z REAL DEFAULT 0, "    // 导线终点 Z
      "FOREIGN KEY(p_name) REFERENCES t_project(p_name) ON DELETE CASCADE)";

  // 3. 补充：创建剧本步骤表 (存放具体的执行逻辑顺序)
  QString createScenarioTable =
      "CREATE TABLE IF NOT EXISTS t_scenario_steps ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "scenario_name TEXT, "  /* 剧本的唯一名称 */
      "step_index INTEGER, "  /* 执行顺序 1, 2, 3... */
      "file_path TEXT, "      /* 模型文件路径 */
      "action_type INTEGER, " /* 0-添加(IN), 1-删除(OUT) */
      "lon REAL DEFAULT 0, "
      "lat REAL DEFAULT 0, "
      "alt REAL DEFAULT 0)";

  QString createIndex =
      "CREATE INDEX IF NOT EXISTS idx_scenario_name ON "
      "t_scenario_steps(scenario_name)";
  // 执行创建操作
  if (!query.exec(createProjectTable) || !query.exec(createAssetsTable) ||
      !query.exec(createScenarioTable) || !query.exec(createIndex)) {
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
      "INSERT INTO t_project_assets (p_name, file_path, lon, lat, alt, "
      "start_x, start_y, start_z, end_x, end_y, end_z) "
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

  for (const auto& asset : assets) {
    query.addBindValue(pName);
    query.addBindValue(asset.filePath);
    query.addBindValue(asset.lon);
    query.addBindValue(asset.lat);
    query.addBindValue(asset.alt);
    // 新增字段绑定
    query.addBindValue(asset.start_x);
    query.addBindValue(asset.start_y);
    query.addBindValue(asset.start_z);
    query.addBindValue(asset.end_x);
    query.addBindValue(asset.end_y);
    query.addBindValue(asset.end_z);

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
  // 查询字段必须与赋值时的索引对应
  query.prepare(
      "SELECT file_path, lon, lat, alt, start_x, start_y, start_z, end_x, "
      "end_y, end_z "
      "FROM t_project_assets WHERE p_name = ?");
  query.addBindValue(pName);

  if (query.exec()) {
    while (query.next()) {
      AssetRecord rec;
      rec.filePath = query.value(0).toString();
      rec.lon = query.value(1).toDouble();
      rec.lat = query.value(2).toDouble();
      rec.alt = query.value(3).toDouble();

      // 读取新增的 6 个坐标字段
      rec.start_x = query.value(4).toDouble();
      rec.start_y = query.value(5).toDouble();
      rec.start_z = query.value(6).toDouble();
      rec.end_x = query.value(7).toDouble();
      rec.end_y = query.value(8).toDouble();
      rec.end_z = query.value(9).toDouble();

      list.append(rec);
    }
  } else {
    m_lastError = query.lastError().text();
  }
  return list;
}

QString ProjectDataManager::lastError() { return m_lastError; }