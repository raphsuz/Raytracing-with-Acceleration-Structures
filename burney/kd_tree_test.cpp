#include<bits/stdc++.h>
#include<chrono>
#include"kd_tree.h"
using namespace std;
using namespace chrono;
DataPoint get_naive(const vector<DataPoint>&ps,const Point &target){
	double dis=DBL_MAX;
	DataPoint ans;
	for(const auto &p:ps)if(distance(p,target)<dis){
		dis=distance(p,target);
		ans=p;
	}
	return ans;
}
DataPoint get_kd_tree(const KD_tree &kd_tree,const Point &target){
	DataPoint ans;
	kd_tree.query_nearst(target,ans);
	return ans;
}
int main(int argc,char **argv){
	if(argc!=2){clog<<"usage: "<<argv[0]<<" [num_points]"<<endl;return 0;}
	mt19937 gen(chrono::steady_clock::now().time_since_epoch().count());
	auto rand_dbl=bind(uniform_real_distribution<>(0,1),gen);
//    for(int i=0;i<10;i++)cout<<rand_dbl()<<endl;
	const int num_points=atoi(argv[1]);
	clog<<"num_points = "<<num_points<<endl;
	vector<DataPoint>ps;
	for(int i=0;i<num_points;i++){
		DataPoint p;
		for(int j=0;j<NUM_DIMS;j++)p.x[j]=rand_dbl();
		p.data=new int(i);
		ps.push_back(p);
	}
	{
		Point p;
		for(int i=0;i<NUM_DIMS;i++)p.x[i]=rand_dbl();
		const auto ans_naive=get_naive(ps,p);
		cout<<"naive answer: "<<*((int*)ans_naive.data)<<", distance = "<<distance(p,ans_naive)<<endl;
		KD_tree kd_tree;
		kd_tree.build_tree(ps);
		const auto ans_kd_tree=get_kd_tree(kd_tree,p);
		cout<<"kd_tree answer: "<<*((int*)ans_kd_tree.data)<<", distance = "<<distance(p,ans_kd_tree)<<endl;
	}
	cout<<"naive timing..."<<endl;
	vector<int>ans_naive,ans_kd_tree;
	{
		auto time=steady_clock::now();
		vector<DataPoint>_;
		for(int t=1;t<num_points;t++){
			_.push_back(ps[t-1]);
			ans_naive.push_back(*(int*)get_naive(_,ps[t]).data);
		}
		auto period=duration_cast<nanoseconds>(steady_clock::now()-time);
		cout<<"takes "<<double(period.count())*nanoseconds::period::num/nanoseconds::period::den*1000<<" ms."<<endl;
	}
	cout<<"kd_tree timing..."<<endl;
	{
		auto time=steady_clock::now();
		KD_tree _;
		for(int t=1;t<num_points;t++){
			_.insert(ps[t-1]);
			ans_kd_tree.push_back(*(int*)get_kd_tree(_,ps[t]).data);
		}
		auto period=duration_cast<nanoseconds>(steady_clock::now()-time);
		cout<<"takes "<<double(period.count())*nanoseconds::period::num/nanoseconds::period::den*1000<<" ms."<<endl;
	}
	cout<<"verifying..."<<endl;
	assert(ans_naive.size()==ans_kd_tree.size());
	for(int i=0;i<(int)ans_naive.size();i++)assert(ans_naive[i]==ans_kd_tree[i]);
	cout<<"ok!"<<endl;
	return 0;
}
