#ifndef JVI_H
#define JVI_H

#include <gtkmm.h>
#include <jsoncpp/json/json.h>
#include <string>

struct JviRoot {
    struct Gui {
        Glib::RefPtr<Gtk::EntryBuffer> path_entry_buff;
    } gui;
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
    }
    virtual ~JviModel() {}
    Gtk::TreeModelColumn<Glib::ustring> value_text;
    //Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > thumbnail;
};

void setup_gui(JviMainWindow* window,
               Glib::RefPtr<Gtk::TreeStore> mainTreeStorage,
               JviModel* model,
               std::string filename);
Json::Value* parse_json(std::string filename);
auto iterNode(Json::Value& jsonRoot, auto viewRoot, auto mainTreeStorage);


#endif //JVI_H
