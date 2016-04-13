#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <mutex>
#include "Maths.h"
#include "SDL2\SDL.h"

/*! \file Shape.h
\brief A Documented file.

Details.
*/

namespace Patchwork
{
	/*! 
	Function to transform a floating point number into a std::string
	The float will have 2 decimals precision
	*/
	std::string to_string(float f)
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << f;
		return stream.str();
	}

	/*!
	Function to transform a integer into a std::string
	*/
	std::string to_string(int i)
	{
		std::stringstream stream;
		stream << i;
		return stream.str();
	}

	/*!
	A structure for a bounding box object defined by two points :
	upper left and lower right corners
	*/
	struct BoundingBox
	{
		int x_max; /*!< Lower corner x coordinate */
		int x_min; /*!< Upper corner x coordinate */
		int y_max; /*!< Lower corner y coordinate */
		int y_min; /*!< Upper corner y coordinate*/
		BoundingBox() :x_max(0), y_max(0), x_min(10000), y_min(10000){}
	};

	class Transformable
	{
		virtual float area() const = 0;
		virtual float perimeter() const = 0;
		virtual void translate(const Vec2& t) = 0;
		virtual void homothety(float ratio) = 0;
		virtual void homothety(const Vec2& s, float ratio) = 0;
		virtual void rotate(float angle) = 0;
		virtual void rotate(const Vec2& p, double angle) = 0;
		virtual void centralSym(const Vec2& c) = 0;
		virtual void axialSym(const Vec2& p, const Vec2& d) = 0;
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Shape
	{
	public:
		enum Derivedtype { CIRCLE=0, POLYGON, LINE, ELLIPSE, END_ENUM };
		enum Functions { ROTATION = 0, HOMOTHETY, TRANSLATE, AXIAL_SYMETRY, CENTRAL_SYMETRY, UNKNOWN };
		static const std::vector<std::string> transforms;
		static const std::vector<std::string> shapes;

		Shape(Derivedtype type, Color color) : m_type(type), m_color(color){};
		~Shape(){};

		Derivedtype type() const { return(m_type); }
		const Color color() const { return(m_color); }

		static Derivedtype ShapeStringToEnum(std::string s)
		{
			for (int i = 0; i < shapes.size(); ++i)
			{
				if (shapes.at(i) == s)
				{
					return static_cast<Derivedtype>(i);
				}
			}
			return Derivedtype::END_ENUM;
		}

		static Functions FuncStringToEnum(std::string s)
		{
			for (int i = 0; i < transforms.size(); ++i)
			{
				if (transforms.at(i) == s)
				{
					return static_cast<Functions>(i);
				}
			}
			return Functions::UNKNOWN;
		}

		virtual float area() const = 0;
		virtual float perimeter() const = 0;
		virtual void translate(const Vec2& t) = 0;
		virtual void homothety(float ratio) = 0;
		virtual void homothety(const Vec2& s, float ratio) = 0;
		virtual void rotate(float angle) = 0;
		virtual void rotate(const Vec2& p, double angle) = 0;
		virtual void centralSym(const Vec2& c) = 0;
		virtual void axialSym(const Vec2& p, const Vec2& d) = 0;
		virtual void display(SDL_Renderer* renderer) = 0;
		virtual void serialize( std::string& serial ) = 0;

		friend std::ostream& operator<< (std::ostream &out, Shape &Shape);
	protected:
		Derivedtype m_type;
		Color m_color;
	};
	const std::vector<std::string> Shape::transforms = { "rotate", "homothety", "translate", "axial_sym", "central_sym" };
	const std::vector<std::string> Shape::shapes = { "circle", "polygon", "line", "ellipse" };

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Circle : public Shape
	{
	public:
		Circle(Vec2 origin, float radius, Color color) :
			Shape(Shape::Derivedtype::CIRCLE, color),
			m_origin(origin),
			m_radius(radius)
		{}


		//Circle specific functions
		const Vec2& origin() const { return (m_origin); }
		const float radius() const { return(m_radius); }

		//Transformable interface functions
		float area() const { return(PI * (m_radius*m_radius)); }
		float perimeter() const { return(2 * PI* m_radius); }
		void homothety(float ratio) { m_radius *= ratio; }
		void homothety(const Vec2& s, float ratio) 
		{  
			Vec2 u = (s - m_origin);
			m_origin = m_origin + ratio*u;
			m_radius *= ratio;
		}
		void rotate(const Vec2& p, double angle)
		{
			float s = fast_sin(angle);
			float c = fast_cos(angle);
			m_origin.x -= p.x;
			m_origin.y -= p.y;
			float x = (m_origin.x * c - m_origin.y * s) + p.x;
			float y = (m_origin.x * s + m_origin.y * c) + p.y;
			m_origin.x = x;
			m_origin.y = y;
		}
		void rotate(float angle)
		{
			//Use origin as rotation center
			//does not change anythiong
		}
		void translate(const Vec2& t) { m_origin = m_origin + t; }
		void centralSym(const Vec2& c) { Vec2 t = 2 * (c - m_origin); translate(t); }
		void axialSym(const Vec2& p, const Vec2& d)
		{
			Vec2 p1 = p + d;
			Vec2 w = m_origin - p;
			Vec2 vl = p1 - p;
			float b = dot(w, vl) / dot(vl, vl);
			Vec2 intersection = p + b * vl;
			translate(2 * (intersection - m_origin));
		}

		//Displayable interface functions
		void display(SDL_Renderer* renderer)
		{
			SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 0x00);
			for (int i = -(int)(m_radius); i < (int)(m_radius); ++i)
			{
				for (int j = -(int)(m_radius); j < (int)(m_radius); ++j)
				{
					if (i*i + j*j <= m_radius*m_radius)
					{
						SDL_RenderDrawPoint(renderer, (int)(m_origin.x + i), (int) (m_origin.y + j));
					}
				}
			}

		}

		void serialize(std::string& serial)
		{
			serial = serial + " circle";
			serial = serial + " " + to_string(m_origin.x);
			serial = serial + " " + to_string(m_origin.y);
			serial = serial + " " + to_string(m_radius);
			serial = serial + " " + to_string(m_color.r);
			serial = serial + " " + to_string(m_color.g);
			serial = serial + " " + to_string(m_color.b);
		}

		BoundingBox bounding_box()
		{
			BoundingBox bb = {};
			bb.x_min = (int)(m_origin.x - m_radius);
			bb.x_max = (int)(m_origin.x + m_radius);
			bb.y_min = (int)(m_origin.y - m_radius);
			bb.y_max = (int)(m_origin.y + m_radius);
			
			return bb;
		}


		friend std::ostream& operator<< (std::ostream &out, const Circle &Circle);
	private:
		Vec2 m_origin;
		float m_radius;
	};

	bool operator==(const Circle& a, const Circle& b) { if (a.origin() == b.origin() && a.radius() == b.radius() && a.color() == b.color()) return true; return false; }
	bool operator!=(const Circle& a, const Circle& b) { return !(a == b); }
	std::ostream& operator<< (std::ostream &out, const Circle &Circle)
	{
		out << "Circle" << std::endl;
		out << "\t" << Circle.origin() << " " << Circle.radius() << " " << Circle.color() << std::endl;
		return out;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Polygon : public Shape
	{
	public:
		Polygon(std::vector<Vec2> points, Color color) :
			Shape(Shape::Derivedtype::POLYGON, color),
			m_points(points)
		{
			//ASSERT VEC SIZE >= 3
		}

		//Polygon specific functions
		const std::vector<Vec2>& points() const { return(m_points); }

		//Transformable interface functions
		float area() const
		{
			float a = 0.f;
			for (std::vector<Vec2>::const_iterator it = m_points.begin() + 1; it != m_points.end() - 1; ++it)
			{
				a += triangle_area(m_points[0], (*it), (*(it + 1)));
			}
			return(a);
		}

		float perimeter() const
		{
			float p = 0.f;
			for (std::vector<Vec2>::const_iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				if (it == m_points.end() - 1)
					p += norm((*it) - (*(m_points.begin())));
				else
					p += norm((*it) - (*(it + 1)));
			}
			return(p);
		}

		void homothety(float ratio) 
		{ /*compute bounding rectangle and move points by (rect_center - points)*ratio */ 
			BoundingBox bb = bounding_box();
			Vec2 center = Vec2(bb.x_max - ((bb.x_max - bb.x_min) / 2.f), bb.y_max - ((bb.y_max - bb.y_min) / 2.f));
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				Vec2 u = (center - (*it));			
				(*it) = (*it) + ratio*u;
			}
		}
		void homothety(const Vec2& s, float ratio) 
		{  
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				Vec2 u = (s - (*it));
				(*it) = (*it) + ratio*u;
			}
		}
		void rotate(const Vec2& p, double angle)
		{
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				float s = fast_sin(angle);
				float c = fast_cos(angle);
				(*it).x -= p.x;
				(*it).y -= p.y;
				float x = ((*it).x * c - (*it).y * s) + p.x;
				float y = ((*it).x * s + (*it).y * c) + p.y;
				(*it).x = x;
				(*it).y = y;
			}
		}
		void rotate(float angle)
		{
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				float s = fast_sin(angle);
				float c = fast_cos(angle);
				float x = ((*it).x * c - (*it).y * s);
				float y = ((*it).x * s + (*it).y * c);
				(*it).x = x;
				(*it).y = y;
			}
		}
		void translate(const Vec2& t)
		{
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				(*it) = (*it) + t;
			}
		}
		void centralSym(const Vec2& c)
		{
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				Vec2 t = 2 * (c - (*it));
				(*it) = (*it) + t;
			}
		}
		void axialSym(const Vec2& p, const Vec2& d)
		{
			for (std::vector<Vec2>::iterator it = m_points.begin(); it != m_points.end(); ++it)
			{
				Vec2 p1 = p + d;
				Vec2 w = (*it) - p;
				Vec2 vl = p1 - p;
				float b = dot(w, vl) / dot(vl, vl);
				Vec2 intersection = p + b * vl;
				(*it) = (*it) + (2 * (intersection - (*it)));
			}
		}

		void display(SDL_Renderer* renderer)
		{
			SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 0x00);
			BoundingBox bb = bounding_box();
			for (int i = bb.x_min; i < bb.x_max; ++i)
			{
				for (int j = bb.y_min; j < bb.y_max; ++j)
				{
					if (isPointInPolygon(Vec2(i, j)))
					{
						SDL_RenderDrawPoint(renderer, i, j);
					}
				}
			}
		}

		void serialize(std::string& serial)
		{
			serial = serial + " polygon";
			serial = serial + " " + to_string((int)m_points.size());
			for (auto point : m_points)
			{
				serial = serial + " " + to_string(point.x);
				serial = serial + " " + to_string(point.y);
			}
			serial = serial + " " + to_string(m_color.r);
			serial = serial + " " + to_string(m_color.g);
			serial = serial + " " + to_string(m_color.b);
		}

		BoundingBox bounding_box()
		{
			BoundingBox bb;
			for (auto point : m_points)
			{
				if (point.x < bb.x_min)
					bb.x_min = (int)point.x;
				if (point.x > bb.x_max)
					bb.x_max = (int)point.x;
				if (point.y < bb.y_min)
					bb.y_min = (int)point.y;
				if (point.y > bb.y_max)
					bb.y_max = (int)point.y;					
			}
			return bb;
		}

		friend std::ostream& operator<< (std::ostream &out, const Polygon &Polygon);

	private:
		std::vector<Vec2> m_points;
		float triangle_area(Vec2 a, Vec2 b, Vec2 c) const { return((1.f / 2.f) * abs((b.x - a.x)*(c.y - a.y) - (c.x - a.x)*(b.y - a.y))); }
		bool isPointInPolygon(Vec2 p) {
			int i, j, nvert = m_points.size();
			bool c = false;

			for (i = 0, j = nvert - 1; i < nvert; j = i++) {
				if (((m_points[i].y >= p.y) != (m_points[j].y >= p.y)) &&
					(p.x <= (m_points[j].x - m_points[i].x) * (p.y - m_points[i].y) / (m_points[j].y - m_points[i].y) + m_points[i].x)
					)
					c = !c;
			}
			return c;
		}
	};

	bool operator==(const Polygon& a, const Polygon& b)
	{
		if (a.points() == b.points())
			return true;
		return false;
	}

	bool operator!=(const Polygon& a, const Polygon& b)
	{
		return !(a == b);
	}

	std::ostream& operator<< (std::ostream &out, const Polygon &Polygon)
	{
		out << "Polygon" << std::endl;
		for (auto point : Polygon.points())
		{
			out << "\t" << point << std::endl;
		}
		out << "\t" << Polygon.color() << std::endl;
		return out;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Line : public Shape
	{
	public:
		Line(Vec2 point, Vec2 direction, Color color) :
			Shape(Shape::Derivedtype::LINE, color),
			m_point(point),
			m_direction(direction)
		{};

		//Line specific functions
		const Vec2& point() const { return (m_point); }
		const Vec2& direction() const { return (m_direction); }

		//Transformable interface functions
		float area() const { return(1.f); }
		float perimeter() const { return(1.f); }
		void homothety(float ratio) { /*NON SENSE*/ }
		void homothety(const Vec2& s, float ratio) { /*NON SENSE*/ }
		void rotate(float angle) 
		{  
			Vec2 p = m_point + m_direction;
			p.x -= m_point.x;
			p.y -= m_point.y;
			float s = fast_sin(angle);
			float c = fast_cos(angle);
			float x = (p.x * c - p.y * s) + m_point.x;
			float y = (p.x * s + p.y * c) + m_point.y;

			m_direction = (Vec2(x, y) - m_point);
		}
		void rotate(const Vec2& p, double angle) 
		{ 
			Vec2 point = m_point + m_direction;
			point.x -= p.x;
			point.y -= p.y;
			float s = fast_sin(angle);
			float c = fast_cos(angle);
			float x = (point.x * c - point.y * s) + p.x;
			float y = (point.x * s + point.y * c) + p.y;

			m_point.x -= p.x;
			m_point.y -= p.y;
			float c_x = (m_point.x * c - m_point.y * s) + p.x;
			float c_y = (m_point.x * s + m_point.y * c) + p.y;
			m_point.x = c_x;
			m_point.y = c_y;
			m_direction = (Vec2(x, y) - m_point);
		}

		void translate(const Vec2& t) { m_point = m_point + t; }
		void centralSym(const Vec2& c) { Vec2 t = 2 * (c - m_point); translate(t); }
		void axialSym(const Vec2& p, const Vec2& d){ /*NON SENSE*/ }
		void display(SDL_Renderer* renderer)
		{
			SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 0x00);
			SDL_RenderDrawLine(renderer, (int)m_point.x, (int)m_point.y, (int)(m_point.x + m_direction.x), (int) (m_point.y + m_direction.y));
		}

		void serialize(std::string& serial)
		{
			serial = serial + " line";
			serial = serial + " " + to_string(m_point.x);
			serial = serial + " " + to_string(m_point.y);
			serial = serial + " " + to_string(m_direction.x);
			serial = serial + " " + to_string(m_direction.y);
			serial = serial + " " + to_string(m_color.r);
			serial = serial + " " + to_string(m_color.g);
			serial = serial + " " + to_string(m_color.b);
		}

		BoundingBox bounding_box()
		{
			//infinite so we put everything at 0
			BoundingBox bb;
			bb.x_max = 0;
			bb.y_max = 0;
			return bb;
		}

		friend std::ostream& operator<< (std::ostream &out, const Line &Line);

	private:
		Vec2 m_point;
		Vec2 m_direction;
	};

	bool operator==(const Line& a, const Line& b)
	{
		if (a.point() == b.point() && a.direction() == b.direction())
			return true;
		return false;
	}

	bool operator!=(const Line& a, const Line& b)
	{
		return !(a == b);
	}

	std::ostream& operator<< (std::ostream &out, const Line &Line)
	{
		out << "Line" << std::endl;
		out << Line.point() << " " << Line.direction() << " " << Line.color() << std::endl;
		return out;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	class Ellipse : public Shape
	{
	public:
		Ellipse(Vec2 origin, Vec2 radius, Color color) :
			Shape(Shape::Derivedtype::ELLIPSE, color),
			m_origin(origin),
			m_radius(radius)
		{};

		//Ellipse specific functions
		const Vec2& origin() const { return (m_origin); }
		const Vec2& radius() const { return(m_radius); }

		//Transformable interface function
		float area() const { return(PI * m_radius.x * m_radius.y); }
		float perimeter() const
		{ //Ramanujan approx  
			float h = ((m_radius.x - m_radius.y)*(m_radius.x - m_radius.y)) / ((m_radius.x + m_radius.y) * (m_radius.x + m_radius.y));
			float p = PI * (m_radius.x + m_radius.y) * (1 + (3 * h) / (10 + fast_sqrt(4 - 3 * h)));
			return p;
		}
		void homothety(float ratio) { m_radius = ratio*m_radius; }
		void homothety(const Vec2& s, float ratio) 
		{
			Vec2 u = (s - m_origin);
			m_origin = m_origin + ratio*u;
			m_radius = ratio * m_radius;
		}
		void rotate(const Vec2& c, double angle) { /* CANT DO IT */ }
		void rotate(float angle) { /* CANT DO IT (or only pi/2 and pi) */ }
		void translate(const Vec2& t) { m_origin = m_origin + t; }
		void centralSym(const Vec2& c) { Vec2 t = 2 * (c - m_origin); translate(t); }
		void axialSym(const Vec2& p, const Vec2& d)
		{
			Vec2 p1 = p + d;
			Vec2 w = m_origin - p;
			Vec2 vl = p1 - p;
			float b = dot(w, vl) / dot(vl, vl);
			Vec2 intersection = p + b * vl;
			translate(2 * (intersection - m_origin));
		}

		//Displayable interface function
		void display(SDL_Renderer* renderer)
		{
			SDL_SetRenderDrawColor(renderer, m_color.r, m_color.g, m_color.b, 0x00);
			for (int i = -(int)(m_radius.x); i < (int)(m_radius.x); ++i)
			{
				for (int j = -(int)(m_radius.y); j < (int)(m_radius.y); ++j)
				{
					if (j*j*m_radius.x*m_radius.x + i*i*m_radius.y*m_radius.y <= m_radius.x*m_radius.x*m_radius.y*m_radius.y)
					{
						SDL_RenderDrawPoint(renderer, (int) (m_origin.x + i), (int) (m_origin.y + j));
					}
				}
			}
		}

		void serialize(std::string& serial)
		{
			serial = serial + " ellipse";
			serial = serial + " " + to_string(m_origin.x);
			serial = serial + " " + to_string(m_origin.y);
			serial = serial + " " + to_string(m_radius.x);
			serial = serial + " " + to_string(m_radius.y);
			serial = serial + " " + to_string(m_color.r);
			serial = serial + " " + to_string(m_color.g);
			serial = serial + " " + to_string(m_color.b);
		}

		BoundingBox bounding_box()
		{
			BoundingBox bb;
			bb.x_min = (int)(m_origin.x - m_radius.x);
			bb.x_max = (int)(m_origin.x + m_radius.x);
			bb.y_min = (int)(m_origin.y - m_radius.y);
			bb.y_max = (int)(m_origin.y + m_radius.y);

			return bb;
		}

		friend std::ostream& operator<< (std::ostream &out, const Ellipse &Ellipse);

	private:
		Vec2 m_origin;
		Vec2 m_radius;
	};

	bool operator==(const Ellipse& a, const Ellipse& b)
	{
		if (a.origin() == b.origin() && a.radius() == b.radius())
			return true;
		return false;
	}

	bool operator!=(const Ellipse& a, const Ellipse& b)
	{
		return !(a == b);
	}

	std::ostream& operator<< (std::ostream &out, const Ellipse &Ellipse)
	{
		out << "Ellipse" << std::endl;
		out << Ellipse.origin() << " " << Ellipse.radius() << " " << Ellipse.color() << std::endl;
		return out;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////

	class Image
	{
	public:
		Image() : annotation(std::string()), components_(std::vector<Shape *>()){}
		Image(const Image&) = delete;
		Image& operator=(Image const&) = delete;
		void add_component(Shape* s) 
		{ 
			//std::lock_guard<std::mutex> guard(mutex);
			components_.push_back(s); 
		}
		void display(SDL_Renderer* renderer)
		{
			//std::lock_guard<std::mutex> guard(mutex);
			for (auto component : components_)
			{
				component->display(renderer);
			}
		}
		std::string get_annotation()
		{
			//std::lock_guard<std::mutex> guard(mutex);
			return annotation;
		}

		void annotate(std::string msg)
		{
			//std::lock_guard<std::mutex> guard(mutex);
			annotation = msg;
		}

		void serialize(std::string& serial)
		{
			//std::lock_guard<std::mutex> guard(mutex);
			int max_x = 0, max_y = 0;
			for (auto component : components_)
			{
				component->serialize(serial);
			}
			serial = serial + " annotation " + to_string((int)annotation.size()) + " " + annotation;
		}

		void deserialize(std::string s)
		{
			components_.clear();
			std::istringstream buf(s);
			for (std::string word; buf >> word;)
			{
				if (word == "circle")
				{
					buf >> word;
					float x = std::stof(word);
					buf >> word;
					float y = std::stof(word);
					buf >> word;
					float rad = std::stof(word);
					buf >> word;
					int r = std::stoi(word);
					buf >> word;
					int g = std::stoi(word);
					buf >> word;
					int b = std::stoi(word);
					add_component(new Circle(Vec2(x, y), rad, Color(r, g, b)));
				}

				else if (word == "polygon")
				{
					std::vector<Vec2> points;
					buf >> word;
					int nb_pts = std::stoi(word);
					for (int i = 0; i < nb_pts; i++)
					{
						buf >> word;
						float x = std::stof(word);
						buf >> word;
						float y = std::stof(word);
						points.push_back(Vec2(x, y));
					}
					buf >> word;
					int r = std::stoi(word);
					buf >> word;
					int g = std::stoi(word);
					buf >> word;
					int b = std::stoi(word);
					add_component(new Polygon(points, Color(r, g, b)));
				}

				else if (word == "line")
				{
					buf >> word;
					float x = std::stof(word);
					buf >> word;
					float y = std::stof(word);
					buf >> word;
					float dir_x = std::stof(word);
					buf >> word;
					float dir_y = std::stof(word);
					buf >> word;
					int r = std::stoi(word);
					buf >> word;
					int g = std::stoi(word);
					buf >> word;
					int b = std::stoi(word);
					add_component(new Line(Vec2(x, y), Vec2(dir_x, dir_y), Color(r, g, b)));
				}

				else if (word == "ellipse")
				{
					buf >> word;
					float x = std::stof(word);
					buf >> word;
					float y = std::stof(word);
					buf >> word;
					float rad_x = std::stof(word);
					buf >> word;
					float rad_y = std::stof(word);
					buf >> word;
					int r = std::stoi(word);
					buf >> word;
					int g = std::stoi(word);
					buf >> word;
					int b = std::stoi(word);
					add_component(new Ellipse(Vec2(x, y), Vec2(rad_x, rad_y), Color(r, g, b)));
				}

				else if (word == "annotation")
				{
					buf >> word;
					int string_size = std::stoi(word);
					char buffer[1024];
					buf.getline(buffer, string_size + 2);
					std::string annotation = std::string(buffer);
					annotate(annotation);
				}
			}
		}

		std::vector< Shape* > components()
		{
			//std::lock_guard<std::mutex> guard(mutex);
			return components_;
		}

	private:
		std::vector< Shape* > components_;
		std::string annotation;
		std::mutex mutex;
	};


	///////////////////////////////////////////////////////////////////////////////

	std::ostream& operator<< (std::ostream &out, Shape &Shape)
	{
		switch (Shape.type())
		{
			case Shape::Derivedtype::CIRCLE:
			{
				Circle* c = static_cast<Circle*>(&Shape);
				out << *c;
			}break;
			case Shape::Derivedtype::POLYGON:
			{
				Polygon* p = static_cast<Polygon*>(&Shape);
				out << *p;
			}break;
			case Shape::Derivedtype::ELLIPSE:
			{
				Ellipse* e = static_cast<Ellipse*>(&Shape);
				out << *e;
			}break;
			case Shape::Derivedtype::LINE:
			{
				Line* l = static_cast<Line*>(&Shape);
				out << *l;
			}break;
		}

		return out;
	}

}
