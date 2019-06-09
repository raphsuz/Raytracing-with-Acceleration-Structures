#include<bits/stdc++.h>
using namespace std;
template<class T>inline bool getmin(T&a,const T&b){return b<a?(a=b,true):false;}
template<class T>inline bool getmax(T&a,const T&b){return a<b?(a=b,true):false;}
const int NUM_DIMS=2;
class Point{
  public:
	double x[NUM_DIMS];
};
class DataPoint:public Point{
	void *data=NULL;
};
struct Box{
	Box(){}
	Box(const Point &mn,const Point &mx):mn(mn),mx(mx){}
	Point mn,mx;
	static Box get_box(const vector<Point>&s){
		Point mn,mx;
		for(int i=0;i<NUM_DIMS;i++)mn.x[i]=DBL_MAX,mx.x[i]=DBL_MIN;
		for(const Point &p:s)for(int i=0;i<NUM_DIMS;i++)getmin(mn.x[i],p.x[i]),getmax(mx.x[i],p.x[i]);
		return Box(mn,mx);
	}
	static Box get_box(const vector<DataPoint>&s){return Box::get_box(vector<Point>(s.begin(),s.end()));}
};
struct Node{
	Node *l=NULL,*r=NULL;
	Box box;
	int split_dim;
	double split_value;
	// leaf only
	DataPoint data;
	bool is_leaf()const{return l==NULL&&r==NULL;}
};
struct KD_tree{
	Node *root=NULL;
	void build_tree(Node* &o,vector<DataPoint>data,int split_dim){
		if(data.empty()){o=NULL;return;}
		o=new Node();
		// leaf
		// data
		if(data.size()==1){o->data=data[0];return;}
		// not leaf
		// sort data, take medium as split_value
		sort(data.begin(),data.end(),[split_dim](const DataPoint &a,const DataPoint &b)->bool{
				return a.x[split_dim]<b.x[split_dim];
		});
		const auto mid=data.begin()+data.size()/2;
		vector<DataPoint>left_data,rigt_data;
		left_data.assign(data.begin(),mid);
		rigt_data.assign(mid,data.end());
		// l,r,box,split_dim,split_value
		o->box=Box::get_box(data);
		o->split_dim=split_dim;
		o->split_value=(prev(mid)->x[split_dim]+mid->x[split_dim])/2;
		build_tree(o->l,left_data,(split_dim+1)%NUM_DIMS);
		build_tree(o->r,rigt_data,(split_dim+1)%NUM_DIMS);
	}
};
