/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "quickaccess.h"
#include <QPaintEvent>
#include <QPainter>
#include <QListWidgetItem>
#include <QTimer>
#include <QScrollBar>
#include <QFontDatabase>
#include <QPropertyAnimation>
#include "helper.h"

const int WIDGET_MIN_WIDTH = 200;
const int WIDGET_MIN_HEIGHT = 100;

const int PARSE_RESULT_TYPE_PHP = 0;
const int PARSE_RESULT_TYPE_JS = 1;
const int PARSE_RESULT_TYPE_CSS = 2;

const int SEARCH_DELAY_MILLISECONDS = 500;

const int ANIMATION_DURATION = 200;
const int ANIMATION_OFFSET = 150;

QuickAccess::QuickAccess(Settings * settings, QWidget *parent) : QFrame(parent)
{
    setFrameStyle(QFrame::Raised);
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);

    setMinimumWidth(WIDGET_MIN_WIDTH);
    setMinimumHeight(WIDGET_MIN_HEIGHT);

    vLayout = new QVBoxLayout(this);

    findEdit = new QLineEdit();
    findEdit->setPlaceholderText(tr("Search for file, class or function"));
    findEdit->setClearButtonEnabled(true);
    vLayout->addWidget(findEdit);

    resultsList = new QListWidget();
    vLayout->addWidget(resultsList);

    vLayout->addStretch();

    connect(findEdit, SIGNAL(textChanged(QString)), this, SLOT(findTextChanged(QString)));
    connect(findEdit, SIGNAL(returnPressed()), this, SLOT(findTextReturned()));
    connect(resultsList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(resultsListItemActivated(QListWidgetItem*)));
    connect(resultsList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(resultsListItemActivated(QListWidgetItem*)));

    findEdit->installEventFilter(this);
    resultsList->installEventFilter(this);

    hide();

    lastSearch = "";
    parseResultFile = "";
    parseResultType = -1;
    findLocked = false;

    // fonts
    QFont outputFont;
    std::string fontFamily = settings->get("editor_font_family");
    std::string fontSize = settings->get("editor_popup_font_size");
    if (fontFamily=="") {
        QFont sysFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        outputFont.setFamily(sysFont.family());
    } else {
        outputFont.setStyleHint(QFont::Monospace);
        outputFont.setFamily(QString::fromStdString(fontFamily));
    }
    outputFont.setPointSize(std::stoi(fontSize));
    resultsList->setFont(outputFont);
}

QuickAccess::~QuickAccess()
{

}

QSize QuickAccess::sizeHint() const {
    return QSize(WIDGET_MIN_WIDTH, WIDGET_MIN_HEIGHT);
}

void QuickAccess::animateIn()
{
    QRect rect = geometry();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(ANIMATION_DURATION);
    animation->setStartValue(QRect(rect.x()+ANIMATION_OFFSET, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));

    QEasingCurve easing(QEasingCurve::OutCubic);
    animation->setEasingCurve(easing);

    animation->start();
}

void QuickAccess::animateOut()
{
    QRect rect = geometry();

    QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
    animation->setDuration(ANIMATION_DURATION);
    animation->setStartValue(QRect(rect.x(), rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.x()+ANIMATION_OFFSET, rect.y(), rect.width(), rect.height()));

    QEasingCurve easing(QEasingCurve::InCubic);
    animation->setEasingCurve(easing);

    animation->start();
}

void QuickAccess::hide()
{
    setVisible(false);
    animationInProgress = false;
}

void QuickAccess::displayed()
{
    animationInProgress = false;
}

void QuickAccess::slideOut()
{
    if (animationInProgress) return;
    animationInProgress = true;
    animateOut();
    QTimer::singleShot(ANIMATION_DURATION, this, SLOT(hide()));
}

void QuickAccess::slideIn(int x, int y, int width, int height)
{
    if (animationInProgress) return;

    if (width < WIDGET_MIN_WIDTH) width = WIDGET_MIN_WIDTH;
    if (height < WIDGET_MIN_HEIGHT) height = WIDGET_MIN_HEIGHT;

    setGeometry(x, y, width, height);
    setVisible(true);

    findEdit->setFocus();

    int hOffset = findEdit->height() + 3 * vLayout->spacing();
    resultsList->setMinimumHeight(height - hOffset);
    resultsList->setMaximumHeight(height - hOffset);

    raise();

    animationInProgress = true;
    animateIn();
    QTimer::singleShot(ANIMATION_DURATION, this, SLOT(displayed()));
}

