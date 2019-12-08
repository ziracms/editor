/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "navigator.h"
#include "helper.h"

Navigator::Navigator(QTreeWidget * widget, Settings * /*settings*/) : treeWidget(widget)
{
    connect(treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), SLOT(navigatorDoubleClicked(QTreeWidgetItem*,int)));
    connect(treeWidget, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(navigatorExpanded(QTreeWidgetItem*)));
    connect(treeWidget, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(navigatorCollapsed(QTreeWidgetItem*)));
}

Navigator::~Navigator()
{

}

void Navigator::clear()
{
    treeWidget->clear();
}

void Navigator::build(ParsePHP::ParseResult result)
{
    clear();
    // constants
    for (int c=0; c<result.constants.size(); c++) {
        ParsePHP::ParseResultConstant constant = result.constants.at(c);
        if (constant.clsName.size() > 0) continue;
        QString name = constant.name;
        int p = constant.name.lastIndexOf("\\");
        if (p >= 0) name = constant.name.mid(p+1);
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, name);
        item->setToolTip(0, constant.name+" = "+constant.value);
        item->setData(0, Qt::UserRole, QVariant(constant.line));
        treeWidget->addTopLevelItem(item);
    }
    // variables
    for (int v=0; v<result.variables.size(); v++) {
        ParsePHP::ParseResultVariable variable = result.variables.at(v);
        if (variable.clsName.size() > 0 || variable.funcName.size() > 0) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, variable.name);
        QString varDesc = variable.name;
        if (variable.type.size() > 0) varDesc += " : " + variable.type;
        item->setToolTip(0, varDesc);
        item->setData(0, Qt::UserRole, QVariant(variable.line));
        treeWidget->addTopLevelItem(item);
    }
    // functions
    for (int f=0; f<result.functions.size(); f++) {
        ParsePHP::ParseResultFunction func = result.functions.at(f);
        if (func.clsName.size() > 0) continue;
        QString name = func.name;
        int p = func.name.lastIndexOf("\\");
        if (p >= 0) name = func.name.mid(p+1);
        QTreeWidgetItem * item = new QTreeWidgetItem();
        QString funcDesc = name;
        if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
        else funcDesc += "()";
        item->setText(0, funcDesc);
        funcDesc = "function "+func.name;
        if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
        else funcDesc += "()";
        if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
        if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
        item->setToolTip(0, funcDesc);
        item->setData(0, Qt::UserRole, QVariant(func.line));
        treeWidget->addTopLevelItem(item);
        // function variables
        for (int v=0; v<func.variableIndexes.size(); v++) {
            if (result.variables.size() <= func.variableIndexes.at(v)) break;
            ParsePHP::ParseResultVariable variable = result.variables.at(func.variableIndexes.at(v));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0, variable.name);
            QString varDesc = variable.name;
            if (variable.type.size() > 0) varDesc += " : " + variable.type;
            child->setToolTip(0, varDesc);
            child->setData(0, Qt::UserRole, QVariant(variable.line));
            item->addChild(child);
        }
    }
    // classes
    for (int i=0; i<result.classes.size(); i++) {
        ParsePHP::ParseResultClass cls = result.classes.at(i);
        QString clsPrettyName = cls.name;
        int p = cls.name.lastIndexOf("\\");
        if (p >= 0) clsPrettyName = cls.name.mid(p+1);
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, clsPrettyName);
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
        item->setToolTip(0, clsDesc);
        item->setData(0, Qt::UserRole, QVariant(cls.line));
        treeWidget->addTopLevelItem(item);
        // class constants
        for (int c=0; c<cls.constantIndexes.size(); c++) {
            if (result.constants.size() <= cls.constantIndexes.at(c)) break;
            ParsePHP::ParseResultConstant constant = result.constants.at(cls.constantIndexes.at(c));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0, constant.name);
            child->setToolTip(0, clsPrettyName+"::"+constant.name+" = "+constant.value);
            child->setData(0, Qt::UserRole, QVariant(constant.line));
            item->addChild(child);
        }
        // class variables
        for (int v=0; v<cls.variableIndexes.size(); v++) {
            if (result.variables.size() <= cls.variableIndexes.at(v)) break;
            ParsePHP::ParseResultVariable variable = result.variables.at(cls.variableIndexes.at(v));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0, variable.name);
            QString varDesc = "";
            if (variable.visibility.size() > 0) varDesc += variable.visibility + " ";
            if (variable.isStatic) varDesc += "static ";
            varDesc += clsPrettyName+"::"+variable.name;
            if (variable.type.size() > 0) varDesc += " : " + variable.type;
            child->setToolTip(0, varDesc);
            child->setData(0, Qt::UserRole, QVariant(variable.line));
            item->addChild(child);
        }
        // class methods
        for (int f=0; f<cls.functionIndexes.size(); f++) {
            if (result.functions.size() <= cls.functionIndexes.at(f)) break;
            ParsePHP::ParseResultFunction func = result.functions.at(cls.functionIndexes.at(f));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            QString funcDesc = func.name;
            if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
            else funcDesc += "()";
            child->setText(0, funcDesc);
            funcDesc = "";
            if (func.isAbstract) funcDesc += "abstract ";
            if (func.visibility.size() > 0) funcDesc += func.visibility + " ";
            if (func.isStatic) funcDesc += "static ";
            funcDesc += "function "+clsPrettyName+"::"+func.name;
            if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
            else funcDesc += "()";
            if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
            if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
            child->setToolTip(0, funcDesc);
            child->setData(0, Qt::UserRole, QVariant(func.line));
            item->addChild(child);
            // class method variables
            for (int v=0; v<func.variableIndexes.size(); v++) {
                if (result.variables.size() <= func.variableIndexes.at(v)) break;
                ParsePHP::ParseResultVariable variable = result.variables.at(func.variableIndexes.at(v));
                QTreeWidgetItem * subchild = new QTreeWidgetItem();
                subchild->setText(0, variable.name);
                QString varDesc = variable.name;
                if (variable.type.size() > 0) varDesc += " : " + variable.type;
                subchild->setToolTip(0, varDesc);
                subchild->setData(0, Qt::UserRole, QVariant(variable.line));
                child->addChild(subchild);
            }
        }
        treeWidget->expandItem(item);
    }
    // comments
    if (result.comments.size() > 0) {
        QTreeWidgetItem * parent = new QTreeWidgetItem();
        parent->setText(0, "//"+tr("comments"));
        parent->setToolTip(0, tr("Comments"));
        parent->setData(0, Qt::UserRole, QVariant(0));
        treeWidget->addTopLevelItem(parent);
        for (int i=0; i<result.comments.size(); i++) {
            ParsePHP::ParseResultComment comment = result.comments.at(i);
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0, comment.name);
            item->setToolTip(0, comment.text);
            item->setData(0, Qt::UserRole, QVariant(comment.line));
            parent->addChild(item);
        }
        //treeWidget->expandItem(parent);
    }
    treeWidget->resizeColumnToContents(0);
}

