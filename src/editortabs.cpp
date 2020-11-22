/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "editortabs.h"
#include "editortab.h"
#include "helper.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QFileDialog>
#include <QShortcut>
#include <iomanip>
#include <sstream>
#include "fileiconprovider.h"

EditorTabs::EditorTabs(SpellCheckerInterface * spellChecker, QTabWidget * widget):
    spellChecker(spellChecker),
    tabWidget(widget)
{
    editor = nullptr;
    blockSig = false;
    tabWidget->setFocusPolicy(Qt::NoFocus);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(switchTab(int)));
    connect(tabWidget->tabBar(), SIGNAL(tabMoved(int,int)), SLOT(movedTab(int,int)));

    tabWidget->setCursor(Qt::ArrowCursor);

    tabWidget->tabBar()->installEventFilter(this);
    tabWidget->installEventFilter(this);
}

Editor * EditorTabs::getActiveEditor()
{
    return editor;
}

Editor * EditorTabs::getTabEditor(int index)
{
    QWidget * tab = tabWidget->widget(index);
    if (tab == nullptr) return nullptr;
    EditorTab * editorTab = static_cast<EditorTab *>(tab);
    return editorTab->getEditor();
}

QString EditorTabs::getTabNameFromPath(QString filepath)
{
    QString tabName = filepath;
    QRegularExpression regexp = QRegularExpression("[/]([^/]+)$");
    QRegularExpressionMatch regexpMatch = regexp.match(filepath);
    if (regexpMatch.capturedStart(1)>=0) {
        tabName = regexpMatch.captured(1);
    }
    return tabName;
}

void EditorTabs::createTab(QString filepath, bool initHighlight)
{
    if (filepath.size() == 0 || !Helper::fileExists(filepath)) return;
    qint64 bytes = Helper::getFileSize(filepath);
    if (bytes >= TOO_BIG_FILE_SIZE) {
        std::stringstream sizeStream;
        sizeStream << std::fixed << std::setprecision(2) << (bytes * 1.0)/1048576;
        QString fileSizeStr = QString::fromStdString(sizeStream.str());
        if (!Helper::showQuestion(tr("Open"), tr("File size: %1 MB. Do you really want to open it ?").arg(fileSizeStr))) return;
    }

    EditorTab * tab = new EditorTab();
    editor = new Editor(spellChecker);
    tab->setEditor(editor);

    QString tabName = getTabNameFromPath(filepath);
    tabWidget->blockSignals(true);
    int tabIndex = tabWidget->addTab(tab, tabName);
    tabWidget->setTabToolTip(tabIndex, filepath);
    tabWidget->setCurrentIndex(tabIndex);
    tabWidget->blockSignals(false);

    editor->setTabIndex(tabIndex);
    editor->init();
    //editor->resetExtraSelections();

    connect(editor, SIGNAL(statusBarText(int, QString)), this, SLOT(statusBarTextChangeRequested(int, QString)));
    connect(editor, SIGNAL(modifiedStateChanged(int, bool)), this, SLOT(editorModifiedStateChanged(int, bool)));
    connect(editor, SIGNAL(filenameChanged(int, QString)), this, SLOT(filenameChanged(int, QString)));
    connect(editor, SIGNAL(saved(int)), this, SLOT(saved(int)));
    connect(editor, SIGNAL(reloaded(int)), this, SLOT(reloaded(int)));
    connect(editor, SIGNAL(ready(int)), this, SLOT(ready(int)));
    connect(editor, SIGNAL(showDeclaration(int,QString)), this, SLOT(showDeclaration(int,QString)));
    connect(editor, SIGNAL(showHelp(int,QString)), this, SLOT(showHelp(int,QString)));
    connect(editor, SIGNAL(parsePHP(int,QString)), this, SLOT(parsePHP(int,QString)));
    connect(editor, SIGNAL(parseJS(int,QString)), this, SLOT(parseJS(int,QString)));
    connect(editor, SIGNAL(parseCSS(int,QString)), this, SLOT(parseCSS(int,QString)));
    connect(editor, SIGNAL(undoRedoChanged(int)), this, SLOT(undoRedoChanged(int)));
    connect(editor, SIGNAL(backForwardChanged(int)), this, SLOT(backForwardChanged(int)));
    connect(editor, SIGNAL(searchInFiles(QString)), this, SLOT(searchInFiles(QString)));
    connect(editor, SIGNAL(focusIn(int)), this, SLOT(focusIn(int)));
    connect(editor, SIGNAL(breadcrumbsClick(int)), this, SLOT(breadcrumbsClick(int)));
    connect(editor, SIGNAL(warning(int,QString,QString)), this, SLOT(warning(int,QString,QString)));
    connect(editor, SIGNAL(showPopupText(int,QString)), this, SLOT(showPopupText(int,QString)));
    connect(editor, SIGNAL(showPopupError(int,QString)), this, SLOT(showPopupError(int,QString)));

    QString txt = Helper::loadFile(filepath, editor->getEncoding(), editor->getFallbackEncoding());
    QString ext = "";
    QRegularExpression regexp = QRegularExpression("[.]([^.]+)$");
    QRegularExpressionMatch regexpMatch = regexp.match(filepath);
    if (regexpMatch.capturedStart(1)>=0) {
        ext = regexpMatch.captured(1);
    }
    editor->reset();
    editor->setFileName(filepath);

    if (txt.size() >= BIG_FILE_SIZE) editor->setIsBigFile(true);
    else editor->setIsBigFile(false);

    editor->convertNewLines(txt);
    editor->setPlainText(txt);
    //editor->resetExtraSelections();
    editor->initMode(ext);
    editor->setFocus();
    editor->detectTabsMode();

    emit tabOpened(tabIndex);
    if (initHighlight) {
        editor->initHighlighter();
    }
}

