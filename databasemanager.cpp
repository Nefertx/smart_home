#include "databasemanager.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QDateTime>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance() {
    if (!m_instance) m_instance = new DatabaseManager();
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject* parent) : QObject(parent) {}

bool DatabaseManager::init(const QString& dbPath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbPath);
    if (!m_db.open()) {
        qCritical() << "DB open error:" << m_db.lastError().text();
        return false;
    }
    createTables();
    // 初始化默认管理员
    QSqlQuery q(m_db);
    q.exec("SELECT COUNT(*) FROM users");
    if (q.next() && q.value(0).toInt() == 0) {
        addUser("admin", "admin123", "admin");
    }
    return true;
}

void DatabaseManager::createTables() {
    QSqlQuery q(m_db);
    q.exec("PRAGMA foreign_keys = ON");

    q.exec(R"(CREATE TABLE IF NOT EXISTS users(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE NOT NULL,
        password TEXT NOT NULL,
        role TEXT DEFAULT 'user',
        created_at TEXT DEFAULT (datetime('now','localtime'))
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS devices(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        type TEXT NOT NULL,
        grp TEXT DEFAULT '默认',
        ip TEXT DEFAULT '127.0.0.1',
        port INTEGER DEFAULT 8080,
        status TEXT DEFAULT 'offline',
        params TEXT DEFAULT '',
        created_at TEXT DEFAULT (datetime('now','localtime'))
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS scenes(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT UNIQUE NOT NULL,
        description TEXT DEFAULT '',
        created_at TEXT DEFAULT (datetime('now','localtime'))
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS scene_devices(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        scene_id INTEGER NOT NULL,
        device_id INTEGER NOT NULL,
        action TEXT NOT NULL,
        params TEXT DEFAULT ''
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS operation_logs(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT DEFAULT 'admin',
        device_name TEXT NOT NULL,
        device_type TEXT DEFAULT '',
        action TEXT NOT NULL,
        result TEXT DEFAULT 'success',
        created_at TEXT DEFAULT (datetime('now','localtime'))
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS env_data(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        temperature REAL DEFAULT 0,
        humidity REAL DEFAULT 0,
        air_quality REAL DEFAULT 0,
        created_at TEXT DEFAULT (datetime('now','localtime'))
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS alarms(
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        type TEXT NOT NULL,
        content TEXT NOT NULL,
        device_name TEXT DEFAULT '',
        is_read INTEGER DEFAULT 0,
        created_at TEXT DEFAULT (datetime('now','localtime'))
    ))");

    q.exec(R"(CREATE TABLE IF NOT EXISTS settings(
        key TEXT PRIMARY KEY,
        value TEXT NOT NULL
    ))");

    // 初始化一些演示设备
    q.exec("SELECT COUNT(*) FROM devices");
    if (q.next() && q.value(0).toInt() == 0) {
        QStringList initDevices = {
            "INSERT INTO devices(name,type,grp,ip,port,status,params) VALUES('客厅灯','灯光','客厅','127.0.0.1',8081,'online','power=on;brightness=80;color=暖白')",
            "INSERT INTO devices(name,type,grp,ip,port,status) VALUES('卧室灯','灯光','卧室','127.0.0.1',8082,'offline')",
            "INSERT INTO devices(name,type,grp,ip,port,status,params) VALUES('客厅空调','空调','客厅','127.0.0.1',8083,'online','power=on;mode=制冷;temp=24;fan=中')",
            "INSERT INTO devices(name,type,grp,ip,port,status,params) VALUES('客厅窗帘','窗帘','客厅','127.0.0.1',8084,'online','power=on;open=60')",
            "INSERT INTO devices(name,type,grp,ip,port,status,params) VALUES('门口摄像头','摄像头','门口','127.0.0.1',8085,'online','power=on')"
        };
        for (const auto& sql : initDevices) q.exec(sql);

        QStringList envSeed = {
            "INSERT INTO env_data(temperature,humidity,air_quality) VALUES(25.2,56.0,31.2)",
            "INSERT INTO env_data(temperature,humidity,air_quality) VALUES(24.8,57.3,30.6)",
            "INSERT INTO env_data(temperature,humidity,air_quality) VALUES(25.0,55.4,29.9)"
        };
        for (const auto& sql : envSeed) q.exec(sql);

        // 初始化演示场景
        q.exec("INSERT INTO scenes(name,description) VALUES('回家模式','开灯、开空调、拉开窗帘')");
        q.exec("INSERT INTO scenes(name,description) VALUES('睡眠模式','关灯、关空调、关闭窗帘')");
        q.exec("INSERT INTO scenes(name,description) VALUES('离家模式','关闭所有设备')");
    }
}

QString DatabaseManager::hashPassword(const QString& password) {
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool DatabaseManager::addUser(const QString& username, const QString& password, const QString& role) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO users(username,password,role) VALUES(?,?,?)");
    q.addBindValue(username);
    q.addBindValue(hashPassword(password));
    q.addBindValue(role);
    return q.exec();
}

bool DatabaseManager::validateUser(const QString& username, const QString& password) {
    QSqlQuery q(m_db);
    q.prepare("SELECT id FROM users WHERE username=? AND password=?");
    q.addBindValue(username);
    q.addBindValue(hashPassword(password));
    q.exec();
    return q.next();
}

bool DatabaseManager::resetPassword(const QString& username, const QString& newPassword) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE users SET password=? WHERE username=?");
    q.addBindValue(hashPassword(newPassword));
    q.addBindValue(username);
    return q.exec() && q.numRowsAffected() > 0;
}

bool DatabaseManager::addDevice(const QString& name, const QString& type,
                                const QString& group, const QString& ip, int port) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO devices(name,type,grp,ip,port) VALUES(?,?,?,?,?)");
    q.addBindValue(name); q.addBindValue(type); q.addBindValue(group);
    q.addBindValue(ip); q.addBindValue(port);
    return q.exec();
}

bool DatabaseManager::deleteDevice(int id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM devices WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::updateDevice(int id, const QString& name,
                                   const QString& type, const QString& group) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE devices SET name=?,type=?,grp=? WHERE id=?");
    q.addBindValue(name); q.addBindValue(type); q.addBindValue(group); q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::updateDeviceStatus(int id, const QString& status, const QString& params) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE devices SET status=?,params=? WHERE id=?");
    q.addBindValue(status); q.addBindValue(params); q.addBindValue(id);
    return q.exec();
}

QList<QMap<QString,QVariant>> DatabaseManager::getDevices() {
    QList<QMap<QString,QVariant>> result;
    QSqlQuery q("SELECT id,name,type,grp,ip,port,status,params FROM devices ORDER BY grp,type", m_db);
    while (q.next()) {
        QMap<QString,QVariant> row;
        row["id"] = q.value(0); row["name"] = q.value(1);
        row["type"] = q.value(2); row["grp"] = q.value(3);
        row["ip"] = q.value(4); row["port"] = q.value(5);
        row["status"] = q.value(6); row["params"] = q.value(7);
        result.append(row);
    }
    return result;
}

QMap<QString,QVariant> DatabaseManager::getDeviceById(int id) {
    QMap<QString,QVariant> row;
    QSqlQuery q(m_db);
    q.prepare("SELECT id,name,type,grp,ip,port,status,params FROM devices WHERE id=?");
    q.addBindValue(id); q.exec();
    if (q.next()) {
        row["id"] = q.value(0); row["name"] = q.value(1);
        row["type"] = q.value(2); row["grp"] = q.value(3);
        row["ip"] = q.value(4); row["port"] = q.value(5);
        row["status"] = q.value(6); row["params"] = q.value(7);
    }
    return row;
}

bool DatabaseManager::addScene(const QString& name, const QString& description) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO scenes(name,description) VALUES(?,?)");
    q.addBindValue(name); q.addBindValue(description);
    return q.exec();
}

bool DatabaseManager::deleteScene(int id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM scenes WHERE id=?");
    q.addBindValue(id);
    bool ok = q.exec();
    q.prepare("DELETE FROM scene_devices WHERE scene_id=?");
    q.addBindValue(id);
    q.exec();
    return ok;
}

bool DatabaseManager::updateScene(int id, const QString& name, const QString& description) {
    QSqlQuery q(m_db);
    q.prepare("UPDATE scenes SET name=?,description=? WHERE id=?");
    q.addBindValue(name); q.addBindValue(description); q.addBindValue(id);
    return q.exec();
}

QList<QMap<QString,QVariant>> DatabaseManager::getScenes() {
    QList<QMap<QString,QVariant>> result;
    QSqlQuery q("SELECT id,name,description FROM scenes", m_db);
    while (q.next()) {
        QMap<QString,QVariant> row;
        row["id"] = q.value(0); row["name"] = q.value(1); row["description"] = q.value(2);
        result.append(row);
    }
    return result;
}

bool DatabaseManager::addSceneDevice(int sceneId, int deviceId,
                                     const QString& action, const QString& params) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO scene_devices(scene_id,device_id,action,params) VALUES(?,?,?,?)");
    q.addBindValue(sceneId); q.addBindValue(deviceId);
    q.addBindValue(action); q.addBindValue(params);
    return q.exec();
}

