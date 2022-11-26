#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glAreas.h"
#include "ObjLoader.h"


class MainWindow : public Gtk::Window {
public:
	Glib::RefPtr<Gtk::Builder> builder;
	std::string name = "MainWindow";
	
	glDrawingArea *main_glArea = nullptr;
	glDrawingArea *sliced_color_area = nullptr;
	glDrawingArea *sliced_norml_area = nullptr;
	
	drawingArea *cltextDrawArea = nullptr;
	drawingArea *nmtextDrawArea = nullptr;
	
	ObjModel blender_object;
	
	Glib::RefPtr<Gtk::ActionGroup> mRefActionGroup;
	Glib::RefPtr<Gtk::UIManager> mRefUIManager;
	
	MainWindow();
	MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const& builder); // CONSTRUCTOR PARA EL FORMATO XML
	virtual ~MainWindow();
	void setMenu();
	void configureButtons();
	void loadMesh();
	void sliceMesh();
	void saveTileSet();
	void showFileChooser(Gtk::Entry *entry);
	void save_files_tileSet(Glib::ustring path, Glib::ustring tile_set_name, vec3Di tile_set_dimensions, vec2D tile_dimensions);
	bool check_if_layer_is_empty(Glib::RefPtr<Gdk::Pixbuf> layer);
	
	
};



#endif