void EditorTabs::switchTab(int index)
{
    if (index >= 0) {
        editor = getTabEditor(index);
        if (editor != nullptr) {
            editor->updateSizes();
        }
    } else {
        editor = nullptr;
    }
    if (!blockSig) emit tabSwitched(index);
}

void EditorTabs::movedTab(int /*from*/, int /*to*/)
{
    // update indexes
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr) {
            textEditor->setTabIndex(i);
        }
    }
}

void EditorTabs::closeTab(int index)
{
    // check modified
    Editor * textEditor = getTabEditor(index);
    if (textEditor != nullptr && textEditor->isModified() &&
        !Helper::showQuestion(tr("File is not saved"), tr("File \"%1\"  is not saved. Close it anyway ?").arg(textEditor->getFileName()))
        //QMessageBox::question(tabWidget, tr("File is not saved"), tr("File \"%1\"  is not saved. Close it anyway ?").arg(textEditor->getFileName()), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok
    ) {
        return;
    }
    if (textEditor != nullptr) textEditor->setTabIndex(-1);
    blockSig = true;
    bool isSwitched = false;
    if (editor == textEditor) {
        editor = nullptr;
        isSwitched = true;
    }
    QWidget * tab = tabWidget->widget(index);
    tabWidget->removeTab(index);
    // switchTab slot called here (emit blocked)
    if (tab != nullptr) tab->deleteLater();
    // update indexes
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr) {
            textEditor->setTabIndex(i);
        }
    }
    blockSig = false;
    if (isSwitched) {
        if (editor != nullptr) emit tabSwitched(editor->getTabIndex());
        else emit tabSwitched(-1);
    }
    emit tabClosed(index);
}

void EditorTabs::editorModifiedStateChanged(int index, bool m)
{
    Editor * textEditor = getTabEditor(index);
    if (textEditor == nullptr) return;
    QString tabText = tabWidget->tabText(index);
    if (tabText.size()>1 && tabText.mid(tabText.size()-2)==" *") {
        tabText = tabText.mid(0, tabText.size()-2);
    }
    if (m) {
        tabText += " *";
    }
    tabWidget->setTabText(index, tabText);
    emit modifiedStateChanged(m);
}

void EditorTabs::openFile(QString filepath, bool initHighlight)
{
    if (!Helper::fileExists(filepath)) return;
    // check open tab
    for (int i=0; i<tabWidget->count(); i++){
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr && textEditor->getFileName() == filepath) {
            int tabIndex = textEditor->getTabIndex();
            tabWidget->setCurrentIndex(tabIndex);
            return;
        }
    }
    createTab(filepath, initHighlight);
}