void QuickAccess::setParseResult(ParsePHP::ParseResult result, QString file)
{
    parseResultPHP = result;
    parseResultFile = file;
    parseResultType = PARSE_RESULT_TYPE_PHP;
    findEdit->setText("");
    lastSearch = "";
    resultsList->clear();
    // constants
    for (int c=0; c<result.constants.size(); c++) {
        ParsePHP::ParseResultConstant constant = result.constants.at(c);
        if (constant.clsName.size() > 0) continue;
        QString name = constant.name;
        int p = constant.name.lastIndexOf("\\");
        if (p >= 0) name = constant.name.mid(p+1);
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(name);
        item->setToolTip(constant.name+" = "+constant.value);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(constant.line));
        resultsList->addItem(item);
    }
    // variables
    for (int v=0; v<result.variables.size(); v++) {
        ParsePHP::ParseResultVariable variable = result.variables.at(v);
        if (variable.clsName.size() > 0 || variable.funcName.size() > 0) continue;
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(variable.name);
        QString varDesc = variable.name;
        if (variable.type.size() > 0) varDesc += " : " + variable.type;
        item->setToolTip(varDesc);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(variable.line));
        resultsList->addItem(item);
    }
    // functions
    for (int f=0; f<result.functions.size(); f++) {
        ParsePHP::ParseResultFunction func = result.functions.at(f);
        if (func.clsName.size() > 0) continue;
        QString name = func.name;
        int p = func.name.lastIndexOf("\\");
        if (p >= 0) name = func.name.mid(p+1);
        QListWidgetItem * item = new QListWidgetItem();
        QString funcDesc = name;
        if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
        else funcDesc += "()";
        item->setText(funcDesc);
        funcDesc = "function "+func.name;
        if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
        else funcDesc += "()";
        if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
        if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
        item->setToolTip(funcDesc);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(func.line));
        resultsList->addItem(item);
    }
    // classes
    for (int i=0; i<result.classes.size(); i++) {
        ParsePHP::ParseResultClass cls = result.classes.at(i);
        QString clsPrettyName = cls.name;
        int p = cls.name.lastIndexOf("\\");
        if (p >= 0) clsPrettyName = cls.name.mid(p+1);
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(clsPrettyName);
        QString clsDesc = "";
        if (cls.isAbstract) clsDesc += "abstract ";
        if (cls.isInterface) clsDesc += "interface ";
        else if (cls.isTrait) clsDesc += "trait ";
        else clsDesc += "class ";
        clsDesc += cls.name;
        if (cls.parent.size() > 0) clsDesc += " extends "+cls.parent;
        if (cls.interfaces.size() > 0) {
            clsDesc += " implements ";
            for (int y=0; y<cls.interfaces.size(); y++) {
                if (y>0) clsDesc += ", ";
                clsDesc += cls.interfaces.at(y);
            }
        }
        item->setToolTip(clsDesc);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(cls.line));
        resultsList->addItem(item);
        // class constants
        for (int c=0; c<cls.constantIndexes.size(); c++) {
            if (result.constants.size() <= cls.constantIndexes.at(c)) break;
            ParsePHP::ParseResultConstant constant = result.constants.at(cls.constantIndexes.at(c));
            QListWidgetItem * item = new QListWidgetItem();
            item->setText(clsPrettyName+"::"+constant.name);
            item->setToolTip(clsPrettyName+"::"+constant.name+" = "+constant.value);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(constant.line));
            resultsList->addItem(item);
        }
        // class variables
        for (int v=0; v<cls.variableIndexes.size(); v++) {
            if (result.variables.size() <= cls.variableIndexes.at(v)) break;
            ParsePHP::ParseResultVariable variable = result.variables.at(cls.variableIndexes.at(v));
            QListWidgetItem * item = new QListWidgetItem();
            item->setText(clsPrettyName+"::"+variable.name);
            QString varDesc = "";
            if (variable.visibility.size() > 0) varDesc += variable.visibility + " ";
            if (variable.isStatic) varDesc += "static ";
            varDesc += clsPrettyName+"::"+variable.name;
            if (variable.type.size() > 0) varDesc += " : " + variable.type;
            item->setToolTip(varDesc);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(variable.line));
            resultsList->addItem(item);
        }
        // class methods
        for (int f=0; f<cls.functionIndexes.size(); f++) {
            if (result.functions.size() <= cls.functionIndexes.at(f)) break;
            ParsePHP::ParseResultFunction func = result.functions.at(cls.functionIndexes.at(f));
            QListWidgetItem * item = new QListWidgetItem();
            QString funcDesc = clsPrettyName+"::"+func.name;
            if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
            else funcDesc += "()";
            item->setText(funcDesc);
            funcDesc = "";
            if (func.isAbstract) funcDesc += "abstract ";
            if (func.visibility.size() > 0) funcDesc += func.visibility + " ";
            if (func.isStatic) funcDesc += "static ";
            funcDesc += "function "+clsPrettyName+"::"+func.name;
            if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
            else funcDesc += "()";
            if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
            if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
            item->setToolTip(funcDesc);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(func.line));
            resultsList->addItem(item);
        }
    }
    resultsList->sortItems();
}

