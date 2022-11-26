#include "ObjLoader.h"
#include <regex>
#include <algorithm>

ObjModel::ObjModel()
{
}

ObjModel::~ObjModel()
{
}

bool ObjModel::startsWith(std::string& line, const char* text)
{
	size_t textlen = strlen(text);
	if (line.size() < textlen)
	{
		return false;
	}
	for (size_t i = 0; i < textlen; i++)
	{
		if (line[i] == text[i]) continue;
		else return false;
	}
	
	return true;
}

void ObjModel::loadFromFile(const char* filename)
{
	std::cout << "revisando archivo obj: " << filename << std::endl;	
	std::ifstream file(filename);

	indexes.resize(0);
	uv_indexes.resize(0);
	n_indexes.resize(0);
	positions.resize(0);
	normals.resize(0);
	uvs.resize(0);
	
	if (file)
	{
		std::string line;
		while (getline(file, line))
		{
			std::replace(line.begin(), line.end(), '.', ',');
			if (startsWith(line, "v ")) {
				vec3D pos;
				sscanf(line.c_str(), "v %f %f %f", &pos.x, &pos.y, &pos.z);
				
				positions.push_back(pos);
			}
			
			else if (startsWith(line, "vn ")) {
				vec3D norm;
				sscanf(line.c_str(), "vn %f %f %f", &norm.x, &norm.y, &norm.z);
				
//				char str_nx[20], str_ny[20], str_nz[20];
//				
//				sprintf(str_nx, "%.3f", norm.x);
//				sprintf(str_ny, "%.3f", norm.y);
//				sprintf(str_nz, "%.3f", norm.z);
//				
//				sscanf(str_nx, "%f", &norm.x);
//				sscanf(str_ny, "%f", &norm.y);
//				sscanf(str_nz, "%f", &norm.z);
				
				normals.push_back(norm);
			}
			
			else if (startsWith(line, "vt ")) {
				vec2D uv;
				sscanf(line.c_str(), "vt %f %f", &uv.x, &uv.y);
				uvs.push_back(uv);
			}
			
			else if (startsWith(line, "f "))
			{
				std::regex regx ("(\\d+/)");
				std::smatch match;
				std::regex_search(line, match, regx);
				
				vec3Di index;
				vec3Di uv_index;
				vec3Di n_index;
				
				if (match.size() == 0) {
					sscanf(line.c_str(), "f %d %d %d", &index.x, &index.y, &index.z);
					index = {index.x - 1, index.y - 1, index.z - 1};
					indexes.push_back(index);
				}
				
				else if (match.size() == 1) {
					sscanf(line.c_str(), "f %d/%d %d/%d %d/%d",
										&index.x, &uv_index.x,
										&index.y, &uv_index.y,
										&index.z, &uv_index.z);
					
					index = {index.x - 1, index.y - 1, index.z - 1};
					indexes.push_back(index);
					uv_index = {uv_index.x - 1, uv_index.y - 1, uv_index.z - 1};
					uv_indexes.push_back(uv_index);
				}
				
				else if (match.size() == 2) {
					sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
										&index.x, &uv_index.x, &n_index.x,
										&index.y, &uv_index.y, &n_index.y,
										&index.z, &uv_index.z, &n_index.z);
					index = {index.x - 1, index.y - 1, index.z - 1};
					indexes.push_back(index);
					uv_index = {uv_index.x - 1, uv_index.y - 1, uv_index.z - 1};
					uv_indexes.push_back(uv_index);
					n_index = {n_index.x - 1, n_index.y - 1, n_index.z - 1};
					n_indexes.push_back(n_index);
				}
				
			}
		}
		
		std::cout << "Cargando textura... " << std::endl;
		loadTextureFromFile(filename);
		set_dimensions_from_load();
	}
	else
	{
		std::cout << "Problem loading Obj file" << std::endl; 
	}
}

void ObjModel::loadTextureFromFile(const char* filename) 
{
	std::string str = filename;
	std::string texturepath = str.substr(0, str.size() - 4);
	texturepath = texturepath + ".mtl";
	
	std::ifstream file(texturepath);
	
	if (file) {
		
		std::string line;
		
		while(getline(file, line)) {
			if (startsWith(line, "map_Kd ")) {
				std::string texture_path = line.substr(7, line.size());
				
				try {
					texture = Gdk::Pixbuf::create_from_file(texture_path);
					std::cout << "texture loaded" << std::endl;
				}
				
				catch (Glib::FileError &error) {
					std::cerr << "Error loading file at: " << texture_path << "; " << error.what() << std::endl;
				}
				
				catch (Gdk::PixbufError &perror){
					std::cerr << "Error creating texture: " << perror.what() << std::endl;
				}
			}
		}
	}
	
	else {
		std::cout << "No existe archivo de texturas... "<< std::endl;
	}
	
	
}

