#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QLabel>

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);

signals:
    void loginSuccess(const QString& username);

private slots:
    void onLogin();
    void onRegister();
    void onResetPassword();

private:
    QLineEdit* m_userEdit  = nullptr;
    QLineEdit* m_passEdit  = nullptr;
    QLabel*    m_statusLbl = nullptr;
    void setupUI();
};
