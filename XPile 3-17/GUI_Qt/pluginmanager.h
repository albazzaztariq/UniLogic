#pragma once
#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

class MainWindow;
class IUniLogicPlugin;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(MainWindow *window, QObject *parent = nullptr);
    void loadPlugins();
    QVector<QPair<QString, QString>> loadedPlugins() const;
    void shutdownAll();
private:
    MainWindow *m_window = nullptr;
    QVector<IUniLogicPlugin*> m_plugins;
};