void ObjModel::set_dimensions_from_load() 
{
	float min_x, max_x, min_y, max_y, min_z, max_z;
	max_x = min_x = positions[0].x;
	max_y = min_y = positions[0].z;
	max_z = min_z = positions[0].y;
	
	for (auto v : positions) {
		if (v.x < min_x) min_x = v.x;
		if (v.x > max_x) max_x = v.x;
		
		if (v.z < min_y) min_y = v.z;
		if (v.z > max_y) max_y = v.z;
		
		if (v.y < min_z) min_z = v.y;
		if (v.y > max_z) max_z = v.y;
	}
	
	origin = {min_x, min_y, min_z};
	dimensiones = {int(max_x - min_x), int(max_y - min_y), int(max_z - min_z)};
	min_dimensions = {int(max_x) + 1, int(max_y) + 1, int(max_z) + 1};
	std::printf("Las dimensiones del objeto son: (%d, %d, %d)\n", dimensiones.x, dimensiones.y, dimensiones.z);
}

std::vector<float> ObjModel::getVertexPositions()
{
	std::vector<float> vpos;
	for (vec3D v : positions) {
		vpos.push_back(v.x);
		vpos.push_back(v.y);
		vpos.push_back(v.z);
	}
	
	return vpos;
}

std::vector<float> ObjModel::getVertexNormals()
{
	std::vector<float> vpos;
	for (vec3D v : normals) {
		vpos.push_back(v.x);
		vpos.push_back(v.y);
		vpos.push_back(v.z);
	}
	
	return vpos;
}

std::vector<float> ObjModel::getVertexTextCoords()
{
	std::vector<float> vpos;
	for (vec2D v : uvs) {
		vpos.push_back(v.x);
		vpos.push_back(v.y);
	}
	
	return vpos;
}

std::vector<int> ObjModel::getIndices()
{
	std::vector<int> vpos;
	for (vec3Di v : indexes) {
		vpos.push_back(v.x);
		vpos.push_back(v.y);
		vpos.push_back(v.z);
	}
	
	return vpos;
}

std::vector<int> ObjModel::getUVIndices() 
{
	std::vector<int> vpos;
	for (vec3Di v : uv_indexes) {
		vpos.push_back(v.x);
		vpos.push_back(v.y);
		vpos.push_back(v.z);
	}
	
	return vpos;
}

void ObjModel::bind_textures() 
{
	std::cout << "binding textures..." << std::endl;
	glGenTextures(1, &gen_textures);
	glBindTexture(GL_TEXTURE_2D, gen_textures);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->get_width(), texture->get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->get_pixels());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}

std::vector<Glib::RefPtr<Gdk::Pixbuf>> ObjModel::sliceObject()
{
	std::vector<Glib::RefPtr<Gdk::Pixbuf>> pix_vector;
	
	std::cout << "Creating pixbuf array..." << std::endl;
	
	for (int c = 0; c < dimensiones.z * 2; c++) {
		Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, dimensiones.x, dimensiones.y);
		pixbuf->fill(0x00000000);
		pix_vector.push_back(pixbuf);
	}
	
	std::cout << "Rasterizing mesh..." << std::endl;
	
	for (int s = 0; s < int(indexes.size()); s++) {
		vec3Di v_indices = indexes[s];
		vec3Di uv_indices = uv_indexes[s];
		vec3Di n_indices = n_indexes[s];
		
		vec3D point_color = normals[n_indices.x];
		
		vertex vertex_1 = {positions[v_indices.x], uvs[uv_indices.x]};
		vertex vertex_2 = {positions[v_indices.y], uvs[uv_indices.y]};
		vertex vertex_3 = {positions[v_indices.z], uvs[uv_indices.z]};
		
		if (vertex_1.positions.y > vertex_2.positions.y) std::swap(vertex_1, vertex_2);
		if (vertex_1.positions.y > vertex_3.positions.y) std::swap(vertex_1, vertex_3);
		if (vertex_2.positions.y > vertex_3.positions.y) std::swap(vertex_2, vertex_3);

		raster(vertex_1, vertex_2, vertex_3, pix_vector, point_color);
	}
	
	
	return pix_vector;
	
}