void QuickAccess::setParseResult(ParseJS::ParseResult result, QString file)
{
    parseResultJS = result;
    parseResultFile = file;
    parseResultType = PARSE_RESULT_TYPE_JS;
    findEdit->setText("");
    lastSearch = "";
    resultsList->clear();
    // constants
    for (int c=0; c<result.constants.size(); c++) {
        ParseJS::ParseResultConstant constant = result.constants.at(c);
        if (constant.clsName.size() > 0 || constant.funcName.size() > 0) continue;
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(constant.name);
        item->setToolTip(constant.name+" = "+constant.value);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(constant.line));
        resultsList->addItem(item);
    }
    // variables
    for (int v=0; v<result.variables.size(); v++) {
        ParseJS::ParseResultVariable variable = result.variables.at(v);
        if (variable.clsName.size() > 0 || variable.funcName.size() > 0) continue;
        QListWidgetItem * item = new QListWidgetItem();
        item->setText(variable.name);
        item->setToolTip(variable.name);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(variable.line));
        resultsList->addItem(item);
    }
    // functions
    for (int f=0; f<result.functions.size(); f++) {
        ParseJS::ParseResultFunction func = result.functions.at(f);
        if (func.clsName.size() > 0) continue;
        QListWidgetItem * item = new QListWidgetItem();
        QString funcDesc = func.name;
        if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
        else funcDesc += "()";
        item->setText(funcDesc);
        funcDesc = "function "+func.name;
        if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
        else funcDesc += "()";
        if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
        if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
        item->setToolTip(funcDesc);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(func.line));
        resultsList->addItem(item);
    }
    // classes
    for (int i=0; i<result.classes.size(); i++) {
        ParseJS::ParseResultClass cls = result.classes.at(i);
        // class variables
        for (int v=0; v<cls.variableIndexes.size(); v++) {
            if (result.variables.size() <= cls.variableIndexes.at(v)) break;
            ParseJS::ParseResultVariable variable = result.variables.at(cls.variableIndexes.at(v));
            QListWidgetItem * item = new QListWidgetItem();
            item->setText(cls.name+"::"+variable.name);
            QString varDesc = variable.name;
            item->setToolTip(varDesc);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(variable.line));
            resultsList->addItem(item);
        }
        // class methods
        for (int f=0; f<cls.functionIndexes.size(); f++) {
            if (result.functions.size() <= cls.functionIndexes.at(f)) break;
            ParseJS::ParseResultFunction func = result.functions.at(cls.functionIndexes.at(f));
            QListWidgetItem * item = new QListWidgetItem();
            QString funcDesc = cls.name+"::"+func.name;
            if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
            else funcDesc += "()";
            item->setText(funcDesc);
            funcDesc = "function "+func.name;
            if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
            else funcDesc += "()";
            if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
            if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
            item->setToolTip(funcDesc);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(func.line));
            resultsList->addItem(item);
        }
    }
    resultsList->sortItems();
}

