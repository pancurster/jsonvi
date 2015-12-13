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

auto iterNode(Json::Value& jsonRoot, auto viewRoot, auto mainTreeStorage)
{
    Model model;
    auto makeNodeView = [&model](auto name, auto val, auto child) {
        if (val.size()) {
            (*child)[model.value_text] = name + " : " + val;
        } else {
            (*child)[model.value_text] = name;
        }
    };

    int index = 0;
    for (auto i = jsonRoot.begin(); i != jsonRoot.end(); ++i)
    {
        string name = i.name();
        if (jsonRoot.type() == Json::ValueType::arrayValue) {
            name = "[" + to_string(index++) + "]";
        }
        string val = "";
        if (i->isConvertibleTo(Json::ValueType::stringValue)) {
            val = i->asString();
        }

        auto child = mainTreeStorage->append(viewRoot->children());

        if (i->size()) {
            makeNodeView(name, val, child);
            iterNode(*i, child, mainTreeStorage);
        } else {
            makeNodeView(name, val, child);
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
    auto mainTreeStorage = Gtk::TreeStore::create(model);

    Gtk::TreeView treeView(mainTreeStorage);
    treeView.append_column("JSON", model.value_text);
    treeView.set_hover_selection(true);
    treeView.set_enable_tree_lines(true);
    scrolledWindow.add(treeView);
    treeView.show();

    Json::Value jsonRoot;
    Json::Reader reader;
    //if (argv[1]) {
    //string filename = "sample.json";
    string filename = "big.json";
    if (filename.size()) {
        std::filebuf fb;
        if (fb.open(filename, std::ios::in)) {
            std::istream is(&fb);
            auto success = reader.parse(is, jsonRoot);
            if (!success) {
                cout << "Ups! Problem parsing JSON\n";
                cout << reader.getFormattedErrorMessages();
                return 1;
            }
            fb.close();
        }

        auto viewRoot = mainTreeStorage->append();
        (*viewRoot)[model.value_text] = filename;

        iterNode(jsonRoot, viewRoot, mainTreeStorage);

        cout << "size:" << jsonRoot.size() << "\n";
    }

    return app->run(window);
}