void EditorTabs::open(QString dir)
{
    if (editor != nullptr) editor->hidePopups();
    if (dir.size() == 0) dir = QString::fromStdString(Settings::get("file_dialog_path"));
    QString filter = tr(Settings::get("file_dialog_filter").c_str());
    QString fileName = Helper::getExistingFile(tabWidget, tr("Select file"), dir, filter);
    if (fileName.size() > 0) {
        openFile(fileName);
    }
}

void EditorTabs::save()
{
    if (editor != nullptr) editor->save();
}

void EditorTabs::saveAll()
{
    if (editor == nullptr) return;
    Editor * currentEditor = editor;
    for (int i=0; i<tabWidget->count(); i++) {
        editor = getTabEditor(i);
        if (editor != nullptr) {
            editor->save();
        }
    }
    editor = currentEditor;
}

void EditorTabs::saveAs()
{
    if (editor == nullptr) return;
    QString filename = editor->getFileName();
    QFileInfo fInfo(filename);
    QString dir = fInfo.dir().absolutePath();
    if (dir.size() == 0) {
        dir = QString::fromStdString(Settings::get("file_dialog_path"));
        if (dir.size() == 0) {
            QStringList stddirs = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
            if (stddirs.size()>0) dir = stddirs.at(0);
        }
    }
    if (dir.size() == 0) dir = ".";
    std::string filter = Settings::get("file_dialog_filter");
    QString ext = editor->getFileExtension();
    if (ext.size() > 0) filter = "File (*."+ext.toStdString()+");;All files (*)";
    //QString newName = QFileDialog::getSaveFileName(tabWidget, tr("Save as"), dir, tr(filter.c_str()));
    QString newName = Helper::getSaveFileName(tabWidget, tr("Save as"), dir, tr(filter.c_str()));
    if (newName.size() > 0) {
        editor->save(newName);
    }
}

void EditorTabs::close()
{
    if (editor == nullptr) return;
    int tabIndex = editor->getTabIndex();
    closeTab(tabIndex);
}

void EditorTabs::closeSaved()
{
    int i=0;
    while (i<tabWidget->count()) {
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr && !textEditor->isModified()) {
            int tabIndex = textEditor->getTabIndex();
            closeTab(tabIndex);
            continue;
        }
        i++;
    }
}

bool EditorTabs::closeWindowAllowed()
{
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr && textEditor->isModified() &&
            !Helper::showQuestion(tr("File is not saved"), tr("File \"%1\"  is not saved. Close it anyway ?").arg(textEditor->getFileName()))
            //QMessageBox::question(tabWidget, tr("File is not saved"), tr("File \"%1\"  is not saved. Close it anyway ?").arg(textEditor->getFileName()), QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok
        ) {
            return false;
        }
    }
    return true;
}

QStringList EditorTabs::getOpenTabFiles()
{
    QStringList files;
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * tabEditor = getTabEditor(i);
        if (tabEditor != nullptr) {
            QString file = tabEditor->getFileName();
            if (Helper::fileExists(file)) {
                files.append(file);
            }
        }
    }
    return files;
}

QList<int> EditorTabs::getOpenTabLines()
{
    QList<int> lines;
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * tabEditor = getTabEditor(i);
        if (tabEditor != nullptr) {
            int line = tabEditor->getCursorLine();
            lines.append(line);
        }
    }
    return lines;
}

void EditorTabs::setTabLines(QList<int> lines)
{
    for (int i=0; i<lines.size(); i++) {
        Editor * tabEditor = getTabEditor(i);
        if (tabEditor != nullptr) {
            int line = lines.at(i);
            tabEditor->gotoLine(line);
            tabEditor->resetExtraSelections();
        }
    }
}

int EditorTabs::getCurrentTabIndex()
{
    Editor * tabEditor = getActiveEditor();
    if (tabEditor != nullptr) {
        return tabEditor->getTabIndex();
    }
    return -1;
}

