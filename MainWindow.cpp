#include "MainWindow.h"
#include <regex>
#include <string>

MainWindow::MainWindow(){
	
}

MainWindow::MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const& builder)
	:Gtk::Window(obj)
	, builder{builder}
{
	setMenu();
	configureButtons();
	
	builder->get_widget_derived("ObjArea", main_glArea);
	builder->get_widget_derived("colorArea", sliced_color_area);
	builder->get_widget_derived("normalsArea", sliced_norml_area);
	
	main_glArea->tipe = object_Type;
	sliced_color_area->tipe = sliced_color_Type;
	sliced_norml_area->tipe = sliced_normals_Type;
	
	
}

MainWindow::~MainWindow(){

}

void MainWindow::setMenu()
{
	auto mRefActionGroup = Gtk::ActionGroup::create();
	auto mRefUIManager = Gtk::UIManager::create();
	
	// ACCIONES DE LA VENTANA PRINCIPAL
	
	mRefActionGroup->add(Gtk::Action::create("FileMenu", "Archivo"));
	mRefActionGroup->add(Gtk::Action::create("ImportObj", "Importar Blender Object"), sigc::mem_fun(*this, &MainWindow::loadMesh));
	mRefActionGroup->add(Gtk::Action::create("Save", "Guardar TileSet"), sigc::mem_fun(*this, &MainWindow::saveTileSet));
	
	mRefActionGroup->add(Gtk::Action::create("Editar", "Editar"));
	mRefActionGroup->add(Gtk::Action::create("LaminarObj", "Laminar Blender Object"), sigc::mem_fun(*this, &MainWindow::sliceMesh));
	
	

	mRefUIManager->insert_action_group(mRefActionGroup);
	add_accel_group(mRefUIManager->get_accel_group());

	// EXPLICITACION DE LA JERARQUIA DEL MENU

	Glib::ustring ui_info = 

		"<ui>"
		"	<menubar name = 'MenuBar'>"
		"		<menu action = 'FileMenu'>"
		"			<menuitem action = 'ImportObj'/>"
		"			<menuitem action = 'Save'/>"
		"		</menu>"
		"		<menu action = 'Editar'>"
		"			<menuitem action = 'LaminarObj'/>"
		"		</menu>"
		"	</menubar>"
		"</ui>";
		

	
	// CONTROL DE ERRORES EN EL MENU

	#ifdef GLIBMM_EXCEPTIONS_ENABLED
	try
	{
		mRefUIManager->add_ui_from_string(ui_info);
	}
	catch(const Glib::Error& ex)
	{
		std::cerr << "building menus failed: " << ex.what();
	}
	#else
	std::auto_ptr<Glib::Error> ex;

	mRefUIManager->add_ui_from_string(ui_info, ex);
	if(ex.get())
	{
		std::cerr << "building menus failed: " << ex->what();
	}
	#endif //GLIBMM_EXCEPTIONS_ENABLED
	
	Gtk::Box *mainBox = nullptr;
	builder->get_widget("MainBox", mainBox);
	
	Gtk::Widget *pMenu = mRefUIManager->get_widget("/MenuBar");
	mainBox->pack_start(*pMenu, Gtk::PACK_SHRINK);
}


void MainWindow::configureButtons()
{
	Gtk::Button *file_chooser_btn = nullptr;
	Gtk::Entry *entry = nullptr;
	
	
	builder->get_widget("FileChooserButton", file_chooser_btn);
	builder->get_widget("direction_entry", entry);
	
	file_chooser_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MainWindow::showFileChooser), entry));
}

void MainWindow::loadMesh()
{
	
	Gtk::FileChooserDialog Browser(*this, "Elige un objeto blender(.obj)", Gtk::FILE_CHOOSER_ACTION_OPEN);
	Browser.set_filename("/home");
	Browser.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	Browser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);


	if (Browser.run() == Gtk::RESPONSE_OK)
	{ 
		blender_object.loadFromFile(Browser.get_filename().c_str());
		main_glArea->blender_object = &blender_object;
		sliced_color_area->blender_object = &blender_object;
		sliced_norml_area->blender_object = &blender_object;
		
		if (blender_object.texture) {
			
			main_glArea->makeCurrentContext();
			main_glArea->blender_object->bind_textures();
		}
		
	}
}