void Navigator::build(ParseJS::ParseResult result)
{
    clear();
    // constants
    for (int c=0; c<result.constants.size(); c++) {
        ParseJS::ParseResultConstant constant = result.constants.at(c);
        if (constant.clsName.size() > 0 || constant.funcName.size() > 0) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, constant.name);
        item->setToolTip(0, constant.name+" = "+constant.value);
        item->setData(0, Qt::UserRole, QVariant(constant.line));
        treeWidget->addTopLevelItem(item);
    }
    // variables
    for (int v=0; v<result.variables.size(); v++) {
        ParseJS::ParseResultVariable variable = result.variables.at(v);
        if (variable.clsName.size() > 0 || variable.funcName.size() > 0) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, variable.name);
        item->setToolTip(0, variable.name);
        item->setData(0, Qt::UserRole, QVariant(variable.line));
        treeWidget->addTopLevelItem(item);
    }
    // functions
    for (int f=0; f<result.functions.size(); f++) {
        ParseJS::ParseResultFunction func = result.functions.at(f);
        if (func.clsName.size() > 0) continue;
        QTreeWidgetItem * item = new QTreeWidgetItem();
        QString funcDesc = func.name;
        if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
        else funcDesc += "()";
        item->setText(0, funcDesc);
        funcDesc = "function "+func.name;
        if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
        else funcDesc += "()";
        if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
        if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
        item->setToolTip(0, funcDesc);
        item->setData(0, Qt::UserRole, QVariant(func.line));
        treeWidget->addTopLevelItem(item);
        // function constants
        for (int c=0; c<func.constantIndexes.size(); c++) {
            if (result.constants.size() <= func.constantIndexes.at(c)) break;
            ParseJS::ParseResultConstant constant = result.constants.at(func.constantIndexes.at(c));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0, constant.name);
            child->setToolTip(0, constant.name+" = "+constant.value);
            child->setData(0, Qt::UserRole, QVariant(constant.line));
            item->addChild(child);
        }
        // function variables
        for (int v=0; v<func.variableIndexes.size(); v++) {
            if (result.variables.size() <= func.variableIndexes.at(v)) break;
            ParseJS::ParseResultVariable variable = result.variables.at(func.variableIndexes.at(v));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0, variable.name);
            child->setToolTip(0, variable.name);
            child->setData(0, Qt::UserRole, QVariant(variable.line));
            item->addChild(child);
        }
    }
    // classes
    for (int i=0; i<result.classes.size(); i++) {
        ParseJS::ParseResultClass cls = result.classes.at(i);
        QString name = cls.name;
        int p = cls.name.lastIndexOf("\\");
        if (p >= 0) name = cls.name.mid(p+1);
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, name);
        QString clsDesc = "object ";
        clsDesc += cls.name;
        item->setToolTip(0, clsDesc);
        item->setData(0, Qt::UserRole, QVariant(cls.line));
        treeWidget->addTopLevelItem(item);
        // class variables
        for (int v=0; v<cls.variableIndexes.size(); v++) {
            if (result.variables.size() <= cls.variableIndexes.at(v)) break;
            ParseJS::ParseResultVariable variable = result.variables.at(cls.variableIndexes.at(v));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            child->setText(0, variable.name);
            QString varDesc = variable.name;
            child->setToolTip(0, varDesc);
            child->setData(0, Qt::UserRole, QVariant(variable.line));
            item->addChild(child);
        }
        // class methods
        for (int f=0; f<cls.functionIndexes.size(); f++) {
            if (result.functions.size() <= cls.functionIndexes.at(f)) break;
            ParseJS::ParseResultFunction func = result.functions.at(cls.functionIndexes.at(f));
            QTreeWidgetItem * child = new QTreeWidgetItem();
            QString funcDesc = func.name;
            if (func.args.size() > 0) funcDesc += "("+QString(".").repeated(func.maxArgs)+")";
            else funcDesc += "()";
            child->setText(0, funcDesc);
            funcDesc = "function "+func.name;
            if (func.args.size() > 0) funcDesc += " ( "+func.args+" )";
            else funcDesc += "()";
            if (func.returnType.size() > 0) funcDesc += " : "+func.returnType;
            if (func.comment.size() > 0) funcDesc += "\n\n"+func.comment;
            child->setToolTip(0, funcDesc);
            child->setData(0, Qt::UserRole, QVariant(func.line));
            item->addChild(child);
            // class method constants
            for (int c=0; c<func.constantIndexes.size(); c++) {
                if (result.constants.size() <= func.constantIndexes.at(c)) break;
                ParseJS::ParseResultConstant constant = result.constants.at(func.constantIndexes.at(c));
                QTreeWidgetItem * subchild = new QTreeWidgetItem();
                subchild->setText(0, constant.name);
                subchild->setToolTip(0, constant.name+" = "+constant.value);
                subchild->setData(0, Qt::UserRole, QVariant(constant.line));
                child->addChild(subchild);
            }
            // class method variables
            for (int v=0; v<func.variableIndexes.size(); v++) {
                if (result.variables.size() <= func.variableIndexes.at(v)) break;
                ParseJS::ParseResultVariable variable = result.variables.at(func.variableIndexes.at(v));
                QTreeWidgetItem * subchild = new QTreeWidgetItem();
                subchild->setText(0, variable.name);
                subchild->setToolTip(0, variable.name);
                subchild->setData(0, Qt::UserRole, QVariant(variable.line));
                child->addChild(subchild);
            }
        }
        treeWidget->expandItem(item);
    }
    // comments
    if (result.comments.size() > 0) {
        QTreeWidgetItem * parent = new QTreeWidgetItem();
        parent->setText(0, "//"+tr("comments"));
        parent->setToolTip(0, tr("Comments"));
        parent->setData(0, Qt::UserRole, QVariant(0));
        treeWidget->addTopLevelItem(parent);
        for (int i=0; i<result.comments.size(); i++) {
            ParseJS::ParseResultComment comment = result.comments.at(i);
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0, comment.name);
            item->setToolTip(0, comment.text);
            item->setData(0, Qt::UserRole, QVariant(comment.line));
            parent->addChild(item);
        }
        //treeWidget->expandItem(parent);
    }
    treeWidget->resizeColumnToContents(0);
}

