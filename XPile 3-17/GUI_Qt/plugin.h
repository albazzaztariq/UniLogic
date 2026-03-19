#pragma once
// plugin.h -- Plugin interface for UniLogic Project Manager

#include <QString>
#include <QtPlugin>

class MainWindow;

class IUniLogicPlugin
{
public:
    virtual ~IUniLogicPlugin() = default;
    virtual QString name() = 0;
    virtual QString version() = 0;
    virtual void initialize(MainWindow *window) = 0;
    virtual void shutdown() = 0;
};

#define IUniLogicPlugin_iid "com.unilogic.Plugin/1.0"
Q_DECLARE_INTERFACE(IUniLogicPlugin, IUniLogicPlugin_iid)