bool DatabaseManager::deleteSceneDevices(int sceneId) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM scene_devices WHERE scene_id=?");
    q.addBindValue(sceneId);
    return q.exec();
}

QList<QMap<QString,QVariant>> DatabaseManager::getSceneDevices(int sceneId) {
    QList<QMap<QString,QVariant>> result;
    QSqlQuery q(m_db);
    q.prepare("SELECT sd.device_id,d.name,d.type,sd.action,sd.params "
              "FROM scene_devices sd JOIN devices d ON sd.device_id=d.id WHERE sd.scene_id=?");
    q.addBindValue(sceneId); q.exec();
    while (q.next()) {
        QMap<QString,QVariant> row;
        row["device_id"] = q.value(0); row["name"] = q.value(1);
        row["type"] = q.value(2); row["action"] = q.value(3); row["params"] = q.value(4);
        result.append(row);
    }
    return result;
}

bool DatabaseManager::addOperationLog(const QString& user, const QString& deviceName,
                                      const QString& action, const QString& result) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO operation_logs(username,device_name,action,result) VALUES(?,?,?,?)");
    q.addBindValue(user); q.addBindValue(deviceName);
    q.addBindValue(action); q.addBindValue(result);
    return q.exec();
}

QList<QMap<QString,QVariant>> DatabaseManager::getOperationLogs(const QString& deviceType,
                                                                  const QString& startDate,
                                                                  const QString& endDate) {
    QList<QMap<QString,QVariant>> result;
    QString sql = "SELECT id,username,device_name,action,result,created_at FROM operation_logs WHERE 1=1";
    if (!deviceType.isEmpty()) sql += " AND device_name LIKE '%" + deviceType + "%'";
    if (!startDate.isEmpty()) sql += " AND created_at >= '" + startDate + "'";
    if (!endDate.isEmpty()) sql += " AND created_at <= '" + endDate + " 23:59:59'";
    sql += " ORDER BY created_at DESC LIMIT 500";
    QSqlQuery q(sql, m_db);
    while (q.next()) {
        QMap<QString,QVariant> row;
        row["id"] = q.value(0); row["username"] = q.value(1);
        row["device_name"] = q.value(2); row["action"] = q.value(3);
        row["result"] = q.value(4); row["created_at"] = q.value(5);
        result.append(row);
    }
    return result;
}