void ObjModel::raster(vertex v1, vertex v2, vertex v3, std::vector<Glib::RefPtr<Gdk::Pixbuf>> pix_vector, vec3D normal_color)
{
	// Entran los tres vertices 3d ordenados de menor a mayor z

	// Paso 1: proyectar los tres vertices en cada uno de los planos  ZX, ZY, XY
	
	if (v1.positions.y > v2.positions.y) std::swap(v1, v2);
	if (v1.positions.y > v3.positions.y) std::swap(v1, v3);
	if (v2.positions.y > v3.positions.y) std::swap(v2, v3);
	
	auto proyection_ZX = projectedTriangle(v1, v2, v3, ZX);
	auto proyection_ZY = projectedTriangle(v1, v2, v3, ZY);
	auto proyection_XY = projectedTriangle(v1, v2, v3, XY);
	
	proyection_ZX.getInclination(v1, v2, v3);
	
	// Si el plano tiene proyection regular en ZX, esto es, si la proyection del triangulo en zx no es una linea o un punto, procedemos 
	// a la rasterizacion normal del polígono
	
	float avanze_en_raster = 0.1;
	
	if (proyection_ZX.inclination_type == HORIZONTAL) {
		
		//std::cout << "Horizontal plano" << std::endl;
		
		int index_color_layer_map = int(v1.positions.y);
		int index_normal_layer_map = int(v1.positions.y) + dimensiones.z;
		
		// Si la proyection zx es horizontal, esto es, es una linea recta o un punto, solo nos servira el plano xy para la rasterizacion del 
		// poligono. Para ello ordenamos nuevamente los vertices de menor a mayor x de modo que el v1 es el primer vertice, a la izquierda, y 
		// el v3 el último vertice, a la derecha
		
		if (v1.positions.x > v2.positions.x) std::swap(v1, v2);
		if (v1.positions.x > v3.positions.x) std::swap(v1, v3);
		if (v2.positions.x > v3.positions.x) std::swap(v2, v3);
		
		// Si cambiamos la numeración del los vertices, debemos volver a proyectar el plano, para que las lineas a, b, y c coincidan con sus
		// respectivos vertices
		
		proyection_XY = projectedTriangle(v1, v2, v3, XY);
		
		// Una vez conseguido esto, pasamos a rasterizar el triangulo;
		
		for (float x = v1.positions.x; x < v3.positions.x; x += avanze_en_raster) {
			
			line raster_line = line ({x, 0}, {x, float(dimensiones.y)});
			std::vector<vec2D> intersection_vector;
			vec2D point_1_uv, point_2_uv;
			
			try {
				if (x < v2.positions.x) {
					intersection_vector = proyection_XY.raster_for_intersection_points(raster_line, AB);
					float distance_to_map = (v2.positions.x - v1.positions.x);
					float current_distance = x - v1.positions.x;
					
					point_1_uv = get_uv_point(v1.uvs, v2.uvs, distance_to_map, current_distance);
				}
				else {
					intersection_vector = proyection_XY.raster_for_intersection_points(raster_line, CB);
					float distance_to_map = (v3.positions.x - v2.positions.x);
					float current_distance = x - v2.positions.x;
					
					point_1_uv = get_uv_point(v2.uvs, v3.uvs, distance_to_map, current_distance);
					
				}
				
				float distance_to_map = (v3.positions.x - v1.positions.x);
				float current_distance = x - v1.positions.x;
				
				point_2_uv = get_uv_point(v1.uvs, v3.uvs, distance_to_map, current_distance);
				
				
				vec2D point_1 = intersection_vector[0];
				vec2D point_2 = intersection_vector[1];
				
				if (point_1.y > point_2.y) {
					std::swap(point_1, point_2);
					std::swap(point_1_uv, point_2_uv);
				}
				
				distance_to_map = point_2.y - point_1.y;
				
				for (float y = point_1.y; y < point_2.y; y += avanze_en_raster) {
					current_distance = y - point_1.y;
					auto current_uv = get_uv_point(point_1_uv, point_2_uv, distance_to_map, current_distance);
					int x = point_1.x;
					auto texture_color = get_uv_color_from_texture(current_uv.x, current_uv.y, texture);
					
					drawPixelInLayer(x, int(y), pix_vector[index_color_layer_map], texture_color, true);
					drawPixelInLayer(x, int(y), pix_vector[index_normal_layer_map], normal_color, false);
				}
				
			}
			
			catch (const std::exception & e) {
				std::cout << "Problemas en rasterizado en plano ZY de proyeccion horizontal" << e.what() << std::endl;
			}
			
		}
		
	}

	else {
		
		for (float z = v1.positions.y; z < v3.positions.y; z+= avanze_en_raster) { 	// para cada capa hallamos los puntos de corte en la proyection zx
		
			int index_color_layer_map = int(z);
			int index_normal_layer_map = int(z) + dimensiones.z;
			
			line raster_line = line( {0, float(z)}, {float(dimensiones.x), float(z)}); 				// trazando una linea de corte
			
			std::vector<vec2D> intersection_vector_ZX, intersection_vector_ZY; 
			
			// Si la capa z es menor que la z del v2, el raster se aplica al los segmentos AB, en caso contrario, se aplica a los segmentos CB del plano zx
			line raster_line_1, raster_line_2;
			vec2D point_1, point_2;
			vec2D point_1_uv, point_2_uv; 
			int distance_to_map1, distance_to_map2;
			int current_distance = z - v1.positions.y;
			
			if (z < v2.positions.y) {
				
				intersection_vector_ZX = proyection_ZX.raster_for_intersection_points(raster_line, AB);
				intersection_vector_ZY = proyection_ZY.raster_for_intersection_points(raster_line, AB);
				
				point_1 = {intersection_vector_ZX[0].x, intersection_vector_ZY[0].x};
				point_2 = {intersection_vector_ZX[1].x, intersection_vector_ZY[1].x};
				
				if (proyection_ZX.inclination_type == VERTICAL) {
					// Obtenemos las distancias en el plano ZY
					
					distance_to_map1 = abs(v2.positions.y - v1.positions.y);
					distance_to_map2 = abs(v3.positions.y - v1.positions.y);
				}
				else {
					// Obtenemos las distancias en el plano ZX
					
					distance_to_map1 = abs(v2.positions.y - v1.positions.y);
					distance_to_map2 = abs(v3.positions.y - v1.positions.y);
				}
				
				point_1_uv = get_uv_point(v1.uvs, v2.uvs, distance_to_map1, current_distance);
				point_2_uv = get_uv_point(v1.uvs, v3.uvs, distance_to_map2, current_distance);
				
			}
			else {
				
				intersection_vector_ZX = proyection_ZX.raster_for_intersection_points(raster_line, CB);
				intersection_vector_ZY = proyection_ZY.raster_for_intersection_points(raster_line, CB);
				
				point_1 = {intersection_vector_ZX[0].x, intersection_vector_ZY[0].x};
				point_2 = {intersection_vector_ZX[1].x, intersection_vector_ZY[1].x};
				
				
				if (proyection_ZX.inclination_type == VERTICAL) {
					// Obtenemos las distancias en el plano ZY
					
					distance_to_map1 = abs(v3.positions.y - v2.positions.y);
					distance_to_map2 = abs(v3.positions.y - v1.positions.y);
				}
				else {
					// Obtenemos las distancias en el plano ZX
					
					distance_to_map1 = abs(v3.positions.y - v2.positions.y);
					distance_to_map2 = abs(v3.positions.y - v1.positions.y);
				}
				
				point_1_uv = get_uv_point(v2.uvs, v3.uvs, distance_to_map1, current_distance);
				point_2_uv = get_uv_point(v1.uvs, v3.uvs, distance_to_map2, current_distance);
			}
			
			// Calculamos las uvs de los puntos de corte con el plano ZY si la proyeccion en ZX es una linea vertical
			
			// ESTA PARTE ESTA SUJETA A CAMBIOS BUSCANDO MEJORAR LA EFICIENCIA Y SU REUTILIZACIÓN... 
			
			int dx = abs(point_1.x - point_2.x);
			int dy = abs(point_1.y - point_2.y);
			
			if (dx > dy) {
				if (point_1.x > point_2.x){
					std::swap(point_1, point_2);
					std::swap(point_1_uv, point_2_uv);
				}
				
				line final_line = line (point_1, point_2);
				
				for (float x = final_line.v1.x; x < final_line.v2.x; x++) {
					try {
						int y = final_line.getY(x);
						
						int distance_to_map = abs(point_2.x - point_1.x);
						int current_distance = x - final_line.v1.x;
						
						auto current_uv = get_uv_point(point_1_uv, point_2_uv, distance_to_map, current_distance);
						auto texture_color = get_uv_color_from_texture(current_uv.x, current_uv.y, texture);
						
						drawPixelInLayer(int(x), y, pix_vector[index_color_layer_map], texture_color, true);
						drawPixelInLayer(int(x), y, pix_vector[index_normal_layer_map], normal_color, false);
					}
					
					catch (const std::exception &exc) {
						std::cout << "Pixel sin dibujar en: (" << x << ", indef) debido a: " << exc.what() << std::endl;
					}
				}
			}
			else {
				if (point_1.y > point_2.y) {
					std::swap(point_1, point_2);
					std::swap(point_1_uv, point_2_uv);
				}
				
				line final_line = line (point_1, point_2);
				
				for (float y = final_line.v1.y; y < final_line.v2.y; y++) {
					try {
						int x = final_line.getX(y);
						
						int distance_to_map = abs(point_2.y - point_1.y);
						int current_distance = y - final_line.v1.y;
						
						auto current_uv = get_uv_point(point_1_uv, point_2_uv, distance_to_map, current_distance);
						auto texture_color = get_uv_color_from_texture(current_uv.x, current_uv.y, texture);
						
						drawPixelInLayer(x, int(y), pix_vector[index_color_layer_map], texture_color, true);
						drawPixelInLayer(x, int(y), pix_vector[index_normal_layer_map], normal_color, false);
						
					}
					catch (const std::exception &exc) {
						std::cout << "Pixel sin dibujar en: (indef, " << y << ") debido a: " << exc.what() << std::endl;
					}
				}
			}
			
		}
		
	} 
}

