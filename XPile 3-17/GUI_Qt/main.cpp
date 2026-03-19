// main.cpp -- UniLogic Project Manager entry point with crash handling

#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QClipboard>
#include "mainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shobjidl.h>
#endif

#include <csignal>

// ── Crash log path ──────────────────────────────────────────────────────

static QString crashLogPath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/crash_log.txt";
}

// ── Signal handler (segfault etc.) ──────────────────────────────────────

static void signalHandler(int sig)
{
    QString path = crashLogPath();
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "UniLogic Project Manager Crash Report\n";
        out << "Timestamp: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
        out << "Signal: " << sig << "\n";
        file.close();
    }
    std::_Exit(1);
}

// ── Check for previous crash ────────────────────────────────────────────

static void checkCrashLog()
{
    QString path = crashLogPath();
    QFile file(path);
    if (!file.exists()) return;

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = QTextStream(&file).readAll();
        file.close();

        QMessageBox box;
        box.setWindowTitle("Previous Crash Detected");
        box.setText("UniLogic Project Manager crashed during the last session.");
        box.setDetailedText(content);
        box.addButton("Dismiss", QMessageBox::AcceptRole);
        auto *copyBtn = box.addButton("Copy to Clipboard", QMessageBox::ActionRole);
        box.exec();

        if (box.clickedButton() == copyBtn)
            QApplication::clipboard()->setText(content);

        QFile::remove(path);
    }
}

// ── Main ────────────────────────────────────────────────────────────────

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    SetCurrentProcessExplicitAppUserModelID(L"UniLogic.ProjectManager");
#endif

    // Install signal handlers for crash reporting
    std::signal(SIGSEGV, signalHandler);
    std::signal(SIGABRT, signalHandler);
    std::signal(SIGFPE,  signalHandler);

    QApplication app(argc, argv);
    app.setApplicationName("UniLogic Project Manager");
    app.setOrganizationName("UniLogic");

    // Check for crash from last session
    checkCrashLog();

    int result = 0;
    try {
        MainWindow window;

        window.resize(1000, 700);
        if (auto *screen = QApplication::primaryScreen()) {
            QRect geo = screen->geometry();
            window.move((geo.width()  - 1000) / 2,
                         (geo.height() - 700)  / 2);
        }

        window.show();
        result = app.exec();
    } catch (const std::exception &e) {
        // Write crash log
        QFile file(crashLogPath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "UniLogic Project Manager Crash Report\n";
            out << "Timestamp: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
            out << "Exception: " << e.what() << "\n";
            file.close();
        }
        QMessageBox::critical(nullptr, "UniLogic Project Manager Error",
            QStringLiteral("The application encountered an error:\n%1\n\n"
                "A crash log has been saved.").arg(e.what()));
        return 1;
    }

    return result;
}
