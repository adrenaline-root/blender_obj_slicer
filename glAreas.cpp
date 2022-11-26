#include "glAreas.h"
#define PI 3.14159265358979323846

// ================================================================
// --================ METODOS DE LA CLASE GL_AREA ===============--
// ================================================================

glDrawingArea::glDrawingArea()
{
	add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK);
	signal_realize().connect(sigc::mem_fun(*this, &glDrawingArea::attach_xwindow));
}

glDrawingArea::glDrawingArea(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder>const& builder)
	:Gtk::DrawingArea(obj)
	, builder{builder}
{
	add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK | Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK);
	set_double_buffered(false);
	set_can_focus(true);
	set_sensitive(true);
	signal_realize().connect(sigc::mem_fun(*this, &glDrawingArea::attach_xwindow));
}

glDrawingArea::~glDrawingArea()
{
}

void glDrawingArea::attach_xwindow()
{
	Gtk::Allocation allocation = get_allocation();
	memset(&attributes, 0, sizeof(attributes));
	
	std::cout << "attaching window..." << std::endl;

	attributes.x = allocation.get_x();
	attributes.y = allocation.get_y();
	attributes.width = allocation.get_width();
	attributes.height = allocation.get_height();

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.wclass = GDK_INPUT_OUTPUT;

	parent = get_window()->gobj();

	display = gdk_window_get_display(parent);
	xdisplay = GDK_DISPLAY_XDISPLAY(display);
	
	root = DefaultRootWindow(xdisplay);
	vi = glXChooseVisual(xdisplay, 0, att);

	win = GDK_WINDOW_XID(parent);
	XMapWindow(xdisplay, win);

	glc = glXCreateContext(xdisplay, vi, NULL, GL_TRUE);
	//glc2 = glXCreateContext(xdisplay, vi, NULL, GL_TRUE);
	

	
	//boxSelector = Model();
	//boxSelector.createCube(rejillaSize);
	
	attached = true;
	std::cout << "window attached..." << std::endl;
}

bool glDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	glClearColor(.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	if (attached == true)
	{
		
		makeCurrentContext();
		//glEnable(GL_DEPTH_TEST); 

		XGetWindowAttributes(xdisplay, win, &gwa);
		glViewport(0, 0, gwa.width, gwa.height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		GLdouble orto = gwa.width; //get_parent()->get_width();
		GLdouble ortov = gwa.height; //get_parent()->get_height();

		float z_aspect_correction = 1.0 / std::sin(45 * PI / 180);
		
		
		// Perspectiva normal: (estrechamiento del eje z -profundidad-);
		//glOrtho( -orto / 2, orto / 2, -ortov / z_aspect_correction / 2 , ortov / z_aspect_correction / 2, -ortov * orto * scale / 2, ortov * orto * scale / 2);
		glOrtho( -orto / 2, orto / 2, -ortov / 2 , ortov / 2, -ortov * orto * scale / 2, ortov * orto * scale / 2);
		
		glRotatef(45.0, 1.0, 0.0, 0.0);
		glRotatef(rotation, 0.0, 1.0, 0.0);
		glScalef(scale, scale, scale);
		glScalef(1.0, z_aspect_correction, 1.0);
		
				
		glTranslatef(tranlation_x, 0.0, tranlation_y);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		drawRejilla();
		drawAxis();
		glColor3f(1.0, 1.0, 1.0);
		glEnable(GL_DEPTH_TEST);
		
		if (blender_object) {
			
			draw_based_on_type();
		}
		
		glDisable(GL_DEPTH_TEST);
		glXSwapBuffers(xdisplay, win);

	}

	return true;
}

void glDrawingArea::draw_based_on_type()
{
	if (tipe == object_Type) {
		
		std::vector<int> indices = blender_object->getIndices();
		std::vector<float> vertices = blender_object->getVertexPositions();
		std::vector<float> uv_ = blender_object->getVertexTextCoords();
		std::vector<int> uv_indices = blender_object->getUVIndices();
		
		if (blender_object->texture) {
			glColor3f(1., 1., 1.);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, blender_object->gen_textures);
		}
		else glColor3f(.5, .5, .0);
		
		glBegin(GL_TRIANGLES);
		for (int x = 0; x < int(indices.size()); x += 3) {
			int index1 = indices[x] * 3;
			int index2 = indices[x + 1] * 3;
			int index3 = indices[x + 2] * 3;
			
			// Si el objeto no tiene uvs el programa .... petará
			
			int uv_index1 = uv_indices[x] * 2;
			int uv_index2 = uv_indices[x + 1] * 2;
			int uv_index3 = uv_indices[x + 2] * 2;
			
				glTexCoord2d(uv_[uv_index1], uv_[uv_index1 + 1]);
				//glTexCoord2d(.2, .2);
				glVertex3f(vertices[index1], vertices[index1 + 1], vertices[index1 + 2]);
				
				glTexCoord2d(uv_[uv_index2], uv_[uv_index2 + 1]);
				//glTexCoord2d(.8, .8);
				glVertex3f(vertices[index2], vertices[index2 + 1], vertices[index2 + 2]);
				
				glTexCoord2d(uv_[uv_index3], uv_[uv_index3 + 1]);
				//glTexCoord2d(.6, .6);
				glVertex3f(vertices[index3], vertices[index3 + 1], vertices[index3 + 2]);
		}
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
	
	else if (blender_object->has_been_sliced) {
		
		if (tipe == sliced_color_Type) {
			
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, blender_object->gen_textures);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blender_object->color_map->get_width(), blender_object->color_map->get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blender_object->color_map->get_pixels());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			
		}
		
		else {
			
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, blender_object->gen_textures);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blender_object->normal_map->get_width(), blender_object->normal_map->get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, blender_object->normal_map->get_pixels());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			
		auto faces = blender_object->sliced_indexes;
		auto positions = blender_object->sliced_positions;
		auto uvs = blender_object->sliced_uvs;
		
		
		for (auto face : faces) {
			
			vec3D v1 = positions[face.x];
			vec3D v2 = positions[face.y];
			vec3D v3 = positions[face.z];
			
			vec2D v1_uv = uvs[face.x];
			vec2D v2_uv = uvs[face.y];
			vec2D v3_uv = uvs[face.z];
			
			glBegin(GL_TRIANGLES);
			
				glTexCoord2d(v1_uv.x, v1_uv.y);
				glVertex3f(v1.x, v1.z, v1.y);
				
				glTexCoord2d(v2_uv.x, v2_uv.y);
				glVertex3f(v2.x, v2.z, v2.y);
				
				glTexCoord2d(v3_uv.x, v3_uv.y);
				glVertex3f(v3.x, v3.z, v3.y);
			glEnd();
		
		}
		
		
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
	}
}



