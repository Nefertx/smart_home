#pragma once
#include <QWidget>
#include <QString>
#include <QList>
#include <QMap>
#include <QPoint>
#include <QVariant>

class QListWidget;
class QListWidgetItem;

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
    void onCardItemClicked(QListWidgetItem* item);
    void onCardContextMenu(const QPoint& pos);
    void onCardOrderChanged();

private:
    void setupUI();
    void loadDevices();
    void addDeviceCard(const QMap<QString, QVariant>& device);
    void onCardClicked(int deviceId);
    void showDeviceControlDialog(const QMap<QString, QVariant>& device);
    bool tryPairDevice(int deviceId, const QMap<QString, QVariant>& device);
    void restoreDeviceOrderFromSetting();
    void persistDeviceOrderToSetting();
    int orderRank(int deviceId) const;
    static QMap<QString, QString> parseParams(const QString& params);
    static QString buildParams(const QMap<QString, QString>& kv);

    QString m_username;
    int     m_selectedDeviceId = -1;

    QList<QMap<QString, QVariant>> m_devices;
    QList<int> m_deviceOrder;

    class QComboBox*    m_filterCombo = nullptr;
    class QListWidget*  m_cardList    = nullptr;
};