QString EditorTabs::getCurrentTabFilename()
{
    QString filename = "";
    Editor * tabEditor = getActiveEditor();
    if (tabEditor != nullptr) {
        filename = tabEditor->getFileName();
    }
    return filename;
}

void EditorTabs::setActiveTab(int index)
{
    if (index < 0 || index >= tabWidget->count()) return;
    tabWidget->setCurrentIndex(index);
    Editor * tabEditor = getActiveEditor();
    if (tabEditor != nullptr && tabEditor->getTabIndex() == index) {
        tabEditor->setFocus();
    }
}

void EditorTabs::initHighlighters()
{
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * tabEditor = getTabEditor(i);
        if (tabEditor != nullptr) {
            tabEditor->initHighlighter();
        }
    }
}

void EditorTabs::showDeclaration(int index, QString name)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorShowDeclaration(name);
}

void EditorTabs::showHelp(int index, QString name)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorShowHelp(name);
}

void EditorTabs::statusBarTextChangeRequested(int index, QString text)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit statusBarText(text);
}

void EditorTabs::filenameChanged(int index, QString name)
{
    Editor * textEditor = getTabEditor(index);
    if (textEditor == nullptr) return;
    QString tabName = getTabNameFromPath(name);
    tabWidget->setTabText(index, tabName);
    tabWidget->setTabToolTip(index, name);
    emit editorFilenameChanged(name);
}

void EditorTabs::saved(int index)
{
    Editor * textEditor = getTabEditor(index);
    if (textEditor == nullptr) return;
    QString tabText = tabWidget->tabText(index);
    if (tabText.indexOf("[")>0) {
        tabText = tabText.mid(0, tabText.indexOf("["));
        tabWidget->setTabText(index, tabText);
    }
    emit editorSaved(index);
}

void EditorTabs::reloaded(int index)
{
    Editor * textEditor = getTabEditor(index);
    if (textEditor == nullptr) return;
    QString tabText = tabWidget->tabText(index);
    if (tabText.indexOf("[")>0) {
        tabText = tabText.mid(0, tabText.indexOf("["));
        tabWidget->setTabText(index, tabText);
    }
}

void EditorTabs::fileBrowserCreated(QString path)
{
    openFile(path);
    //emit updateProject();
    emit gitTabRefreshRequested();
}

void EditorTabs::fileBrowserRenamed(QString oldpath, QString newpath)
{
    if (oldpath.size() == 0 || newpath.size() == 0) return;
    QFileInfo nInfo(newpath);
    if (nInfo.isFile()) fileBrowserFileRenamed(oldpath, newpath);
    if (nInfo.isDir()) fileBrowserFolderRenamed(oldpath, newpath);
    emit gitTabRefreshRequested();
}

void EditorTabs::fileBrowserFileRenamed(QString oldpath, QString newpath)
{
    bool doEmit = false;
    if (editor != nullptr && editor->getFileName() == oldpath) doEmit = true;
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr && textEditor->getFileName() == oldpath) {
            textEditor->setFileName(newpath);
            int tabIndex = textEditor->getTabIndex();
            QString tabName = getTabNameFromPath(newpath);
            tabWidget->setTabText(tabIndex, tabName);
            tabWidget->setTabToolTip(tabIndex, newpath);
        }
    }
    if (doEmit) emit editorFilenameChanged(newpath);
}

void EditorTabs::fileBrowserFolderRenamed(QString oldpath, QString newpath)
{
    if (oldpath.mid(oldpath.size()-1, 1) == "/") oldpath = oldpath.mid(0, oldpath.size()-1);
    if (newpath.mid(newpath.size()-1, 1) == "/") newpath = newpath.mid(0, newpath.size()-1);
    bool doEmit = false;
    if (editor != nullptr && editor->getFileName().indexOf(oldpath+"/") == 0) doEmit = true;
    for (int i=0; i<tabWidget->count(); i++) {
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr && textEditor->getFileName().indexOf(oldpath+"/") == 0) {
            QString editorFilename = textEditor->getFileName();
            QString editorNewFilename = newpath + editorFilename.mid(oldpath.size());
            textEditor->setFileName(editorNewFilename);
            int tabIndex = textEditor->getTabIndex();
            QString tabName = getTabNameFromPath(editorNewFilename);
            tabWidget->setTabText(tabIndex, tabName);
            tabWidget->setTabToolTip(tabIndex, editorNewFilename);
        }
    }
    if (doEmit) emit editorFilenameChanged(editor->getFileName());
}

