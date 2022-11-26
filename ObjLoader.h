#ifndef OBJECTLOADER_H
#define OBJECTLOADER_H

#include "headers.h"
#include "geometry.h"

class ObjModel {
public:
	
	// Atributos de Objeto Wavefront
	
	std::vector<vec3D> positions, normals;
	std::vector<vec2D> uvs;
	std::vector<vec3Di> indexes;
	std::vector<vec3Di> uv_indexes;
	std::vector<vec3Di> n_indexes;
	
	// Atributos de apartado gráfico
	
	GLuint gen_textures, gen_textures1, gen_textures2;
	Glib::RefPtr<Gdk::Pixbuf> texture;
	
	// Atributos de referencia local
	
	vec3Di dimensiones = { 0, 0, 0 };
	vec3Di min_dimensions = { 0, 0, 0 };
	vec3D origin = { 0.0, 0.0, 0.0 };
	
	//Atributos de sliced object
	
	std::vector<vec3D> sliced_positions;
	std::vector<vec2D> sliced_uvs;
	std::vector<vec3Di> sliced_indexes;
	bool has_been_sliced = false;
	
	Glib::RefPtr<Gdk::Pixbuf> normal_map;
	Glib::RefPtr<Gdk::Pixbuf> color_map;
	
	// Métodos de la clase
	
	ObjModel();
	virtual ~ObjModel();
	
	// Metodos de carga de objeto
	
	bool startsWith(std::string& line, const char* text);
	void loadFromFile(const char* filename);
	void loadTextureFromFile(const char* filename);
	void set_dimensions_from_load();
	
	// Metodos de geometría de objeto
	
	std::vector<float> getVertexPositions();
	std::vector<float> getVertexNormals();
	std::vector<float> getVertexTextCoords();
	std::vector<int> getIndices();
	std::vector<int> getUVIndices();
	
	// Metodos de apartado gráfico
	
	void bind_textures();
	
	// Metodos de laminado
	
	std::vector<Glib::RefPtr<Gdk::Pixbuf>> sliceObject();
	void raster(vertex v1, vertex v2, vertex v3, std::vector<Glib::RefPtr<Gdk::Pixbuf>> pix_vector, vec3D normal_color);
	void drawPixelInLayer(int x, int y, Glib::RefPtr<Gdk::Pixbuf> pixbuf, vec3D color, bool is_color);
	vec2D get_uv_point(vec2D p_init_uv, vec2D p_end_uv, float distance_to_map, float current_distance);
	vec3D get_uv_color_from_texture(float u, float v, Glib::RefPtr<Gdk::Pixbuf> texture_pixbuf); 
	void parse_to_maps(std::vector<Glib::RefPtr<Gdk::Pixbuf>> pixbuf_v);
	bool check_size_for_map(int size_suggest);
	void engrosar_laminado();
	void fill_volume();
	
	std::vector<int> get_color_pixel_from_pixbuff(int x, int y, Glib::RefPtr<Gdk::Pixbuf> pixbuf);
	void draw_pixel_at(int x, int y, Glib::RefPtr<Gdk::Pixbuf> target_pixbuff, std::vector<int> color);
	
	// Metodos de geometria de sliced object
	
	void set_sliced_geometry();
	
	// 
};

#endif