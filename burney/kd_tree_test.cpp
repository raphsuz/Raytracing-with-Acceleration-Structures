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
KD_tree kd_tree;
DataPoint get_kd_tree(const Point &target){
	DataPoint ans;
	kd_tree.query_nearst(target,ans);
	return ans;
}
int main(){
	mt19937 gen(chrono::steady_clock::now().time_since_epoch().count());
	auto rand_dbl=bind(uniform_real_distribution<>(0,1),gen);
//    for(int i=0;i<10;i++)cout<<rand_dbl()<<endl;
	vector<DataPoint>ps;
	for(int i=0;i<100000;i++){
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
		kd_tree.build_tree(ps);
		const auto ans_kd_tree=get_kd_tree(p);
		cout<<"kd_tree answer: "<<*((int*)ans_kd_tree.data)<<", distance = "<<distance(p,ans_kd_tree)<<endl;
	}
	const int num_case=1000;
	cout<<"naive timing..."<<endl;
	{
		auto time=steady_clock::now();
		for(int t=0;t<num_case;t++){
			Point p;
			for(int i=0;i<NUM_DIMS;i++)p.x[i]=rand_dbl();
			get_naive(ps,p);
		}
		auto period=duration_cast<nanoseconds>(steady_clock::now()-time);
		cout<<"takes average "<<double(period.count())*nanoseconds::period::num/nanoseconds::period::den*1000000/num_case<<" us."<<endl;
	}
	cout<<"kd_tree timing..."<<endl;
	{
		auto time=steady_clock::now();
		for(int t=0;t<num_case;t++){
			Point p;
			for(int i=0;i<NUM_DIMS;i++)p.x[i]=rand_dbl();
			get_kd_tree(p);
		}
		auto period=duration_cast<nanoseconds>(steady_clock::now()-time);
		cout<<"takes average "<<double(period.count())*nanoseconds::period::num/nanoseconds::period::den*1000000/num_case<<" us."<<endl;
	}
	return 0;
}