void ObjModel::drawPixelInLayer(int x, int y, Glib::RefPtr<Gdk::Pixbuf> pixbuf, vec3D color, bool is_color)
{
	guint red_index = y * pixbuf->get_rowstride() + x * pixbuf->get_n_channels();
	guint8 *pixels = pixbuf->get_pixels();
	
	if (pixels[red_index + 3] == 0) {
		
		if (is_color) {
			// Transformaciones para los colores (float, x, float y, float z ) < 1.0
			
			pixels[red_index] = guint(int(color.x * 255.0));
			pixels[red_index + 1] = guint(int(color.y * 255.0));
			pixels[red_index + 2] = guint(int(color.z * 255.0));
			pixels[red_index + 3] = 255;
		}
		else {
			// Transformaciones para las normales (float, x, float y, float z ) < 1.0 && > -1.0
			
			pixels[red_index] = guint(int(127.0 + color.x * 127.0));
			pixels[red_index + 1] = guint(int(127.0 + color.z * 127.0)); // Transformamos las normales del formato (x, y, z) donde y representa la altura
			pixels[red_index + 2] = guint(int(127.0 + color.y * 127.0)); // al formato (x, y, z) donde z (el verde) es la que representa la altura
			pixels[red_index + 3] = 255;
		}
	}
	
}

