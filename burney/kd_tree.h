#include<bits/stdc++.h>
using namespace std;
template<class T>inline bool getmin(T&a,const T&b){return b<a?(a=b,true):false;}
template<class T>inline bool getmax(T&a,const T&b){return a<b?(a=b,true):false;}
const int NUM_DIMS=2;
class Point{
  public:
	double x[NUM_DIMS];
};
double distance(const Point &a,const Point &b){
	double ans=0;
	for(int i=0;i<NUM_DIMS;i++)ans+=(a.x[i]-b.x[i])*(a.x[i]-b.x[i]);
	return ans;
}
class DataPoint:public Point{
  public:
	void *data=NULL;
};
struct Box{
	Box(){}
	Box(const Point &_mn,const Point &_mx):mn(_mn),mx(_mx){}
	Point mn,mx;
	static Box get_box(const vector<Point>&s){
		Point mn,mx;
		for(int i=0;i<NUM_DIMS;i++)mn.x[i]=DBL_MAX,mx.x[i]=DBL_MIN;
		for(const Point &p:s)for(int i=0;i<NUM_DIMS;i++)getmin(mn.x[i],p.x[i]),getmax(mx.x[i],p.x[i]);
		return Box(mn,mx);
	}
	static Box get_box(const vector<DataPoint>&s){return Box::get_box(vector<Point>(s.begin(),s.end()));}
};
double distance(const Point &a,const Box &b){
	double ans=0;
	for(int i=0;i<NUM_DIMS;i++){
		double d=0;
		if(a.x[i]<b.mn.x[i])d=b.mn.x[i]-a.x[i];
		if(a.x[i]>b.mx.x[i])d=a.x[i]-b.mx.x[i];
		ans+=d*d;
	}
	return ans;
}
struct Node{
	Node *l=NULL,*r=NULL;
	Box box;
	int split_dim;
	double split_value;
	// leaf only
	DataPoint data;
	bool is_leaf()const{return l==NULL&&r==NULL;}
};
Node *NewNode(){
	return new Node();
	const int chunk_size=1<<20;
	static Node *chunk=NULL;
	static int chunk_ptr=chunk_size;
	if(chunk_ptr>=chunk_size){
		chunk=new Node[chunk_size];
		chunk_ptr=0;
	}
	return chunk+(chunk_ptr++);
}
class KD_tree_naive{
  public:
	void build_tree(const vector<DataPoint>&data){
		static vector<DataPoint>data_copy;
		data_copy.assign(data.begin(),data.end());
		if(!data_copy.empty())build_tree(root,data_copy,0);
	}
	bool query_nearst(const Point &target,DataPoint &result)const{
		if(root==NULL)return false;
		query_nearst(root,target,result,false);
		return true;
	}
  private:
	Node *root=NULL;
	void query_nearst(Node const *o,const Point &target,DataPoint &result,bool result_assigned)const{
		assert(o!=NULL);
		if(result_assigned && distance(target,result)<=distance(target,o->box))return;
		if(o->is_leaf()){result=o->data;return;}
		int split_dim=o->split_dim;
		double split_value=o->split_value;
		if(target.x[split_dim]<split_value){
			query_nearst(o->l,target,result,result_assigned);
			query_nearst(o->r,target,result,true);
		}else{
			query_nearst(o->r,target,result,result_assigned);
			query_nearst(o->l,target,result,true);
		}
	}
	void build_tree(Node* &o,vector<DataPoint>&data,int split_dim){
		assert(!data.empty());
		o=NewNode();
		o->box=Box::get_box(data);

		// leaf: assign data
		if(data.size()==1){o->data=data[0];return;}

		// not leaf
		// sort data, take medium as split_value
		nth_element(data.begin(),data.begin()+data.size()/2,data.end(),[split_dim](const DataPoint &a,const DataPoint &b)->bool{
			return a.x[split_dim]<b.x[split_dim];});
		const double mid=(data.begin()+data.size()/2)->x[split_dim];
		vector<DataPoint>left_data,rigt_data,mid_data;
		left_data.reserve(data.size()/2);
		rigt_data.reserve(data.size()-data.size()/2);
		for(const auto &p:data)(p.x[split_dim]<mid?left_data:p.x[split_dim]>mid?rigt_data:mid_data).push_back(p);
		for(const auto &p:mid_data)(left_data.size()<rigt_data.size()?left_data:rigt_data).push_back(p);

		// l, r, split_dim, split_value
		o->split_dim=split_dim;
		o->split_value=mid;
		build_tree(o->l,left_data,(split_dim+1)%NUM_DIMS);
		build_tree(o->r,rigt_data,(split_dim+1)%NUM_DIMS);
	}
};
class KD_tree{
  public:
	void build_tree(const vector<DataPoint>&data){
		clear();
		for(const auto &dadium:data)insert(dadium);
	}
	void clear(){
		main_data.clear();
		kd_trees.clear();
	}
	void insert(const DataPoint &data){
		size_t sz=1;
		while(main_data.size()&sz)sz<<=1,kd_trees.pop_back();
		kd_trees.push_back(KD_tree_naive());
		main_data.push_back(data);
		vector<DataPoint>ps;
		ps.assign(main_data.end()-sz,main_data.end());
		kd_trees.back().build_tree(ps);
	}
	bool query_nearst(const Point &target,DataPoint &result)const{
		bool ans=false;
		DataPoint now;
		for(const auto &kd_tree:kd_trees)if(kd_tree.query_nearst(target,now)){
			if(!ans||distance(target,now)<distance(target,result))result=now,ans=true;
		}
		return ans;
	}
	size_t size()const{return main_data.size();}
  private:
	vector<KD_tree_naive>kd_trees;
	vector<DataPoint>main_data;
};
