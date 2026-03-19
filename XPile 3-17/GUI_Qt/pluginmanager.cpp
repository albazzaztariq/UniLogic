#include "pluginmanager.h"
#include "plugin.h"
#include "mainwindow.h"
#include <QDir>
#include <QPluginLoader>
#include <QStandardPaths>
#include <QCoreApplication>

PluginManager::PluginManager(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window)
{
}

void PluginManager::loadPlugins()
{
    QString pluginDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                        + "/plugins";
    QDir dir(pluginDir);
    if (!dir.exists()) {
        dir.mkpath(".");
        return;
    }

    for (const QString &fileName : dir.entryList({"*.dll"}, QDir::Files)) {
        QPluginLoader loader(dir.absoluteFilePath(fileName));
        QObject *obj = loader.instance();
        if (!obj) continue;

        auto *plugin = qobject_cast<IUniLogicPlugin*>(obj);
        if (plugin) {
            plugin->initialize(m_window);
            m_plugins.append(plugin);
        }
    }
}

QVector<QPair<QString, QString>> PluginManager::loadedPlugins() const
{
    QVector<QPair<QString, QString>> result;
    for (auto *p : m_plugins)
        result.append({p->name(), p->version()});
    return result;
}

void PluginManager::shutdownAll()
{
    for (auto *p : m_plugins)
        p->shutdown();
    m_plugins.clear();
}
