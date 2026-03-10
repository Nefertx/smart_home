#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager* instance();
    bool init(const QString& dbPath = "smart_home.db");

    // 用户操作
    bool addUser(const QString& username, const QString& password, const QString& role = "user");
    bool validateUser(const QString& username, const QString& password);
    bool resetPassword(const QString& username, const QString& newPassword);

    // 设备操作
    bool addDevice(const QString& name, const QString& type,
                   const QString& group, const QString& ip, int port);
    bool deleteDevice(int id);
    bool updateDevice(int id, const QString& name, const QString& type, const QString& group);
    bool updateDeviceStatus(int id, const QString& status, const QString& params);
    QList<QMap<QString,QVariant>> getDevices();
    QMap<QString,QVariant> getDeviceById(int id);

    // 场景操作
    bool addScene(const QString& name, const QString& description);
    bool deleteScene(int id);
    bool updateScene(int id, const QString& name, const QString& description);
    QList<QMap<QString,QVariant>> getScenes();
    bool addSceneDevice(int sceneId, int deviceId, const QString& action, const QString& params);
    bool deleteSceneDevices(int sceneId);
    QList<QMap<QString,QVariant>> getSceneDevices(int sceneId);

    // 操作日志
    bool addOperationLog(const QString& user, const QString& deviceName,
                         const QString& action, const QString& result);
    QList<QMap<QString,QVariant>> getOperationLogs(const QString& deviceType = "",
                                                    const QString& startDate = "",
                                                    const QString& endDate = "");

    // 环境数据
    bool addEnvData(double temperature, double humidity, double airQuality);
    QList<QMap<QString,QVariant>> getEnvData(const QString& startDate = "",
                                              const QString& endDate = "");

    // 报警记录
    bool addAlarm(const QString& type, const QString& content, const QString& deviceName);
    QList<QMap<QString,QVariant>> getAlarms();
    bool deleteAlarm(int id);
    bool clearAlarms();

    // 系统设置
    bool setSetting(const QString& key, const QString& value);
    QString getSetting(const QString& key, const QString& defaultVal = "");

private:
    explicit DatabaseManager(QObject* parent = nullptr);
    static DatabaseManager* m_instance;
    QSqlDatabase m_db;
    void createTables();
    QString hashPassword(const QString& password);
};