void MainWindow::sliceMesh()
{
	Gtk::Dialog *dimen_dialog = nullptr;
	Gtk::Button *accept_btn = nullptr;
	Gtk::Button *cancel_btn = nullptr;

	Gtk::SpinButton *spin_x = nullptr;
	Gtk::SpinButton *spin_y = nullptr;
	Gtk::SpinButton *spin_z = nullptr; 
	
	builder->get_widget("DimensionesDialog", dimen_dialog);
	builder->get_widget("accept_button", accept_btn);
	builder->get_widget("cancel_button", cancel_btn);
	
	builder->get_widget("gtkSpin1", spin_x);
	builder->get_widget("gtkSpin2", spin_y);
	builder->get_widget("gtkSpin3", spin_z);
	
	accept_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*dimen_dialog, &Gtk::Dialog::response), Gtk::RESPONSE_OK));
	cancel_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*dimen_dialog, &Gtk::Dialog::response), Gtk::RESPONSE_CLOSE));
	
	auto adjustment_x = spin_x->get_adjustment();
	auto adjustment_y = spin_y->get_adjustment();
	auto adjustment_z = spin_z->get_adjustment();
	
	auto d = blender_object.min_dimensions;
	
	adjustment_x->set_lower(float(d.x));
	adjustment_y->set_lower(float(d.y));
	adjustment_z->set_lower(float(d.z));
	
	spin_x->set_value(float(d.x));
	spin_y->set_value(float(d.y));
	spin_z->set_value(float(d.z));

	if (dimen_dialog->run() == Gtk::RESPONSE_OK) {
		blender_object.dimensiones = {int(spin_x->get_value()), int(spin_y->get_value()), int(spin_z->get_value())};
		
		// Creamos dos texturas basadas en el laminado de sprites: una para el color y otra para las normales
		
		auto sliced_pixbuffs = blender_object.sliceObject();
		blender_object.parse_to_maps(sliced_pixbuffs);
		
		// Rellenamos los volúmenes creados de color sólido;
		
		//blender_object.fill_volume();
		
		// Creamos una serie de data que reflejan los atributos del sliced object (en este punto es necesario que se hallan creado las texturas)

		blender_object.set_sliced_geometry(); // -> el atributo has_been_sliced pasa a true

		dimen_dialog->close();
	}
	
	else {
		dimen_dialog->close();
	}
	
}

void MainWindow::saveTileSet()
{
	Gtk::Dialog *save_dialog = nullptr;
	Gtk::Button *accept_btn = nullptr;
	Gtk::Button *cancel_btn = nullptr;
	Gtk::Button *file_chooser_btn = nullptr;

	Gtk::SpinButton *spin_x = nullptr;
	Gtk::SpinButton *spin_y = nullptr;
	Gtk::SpinButton *spin_z = nullptr;
	
	Gtk::SpinButton *spin_x2 = nullptr;
	Gtk::SpinButton *spin_y2 = nullptr;
	
	Gtk::Entry *direction_entry = nullptr;
	Gtk::Entry *name_entry = nullptr;
	
	
	builder->get_widget("SaveDialog", save_dialog);
	builder->get_widget("save_button", accept_btn);
	builder->get_widget("cancel_save_button", cancel_btn);
	builder->get_widget("FileChooserButton", file_chooser_btn);
	
	builder->get_widget("gtkSpin4", spin_x);
	builder->get_widget("gtkSpin5", spin_y);
	builder->get_widget("gtkSpin6", spin_z);
	
	builder->get_widget("gtkSpin7", spin_x2);
	builder->get_widget("gtkSpin8", spin_y2);
	
	builder->get_widget("direction_entry", direction_entry);
	builder->get_widget("name_entry", name_entry);
	
	accept_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*save_dialog, &Gtk::Dialog::response), Gtk::RESPONSE_OK));
	cancel_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*save_dialog, &Gtk::Dialog::response), Gtk::RESPONSE_CLOSE));
	
	//file_chooser_btn->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MainWindow::showFileChooser), entry));
	
	
	auto adjustment_x = spin_x->get_adjustment();
	auto adjustment_y = spin_y->get_adjustment();
	auto adjustment_z = spin_z->get_adjustment();
	
	auto adjustment_x2 = spin_x2->get_adjustment();
	auto adjustment_y2 = spin_y2->get_adjustment();
	
	auto d = blender_object.dimensiones;
	
	adjustment_x->set_lower(float(d.x));
	adjustment_y->set_lower(float(d.y));
	adjustment_z->set_lower(float(d.z));
	
	adjustment_x2->set_lower(float(0));
	adjustment_y2->set_lower(float(0));
	
	
	spin_x->set_value(float(d.x));
	spin_y->set_value(float(d.y));
	spin_z->set_value(float(d.z));
	
	spin_x2->set_value(float(d.x));
	spin_y2->set_value(float(d.y));
	
	name_entry->set_text("untitled");
	
	if (save_dialog->run() == Gtk::RESPONSE_OK) {
		save_files_tileSet(direction_entry->get_text(), name_entry->get_text(), {spin_x->get_value(), spin_y->get_value(), spin_z->get_value()}, {spin_x2->get_value(), spin_y2->get_value()});
		save_dialog->close();
	}
	
	else {
		save_dialog->close();
	}
}