void EditorTabs::fileBrowserDeleted(QString path)
{
    for (int i=0; i<tabWidget->count(); i++){
        Editor * textEditor = getTabEditor(i);
        if (textEditor != nullptr && textEditor->getFileName() == path) {
            if (Helper::showQuestion(tr("Delete"), tr("Close tab with deleted file \"%1\" ?").arg(path))) {
            //if (QMessageBox::question(tabWidget, tr("Delete"), tr("Close tab with deleted file \"%1\" ?").arg(path), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
                closeTab(textEditor->getTabIndex());
            } else {
                textEditor->setModified(true);
            }
            break;
        }
    }
    emit gitTabRefreshRequested();
}

void EditorTabs::ready(int index)
{
    Editor * textEditor = getTabEditor(index);
    if (textEditor == nullptr) return;
    emit editorReady(index);
}

void EditorTabs::parsePHP(int index, QString text)
{
    if (editor == nullptr || editor->getTabIndex() != index || !editor->isReady()) return;
    emit editorParsePHPRequested(index, text);
}

void EditorTabs::parseJS(int index, QString text)
{
    if (editor == nullptr || editor->getTabIndex() != index || !editor->isReady()) return;
    emit editorParseJSRequested(index, text);
}

void EditorTabs::parseCSS(int index, QString text)
{
    if (editor == nullptr || editor->getTabIndex() != index || !editor->isReady()) return;
    emit editorParseCSSRequested(index, text);
}

void EditorTabs::undoRedoChanged(int index)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorUndoRedoChanged();
}

void EditorTabs::backForwardChanged(int index)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorBackForwardChanged();
}

void EditorTabs::searchInFiles(QString text)
{
    emit editorSearchInFilesRequested(text);
}

void EditorTabs::searchInFilesRequested()
{
    QString text = "";
    if (editor != nullptr) {
        QTextCursor curs = editor->textCursor();
        text = curs.selectedText();
    }
    searchInFiles(text);
}

void EditorTabs::focusIn(int /*index*/)
{
    emit editorFocused();
}

void EditorTabs::breadcrumbsClick(int index)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorBreadcrumbsClick();
}

QRect EditorTabs::getGeometry()
{
    return tabWidget->geometry();
}

QRect EditorTabs::getGeometryGlobal()
{
    QRect r = tabWidget->geometry();
    QPoint p(r.x(), r.y());
    QPoint pG = tabWidget->mapToGlobal(p);
    return QRect(pG.x(), pG.y(), r.width(), r.height());
}

QRect EditorTabs::getGeometryMappedTo(QWidget * parent)
{
    QRect r = tabWidget->geometry();
    QPoint p(r.x(), r.y());
    QPoint pG = tabWidget->mapTo(parent, p);
    return QRect(pG.x(), pG.y(), r.width(), r.height());
}

void EditorTabs::warning(int index, QString slug, QString text)
{
    Editor * textEditor = getTabEditor(index);
    if (textEditor == nullptr) return;
    QString tabText = tabWidget->tabText(index);
    if (tabText.indexOf("[")>0) tabText = tabText.mid(0, tabText.indexOf("["));
    tabWidget->setTabText(index, tabText+" ["+slug+"]");
    tabWidget->setTabToolTip(index, text);
}

void EditorTabs::showPopupText(int index, QString text)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorShowPopupTextRequested(text);
}

void EditorTabs::showPopupError(int index, QString text)
{
    if (editor == nullptr || editor->getTabIndex() != index) return;
    emit editorShowPopupErrorRequested(text);
}

bool EditorTabs::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == tabWidget->tabBar() && event->type() == QEvent::Resize) {
        emit editorTabsResize();
    }
    if (watched == tabWidget && event->type() == QEvent::Resize) {
        emit editorPaneResize();
    }
    return false;
}
