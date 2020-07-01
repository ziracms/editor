/*******************************************
 * Zira Editor
 * A lightweight PHP Editor
 * (C)2019 https://github.com/ziracms/editor
 *******************************************/

#include "search.h"
#include "helper.h"
#include <QPainter>
#include <QAction>
#include <QTimer>

const int INPUT_WIDTH_MIN = 100;
const int INPUT_WIDTH_MAX = 300;

Search::Search(Editor * codeEditor) : QWidget(codeEditor)
{
    editor = codeEditor;

    hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);

    vLayout = new QVBoxLayout();
    scrollArea = new QScrollArea();

    scrollWidget = new QWidget();
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    scrollWidget->setLayout(vLayout);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(scrollWidget);

    hLayout->addWidget(scrollArea);

    scrollBar = new QScrollBar(Qt::Horizontal, this);
    scrollBar->setVisible(false);

    findEdit = new QLineEdit();
    findEdit->setMinimumWidth(INPUT_WIDTH_MIN);
    findEdit->setMaximumWidth(INPUT_WIDTH_MAX);

    findButton = new QPushButton();
    findButton->setText(tr("Find"));
    findButton->setIcon(QIcon(":/icons/go-next.png"));

    findPrevButton = new QPushButton();
    findPrevButton->setText(tr("Prev."));
    findPrevButton->setIcon(QIcon(":/icons/go-previous.png"));

    findCaseSensitive = new QCheckBox();
    findCaseSensitive->setText("CaSe");
    findCaseSensitive->setToolTip(tr("Case-Sensitive"));
    findCaseSensitive->setChecked(false);

    findWholeWords = new QCheckBox();
    findWholeWords->setText("Word");
    findWholeWords->setToolTip(tr("Find whole words"));
    findWholeWords->setChecked(false);

    findRegexp = new QCheckBox();
    findRegexp->setText("RegE");
    findRegexp->setToolTip(tr("Regular expression search"));
    findRegexp->setChecked(false);

    hLayoutFind = new QHBoxLayout();
    hLayoutFind->addWidget(findEdit);
    hLayoutFind->addWidget(findButton);
    hLayoutFind->addWidget(findPrevButton);
    hLayoutFind->addWidget(findCaseSensitive);
    hLayoutFind->addWidget(findWholeWords);
    hLayoutFind->addWidget(findRegexp);
    hLayoutFind->addStretch();

    vLayout->addLayout(hLayoutFind);

    replaceEdit = new QLineEdit();
    replaceEdit->setMinimumWidth(INPUT_WIDTH_MIN);
    replaceEdit->setMaximumWidth(INPUT_WIDTH_MAX);

    replaceButton = new QPushButton();
    replaceButton->setText(tr("Replace"));

    replaceAllButton = new QPushButton();
    replaceAllButton->setText(tr("Replace all"));

    closeButton = new QPushButton();
    closeButton->setText(tr("Close"));
    closeButton->setIcon(QIcon(":/icons/close.png"));

    hLayoutReplace = new QHBoxLayout();
    hLayoutReplace->addWidget(replaceEdit);
    hLayoutReplace->addWidget(replaceButton);
    hLayoutReplace->addWidget(replaceAllButton);
    hLayoutReplace->addWidget(closeButton, 1, Qt::AlignRight);
    hLayoutReplace->addStretch();

    vLayout->addLayout(hLayoutReplace);

    setVisible(false);

    connect(findButton, SIGNAL(pressed()), this, SLOT(findPressed()));
    connect(findPrevButton, SIGNAL(pressed()), this, SLOT(findPrevPressed()));
    connect(replaceButton, SIGNAL(pressed()), this, SLOT(replacePressed()));
    connect(replaceAllButton, SIGNAL(pressed()), this, SLOT(replaceAllPressed()));
    connect(closeButton, SIGNAL(pressed()), this, SLOT(closePressed()));
    connect(findCaseSensitive, SIGNAL(stateChanged(int)), this, SLOT(findCaseSensitiveChanged(int)));
    connect(findWholeWords, SIGNAL(stateChanged(int)), this, SLOT(findWholeWordsChanged(int)));
    connect(findRegexp, SIGNAL(stateChanged(int)), this, SLOT(findRegexpChanged(int)));
    connect(findEdit, SIGNAL(returnPressed()), this, SLOT(findEnterPressed()));
    connect(replaceEdit, SIGNAL(returnPressed()), this, SLOT(replaceEnterPressed()));
    connect(findEdit, SIGNAL(textChanged(QString)), this, SLOT(findTextChanged(QString)));
    connect(scrollBar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarValueChanged(int)));
    connect(scrollArea->horizontalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(scrollAreaScrollBarRangeChanged(int, int)));

    CaSe = false;
    Word = false;
    RegE = false;
    Prev = false;
}

