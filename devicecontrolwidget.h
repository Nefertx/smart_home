#pragma once
#include <QWidget>
#include <QString>
#include <QList>
#include <QMap>
#include <QVariant>

class DeviceControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit DeviceControlWidget(const QString& username, QWidget* parent = nullptr);

signals:
    void deviceChanged();

private slots:
    void onRefresh();
    void onAddDevice();
    void onEditDevice();
    void onDeleteDevice();

private:
    void setupUI();
    void loadDevices();
    void addDeviceCard(const QMap<QString, QVariant>& device, int index);
    void onCardClicked(int deviceId);
    void showDeviceControlDialog(const QMap<QString, QVariant>& device);
    bool tryPairDevice(int deviceId, const QMap<QString, QVariant>& device);
    static QMap<QString, QString> parseParams(const QString& params);
    static QString buildParams(const QMap<QString, QString>& kv);

    QString m_username;
    int     m_selectedDeviceId = -1;

    QList<QMap<QString, QVariant>> m_devices;

    class QComboBox*    m_filterCombo = nullptr;
    class QGridLayout*  m_cardGrid    = nullptr;
    class QWidget*      m_cardHost    = nullptr;
};
