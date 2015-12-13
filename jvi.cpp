#include <gtkmm.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <fstream>
#include <functional>

using namespace std;

class HelloWorld : public Gtk::Window
{
public:
    HelloWorld() : button("Hello World") {
        set_border_width(10);

        button
            .signal_clicked()
            .connect(sigc::mem_fun(*this, &HelloWorld::on_button_clicked));
    }
    virtual ~HelloWorld() {}

protected:
    void on_button_clicked() {
        cout << "Hello world\n";
    }
    Gtk::Button button;
};

class Model : public Gtk::TreeModelColumnRecord
{
public:
    Model() {
        add(value_text);
    }
    virtual ~Model() {}
    Gtk::TreeModelColumn<Glib::ustring> value_text;
};

auto iterNode(Json::Value root, auto viewParent, auto makeNodeView) {
    for (auto i = root.begin(); i != root.end(); ++i) {
        if (i->size()) {
            auto subViewParent = makeNodeView(i, viewParent);
            iterNode(*i, subViewParent, makeNodeView);
        } else {
            makeNodeView(i, viewParent);
        }
    }
};

int main(int argc, char *argv[])
{
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.base");

    HelloWorld window;
    window.set_default_size(400, 400);
    Gtk::ScrolledWindow scrolledWindow;
    window.add(scrolledWindow);
    scrolledWindow.show();

    Model model;
    auto refTreeStore = Gtk::TreeStore::create(model);

    Gtk::TreeView treeView(refTreeStore);
    treeView.append_column("JSON", model.value_text);
    treeView.set_hover_selection(true);
    treeView.set_enable_tree_lines(true);
    scrolledWindow.add(treeView);
    treeView.show();

    auto makeNodeView = [&refTreeStore, &model](auto jsonVal, auto parent) {
        auto child = refTreeStore->append(parent->children());
        if (jsonVal->size()) {
            (*child)[model.value_text] = jsonVal.name();
        } else {
            if (jsonVal->isConvertibleTo(Json::ValueType::stringValue)) {
                (*child)[model.value_text] =
                    jsonVal.name() + " : " + jsonVal->asString();
            } else {
                (*child)[model.value_text] = jsonVal.name();
                cout << "Not convertible to string:" << *jsonVal;
            }
        }
        return child;
    };

    Json::Value root;
    Json::Reader reader;
    //if (argv[1]) {
    //string filename = "sample.json";
    string filename = "big.json";
    if (filename.size()) {
        std::filebuf fb;
        if (fb.open(filename, std::ios::in)) {
            std::istream is(&fb);
            auto success = reader.parse(is, root);
            if (!success) {
                cout << "Ups! Problem parsing JSON\n";
                cout << reader.getFormattedErrorMessages();
                return 1;
            }
            fb.close();
        }

        auto row_iter = refTreeStore->append();
        (*row_iter)[model.value_text] = filename;

        iterNode(root, row_iter, makeNodeView);

        cout << "size:" << root.size() << "\n";
    }

    return app->run(window);
}
