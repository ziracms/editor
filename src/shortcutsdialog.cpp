#include "shortcutsdialog.h"
#include "ui_shortcutsdialog.h"
#include <QLabel>
#include <QKeySequenceEdit>
#include "settings.h"
#include "scroller.h"

const char * SEQUENCE_NAME_PROPERTY = "settings_name";

ShortcutsDialog::ShortcutsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutsDialog)
{
    ui->setupUi(this);

    std::vector<std::pair<std::string, std::string>> names = getShortcutNames();
    for (auto namesIterator : names) {
        std::string name = namesIterator.first;
        std::string label = namesIterator.second;

        QLabel * nameLabel = new QLabel(tr(label.c_str()), this);
        nameLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ui->shortcutsGridLayout->addWidget(nameLabel);

        QKeySequenceEdit * nameEdit = new QKeySequenceEdit(this);
        nameEdit->setKeySequence(QKeySequence(QString::fromStdString(Settings::get(name))));
        nameEdit->setProperty(SEQUENCE_NAME_PROPERTY, QVariant(name.c_str()));
        ui->shortcutsGridLayout->addWidget(nameEdit);
    }

    // maximize dialog in Android
    #if defined(Q_OS_ANDROID)
    setWindowState( windowState() | Qt::WindowMaximized);
    // scrolling by gesture
    if (Settings::get("enable_android_gestures") == "yes") {
        Scroller::enableGestures(ui->shortcutsScrollArea);
    }
    #endif
}

ShortcutsDialog::~ShortcutsDialog()
{
    delete ui;
}

std::vector<std::pair<std::string, std::string>> ShortcutsDialog::getShortcutNames()
{
    return std::vector<std::pair<std::string, std::string>> {
        {"shortcut_open_file", "Open file"},
        {"shortcut_open_project", "Open project"},
        {"shortcut_new_file", "Create file"},
        {"shortcut_new_folder", "Create folder"},
        {"shortcut_save", "Save"},
        {"shortcut_save_all", "Save All"},
        {"shortcut_comment", "Comment"},
        {"shortcut_overwrite_mode", "Overwrite mode"},
        {"shortcut_previous_tab", "Previous tab"},
        {"shortcut_next_tab", "Next tab"},
        {"shortcut_tabs_list", "Tabs list"},
        {"shortcut_split_tab", "Split tab"},
        {"shortcut_tooltip", "Show tooltip"},
        {"shortcut_search", "Search"},
        {"shortcut_search_in_files", "Search in files"},
        {"shortcut_select_word", "Select word"},
        {"shortcut_multiselect", "Multi-Selection"},
        {"shortcut_help", "Context help"},
        {"shortcut_quick_access", "Quick Access"},
        {"shortcut_goto", "Go to line"},
        {"shortcut_duplicate_line", "Duplicate line"},
        {"shortcut_delete_line", "Delete line"},
        {"shortcut_context_menu", "Context menu"},
        {"shortcut_focus_tree", "File Browser"},
        {"shortcut_sidebar", "Show sidebar"},
        {"shortcut_toolbar", "Show toolbar"},
        {"shortcut_output", "Show output"},
        {"shortcut_execute", "Execute file"},
        {"shortcut_execute_selection", "Execute selection"},
        {"shortcut_terminal", "Show terminal"},
        {"shortcut_close_tab", "Close tab"},
        {"shortcut_close_project", "Close project"},
        {"shortcut_close_app", "Quit"}
    };
}

std::unordered_map<std::string, std::string> ShortcutsDialog::getData()
{
    std::unordered_map<std::string, std::string> data;
    std::vector<std::pair<std::string, std::string>> names = getShortcutNames();

    QList<QKeySequenceEdit *> edits = findChildren<QKeySequenceEdit *>();
    for (QKeySequenceEdit * edit : edits) {
        QVariant nameV = edit->property(SEQUENCE_NAME_PROPERTY);
        if (!nameV.isValid()) continue;
        std::string name = nameV.toString().toStdString();
        auto it = std::find_if( names.begin(), names.end(), [&](const std::pair<std::string, std::string>& pair){ return pair.first == name;} );
        if (it == names.end()) continue;
        std::string value = edit->keySequence().toString().toStdString();
        data[name] = value;
    }

    return data;
}