bool DatabaseManager::addEnvData(double temperature, double humidity, double airQuality) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO env_data(temperature,humidity,air_quality) VALUES(?,?,?)");
    q.addBindValue(temperature); q.addBindValue(humidity); q.addBindValue(airQuality);
    return q.exec();
}

QList<QMap<QString,QVariant>> DatabaseManager::getEnvData(const QString& startDate,
                                                           const QString& endDate) {
    QList<QMap<QString,QVariant>> result;
    QString sql = "SELECT temperature,humidity,air_quality,created_at FROM env_data WHERE 1=1";
    if (!startDate.isEmpty()) sql += " AND created_at >= '" + startDate + "'";
    if (!endDate.isEmpty()) sql += " AND created_at <= '" + endDate + " 23:59:59'";
    sql += " ORDER BY created_at DESC LIMIT 1000";
    QSqlQuery q(sql, m_db);
    while (q.next()) {
        QMap<QString,QVariant> row;
        row["temperature"] = q.value(0); row["humidity"] = q.value(1);
        row["air_quality"] = q.value(2); row["created_at"] = q.value(3);
        result.append(row);
    }
    return result;
}

bool DatabaseManager::addAlarm(const QString& type, const QString& content,
                                const QString& deviceName) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO alarms(type,content,device_name) VALUES(?,?,?)");
    q.addBindValue(type); q.addBindValue(content); q.addBindValue(deviceName);
    return q.exec();
}

QList<QMap<QString,QVariant>> DatabaseManager::getAlarms() {
    QList<QMap<QString,QVariant>> result;
    QSqlQuery q("SELECT id,type,content,device_name,is_read,created_at FROM alarms ORDER BY created_at DESC", m_db);
    while (q.next()) {
        QMap<QString,QVariant> row;
        row["id"] = q.value(0); row["type"] = q.value(1);
        row["content"] = q.value(2); row["device_name"] = q.value(3);
        row["is_read"] = q.value(4); row["created_at"] = q.value(5);
        result.append(row);
    }
    return result;
}

bool DatabaseManager::deleteAlarm(int id) {
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM alarms WHERE id=?");
    q.addBindValue(id);
    return q.exec();
}

bool DatabaseManager::clearAlarms() {
    return QSqlQuery("DELETE FROM alarms", m_db).exec();
}

bool DatabaseManager::setSetting(const QString& key, const QString& value) {
    QSqlQuery q(m_db);
    q.prepare("INSERT OR REPLACE INTO settings(key,value) VALUES(?,?)");
    q.addBindValue(key); q.addBindValue(value);
    return q.exec();
}

QString DatabaseManager::getSetting(const QString& key, const QString& defaultVal) {
    QSqlQuery q(m_db);
    q.prepare("SELECT value FROM settings WHERE key=?");
    q.addBindValue(key); q.exec();
    return q.next() ? q.value(0).toString() : defaultVal;
}