vec3D ObjModel::get_uv_color_from_texture(float u, float v, Glib::RefPtr<Gdk::Pixbuf> texture_pixbuf)
{
	int x = (texture_pixbuf->get_width() - 1) * u;
	int y = (texture_pixbuf->get_height() - 1) * v;
	
	
	if ((x >= 0 && x < texture_pixbuf->get_width()) && (y >= 0 && y < texture_pixbuf->get_height())) {
		guint red_index = y * texture_pixbuf->get_rowstride() + x * texture_pixbuf->get_n_channels();
		guint8 *pixels = texture_pixbuf->get_pixels();
		
		vec3D color = {float(pixels[red_index]) / float(255), float(pixels[red_index + 1]) / float(255), float(pixels[red_index + 2]) / float(255)};
		return color;
		
	}
	
	else return {0.0, 0.0, 0.0};
}

vec2D ObjModel::get_uv_point(vec2D init_point_uv, vec2D end_point_uv,  float distance_to_map, float current_distance)
{
	float fraction_distance;

	if (distance_to_map != 0.0) fraction_distance = current_distance / distance_to_map;
	else fraction_distance = 0.0;
	
	float du = end_point_uv.x - init_point_uv.x;
	float dv = end_point_uv.y - init_point_uv.y;
	
	float avance_u = du * fraction_distance;
	float avance_v = dv * fraction_distance;
	
	return {init_point_uv.x + avance_u, init_point_uv.y + avance_v};
}

void ObjModel::parse_to_maps(std::vector<Glib::RefPtr<Gdk::Pixbuf>> pixbuf_v) 
{
	std::cout << "parsing pixbuf to maps...." << std::endl;
	
	int limits[] = {512, 1024, 2048, 4096, 8192};
	
	int counter = 0;
	while ((dimensiones.x * dimensiones.y * dimensiones.z) > (limits[counter] * limits[counter])) counter++;
	
	if (counter < 5) {
		
		try {
			
			int map_limit_size = limits[counter];
		
			int margin_width = map_limit_size % dimensiones.x;
			int margin_height = map_limit_size % dimensiones.y;
			
			int map_width = map_limit_size - margin_width;
			int map_height = map_limit_size - margin_height;
			
			int total_columns = map_limit_size / dimensiones.x;
			int total_rows = map_limit_size / dimensiones.y;
			
			
			
			Cairo::RefPtr<Cairo::ImageSurface> color_map_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, map_limit_size, map_limit_size);
			Cairo::RefPtr<Cairo::ImageSurface> normal_map_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, map_limit_size, map_limit_size);
			
			Cairo::RefPtr<Cairo::Context> cr_c = Cairo::Context::create(color_map_surface);
			Cairo::RefPtr<Cairo::Context> cr_n = Cairo::Context::create(normal_map_surface);
			
			cr_c->set_source_rgba(0.0, 0.0, 0.0, 0.0);
			cr_c->paint();
			
			cr_n->set_source_rgba(0.0, 0.0, 0.0, 0.0);
			cr_n->paint();
			
			for (int row = 0; row < total_rows; row++) {
				for (int column = 0; column < total_columns; column++) {
					int pix_index = row * total_columns + column;
					
					if (pix_index < int(pixbuf_v.size() / 2)) {
						cr_c->save();
						cr_n->save();
						
						cr_c->translate(dimensiones.x * column, dimensiones.y * row);
						cr_n->translate(dimensiones.x * column, dimensiones.y * row);

						Gdk::Cairo::set_source_pixbuf(cr_c, pixbuf_v[pix_index], 0, 0);
						Gdk::Cairo::set_source_pixbuf(cr_n, pixbuf_v[dimensiones.z + pix_index], 0, 0);
						
						cr_c->paint();
						cr_n->paint();
						
						cr_c->restore();
						cr_n->restore();
						
					}
					else break;
				}
			}
			
			color_map = Gdk::Pixbuf::create(color_map_surface, 0, 0, color_map_surface->get_width(), color_map_surface->get_height());
			normal_map = Gdk::Pixbuf::create(normal_map_surface, 0, 0, normal_map_surface->get_width(), normal_map_surface->get_height());
			
			fill_volume();

			//engrosar_laminado();
			//engrosar_laminado();
			
			//color_map->save("/home/usuario/Prácticas_Programación/sliced_color_map.png", "png");
			//normal_map->save("/home/usuario/Prácticas_Programación/sliced_normal_map.png", "png");
			
			
		}
		
		catch (const Glib::Error &e) {
			std::cout << "creating sliced texture failed due to: " << e.what() << std::endl;
		}
	}
	else std::cout << "Las dimensiones de textura exceden las dimensiones propuestas" << std::endl;
	
	
	
}

