#pragma once
#include <QWidget>
#include <QString>

class AlarmWidget : public QWidget {
    Q_OBJECT
public:
    explicit AlarmWidget(QWidget* parent = nullptr);
    void addAlarm(const QString& type, const QString& content, const QString& deviceName);

signals:
    void alarmTriggered(const QString& message);

private slots:
    void onDeleteAlarm();
    void onClearAlarms();
    void onCheckAlarms();

private:
    void setupUI();
    void loadAlarms();
    void checkThresholds();

    class QTableWidget* m_table        = nullptr;
    class QSpinBox*     m_tempThresh   = nullptr;
    class QSpinBox*     m_humidThresh  = nullptr;
    class QLabel*       m_statusLbl    = nullptr;
    class QTimer*       m_checkTimer   = nullptr;
};
