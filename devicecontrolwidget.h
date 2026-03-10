#pragma once
#include <QWidget>
#include <QString>

class DeviceControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit DeviceControlWidget(const QString& username, QWidget* parent = nullptr);

signals:
    void deviceChanged();

private slots:
    void onDeviceSelected(int row);
    void onToggleDevice();
    void onSetParam();
    void onRefresh();

private:
    void setupUI();
    void loadDevices();
    void updateControlPanel(const QMap<QString,QVariant>& device);
    void sendCommand(int deviceId, const QString& cmd, const QString& params);

    QString m_username;
    int     m_selectedDeviceId = -1;

    class QListWidget*  m_deviceList  = nullptr;
    class QLabel*       m_nameLbl     = nullptr;
    class QLabel*       m_typeLbl     = nullptr;
    class QLabel*       m_statusLbl   = nullptr;
    class QPushButton*  m_toggleBtn   = nullptr;
    class QSlider*      m_paramSlider = nullptr;
    class QLabel*       m_paramLbl    = nullptr;
    class QComboBox*    m_filterCombo = nullptr;
    class QWidget*      m_ctrlPanel   = nullptr;
};
