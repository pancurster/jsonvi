#include <gtkmm.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <functional>
#include "jvi.h"

// TODO:
// add bookmarks
// add some colors
// remove mark over mousecursor (instead mark on click)

using namespace std;

JviRoot root;

int main(int argc, char *argv[])
{
    root.filename = argv[1] ? argv[1] : "";

    int argcgtk = 1;
    auto app = Gtk::Application::create(argcgtk, argv, "org.gtkmm.examples.base");

    /* load resources */
    root.res.icon_doc = Gtk::IconTheme::get_default()->load_icon("text-x-generic", 16);
    root.res.icon_obj = Gtk::IconTheme::get_default()->load_icon("edit-copy", 16);
    root.res.icon_key = Gtk::IconTheme::get_default()->load_icon("media-playback-stop", 16);

    /* setup main widgets */
    JviMainWindow* window = new JviMainWindow;
    JviModel* root_model = new JviModel;
    Glib::RefPtr<Gtk::TreeStore> main_tree_storage = Gtk::TreeStore::create(*root_model);
    setup_gui(window, main_tree_storage, root_model);

    auto view_root = main_tree_storage->append();
    (*view_root)[root_model->value_text] = root.filename;
    (*view_root)[root_model->value_icon] = root.res.icon_doc;

    /* parse json file */
    Json::Value* json_object = parse_json(root.filename);

    /* populate view by json */
    iter_node(*json_object, view_root, main_tree_storage);

    cout << "size:" << json_object->size() << "\n";

    return app->run(*window);
}

void click_tree_node_handler(const Gtk::TreeModel::Path& c_path, Gtk::TreeViewColumn* view)
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

void setup_gui(JviMainWindow*               window,
               Glib::RefPtr<Gtk::TreeStore> main_tree_storage,
               JviModel*                    model)
{
    /* main window */
    window->set_default_size(800, 800);
    window->set_title("jvi - " + root.filename);

    /* layout */
    Gtk::VBox* main_vbox = new Gtk::VBox;
    window->add(*main_vbox);

    Gtk::Entry* path_entry = new Gtk::Entry;
    path_entry->show();
    root.gui.path_entry_buff = path_entry->get_buffer();

    Gtk::ScrolledWindow* scrolled_window = new Gtk::ScrolledWindow;
    scrolled_window->show();

    main_vbox->pack_start(*path_entry, false, false);
    main_vbox->pack_end(*scrolled_window, true, true);
    main_vbox->show();


    /* other settings */
    Gtk::TreeView* tree_view = new Gtk::TreeView(main_tree_storage);

    Gtk::TreeView::Column* pColumn = Gtk::manage(new Gtk::TreeView::Column("JSON"));
    pColumn->pack_start(model->value_icon, false);
    pColumn->pack_start(model->value_text);
    tree_view->append_column(*pColumn);

    tree_view->set_hover_selection(true);
    tree_view->set_enable_tree_lines(true);
    tree_view->set_activate_on_single_click(true);
    tree_view->signal_row_activated().connect(sigc::ptr_fun(&click_tree_node_handler));
    scrolled_window->add(*tree_view);
    tree_view->show();
}

Json::Value* parse_json(string filename)
{
    Json::Value* json_root = new Json::Value;
    Json::Reader reader;

    if (filename.size()) {
        std::filebuf fb;
        if (fb.open(filename, std::ios::in)) {
            std::istream is(&fb);
            auto success = reader.parse(is, *json_root);
            if (!success) {
                cout << "Ups! Problem parsing JSON\n";
                cout << reader.getFormattedErrorMessages();
                return nullptr;
            }
            fb.close();
        }
    }
    return json_root;
}

auto iter_node(Json::Value& json_root, auto view_root, auto main_tree_storage)
{
    JviModel model;
    auto make_node_view = [&model](auto name, auto val, auto child) {
        if (val.size()) {
            (*child)[model.value_text] = name + " : " + val;
            (*child)[model.value_icon] = root.res.icon_key;
        } else {
            (*child)[model.value_text] = name;
            (*child)[model.value_icon] = root.res.icon_obj;
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

        auto child = main_tree_storage->append(view_root->children());

        if (i->size()) {
            make_node_view(name, val, child);
            iter_node(*i, child, main_tree_storage);
        } else {
            make_node_view(name, val, child);
        }
    }
};

