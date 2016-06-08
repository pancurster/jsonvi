#include <gtkmm.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <functional>
#include "jvi.h"

using namespace std;

int main(int argc, char *argv[])
{
    int argcgtk = 1;
    auto app = Gtk::Application::create(argcgtk, argv, "org.gtkmm.examples.base");

    JviMainWindow* window = new JviMainWindow;
    JviModel* root_model = new JviModel;
    Glib::RefPtr<Gtk::TreeStore> main_tree_storage = Gtk::TreeStore::create(*root_model);
    setup_gui(window, main_tree_storage, root_model);

    string filename = argv[1] ? argv[1] : "";
    Json::Value* json_object = parse_json(filename);

    auto view_root = main_tree_storage->append();
    (*view_root)[root_model->value_text] = filename;

    iterNode(*json_object, view_root, main_tree_storage);

    cout << "size:" << json_object->size() << "\n";

    return app->run(*window);
}

void setup_gui(JviMainWindow*               window,
               Glib::RefPtr<Gtk::TreeStore> main_tree_storage,
               JviModel*                    model)
{
    window->set_default_size(800, 800);

    Gtk::ScrolledWindow* scrolled_window = new Gtk::ScrolledWindow;
    window->add(*scrolled_window);

    scrolled_window->show();

    Gtk::TreeView* tree_view = new Gtk::TreeView(main_tree_storage);
    tree_view->append_column("JSON", model->value_text);
    tree_view->set_hover_selection(true);
    tree_view->set_enable_tree_lines(true);
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

auto iterNode(Json::Value& json_root, auto view_root, auto main_tree_storage)
{
    JviModel model;
    auto make_node_view = [&model](auto name, auto val, auto child) {
        if (val.size()) {
            (*child)[model.value_text] = name + " : " + val;
        } else {
            (*child)[model.value_text] = name;
        }
    };

    int index = 0;
    for (auto i = json_root.begin(); i != json_root.end(); ++i)
    {
        string name = i.name();
        if (json_root.type() == Json::ValueType::arrayValue) {
            name = "[" + to_string(index++) + "]";
        }
        string val = "";
        if (i->isConvertibleTo(Json::ValueType::stringValue)) {
            val = i->asString();
        }

        auto child = main_tree_storage->append(view_root->children());

        if (i->size()) {
            make_node_view(name, val, child);
            iterNode(*i, child, main_tree_storage);
        } else {
            make_node_view(name, val, child);
        }
    }
};

