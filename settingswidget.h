#pragma once
#include <QWidget>

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    explicit SettingsWidget(QWidget* parent = nullptr);

private slots:
    void onSaveSettings();

private:
    void setupUI();
    class QComboBox*    m_themeCombo    = nullptr;
    class QSpinBox*     m_refreshSpin   = nullptr;
    class QLineEdit*    m_dbPathEdit    = nullptr;
};