void MainWindow::showFileChooser(Gtk::Entry *entry)
{
	Gtk::FileChooserDialog dialog(*this, "Select a folder", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
	dialog.add_button("Abrir", Gtk::RESPONSE_OK);
	dialog.add_button("Cancelar", Gtk::RESPONSE_CANCEL);
	
	if (dialog.run() == Gtk::RESPONSE_OK) {
		entry->set_text(dialog.get_filename());
		dialog.close();
	}
	
	else {
		dialog.close();
	}
}

void MainWindow::save_files_tileSet(Glib::ustring path, Glib::ustring tile_set_name, vec3Di ts_dimensions, vec2D t_dimensions) 
{
	std::cout << "El archivo: " << path << "/" << tile_set_name << " ha sido guardado con éxito" << std::endl;
	std::ofstream archivo;
	archivo.open(path + "/" + tile_set_name + ".tls");
	
	
	if (!archivo) std::cerr << "File couldn't be opened" << std::endl;
	else {
	
		archivo << "Tile Set \nn-- " << tile_set_name << std::endl;
		archivo << "d-- " << ts_dimensions.x << " " << ts_dimensions.y << " " << ts_dimensions.z << std::endl;
		
		int total_elements = ts_dimensions.x / t_dimensions.x;
		std::vector<vec2D> track_array;
		std::vector<std::vector<int>> track_layers;
		
		archivo << "ne- " << total_elements << std::endl;
		archivo << "ed- " << t_dimensions.x << " " << t_dimensions.y << std::endl;
		
		for (int e = 0; e < total_elements; e++) {
			track_array.push_back({ float(e * (t_dimensions.x + 2)), 0.0 });
			track_layers.push_back({});
		}

		// Las texturas se crean con un margen de 1px para cada elemento, es decir, cada elemento es una pequeña isla rodeada por un margen de 1px para evitar los solapamientos a la hora de 
		// renderizar las texturas con opengl
		
		
		Cairo::RefPtr<Cairo::ImageSurface> color_map_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, ts_dimensions.x + (total_elements * 2), ts_dimensions.y * ts_dimensions.z + (total_elements * 2));
		Cairo::RefPtr<Cairo::ImageSurface> normal_map_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, ts_dimensions.x + (total_elements * 2), ts_dimensions.y * ts_dimensions.z + (total_elements * 2));
		
		Cairo::RefPtr<Cairo::Context> cr_c = Cairo::Context::create(color_map_surface);
		Cairo::RefPtr<Cairo::Context> cr_n = Cairo::Context::create(normal_map_surface);
		
		cr_c->set_source_rgba(0.0, 0.0, 0.0, 0.0);
		cr_c->paint();
		
		cr_n->set_source_rgba(0.0, 0.0, 0.0, 0.0);
		cr_n->paint();
		
		// Localizamos cada capa del objeto en el color_map y en el normal_map; la subdividimos en tanto elementos como se hayan indicado y si la region del pixbuf perteneciente a dicha
		// elemento no está vacía (esto es, no todos sus pixeles son transparentes) procedemos a su empaquetamiento: como textura src (en formato png), como archivo src_uv (como txt);
		
		int last_layer = 0;
		
		if (blender_object.has_been_sliced) {
			
			int total_columns = blender_object.color_map->get_width() / ts_dimensions.x;
			int total_rows = blender_object.color_map->get_height() / ts_dimensions.y;
			
			for (int row = 0; row < total_rows; row++) {
				for (int column = 0; column < total_columns; column++) {
					
					int x_region = column * ts_dimensions.x;
					int y_region = row * ts_dimensions.y;
					
					int current_layer = row * total_columns + column;
					
					auto current_ts_layer = Gdk::Pixbuf::create_subpixbuf(blender_object.color_map, x_region, y_region, ts_dimensions.x, ts_dimensions.y);
					auto current_tsn_layer = Gdk::Pixbuf::create_subpixbuf(blender_object.normal_map, x_region, y_region, ts_dimensions.x, ts_dimensions.y);
					
					for (int te = 0; te < total_elements; te++) {
						auto current_tile_element = Gdk::Pixbuf::create_subpixbuf(current_ts_layer, te * t_dimensions.x, 0, t_dimensions.x, t_dimensions.y);
						
						if (check_if_layer_is_empty(current_tile_element) == false) {
							auto current_tile_nelement = Gdk::Pixbuf::create_subpixbuf(current_tsn_layer, te * t_dimensions.x, 0, t_dimensions.x, t_dimensions.y);
							
							// Dibujamos el elemento en el color_map
							
							cr_c->save();
							cr_c->translate(int(track_array[te].x) + 1, int(track_array[te].y) + 1);
							Gdk::Cairo::set_source_pixbuf(cr_c, current_tile_element, 0, 0);
							cr_c->paint();
							cr_c->restore();
							
							// Dibujamos el elemento en el normal_map
							
							cr_n->save();
							cr_n->translate(int(track_array[te].x) + 1, int(track_array[te].y) + 1);
							Gdk::Cairo::set_source_pixbuf(cr_n, current_tile_nelement, 0, 0);
							cr_n->paint();
							cr_n->restore();
							
							// Hacemos seguimiento de las posiciones y las capas que ocupa ese elemento
							
							track_array[te].y += (t_dimensions.y + 2);
							track_layers[te].push_back(current_layer);
							
							// Hacemos seguimiento de la altura total que tendrán nuestros maps 
							
							if (current_layer > last_layer) last_layer = current_layer;
							
						}
						
						
					}
					
				}
			}
			
			auto color_map = Gdk::Pixbuf::create(color_map_surface, 0, 0, color_map_surface->get_width(), color_map_surface->get_height());
			auto final_color_map = Gdk::Pixbuf::create_subpixbuf(color_map, 0, 0, color_map->get_width(), (last_layer + 1) * (ts_dimensions.y + 2));
			final_color_map->save(path + "/" + tile_set_name + "_color_map.png", "png");
			
			auto normal_map = Gdk::Pixbuf::create(normal_map_surface, 0, 0, normal_map_surface->get_width(), normal_map_surface->get_height());
			auto final_normal_map = Gdk::Pixbuf::create_subpixbuf(normal_map, 0, 0, normal_map->get_width(), (last_layer + 1) * (ts_dimensions.y + 2));
			final_normal_map->save(path + "/" + tile_set_name + "_normal_map.png", "png");
			
			float uv_width = float(t_dimensions.x + 2) / float(final_color_map->get_width());
			float uv_height = float(t_dimensions.y + 2) / float(final_color_map->get_height());
			
			float fraction_u = float(1.5) / float(final_color_map->get_width());
			float fraction_v = float(1.5) / float(final_color_map->get_height());
			
			std::vector<vec2D> plane_uvs = {

				{0.0 + fraction_u, 0.0 + fraction_v},
				{uv_width - fraction_u, 0.0 + fraction_v},
				{uv_width - fraction_u, uv_height - fraction_v},
				{0.0 + fraction_u, uv_height - fraction_v}

			};
			
			for (int ele = 0; ele < total_elements; ele++) {
				
				int first_layer = track_layers[ele][0];
				
				for (auto layer : track_layers[ele]) {
					
					archivo << "e-- " << ele << " " << layer << " ";
					
					//std::cout << "element " << ele << " " << layer << std::endl;
					
					
					float u0 = ele * uv_width; 
					float v0 = (layer - first_layer) * uv_height;
					
					for (int uvi = 0; uvi < 4; uvi++) {
						vec2D uv = plane_uvs[uvi];
						uv.x += u0;
						uv.y += v0;
						
						archivo << uv.x << " " << uv.y << " ";
					}
					
					archivo << std::endl;
				}
				archivo << std::endl;
				
			}
			
		}

		archivo.close();
	}
	
}

bool MainWindow::check_if_layer_is_empty(Glib::RefPtr<Gdk::Pixbuf> layer) 
{
	bool _is_empty = true;
	
	for (int y = 0; y < layer->get_height(); y++) {
		for (int x = 0; x < layer->get_width(); x++) {
			
			guint red_index = y * layer->get_rowstride() + x * layer->get_n_channels();
			guint8 *pixels = layer->get_pixels();
			
			if (pixels[red_index + 3] != 0) {
				_is_empty = false;
				break;
			}
		}
		
		if (!_is_empty) break;
	}
	
	return _is_empty;
}
