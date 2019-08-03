#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>
#include <cmath> 
#include <stack>
#include <unistd.h>

using namespace std;
const int max_cluster=16000*16000;//muss größer sein als der maximale Cluster!
long feld[max_cluster+1];
int intervall=200,steps=1000;
int anzahl,size,theta,L,FF,M;
std::string addpoint(std::string name);

int main(int argc, char* argv[])
{
    switch(argc){
    default:std::cout<<"Bitte theta, L, FF, intervall, steps, massstab als konsolenargument übergeben."<<std::endl;
	abort();
    case 3: steps=atoi(argv[1]);
	intervall=atoi(argv[2]);
	std::cout<<"stepanzahl = "<<steps<<"\t"<<"intervallgroesse = "<<intervall<<std::endl;break;
    case 7: 
	theta=atoi(argv[1]);
	L=atoi(argv[2]);
	FF=atoi(argv[3]);
	intervall=atoi(argv[4]);
	steps=atoi(argv[5]);
	M=atoi(argv[6]);
    }
        
    for(int i=0;i<=max_cluster;i++){
	feld[i]=0;
    }
     
    stringstream folder;
    for(int k=0;k<6;k++){
	folder<<"t"<<theta<<"_"<<"L"<<L<<"_"<<"FF"<<FF<<"_"<<"M"<<M;
	for (int i=0;i<steps;i+=intervall){
	    stringstream ss;
	    ss<<folder.str()<<"/clusterstat"<<i<<".dat";
	    
	    ifstream infile(ss.str().c_str());
	    infile>>size>>anzahl;
	    while (infile.good())
		{
		    feld[size]+=anzahl;
		    infile>>size>>anzahl;
		}
	}
	string name=folder.str();
	folder.str(addpoint(name));
	std::cout<<k<<"-ter Durchlauf komplett"<<std::endl;
    }
    stringstream out;
    out<<"t"<<theta<<"_"<<"L"<<L<<"_"<<"FF"<<FF<<"_"<<"M"<<M<<"/clustersumme.dat";
    ofstream outcluster(out.str().c_str());
    for(int i=1;i<=max_cluster;i++){
    	if(feld[i]!=0){
    	    outcluster<<i<<"\t"<<feld[i]<<endl;
    	}
    }
    std::cout<<"clustersumme.dat erfolgreich erstellt."<<std::endl;
    
    stringstream off;
    off<<"t"<<theta<<"_"<<"L"<<L<<"_"<<"FF"<<FF<<"_"<<"M"<<M<<"/anim-cluster.gp";
    ofstream gnuscript(off.str().c_str());
    gnuscript<<"reset"<<std::endl;
    gnuscript<<"set terminal postscript enhanced color solid"<<std::endl;
    gnuscript<<"set output \"clusterstat.eps\""<<std::endl;
    gnuscript<<"set title \"steps "<<steps<<",Intervall "<<intervall<<",theta "<<theta<<",L "<<L<<",FF\""<<FF<<std::endl;
    gnuscript<<"set xlabel \"Clustergroesse\""<<std::endl;
    gnuscript<<"set ylabel \"Anzahl Cluster\""<<std::endl;
    gnuscript<<"set logscale x"<<std::endl;
    gnuscript<<"set logscale y"<<std::endl;
    gnuscript<<"plot \"clustersumme.dat\" using 1:2 notitle"<<std::endl;
    
    stringstream gnu;
    gnu<<"t"<<theta<<"_"<<"L"<<L<<"_"<<"FF"<<FF<<"_"<<"M"<<M;
    chdir(gnu.str().c_str());
    //cout<<system("pwd");
    system("echo \"load \'anim-cluster.gp\'\"|gnuplot");
    chdir("..");
}

std::string addpoint(std::string name){
    stringstream ss;
    ss<<name<<".";
    return ss.str();
}