void Navigator::build(ParseCSS::ParseResult result)
{
    clear();
    // selectors
    for (int i=0; i<result.selectors.size(); i++) {
        ParseCSS::ParseResultSelector selector = result.selectors.at(i);
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, selector.name);
        item->setToolTip(0, selector.name);
        item->setData(0, Qt::UserRole, QVariant(selector.line));
        treeWidget->addTopLevelItem(item);
    }
    // ids & classes
    for (int i=0; i<result.names.size(); i++) {
        ParseCSS::ParseResultName nm = result.names.at(i);
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0, nm.name);
        item->setToolTip(0, nm.name);
        item->setData(0, Qt::UserRole, QVariant(nm.line));
        treeWidget->addTopLevelItem(item);
    }
    // font-face
    if (result.fonts.size() > 0) {
        QTreeWidgetItem * parent = new QTreeWidgetItem();
        parent->setText(0, "@font-face");
        parent->setToolTip(0, "@font-face");
        parent->setData(0, Qt::UserRole, QVariant(0));
        treeWidget->addTopLevelItem(parent);
        for (int i=0; i<result.fonts.size(); i++) {
            ParseCSS::ParseResultFont font = result.fonts.at(i);
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0, font.name);
            item->setToolTip(0, "font-family: "+font.name);
            item->setData(0, Qt::UserRole, QVariant(font.line));
            parent->addChild(item);
        }
        treeWidget->expandItem(parent);
    }
    // media
    if (result.medias.size() > 0) {
        QTreeWidgetItem * parent = new QTreeWidgetItem();
        parent->setText(0, "@media");
        parent->setToolTip(0, "@media");
        parent->setData(0, Qt::UserRole, QVariant(0));
        treeWidget->addTopLevelItem(parent);
        for (int i=0; i<result.medias.size(); i++) {
            ParseCSS::ParseResultMedia media = result.medias.at(i);
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0, media.name);
            item->setToolTip(0, "@media( "+media.name+" )");
            item->setData(0, Qt::UserRole, QVariant(media.line));
            parent->addChild(item);
        }
        treeWidget->expandItem(parent);
    }
    // keyframes
    if (result.keyframes.size() > 0) {
        QTreeWidgetItem * parent = new QTreeWidgetItem();
        parent->setText(0, "@keyframes");
        parent->setToolTip(0, "@keyframes");
        parent->setData(0, Qt::UserRole, QVariant(0));
        treeWidget->addTopLevelItem(parent);
        for (int i=0; i<result.keyframes.size(); i++) {
            ParseCSS::ParseResultKeyframe keyframe = result.keyframes.at(i);
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0, keyframe.name);
            item->setToolTip(0, "@keyframes "+keyframe.name);
            item->setData(0, Qt::UserRole, QVariant(keyframe.line));
            parent->addChild(item);
        }
        treeWidget->expandItem(parent);
    }
    // comments
    if (result.comments.size() > 0) {
        QTreeWidgetItem * parent = new QTreeWidgetItem();
        parent->setText(0, "//"+tr("comments"));
        parent->setToolTip(0, tr("Comments"));
        parent->setData(0, Qt::UserRole, QVariant(0));
        treeWidget->addTopLevelItem(parent);
        for (int i=0; i<result.comments.size(); i++) {
            ParseCSS::ParseResultComment comment = result.comments.at(i);
            QTreeWidgetItem * item = new QTreeWidgetItem();
            item->setText(0, comment.name);
            item->setToolTip(0, comment.text);
            item->setData(0, Qt::UserRole, QVariant(comment.line));
            parent->addChild(item);
        }
        //treeWidget->expandItem(parent);
    }
    treeWidget->resizeColumnToContents(0);
}

void Navigator::navigatorDoubleClicked(QTreeWidgetItem * item, int column)
{
    if (item == 0) return;
    if (column != 0) return;
    int line = item->data(0, Qt::UserRole).toInt();
    if (line == 0) return;
    emit showLine(line);
}

void Navigator::navigatorExpanded(QTreeWidgetItem * item)
{
    if (item == 0) return;
    treeWidget->resizeColumnToContents(0);
}

void Navigator::navigatorCollapsed(QTreeWidgetItem * item)
{
    if (item == 0) return;
    treeWidget->resizeColumnToContents(0);
}
