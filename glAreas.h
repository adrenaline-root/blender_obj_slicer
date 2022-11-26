#ifndef GL_AREAS_H
#define GL_AREAS_H

#include "headers.h"
#include "ObjLoader.h"


enum TypeGlArea { object_Type, sliced_color_Type, sliced_normals_Type };

class glDrawingArea : public Gtk::DrawingArea {
public:
	Glib::RefPtr<Gtk::Builder> builder;
	TypeGlArea tipe;
	
	ObjModel *blender_object;
	
	GdkWindow *parent;
	GdkDisplay *display;
	GdkWindowAttr attributes;
	float* data = NULL;
	Glib::RefPtr<Gdk::Pixbuf> bufferdata;
	float rotation = 0.0;
	float scale = 1.0;
	float tranlation_x = 0.0;
	float tranlation_y = 0.0;
	int rejillaSize = 64;
	//Model boxSelector;

	float zoom = 1.0;
	float giro = 0.0;
	
	bool attached = false;
	
	
	Display *xdisplay;
	Window root;
	GLint att[5] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *vi;
	Window win;
	GLXContext glc;
	XWindowAttributes gwa;
	
	glDrawingArea();
	glDrawingArea(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder>const& builder); //CONSTRUCTOR FORMATO XML
	virtual ~glDrawingArea();
	
	void attach_xwindow();
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
	bool on_event(GdkEvent *Evento);
	bool on_key_press_event(GdkEventKey*);
	void drawRejilla();
	void drawAxis();
	void makeCurrentContext();
	void draw_based_on_type();
};


class drawingArea : public Gtk::DrawingArea {
public:
	Glib::RefPtr<Gdk::Pixbuf> texture;
	Glib::RefPtr<Gtk::Builder> builder;
	
	drawingArea();
	drawingArea(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder>const& builder);
	virtual ~drawingArea();
	bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr);
};


#endif