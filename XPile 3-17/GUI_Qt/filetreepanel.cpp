// filetreepanel.cpp -- Collapsible project file tree panel

#include "filetreepanel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFile>
#include "templatechooser.h"

FileTreePanel::FileTreePanel(const ThemeColors &colors, QWidget *parent)
    : QWidget(parent), m_colors(colors)
{
    setMinimumWidth(180);
    setMaximumWidth(350);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // ── Header ──────────────────────────────────────────────────────────
    auto *headerRow = new QHBoxLayout;
    headerRow->setContentsMargins(8, 6, 8, 6);

    m_titleLbl = new QLabel("EXPLORER");
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 9pt; font-weight: bold; color: %1; background: transparent;")
            .arg(m_colors.textDim));
    headerRow->addWidget(m_titleLbl);
    headerRow->addStretch();

    m_collapseBtn = new QPushButton(QStringLiteral("\u2630"));  // hamburger
    m_collapseBtn->setFixedSize(24, 24);
    m_collapseBtn->setStyleSheet(
        "QPushButton { background: transparent; border: none; color: "
        + m_colors.textDim + "; font-size: 12pt; }"
        "QPushButton:hover { color: " + m_colors.text + "; }");
    m_collapseBtn->setToolTip("Collapse/expand");
    headerRow->addWidget(m_collapseBtn);
    layout->addLayout(headerRow);

    // ── Tree view ───────────────────────────────────────────────────────
    m_model = new QFileSystemModel(this);
    m_model->setReadOnly(false);
    m_model->setNameFilterDisables(false);

    m_tree = new QTreeView;
    m_tree->setModel(m_model);
    m_tree->setAnimated(true);
    m_tree->setIndentation(16);
    m_tree->setSortingEnabled(true);
    m_tree->sortByColumn(0, Qt::AscendingOrder);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Hide size, type, date columns — show only name
    m_tree->hideColumn(1);
    m_tree->hideColumn(2);
    m_tree->hideColumn(3);
    m_tree->header()->hide();

    connect(m_tree, &QTreeView::doubleClicked,
            this, &FileTreePanel::onItemDoubleClicked);
    connect(m_tree, &QTreeView::customContextMenuRequested,
            this, &FileTreePanel::onContextMenu);

    // Toggle visibility
    connect(m_collapseBtn, &QPushButton::clicked, this, [this]() {
        m_tree->setVisible(!m_tree->isVisible());
    });

    layout->addWidget(m_tree, 1);

    // Start hidden until a project is opened
    m_tree->setVisible(false);
}

void FileTreePanel::setRootPath(const QString &path)
{
    m_rootPath = path;
    if (path.isEmpty()) {
        m_tree->setVisible(false);
        return;
    }
    m_model->setRootPath(path);
    m_tree->setRootIndex(m_model->index(path));
    m_tree->setVisible(true);
}

QString FileTreePanel::rootPath() const
{
    return m_rootPath;
}

void FileTreePanel::onThemeChanged(ThemeMode, const ThemeColors &colors)
{
    m_colors = colors;
    m_titleLbl->setStyleSheet(
        QStringLiteral("font-size: 9pt; font-weight: bold; color: %1; background: transparent;")
            .arg(m_colors.textDim));
}

void FileTreePanel::onItemDoubleClicked(const QModelIndex &index)
{
    QString path = m_model->filePath(index);
    if (m_model->isDir(index))
        return;

    // Signal to open in the editor tab
    emit fileActivated(path);
}

void FileTreePanel::onContextMenu(const QPoint &pos)
{
    QModelIndex index = m_tree->indexAt(pos);
    QString path = index.isValid() ? m_model->filePath(index) : m_rootPath;

    QMenu menu(this);
    menu.addAction("New File...", this, &FileTreePanel::newFile)
        ->setData(path);
    if (index.isValid()) {
        menu.addAction("Rename...", this, &FileTreePanel::renameFile)
            ->setData(path);
        menu.addSeparator();
        menu.addAction("Delete", this, &FileTreePanel::deleteFile)
            ->setData(path);
    }
    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}

void FileTreePanel::newFile()
{
    auto *action = qobject_cast<QAction*>(sender());
    QString parentPath = action ? action->data().toString() : m_rootPath;

    QFileInfo fi(parentPath);
    if (fi.isFile())
        parentPath = fi.absolutePath();

    // Show template chooser
    TemplateChooser tmpl(m_colors, this);
    if (tmpl.exec() != QDialog::Accepted) return;

    bool ok;
    QString name = QInputDialog::getText(
        this, "New File", "File name:", QLineEdit::Normal, "new_file.ul", &ok);
    if (!ok || name.isEmpty()) return;

    QString fullPath = QDir(parentPath).filePath(name);
    QFile file(fullPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (!tmpl.selectedContent().isEmpty())
            file.write(tmpl.selectedContent().toUtf8());
        file.close();
        emit fileActivated(fullPath);
    } else {
        QMessageBox::warning(this, "Error",
            QStringLiteral("Could not create file: %1").arg(name));
    }
}

void FileTreePanel::deleteFile()
{
    auto *action = qobject_cast<QAction*>(sender());
    if (!action) return;
    QString path = action->data().toString();
    QFileInfo fi(path);

    auto result = QMessageBox::question(
        this, "Delete",
        QStringLiteral("Delete %1?").arg(fi.fileName()),
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (fi.isDir())
            QDir(path).removeRecursively();
        else
            QFile::remove(path);
    }
}

void FileTreePanel::renameFile()
{
    auto *action = qobject_cast<QAction*>(sender());
    if (!action) return;
    QString oldPath = action->data().toString();
    QFileInfo fi(oldPath);

    bool ok;
    QString newName = QInputDialog::getText(
        this, "Rename", "New name:", QLineEdit::Normal, fi.fileName(), &ok);
    if (!ok || newName.isEmpty() || newName == fi.fileName()) return;

    QString newPath = QDir(fi.absolutePath()).filePath(newName);
    QFile::rename(oldPath, newPath);
}
