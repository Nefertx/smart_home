#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>

class QPushButton;
class QFormLayout;

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

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
    QLabel*    m_heroTitle = nullptr;
    QLabel*    m_heroSub   = nullptr;
    QLabel*    m_hintLbl   = nullptr;
    QPushButton* m_loginBtn = nullptr;
    class QFormLayout* m_formLayout = nullptr;
    class QGroupBox* m_loginBox = nullptr;
    QPixmap    m_bgPixmap;
    void setupUI();
    void applyResponsiveScale();
};
