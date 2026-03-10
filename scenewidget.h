#pragma once
#include <QWidget>
#include <QString>

class SceneWidget : public QWidget {
    Q_OBJECT
public:
    explicit SceneWidget(const QString& username, QWidget* parent = nullptr);

private slots:
    void onAddScene();
    void onDeleteScene();
    void onActivateScene();
    void onEditScene();
    void onSceneSelected(int row);

private:
    void setupUI();
    void loadScenes();

    QString m_username;
    int     m_selectedSceneId = -1;

    class QListWidget* m_sceneList   = nullptr;
    class QLabel*      m_descLbl     = nullptr;
    class QListWidget* m_deviceList  = nullptr;
    class QPushButton* m_activateBtn = nullptr;
};
