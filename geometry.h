#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "headers.h"


enum LineType {HORIZONTAL, VERTICAL, REGULAR, UNDEFINED};
enum ProjectionType {ZX, ZY, XY};
enum RasterArea {AB, CB};;



struct vec2D { float x, y; };

struct vec2Di{ int x, y; };

struct vec3D { float x, y, z; };

struct vec3Di { int x, y, z; };

struct vertex {
	vec3D positions;
	vec2D uvs;
	void cout() {
		std::printf("Vertice: %f, %f, %f\n", positions.x, positions.y, positions.z);
	};
};

struct line {
	float m;
	float c;
	vec2D v1, v2;
	LineType line_type;
	
	line(){};
	line(vec2D v_1, vec2D v_2) {
		v1 = v_1;
		v2 = v_2;
		getEcuation();
	};
	
	void getEcuation() {
		
		float factor_y = abs(v2.y - v1.y);
		float factor_x = abs(v2.x - v1.x);
		
		if (factor_y < 0.01 || factor_x < 0.01)
		{
			if (factor_y < 0.01) {
				line_type = HORIZONTAL;
				m = 0;
				c = v1.y;
			}
			else if (factor_x < 0.01) {
				line_type = VERTICAL;
				m = 1;
				c = v1.x;
			}
		}
		else {
			m = (v2.y - v1.y) / (v2.x - v1.x);
			c = v1.y - m * v1.x;
			line_type = REGULAR;
		}
	};
	
	float getX(float y_coord) {
		if (line_type == HORIZONTAL) return v1.x;
		if (line_type == VERTICAL) return c;
		else return (y_coord - c) / m;
	};
	
	float getY(float x_coord) {
		if (line_type == VERTICAL) return v1.y;
		else return (m * x_coord + c);
	};
	
	vec2D getInterseccion(line linea) {
		vec2D intersectionPoint;
		
//		std::cout << "Getting intersection for line.. " << std::endl;
//		print(); std::cout << line_type;
//		std::cout << "------------" << std::endl;

		if (line_type == linea.line_type) {
			if (line_type == REGULAR) {
				if (m == linea.m) throw std::invalid_argument("Ambas lineas son paralelas Regulares de igual pendiente: imposible hallar un punto de intersección\n");
			}
			else {
				intersectionPoint = v1;
			}
		}
		else {
			if (line_type == VERTICAL) {
				intersectionPoint.x = c;
				if (linea.line_type == REGULAR) intersectionPoint.y = linea.m * intersectionPoint.x + linea.c;
				else intersectionPoint.y = linea.c;
			}
			else if (line_type == HORIZONTAL) {
				intersectionPoint.y = c;
				if (linea.line_type == REGULAR) intersectionPoint.x = (intersectionPoint.y - linea.c) / linea.m;
				else intersectionPoint.x = linea.c;
			}
			else {
				
				if (linea.line_type == VERTICAL) {
					intersectionPoint.x = linea.c; 
					intersectionPoint.y = intersectionPoint.x * m + c;
				}
				else if (linea.line_type == HORIZONTAL) {
					intersectionPoint.y = linea.c;
					intersectionPoint.x = (intersectionPoint.y - c) / m;
				}
				else {
					intersectionPoint.x = (linea.c - c) / (m - linea.m);
					intersectionPoint.y = intersectionPoint.x * m + c;
				}
				
			}
		}
		
		return intersectionPoint;
	};
	
	void print() {
		std::printf("Linea: (%f, %f)(%f, %f)\n", v1.x, v1.y, v2.x, v2.y);
	};
};

struct projectedTriangle {
	line line_a, line_b, line_c;
	ProjectionType projection_type;
	LineType inclination_type;
	
	
	projectedTriangle(vertex v1, vertex v2, vertex v3, ProjectionType pType) {
		projection_type = pType;
		
		if (pType == ZX) {
			line_a = line ({v1.positions.x, v1.positions.y}, {v2.positions.x, v2.positions.y});
			line_b = line ({v1.positions.x, v1.positions.y}, {v3.positions.x, v3.positions.y});
			line_c = line ({v2.positions.x, v2.positions.y}, {v3.positions.x, v3.positions.y});

		}
		
		else if (pType == ZY) {
			line_a = line ({v1.positions.z, v1.positions.y}, {v2.positions.z, v2.positions.y});
			line_b = line ({v1.positions.z, v1.positions.y}, {v3.positions.z, v3.positions.y});
			line_c = line ({v2.positions.z, v2.positions.y}, {v3.positions.z, v3.positions.y});
			
		}
		
		else {
			line_a = line ({v1.positions.x, v1.positions.z}, {v2.positions.x, v2.positions.z});
			line_b = line ({v1.positions.x, v1.positions.z}, {v3.positions.x, v3.positions.z});
			line_c = line ({v2.positions.x, v2.positions.z}, {v3.positions.x, v3.positions.z});

		}
	};
	
	std::vector<vec2D> raster_for_intersection_points(line raster, RasterArea raster_area) {
		
		vec2D point_1, point_2; // El point_1 corresponde o bien al segmento A o bien al segmento C, dependiendo del RasterArea;
								// en cambio, el point_b corresponde al segmento b del triángulo;
		
//		std::cout << "---------- " << std::endl;
//		std::cout << "Raster line: " << std::endl; 
//		raster.print();
//		std::cout << "---------- " << std::endl;
		
		std::vector<vec2D> intersection_vector;
		
		if (raster_area == AB) {
//			std::cout << "Seccion AB:" << std::endl;
//			std::cout << "-------------------------" << std::endl;
//			std::cout << "Linea A: ";
//			line_a.print();
//			std::cout << "Linea B: ";
//			line_b.print();
			
			point_1 = line_a.getInterseccion(raster);
			point_2 = line_b.getInterseccion(raster);
		}
		else if (raster_area == CB) {
//			std::cout << "Seccion CB:" << std::endl;
//			std::cout << "-------------------------" << std::endl;
//			std::cout << "Linea C: ";
//			line_c.print();
//			std::cout << "Linea B: ";
//			line_b.print();
			point_1 = line_c.getInterseccion(raster);
			point_2 = line_b.getInterseccion(raster);
		}
		
		intersection_vector.push_back(point_1);
		intersection_vector.push_back(point_2);
		
		return intersection_vector;
	};
	
	void getInclination(vertex v1, vertex v2, vertex v3) {
		if (projection_type == ZX) {
			
			bool horizontal_condition = (abs(v1.positions.y - v2.positions.y) < 1.0) && (abs(v1.positions.y - v3.positions.y) < 1.0);
			bool vertical_condition = int(v1.positions.x) == int(v2.positions.x) && int(v1.positions.x) == int(v3.positions.x); 
			
			if (horizontal_condition && vertical_condition) inclination_type = UNDEFINED;
			else if (horizontal_condition) 	inclination_type = HORIZONTAL;
			else if (vertical_condition) 	inclination_type = VERTICAL;
			else inclination_type = REGULAR;
		}
		
		else inclination_type = UNDEFINED;
				
	};
	
};


#endif