void QuickAccess::setParseResult(ParseCSS::ParseResult result, QString file)
{
    parseResultCSS = result;
    parseResultFile = file;
    parseResultType = PARSE_RESULT_TYPE_CSS;
    findEdit->setText("");
    lastSearch = "";
    resultsList->clear();
    // font-face
    if (result.fonts.size() > 0) {
        for (int i=0; i<result.fonts.size(); i++) {
            ParseCSS::ParseResultFont font = result.fonts.at(i);
            QListWidgetItem * item = new QListWidgetItem();
            item->setText("@font-face: "+font.name);
            item->setToolTip("font-family: "+font.name);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(font.line));
            resultsList->addItem(item);
        }
    }
    // media
    if (result.medias.size() > 0) {
        for (int i=0; i<result.medias.size(); i++) {
            ParseCSS::ParseResultMedia media = result.medias.at(i);
            QListWidgetItem * item = new QListWidgetItem();
            item->setText("@media( "+media.name+" )");
            item->setToolTip("@media( "+media.name+" )");
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(media.line));
            resultsList->addItem(item);
        }
    }
    // keyframes
    if (result.keyframes.size() > 0) {
        for (int i=0; i<result.keyframes.size(); i++) {
            ParseCSS::ParseResultKeyframe keyframe = result.keyframes.at(i);
            QListWidgetItem * item = new QListWidgetItem();
            item->setText("@keyframes: "+keyframe.name);
            item->setToolTip("@keyframes "+keyframe.name);
            item->setData(Qt::UserRole, QVariant(file));
            item->setData(Qt::UserRole+1, QVariant(keyframe.line));
            resultsList->addItem(item);
        }
    }
    // ids & classes
    for (int i=0; i<result.names.size(); i++) {
        ParseCSS::ParseResultName nm = result.names.at(i);
        QListWidgetItem * item = new QListWidgetItem();
        item->setText("> "+nm.name);
        item->setToolTip(nm.name);
        item->setData(Qt::UserRole, QVariant(file));
        item->setData(Qt::UserRole+1, QVariant(nm.line));
        resultsList->addItem(item);
    }
    resultsList->sortItems();
}

void QuickAccess::resultsListItemActivated(QListWidgetItem *item)
{
    QString file = item->data(Qt::UserRole).toString();
    int line = item->data(Qt::UserRole+1).toInt();
    if (file.size() == 0 || line <= 0) return;
    emit quickAccessRequested(file, line);
}

void QuickAccess::restoreResults()
{
    if (parseResultType == PARSE_RESULT_TYPE_PHP) {
        setParseResult(parseResultPHP, parseResultFile);
    } else if (parseResultType == PARSE_RESULT_TYPE_JS) {
        setParseResult(parseResultJS, parseResultFile);
    } else if (parseResultType == PARSE_RESULT_TYPE_CSS) {
        setParseResult(parseResultCSS, parseResultFile);
    } else {
        findEdit->setText("");
        lastSearch = "";
        resultsList->clear();
    }
}

void QuickAccess::findTextReturned()
{
    QString text = findEdit->text();
    findTextChanged(text);
}

void QuickAccess::findTextChanged(QString text)
{
    text = text.trimmed();
    if (text.size() == 0) {
        restoreResults();
        return;
    }
    lastSearch = text;
    resultsList->clear();
    if (!findLocked) {
        findLocked = true;
        QTimer::singleShot(SEARCH_DELAY_MILLISECONDS, this, SLOT(findTextDelayed()));
    }
}

void QuickAccess::findTextDelayed()
{
    findLocked = false;
    if (lastSearch.size() > 0) emit quickFindRequested(lastSearch);
}

void QuickAccess::quickFound(QString text, QString info, QString file, int line)
{
    if (text != lastSearch) return;
    QListWidgetItem * item = new QListWidgetItem();
    item->setText(info);
    item->setToolTip(info);
    item->setData(Qt::UserRole, QVariant(file));
    item->setData(Qt::UserRole+1, QVariant(line));
    resultsList->addItem(item);
}

bool QuickAccess::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == findEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = (QKeyEvent *)event;
        if (keyEvent->key() == Qt::Key_Down && resultsList->count() > 0) {
            int current = resultsList->currentRow();
            if (current < resultsList->count()-1) current++;
            else current = 0;
            resultsList->setCurrentRow(current);
            resultsList->setFocus();
        } else if (keyEvent->key() == Qt::Key_Up && resultsList->count() > 0) {
            int current = resultsList->currentRow();
            if (current > 0) current--;
            else current = resultsList->count()-1;
            resultsList->setCurrentRow(current);
            resultsList->setFocus();
        }
    }
    if(watched == resultsList && event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = (QKeyEvent *)event;
        if (keyEvent->key() == Qt::Key_Down && resultsList->count() > 0 && resultsList->currentRow() == resultsList->count()-1) {
            resultsList->setCurrentRow(0);
            return true;
        } else if (keyEvent->key() == Qt::Key_Up && resultsList->count() > 0 && resultsList->currentRow() == 0) {
            resultsList->setCurrentRow(resultsList->count()-1);
            return true;
        }
    }
    return false;
}
