#ifndef include_3D
#define include_3D
#include<bits/stdc++.h>
#include"bitmap.h"
using namespace std;
const double PI=acos(-1);
const double EPS=1e-9;
class Point3D
{
  public:
	double x,y,z;
	Point3D(){}
	Point3D(double _x,double _y,double _z):x(_x),y(_y),z(_z){}
	friend istream &operator>>(istream &in,Point3D &p){return in>>p.x>>p.y>>p.z;}
	friend ostream &operator<<(ostream &out,const Point3D &p){return out<<"x:"<<p.x<<", y:"<<p.y<<", z:"<<p.z;}
};
class Vector3D
{
  public:
	double x,y,z;
	Vector3D(){}
	Vector3D(double _x,double _y,double _z):x(_x),y(_y),z(_z){}
	double length()const{return sqrt(x*x+y*y+z*z);}
	Vector3D norm()const{return (*this)/length();}
	friend istream &operator>>(istream &in,Vector3D &p){return in>>p.x>>p.y>>p.z;}
	friend ostream &operator<<(ostream &out,const Vector3D &p){return out<<"x:"<<p.x<<", y:"<<p.y<<", z:"<<p.z;}
	friend Vector3D operator*(const Vector3D &a,const double b){return Vector3D(a.x*b,a.y*b,a.z*b);}
	friend Vector3D operator/(const Vector3D &a,const double b){return Vector3D(a.x/b,a.y/b,a.z/b);}
	friend Vector3D &operator*=(Vector3D &a,const double b){return a=(a*b);}
	friend Vector3D &operator/=(Vector3D &a,const double b){return a=(a/b);}
	friend Vector3D operator+(const Vector3D &a,const Vector3D &b){return Vector3D(a.x+b.x,a.y+b.y,a.z+b.z);}
	friend Vector3D operator-(const Vector3D &a,const Vector3D &b){return Vector3D(a.x-b.x,a.y-b.y,a.z-b.z);}
};
Vector3D operator-(const Point3D &a,const Point3D &b){return Vector3D(a.x-b.x,a.y-b.y,a.z-b.z);}
Point3D operator+(const Point3D &a,const Vector3D &b){return Point3D(a.x+b.x,a.y+b.y,a.z+b.z);}
Point3D operator-(const Point3D &a,const Vector3D &b){return Point3D(a.x-b.x,a.y-b.y,a.z-b.z);}
Vector3D cross(const Vector3D &a,const Vector3D &b){return Vector3D(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x);}
double dot(const Vector3D &a,const Vector3D &b){return a.x*b.x+a.y*b.y+a.z*b.z;}
class Ray
{
  public:
	Ray(const Point3D &_o,const Vector3D &_p):o(_o),p(_p){}
	Point3D o;
	Vector3D p;
	Ray go(const double ratio)const{return Ray(o+p*ratio,p);}
};
class Screen
{
  public:
	int width,height;
	friend istream &operator>>(istream &in,Screen &s){return in>>s.width>>s.height;}
	friend ostream &operator<<(ostream &out,const Screen &s){return out<<"width:"<<s.width<<", height:"<<s.height;}
};
class ColorX
{
  public:
	ColorX():r(0),g(0),b(0){}
	ColorX(const Color &c):r((c.r+0.5)/256),g((c.g+0.5)/256),b((c.b+0.5)/256){}
	ColorX(const double _r,const double _g,const double _b):r(_r),g(_g),b(_b){}
	double r,g,b;
	friend ColorX operator+(const ColorX &a,const ColorX &b){return ColorX(a.r+b.r,a.g+b.g,a.b+b.b);}
	friend ColorX &operator+=(ColorX &a,const ColorX &b){return a=(a+b);}
	friend ColorX operator*(const ColorX &a,const double b){return ColorX(a.r*b,a.g*b,a.b*b);}
	friend ColorX operator*(const ColorX &a,const ColorX &b){return ColorX(a.r*b.r,a.g*b.g,a.b*b.b);}
};
class Object3D;
class Light3D
{
  public:
	virtual ColorX get_specular(const Ray &light_path,const double level)const=0;
	virtual bool is_light_blocked(const Point3D &p,const vector<Object3D*>&objects)const=0;
};
class Material3D
{
  private:
	static int color_counter;
  public:
	Color color=vector<Color>{Color(255,0,0),Color(0,255,0),Color(0,0,255)}[(++color_counter)%=3];
	double ambient=0.2;
	double diffuse=0.5;
	double specular=0.7;
	double specular_exp=100;
	double reflected=0.3;
	double transmitted=0;
	friend istream &operator>>(istream &in,Material3D &m)
	{
		double r,g,b;
		auto &ret=in>>r>>g>>b>>m.ambient>>m.diffuse>>m.specular>>m.specular_exp>>m.reflected;
		m.color=Color((uint8_t)(r*255),(uint8_t)(g*255),(uint8_t)(b*255));
		return ret;
	}
};
int Material3D::color_counter=2;
class Object3D
{
  private:
	Vector3D mirror_vector(const Vector3D &v,const Vector3D &mirror)
	{
		const Vector3D mn=mirror.norm();
		const double mr=dot(v,mn);
		const Vector3D &mv=mn*mr;
		return v+(mv-v)*2;
	}
	friend ColorX get_colorx_globally(const Ray &ray,const vector<Object3D*>&objects,const vector<Light3D*>&lights,const ColorX &background,const double impact,const double min_impact)
	{
		if(impact<min_impact)return background;
		Object3D *object_hit;
		Vector3D normal;
		const double ray_l=hit_test_globally(ray,objects,object_hit,normal);
		if(isnan(ray_l))return background;
		return object_hit->get_colorx(ray,objects,lights,impact,min_impact,background);
	}
	ColorX get_colorx(const Ray &ray,const vector<Object3D*>&objects,const vector<Light3D*>&lights,const double impact,const double min_impact,const ColorX &background)
	{
		Vector3D normal;
		const double ray_l=hit_test(ray,normal);
		assert(!isnan(ray_l));
		const Point3D &hit_point=ray.o+ray.p*ray_l;
		const ColorX cx=ColorX(material.color);
		ColorX ans=cx*material.ambient;
		const Ray light_path=Ray(hit_point,mirror_vector(ray.p,normal*-1)); // cause problem #0000
		const Ray diffuse_path=Ray(hit_point,normal*-1);
		for(Light3D *l:lights)
		{
			ans+=l->get_specular(light_path,material.specular_exp)*material.specular;
			if(l->is_light_blocked(hit_point,objects))continue; // problem #0000
			ans+=l->get_specular(diffuse_path,1)*cx*material.diffuse;
		}
		const Ray reflected_ray=Ray(hit_point,(mirror_vector(ray.p,normal*-1)*-1).norm()).go(EPS);/*prevents wrong-hitting origin object*/
		ans+=get_colorx_globally(reflected_ray,objects,lights,background,impact*material.reflected,min_impact)*material.reflected;
		return ans;
	}
  protected:
	virtual ostream &to_out(ostream &out)const=0;
  public:
	Material3D material;
	// normal:
	//     if hit: normal of the hit point
	//     else:   undefined
	// returns:
	//     if hit: # of ray length to the hit point
	//     else:   NAN
	virtual double hit_test(const Ray &ray,Vector3D &normal)const=0;
	virtual string type()const=0;
	friend Color get_color_globally(const Ray &ray,const vector<Object3D*>&objects,const vector<Light3D*>&lights,const Color background,const double min_impact=0.01)
	{
		const ColorX &x=get_colorx_globally(ray,objects,lights,ColorX(background),1,min_impact);
		Color ans;
		ans.a=255;
		ans.r=(uint8_t)min(255.0,256*x.r);
		ans.g=(uint8_t)min(255.0,256*x.g);
		ans.b=(uint8_t)min(255.0,256*x.b);
		return ans;
	}
	friend double hit_test_globally(const Ray &ray,const vector<Object3D*>&objects,Object3D* &object_hit,Vector3D &normal,const double threshold_dist=0)
	{
		object_hit=NULL;
		double dist_hit=NAN;
		for(const auto o:objects)
		{
			static Vector3D n;
			const double d=o->hit_test(ray,n);
			if((isnan(dist_hit)&&threshold_dist<=d)||d<dist_hit)object_hit=o,normal=n,dist_hit=d;
		}
		return dist_hit;
	}
	friend ostream &operator<<(ostream &out,const Object3D &o){return o.to_out(out);}
};
class Camera
{
  private:
	Point3D get_pixel(const Screen &scn,const int x,const int y)
	{
		const static double dist_to_screen=1;
		const double rad_fov=field_of_view/180*PI;
		const double scn_half_width=dist_to_screen*tan(rad_fov/2);
		const double scn_half_height=scn_half_width/scn.width*scn.height;
		const Vector3D scn_x_vec=cross(look_direction,up_direction).norm();
		const Vector3D scn_y_vec=cross(look_direction,scn_x_vec).norm();
		const Point3D scn_origin=position+look_direction.norm()-scn_x_vec*scn_half_width-scn_y_vec*scn_half_height;
		assert(0<=x&&x<scn.width);
		assert(0<=y&&y<scn.height);
		const double xr=((double)x*2+1)/(scn.width*2);
		const double yr=((double)y*2+1)/(scn.height*2);
		return scn_origin+scn_x_vec*xr+scn_y_vec*yr;
	}
  public:
	double field_of_view; //field of view of screen width, in degrees (not rad)
	Point3D position;
	Vector3D look_direction;
	Vector3D up_direction=Vector3D(0,1,0);
	vector<Color>take_photo(const Screen &scn,const vector<Object3D*>&objects,const vector<Light3D*>&lights,const Color background=Color(255,255,255))
	{
		vector<Color>image;
		for(int i=scn.height-1;i>=0;i--)
		{
			for(int j=0;j<scn.width;j++)
			{
				const Ray &ray=Ray(position,get_pixel(scn,j,i)-position);
				const Color &c=get_color_globally(ray.go(1),objects,lights,background);
				image.push_back(c);
			}
		}
		return image;
	}
//    friend istream &operator>>(istream &in,Camera &c){return in>>c.position>>c.direction>>c.field_of_view;}
	friend ostream &operator<<(ostream &out,const Camera &c){return out<<"position:{"<<c.position<<"}, look:{"<<c.look_direction<<"}, up:{"<<c.up_direction<<"}, field_of_view: "<<c.field_of_view;}
};
#endif