void glDrawingArea::drawRejilla()
{
	
	int limRej[] = {-640, 640};
	
	glColor3f(0.3, 0.3, 0.3);
	for (int x = limRej[0]; x <= limRej[1]; x += rejillaSize) {
		if (x != limRej[0] && x != limRej[1]) {
			glBegin(GL_LINES);
				glVertex3f(x, 0, limRej[0]);
				glVertex3f(x, 0, limRej[1]);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f(limRej[0], 0, x);
				glVertex3f(limRej[1], 0, x);
			glEnd();
		}
		
	}
}

void glDrawingArea::drawAxis()
{
	glColor3f(0.0, 1.0, 0.0); 
	glBegin(GL_LINES);

		glVertex3f(0.0, 0.0, 0.0); 
		glVertex3f(0.0, 1.0*128, 0.0);

	glEnd();
	
	glColor3f(0.0, 0.0, 1.0); 
	glBegin(GL_LINES);

		glVertex3f(0.0, 0.0, 0.0); 
		glVertex3f(1.0*128, 0.0, 0.0);

	glEnd();
	
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);

		glVertex3f(0.0, 0.0, 0.0); 
		glVertex3f(0.0, 0.0, 1.0*128);

	glEnd();
}

bool glDrawingArea::on_event(GdkEvent *evento)
{
	if (evento->any.type == GDK_BUTTON_PRESS) {
		grab_focus();
	}
	
	if (is_focus()) {
		if (evento->any.type == GDK_KEY_PRESS) {
			on_key_press_event(&evento->key);
		}
	}
	
	return true;
}

bool glDrawingArea::on_key_press_event(GdkEventKey *keyevent) 
{
	int rotSpeed = 5;
	int trSpeed = 15 / scale;
	float scaleInc = 0.05;
	
	rotation = int(rotation) % 360;
	int mrotation = int(rotation);
	
	mrotation = mrotation % 360;
	if (rotation < 0) {
		mrotation = 360 + rotation;
	}
	
	if (keyevent->state == GDK_CONTROL_MASK) 
	{
		switch(keyevent->keyval)
		{
			case GDK_KEY_Left:
				rotation += rotSpeed;
				break;
			
			case GDK_KEY_Right:
				rotation -= rotSpeed;
				break;
				
			case GDK_KEY_Up:
				scale += scaleInc;
				break;
				
			case GDK_KEY_Down:
				scale -= scaleInc;
				break;
		}
	}
	
	else if (keyevent->state == GDK_SHIFT_MASK)
	{
		float radians = float((rotation) * PI / 180);
	
		switch(keyevent->keyval)
		{
			case GDK_KEY_Left:
				tranlation_x -= (std::cos(radians) * trSpeed);
				tranlation_y -= (std::sin(radians) * trSpeed);
				                                            
				break;                                      
			                                                
			case GDK_KEY_Right:                             
				tranlation_x += (std::cos(radians) * trSpeed);
				tranlation_y += (std::sin(radians) * trSpeed);
				break;
				
			case GDK_KEY_Up:
				tranlation_y -= (std::cos(radians) * trSpeed);
				tranlation_x += (std::sin(radians) * trSpeed);
				break;
			
			case GDK_KEY_Down:
				tranlation_y += (std::cos(radians) * trSpeed);
				tranlation_x -= (std::sin(radians) * trSpeed);
				break;
				
				
		}
	}
	
	
	queue_draw();
	
	return true;
}

void glDrawingArea::makeCurrentContext()
{
	glXMakeCurrent(xdisplay, win, glc);
}


drawingArea::drawingArea() 
{
	
}


drawingArea::drawingArea(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const& builder)
	: Gtk::DrawingArea(obj)
	, builder{builder}
{
	
		
}

drawingArea::~drawingArea()
{
	
}

bool drawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
	cr->set_source_rgba(1.0, 1.0, 1.0, 1.0); cr->fill(); cr->paint();
	if (texture) {
		Gdk::Cairo::set_source_pixbuf(cr, texture, 0, 0);
		cr->paint();
	}
	
	return true;
}



	