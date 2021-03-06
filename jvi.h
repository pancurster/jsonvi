#ifndef JVI_H
#define JVI_H

#include <gtkmm.h>
#include <json/json.h>
#include <string>

struct JviRoot {
    std::string filename;
    struct Gui {
        Glib::RefPtr<Gtk::EntryBuffer> path_entry_buff;
        Glib::RefPtr<Gtk::TextBuffer> json_text_buff;
        Glib::RefPtr<Gtk::TreeStore> tree_storage;
    } gui;
    struct Resources {
        Glib::RefPtr<Gdk::Pixbuf> icon_doc;
        Glib::RefPtr<Gdk::Pixbuf> icon_obj;
        Glib::RefPtr<Gdk::Pixbuf> icon_key;
    } res;
    struct State {
        int  prev_page = 1;
        bool json_changed = true;
    } state;
};

class JviMainWindow : public Gtk::Window
{
public:
    JviMainWindow() : button("Hello World")\
    {
        set_border_width(10);

        button.signal_clicked()
              .connect(sigc::mem_fun(*this, &JviMainWindow::on_button_clicked));
    }
    virtual ~JviMainWindow() {}

protected:
    void on_button_clicked()
    {
        //cout << "Hello world\n";
    }
    Gtk::Button button;
};

class JviModel : public Gtk::TreeModelColumnRecord
{
public:
    JviModel()
    {
        add(value_text);
        add(value_icon);
    }
    virtual ~JviModel() {}
    Gtk::TreeModelColumn<Glib::ustring> value_text;
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > value_icon;
};

void setup_gui(JviMainWindow* window);
Json::Value* parse_json (std::string filename);
void iter_node(Json::Value& jsonRoot, auto viewRoot, auto mainTreeStorage);
std::string load_file(std::string filename);

#endif //JVI_H