QSize Search::sizeHint() const {
    return QSize(0, editor->searchWidgetHeight());
}

void Search::paintEvent(QPaintEvent *event)
{
    editor->searchPaintEvent(event);
}

void Search::findPressed()
{
    QString search = findEdit->text();
    editor->searchText(search, CaSe, Word, RegE);
}

void Search::findPrevPressed()
{
    QString search = findEdit->text();
    editor->searchText(search, CaSe, Word, RegE, true);
}

void Search::replacePressed()
{
    QString search = findEdit->text();
    QString replace = replaceEdit->text();
    editor->replaceText(search, replace, CaSe, Word, RegE);
}

void Search::replaceAllPressed()
{
    QString search = findEdit->text();
    QString replace = replaceEdit->text();
    editor->replaceAllText(search, replace, CaSe, Word, RegE);
}

void Search::closePressed()
{
    editor->closeSearch();
}

void Search::findTextChanged(QString)
{
    editor->searchFlagsChanged(findEdit->text(), CaSe, Word, RegE);
}

void Search::findCaseSensitiveChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        CaSe = true;
    } else {
        CaSe = false;
    }
    editor->searchFlagsChanged(findEdit->text(), CaSe, Word, RegE);
}

void Search::findWholeWordsChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        Word = true;
    } else {
        Word = false;
    }
    editor->searchFlagsChanged(findEdit->text(), CaSe, Word, RegE);
}

void Search::findRegexpChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        RegE = true;
    } else {
        RegE = false;
    }
    editor->searchFlagsChanged(findEdit->text(), CaSe, Word, RegE);
}

void Search::setFindEditBg(QColor bgColor)
{
    QPalette p;
    p.setColor(QPalette::Base, bgColor);
    findEdit->setPalette(p);
}

void Search::setFindEditProp(const char * prop, QString val)
{
    findEdit->setProperty(prop, val);
    findEdit->style()->unpolish(findEdit);
    findEdit->style()->polish(findEdit);
}

void Search::setReplaceEditBg(QColor bgColor)
{
    QPalette p;
    p.setColor(QPalette::Base, bgColor);
    replaceEdit->setPalette(p);
}

void Search::setReplaceEditProp(const char * prop, QString val)
{
    replaceEdit->setProperty(prop, val);
    replaceEdit->style()->unpolish(replaceEdit);
    replaceEdit->style()->polish(replaceEdit);
}

void Search::setFindEditFocus()
{
    findEdit->setFocus();
    if (findEdit->text().size() > 0) {
        findEdit->selectAll();
    }
}

void Search::setFindEditText(QString str)
{
    findEdit->setText(str);
}

void Search::findEnterPressed()
{
    if (!isVisible()) return;
    findPressed();
}

void Search::replaceEnterPressed()
{
    if (!isVisible()) return;
    replacePressed();
}

bool Search::isFocused()
{
    return findEdit->hasFocus() || replaceEdit->hasFocus();
}

void Search::updateScrollBar()
{
    scrollBar->setGeometry(0, 0, geometry().width(), editor->horizontalScrollBar()->geometry().height());

    scrollBar->setMinimum(editor->horizontalScrollBar()->minimum());
    scrollBar->setMaximum(editor->horizontalScrollBar()->maximum());
    scrollBar->setSingleStep(editor->horizontalScrollBar()->singleStep());
    scrollBar->setPageStep(editor->horizontalScrollBar()->pageStep());
    scrollBar->setValue(editor->horizontalScrollBar()->value());
    if (editor->horizontalScrollBar()->maximum() > editor->horizontalScrollBar()->minimum()) {
        editor->horizontalScrollBar()->show();
    } else {
        editor->horizontalScrollBar()->hide();
    }
    QMargins margins = vLayout->contentsMargins();
    if (isVisible() && scrollBar->maximum() > scrollBar->minimum()) {
        scrollBar->show();
        editor->horizontalScrollBar()->hide();
        vLayout->setContentsMargins(margins.left(), margins.bottom() + scrollBar->height(), margins.right(), margins.bottom());
    } else {
        scrollBar->hide();
        vLayout->setContentsMargins(margins.left(), margins.bottom(), margins.right(), margins.bottom());
    }
}

void Search::scrollBarValueChanged(int value)
{
    editor->horizontalScrollBar()->setValue(value);
}

QScrollBar * Search::horizontalScrollBar()
{
    return scrollBar;
}

QScrollBar * Search::scrollAreaHorizontalScrollBar()
{
    return scrollArea->horizontalScrollBar();
}

void Search::scrollAreaScrollBarRangeChanged(int /*min*/, int /*max*/)
{
    // calling with delay to avoid recursion
    QTimer::singleShot(50, this, SLOT(updateSizes()));
}

void Search::updateSizes()
{
    if (isVisible()) {
        editor->updateSizes(false);
    }
}
