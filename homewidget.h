#pragma once
#include <QWidget>
#include <QString>

class HomeWidget : public QWidget {
    Q_OBJECT
public:
    explicit HomeWidget(const QString& username, QWidget* parent = nullptr);

signals:
    void navigateTo(int pageIndex);

public slots:
    void refresh();

private:
    void setupUI();
    void updateStats();
    QString m_username;

    class QLabel* m_onlineLbl  = nullptr;
    class QLabel* m_offlineLbl = nullptr;
    class QLabel* m_alarmLbl   = nullptr;
    class QLabel* m_timeLbl    = nullptr;
    class QTimer* m_timer      = nullptr;
};
