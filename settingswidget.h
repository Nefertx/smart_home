#pragma once
#include <QWidget>

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);

private slots:
    void onAddDevice();
    void onDeleteDevice();
    void onEditDevice();
    void onTestConnection();
    void onSaveSettings();

private:
    void setupUI();
    void loadDevices();

    class QTableWidget* m_deviceTable   = nullptr;
    class QComboBox*    m_themeCombo    = nullptr;
    class QSpinBox*     m_refreshSpin   = nullptr;
    class QLineEdit*    m_dbPathEdit    = nullptr;
    class QPushButton*  m_testBtn       = nullptr;
};