void ObjModel::set_sliced_geometry() // Se ejecuta una vez se han conseguido los color map y normal map
{
	int indices[] = {0, 1, 2, 2, 3, 0};
	std::vector<vec3D> plane_geometry = {
		
		{0., 0., 0.},
		{float(dimensiones.x), 0., 0.},
		{float(dimensiones.x), float(dimensiones.y), 0.},
		{0., float(dimensiones.y), 0.}
		
	};
	
	// Revisamos si la textura tiene márgenes, con el fin de obtener el area que ocupan los mapas dentro de sus respectivas texturas
	
	int map_width;
	int margin_w = color_map->get_width() % dimensiones.x;
	
	map_width = color_map->get_width() - margin_w;
	
	float uv_width = float(dimensiones.x) / float(color_map->get_width());
	float uv_height = float(dimensiones.y) / float(color_map->get_height()); 
	
	float fraction_u = (uv_width / float(dimensiones.x)) / 2;
	float fraction_v = (uv_height / float(dimensiones.y)) / 2;
	
	std::vector<vec2D> plane_uvs = {
		
		{0.0 + fraction_u, 0.0 + fraction_v},
		{uv_width - fraction_u, 0.0 + fraction_v},
		{uv_width - fraction_u, uv_height - fraction_v},
		{0.0 + fraction_u, uv_height - fraction_v}
	
	};
	
	int total_columns = (map_width / dimensiones.x);
	
	int texture_size;
	
	std::cout << texture_size << std::endl;
	
	for (int z = 0; z < dimensiones.z; z++) {
		
		int row = z / total_columns;
		int column = z % total_columns;
		
		for (auto i : indices) {
			vec3D vertice = {plane_geometry[i]};
			vertice.z = z;
			sliced_positions.push_back(vertice);
			
			vec2D vert_uv = {plane_uvs[i]};
			vert_uv.x += column * uv_width;
			vert_uv.y += row * uv_height;
			
			sliced_uvs.push_back(vert_uv);
		}
		
		for (int i = 0; i < 2; i++) {
			int index = int(sliced_indexes.size()) * 3;
			sliced_indexes.push_back({index, index + 1, index + 2});
		}
	}
	
	has_been_sliced = true;

}


std::vector<int> ObjModel::get_color_pixel_from_pixbuff(int x, int y, Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
	guint red_index = y * pixbuf->get_rowstride() + x * pixbuf->get_n_channels();
	guint8 *pixels = pixbuf->get_pixels();
	
	return {int(pixels[red_index]), int(pixels[red_index + 1]), int(pixels[red_index + 2]), int(pixels[red_index + 3])};
}

