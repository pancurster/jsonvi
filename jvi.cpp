#include <gtkmm.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <functional>
#include "jvi.h"

using namespace std;

JviRoot root;

int main(int argc, char *argv[])
{
    root.filename = argv[1] ? argv[1] : "";

    int argcgtk = 1;
    auto app = Gtk::Application::create(argcgtk, argv, "org.gtkmm.examples.base");

    /* setup main widgets */
    JviMainWindow* window = new JviMainWindow;
    setup_gui(window);

    /* populate text-view */
    root.gui.json_text_buff->set_text(load_file(root.filename));
    return app->run(*window);
}
void on_buff_change()
{
    root.state.json_changed = true;
}
void on_switch_page(Gtk::Widget* page, int page_number)
{
    root.state.prev_page = 1;

    if (root.state.prev_page == 1 && root.state.json_changed) {

        /* parse again json and redraw view */
        string new_json(root.gui.json_text_buff->get_text());
        Json::Value* json_object = parse_json(new_json);

        root.gui.main_tree_storage->clear();
        auto row = root.gui.main_tree_storage->append();

        JviModel* model = new JviModel;
        (*row)[model->value_text] = root.filename;
        (*row)[model->value_icon] = root.res.icon_doc;
        iter_node(*json_object, row, root.gui.main_tree_storage);
    }

    root.state.prev_page ^= 0x1;
    root.state.json_changed = false;
}
void on_click_tree_node(const Gtk::TreeModel::Path& c_path, Gtk::TreeViewColumn* view)
{
    string path_to_object = "";
    JviModel row_model;
    Gtk::TreeModel::Path path = c_path;

    /* update path to clicked node */
    auto model = view->get_tree_view()->get_model();
    while (path) {
        string value = model->get_iter(path)->get_value(row_model.value_text);
        path_to_object = value + "/" + path_to_object;
        path.up();
    }
    root.gui.path_entry_buff->set_text(path_to_object);


    /*  expand/collapse on click */
    if (view->get_tree_view()->row_expanded(c_path))
        view->get_tree_view()->collapse_row(c_path);
    else
        view->get_tree_view()->expand_row(c_path, false);
}

void setup_gui(JviMainWindow* window)
{
    /* load resources */
    root.res.icon_doc = Gtk::IconTheme::get_default()->load_icon("text-x-generic", 16);
    root.res.icon_obj = Gtk::IconTheme::get_default()->load_icon("edit-copy", 16);
    root.res.icon_key = Gtk::IconTheme::get_default()->load_icon("media-playback-stop", 16);

    /* main window */
    window->set_default_size(800, 800);
    window->set_title("jvi - " + root.filename);

    /* widgets creation */
    Gtk::Notebook* notebook = new Gtk::Notebook();
    Gtk::ScrolledWindow* scrolled_window_oview = new Gtk::ScrolledWindow;
    Gtk::ScrolledWindow* scrolled_window_tview = new Gtk::ScrolledWindow;

    /* object-view widgets */
    Gtk::VBox* main_vbox = new Gtk::VBox;
    JviModel* model = new JviModel;
    root.gui.main_tree_storage = Gtk::TreeStore::create(*model);
    Gtk::TreeView* tree_view = new Gtk::TreeView(root.gui.main_tree_storage);
    Gtk::ListBox* bookmark_list = new Gtk::ListBox;
    Gtk::Button* bookmark_button = new Gtk::Button("Bookmarks");
    Gtk::Entry* path_entry = new Gtk::Entry;
    root.gui.path_entry_buff = path_entry->get_buffer();

    /* text-view widgets */
    root.gui.json_text_buff = Gtk::TextBuffer::create();
    Gtk::TextView* text_view = new Gtk::TextView(root.gui.json_text_buff);
    root.gui.json_text_buff->signal_changed().connect(sigc::ptr_fun(&on_buff_change));

    /* layout */
    Gtk::Paned* main_hpaned = new Gtk::Paned;
    window->add(*main_hpaned);

    notebook->append_page(*main_vbox, "Object View");
    notebook->append_page(*scrolled_window_tview, "Text View");
    notebook->signal_switch_page().connect(sigc::ptr_fun(&on_switch_page));

    main_hpaned->pack1(*notebook);
    main_hpaned->pack2(*bookmark_list);
    main_hpaned->set_wide_handle(true);
    main_hpaned->set_position((800/6)*5);

    bookmark_list->append(*bookmark_button);

    main_vbox->pack_start(*path_entry, false, false);
    main_vbox->pack_end(*scrolled_window_oview, true, true);

    text_view->show();
    notebook->show();
    main_hpaned->show();
    main_vbox->show();
    path_entry->show();
    scrolled_window_oview->show();
    scrolled_window_tview->show();
    bookmark_list->show();
    bookmark_button->show();


    /* other settings */
    Gtk::TreeView::Column* pColumn = Gtk::manage(new Gtk::TreeView::Column("JSON"));
    pColumn->pack_start(model->value_icon, false);
    pColumn->pack_start(model->value_text);
    tree_view->append_column(*pColumn);

    tree_view->set_hover_selection(true);
    tree_view->set_enable_tree_lines(true);
    tree_view->set_activate_on_single_click(true);
    tree_view->signal_row_activated().connect(sigc::ptr_fun(&on_click_tree_node));
    scrolled_window_oview->add(*tree_view);
    scrolled_window_tview->add(*text_view);
    tree_view->show();
}
string load_file(string filename)
{
    if (filename.size()) {
        std::filebuf fb;
        if (fb.open(filename, std::ios::in)) {
            std::istream is(&fb);

            stringstream str_stream;
            str_stream << is.rdbuf();
            return str_stream.str();
        }
    }
    return "";
}
Json::Value* parse_json(string file)
{
    Json::Value* json_root = new Json::Value;
    Json::Reader reader;

    auto success = reader.parse(file, *json_root);
    if (!success) {
        cout << "Ups! Problem parsing JSON\n";
        cout << reader.getFormattedErrorMessages();
        return nullptr;
    }
    return json_root;
}

void iter_node(Json::Value& json_root, auto view_root, auto main_tree_storage)
{
    JviModel model;
    auto make_node_view = [&model](auto name, auto val, auto view_child) {
        if (val.size()) {
            (*view_child)[model.value_text] = name + " : " + val;
            (*view_child)[model.value_icon] = root.res.icon_key;
        } else {
            (*view_child)[model.value_text] = name;
            (*view_child)[model.value_icon] = root.res.icon_obj;
        }
    };

    auto get_name = [&json_root](auto i, int& table_index) {
        string name = i.name();
        if (json_root.type() == Json::ValueType::arrayValue) {
            name = "[" + to_string(table_index++) + "]";
        }
        return name;
    };

    auto get_value = [](auto i) {
        string val = "";
        if (i->isConvertibleTo(Json::ValueType::stringValue)) {
            val = i->asString();
        }
        return val;
    };

    int index = 0;
    for (Json::ValueIterator i = json_root.begin(); i != json_root.end(); ++i)
    {
        string name = get_name(i, index);
        string val = get_value(i);

        auto view_child = main_tree_storage->append(view_root->children());

        if (i->size()) {
            make_node_view(name, val, view_child);
            iter_node(*i, view_child, main_tree_storage);
        } else {
            make_node_view(name, val, view_child);
        }
    }
}