void ObjModel::engrosar_laminado() 
{
	Glib::RefPtr<Gdk::Pixbuf> fresh_nmap = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, normal_map->get_width(), normal_map->get_height());
	Glib::RefPtr<Gdk::Pixbuf> fresh_cmap = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, color_map->get_width(), color_map->get_height());
	fresh_nmap->fill(0x00000000);
	fresh_cmap->fill(0x00000000);
	
	for (int y = 0; y < normal_map->get_height(); y++) {
		for (int x = 0; x < normal_map->get_width(); x++) {
			auto v_color = get_color_pixel_from_pixbuff(x, y, normal_map);
			
			if (v_color[3] != 0) {
				vec3Di n_color = {v_color[0], v_color[1], v_color[2]};
				
				float red = float(n_color.x - 127) / float(127);
				float green = float(n_color.y - 127) / float(127);
				float blue = float(n_color.z - 127) / float(127);

				drawPixelInLayer(x, y, fresh_nmap, {red, blue, green}, false);
				
				vec3D inverted_normal = {float(red * -1.0), float(green * -1.0), 0.0};
				
				auto color = get_color_pixel_from_pixbuff(x, y, color_map);
				vec3D c_color = {float(color[0]) / float(255), float(color[1]) / float(255), float(color[2]) / float(255)};
				
				drawPixelInLayer(x, y, fresh_cmap, c_color, true);

				
				if (blue > -0.9 && blue < 0.9) {
					if (inverted_normal.x <= 0.0 && inverted_normal.y <= 0.0) {
						if (abs(inverted_normal.x) == abs(inverted_normal.y)) {
							drawPixelInLayer(x - 1, y - 1, fresh_nmap, {red, blue, green}, false);
							drawPixelInLayer(x - 1, y - 1, fresh_cmap, c_color, true);
						}
						else {
							if (abs(inverted_normal.x) > abs(inverted_normal.y)) {
								drawPixelInLayer(x - 1, y, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x - 1, y, fresh_cmap, c_color, true);
							}
							else {
								drawPixelInLayer(x, y - 1, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x, y - 1, fresh_cmap, c_color, true);
							}
							
						}
					}
					
					else if (inverted_normal.x <= 0.0 && inverted_normal.y >= 0.0) {
						if (abs(inverted_normal.x) == abs(inverted_normal.y)) {
							drawPixelInLayer(x - 1, y + 1, fresh_nmap, {red, blue, green}, false);
							drawPixelInLayer(x - 1, y + 1, fresh_cmap, c_color, true);
						}
						else {
							if (abs(inverted_normal.x) > abs(inverted_normal.y)) {
								drawPixelInLayer(x - 1, y, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x - 1, y, fresh_cmap, c_color, true);
							}
							else {
								drawPixelInLayer(x, y + 1, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x, y + 1, fresh_cmap, c_color, true);
							}
							
						}
					}
					
					else if (inverted_normal.x >= 0.0 && inverted_normal.y <= 0.0) {
						if (abs(inverted_normal.x) == abs(inverted_normal.y)) {
							drawPixelInLayer(x + 1, y - 1, fresh_nmap, {red, blue, green}, false);
							drawPixelInLayer(x + 1, y - 1, fresh_cmap, c_color, true);
						}
						else {
							if (abs(inverted_normal.x) > abs(inverted_normal.y)) {
								drawPixelInLayer(x + 1, y, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x + 1, y, fresh_cmap, c_color, true);
							}
							else {
								drawPixelInLayer(x, y - 1, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x, y - 1, fresh_cmap, c_color, true);
							}
							
						}
					}
					
					else if (inverted_normal.x >= 0.0 && inverted_normal.y >= 0.0) {
						if (abs(inverted_normal.x) == abs(inverted_normal.y)) {
							drawPixelInLayer(x + 1, y + 1, fresh_nmap, {red, blue, green}, false);
							drawPixelInLayer(x + 1, y + 1, fresh_cmap, c_color, true);
						}
						else {
							if (abs(inverted_normal.x) > abs(inverted_normal.y)) {
								drawPixelInLayer(x + 1, y, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x + 1, y, fresh_cmap, c_color, true);
							}
							else {
								drawPixelInLayer(x, y + 1, fresh_nmap, {red, blue, green}, false);
								drawPixelInLayer(x, y + 1, fresh_cmap, c_color, true);
							}
							
						}
					}
					
					
				}
				
			}
		}
	}
	
	normal_map = fresh_nmap;
	color_map = fresh_cmap;
}

void ObjModel::fill_volume()
{
	int pixbuf_width = normal_map->get_width();
	//int pixbuf_height = normal_map->get_height();
	
	int columns = pixbuf_width / dimensiones.x;
	//int rows = pixbuf_height / dimensiones.y;
	
	Cairo::RefPtr<Cairo::ImageSurface> target_normal_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, normal_map->get_width(), normal_map->get_height());
	Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(target_normal_surface);
	
	cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
	cr->paint();
	
	Cairo::RefPtr<Cairo::ImageSurface> target_color_surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, color_map->get_width(), color_map->get_height());
	Cairo::RefPtr<Cairo::Context> cr_c = Cairo::Context::create(target_color_surface);
	
	cr_c->set_source_rgba(0.0, 0.0, 0.0, 0.0);
	cr_c->paint();
	
	
	for (int z = 0; z < dimensiones.z; z++) {
		int x_region = (z % columns) * dimensiones.x;
		int y_region = (z / columns) * dimensiones.y;

		Glib::RefPtr<Gdk::Pixbuf> sub_n_pixbuf = Gdk::Pixbuf::create_subpixbuf(normal_map, x_region, y_region, dimensiones.x, dimensiones.y);
		Glib::RefPtr<Gdk::Pixbuf> sub_c_pixbuf = Gdk::Pixbuf::create_subpixbuf(color_map, x_region, y_region, dimensiones.x, dimensiones.y);
		
		
		std::vector<vec2Di> hits_track;
		std::vector<bool> normals_track;
		
		Glib::RefPtr<Gdk::Pixbuf> target_n_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, sub_n_pixbuf->get_width(), sub_n_pixbuf->get_height());
		target_n_pixbuf->fill(0x00000000);
		
		Glib::RefPtr<Gdk::Pixbuf> target_c_pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, sub_c_pixbuf->get_width(), sub_c_pixbuf->get_height());
		target_c_pixbuf->fill(0x00000000);
		
		
		
		for (int x = 0; x < sub_n_pixbuf->get_width(); x++) {
			for (int y = 0; y < sub_n_pixbuf->get_height(); y++) {
				auto color_at_pixel = get_color_pixel_from_pixbuff(x, y, sub_n_pixbuf);
				
				if (color_at_pixel[3] != 0) {
					
					hits_track.push_back({x, y});
					
					if (color_at_pixel[1] < 127) normals_track.push_back(true);
					else normals_track.push_back(false);
					
				}
			}
		}
		
		for (int i = 0; i < int(hits_track.size()) - 2; i ++) {
			vec2Di current_hit = hits_track[i];
			vec2Di next_hit = hits_track[i + 1];
			
			std::vector<int> current_n_color = get_color_pixel_from_pixbuff(current_hit.x, current_hit.y, sub_n_pixbuf);
			std::vector<int> next_n_color = get_color_pixel_from_pixbuff(next_hit.x, next_hit.y, sub_n_pixbuf);
			
			std::vector<int> current_c_color = get_color_pixel_from_pixbuff(current_hit.x, current_hit.y, sub_c_pixbuf);
			std::vector<int> next_c_color = get_color_pixel_from_pixbuff(next_hit.x, next_hit.y, sub_c_pixbuf);
			
			bool current_normal = normals_track[i];
			bool next_normal = normals_track[i + 1];
			
			if (current_normal == next_normal) {
				continue;
			}
			
			else if (current_normal == true) {
				for (int y = current_hit.y + 1; y < next_hit.y; y++) {
					std::vector<vec2Di> hits;
					for (vec2Di hit : hits_track) {
						if (hit.y == y) hits.push_back(hit);
					}
					
					std::vector<int> final_n_color = current_n_color;
					std::vector<int> final_c_color = current_c_color;
					
					
					
					if (hits.size() == 2) {
						
						vec2Di hit_left;
						vec2Di hit_right;
						
						hit_left = hits[0];
						hit_right = hits[1];
						
						if (hit_left.x > hit_right.x) std::swap(hit_left, hit_right);
						
						std::vector<int> left_n_color = get_color_pixel_from_pixbuff(hit_left.x, hit_left.y, sub_n_pixbuf);
						std::vector<int> rigth_n_color = get_color_pixel_from_pixbuff(hit_right.x, hit_right.y, sub_n_pixbuf);
						
						std::vector<int> left_c_color = get_color_pixel_from_pixbuff(hit_left.x, hit_left.y, sub_c_pixbuf);
						std::vector<int> rigth_c_color = get_color_pixel_from_pixbuff(hit_right.x, hit_right.y, sub_c_pixbuf);
						
						std::vector<int> distances = {abs(hit_left.x - current_hit.x), abs(next_hit.y - y), abs(hit_right.x - current_hit.x), abs(y - current_hit.y)}; 
						
						int min_index = 0;
						int minimo = distances[0];
						for (int i = 0; i < int(distances.size()); i++) {
							
							if (distances[i] < minimo) {
								minimo = distances[i];
								min_index = i;
							}
						}
						
						switch (min_index) {
							case 0:
								final_n_color = left_n_color;
								final_c_color = left_c_color;
								break;
							case 1:
								final_n_color = next_n_color;
								final_c_color = next_c_color;
								break;
								
							case 2:
								final_n_color = rigth_n_color;
								final_c_color = rigth_c_color;
								break;
								
							case 3:
								final_n_color = current_n_color;
								final_c_color = current_c_color;
								break;
								
						}
						
					}
					
					else {
						if (abs(next_hit.y - y) < abs(current_hit.y - y)) {
							final_n_color = next_n_color;
							final_c_color = next_c_color;
							
						}
					}

					draw_pixel_at(current_hit.x, y, target_n_pixbuf, final_n_color);
					draw_pixel_at(current_hit.x, y, target_c_pixbuf, final_c_color);
				}
			}

			
		}
		
		cr->save();
		cr->translate(x_region, y_region);
		Gdk::Cairo::set_source_pixbuf(cr, target_n_pixbuf, 0, 0);
		cr->paint();
		Gdk::Cairo::set_source_pixbuf(cr, sub_n_pixbuf, 0, 0);
		cr->paint();
		cr->restore();
		
		cr_c->save();
		cr_c->translate(x_region, y_region);
		Gdk::Cairo::set_source_pixbuf(cr_c, target_c_pixbuf, 0, 0);
		cr_c->paint();
		Gdk::Cairo::set_source_pixbuf(cr_c, sub_c_pixbuf, 0, 0);
		cr_c->paint();
		cr_c->restore();
		
	}
	
	normal_map = Gdk::Pixbuf::create(target_normal_surface, 0, 0, target_normal_surface->get_width(), target_normal_surface->get_height());
	//normal_map->save("/home/usuario/Prácticas_Programación/fill_test_normal_map.png", "png");
	
	color_map = Gdk::Pixbuf::create(target_color_surface, 0, 0, target_color_surface->get_width(), target_color_surface->get_height());
	//color_map->save("/home/usuario/Prácticas_Programación/fill_test_color_map.png", "png");
	
	
}

void ObjModel::draw_pixel_at(int x, int y, Glib::RefPtr<Gdk::Pixbuf> target_pixbuf, std::vector<int> color)
{
	
	guint red_index = y * target_pixbuf->get_rowstride() + x * target_pixbuf->get_n_channels();
	guint8 *pixels = target_pixbuf->get_pixels();
	
	if (pixels[red_index + 3] == 0) {
		for (int i = 0; i < 4; i++) {
			pixels[red_index + i] = color[i];
		}
	}
	
}