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
																																																																																						

int FF=0, theta, L=400,k;
double dump, edge, fsi,size;
std::string addpoint(std::string name);
double mf1=0,mf2=0,mf3=0,mf4=0,mf5=0,am1=0,am2=0,am3=0,am4=0,am5=0,sfm1=0,sfm2=0,sfm3=0,sfm4=0,sfm5=0,f1=0,f2=0,f3=0,f4=0,f5=0;
double me1=0,me2=0,me3=0,me4=0,me5=0,sem1=0,sem2=0,sem3=0,sem4=0,sem5=0;
int main(int argc, char* argv[])
{   ofstream liste_mittel ("liste.dat");
    stringstream folder;
    theta=1000;
    while (theta<=1075)
	{   for(k=2;k<=7;k++)
		{
	    	
	    	stringstream ss;
	    	ss<<"t"<<theta<<"_L"<<L<<"_0_M"<<k<<"/shape.dat";
	    	// groeße --- dump --- fsi --- edge -- dump --- dump
	    	ifstream infile(ss.str().c_str());
		infile>> size >>dump>> fsi >> edge >> dump >> dump;
		    while (infile.good())
			{
			 if (size ==1)
				{
				mf1=fsi+mf1;
				me1=edge+me1;
				am1=am1+1;
				}	
		 	 if (size ==2)
				{
				mf2=fsi+mf2;
				me2=me2+edge;
				am2=am2+1;
				}
			 if (size ==3)
				{
				mf3=fsi+mf3;
				me3=me3+edge;
				am3=am3+1;
				}
			 if (size ==4)
				{
				mf4=fsi+mf4;
				me4=me4+edge;				
				am4=am4+1;
				}
			 if (size ==5)
				{
				mf5=fsi+mf5;
				me5=me5+edge;				
				am5=am5+1;
				}
			infile>> size >>dump>> fsi >> edge >> dump >> dump;
			//std::cout<<fsi;
			}
		//string name=folder.str();
		//folder.str(addpoint(name));
		//std::cout<<k<<"-ter Durchlauf komplett"<<std::endl;
    	sfm1=mf1/am1;
	sfm2=mf2/am2;
	sfm3=mf3/am3;
	sfm4=mf4/am4;
	sfm5=mf5/am5;
	sem1=me1/am1;
	sem2=me2/am2;
	sem3=me3/am3;
	sem4=me4/am4;
	sem5=me5/am5;
	liste_mittel<<theta<<"\t"<<k<<"\t"<<sfm1<<"\t"<<sfm2<<"\t"<<sfm3<<"\t"<<sfm4<<"\t"<<sfm5<<"\t"<<sem1<<"\t"<<sem2<<"\t"<<sem3<<"\t"<< sem4<<"\t"<<sem5<<std::endl;
	mf1=0;
	mf2=0;
	mf3=0;
	mf4=0;
	mf5=0;
	me1=0;
	me2=0;
	me3=0;
	me4=0;
	me5=0;
 	FF=0;
	am1=0;
	am2=0;
	am3=0;
	am4=0;
	am5=0;
    stringstream off;
    off<<"t"<<theta<<"_L"<<L<<"_0_M"<<k<<"/anim-cluster.gp";
    ofstream gnuscript(off.str().c_str());
    gnuscript<<"reset"<<std::endl;
    gnuscript<<"set terminal postscript enhanced color solid"<<std::endl;
    gnuscript<<"set output \"dichte.eps\""<<std::endl;
    gnuscript<<"set xlabel \"Zeit\""<<std::endl;
    gnuscript<<"set ylabel \"Dichte\""<<std::endl;
    gnuscript<<"plot \"dichte.dat\" using 1:2 notitle"<<std::endl;
    stringstream gnu;
    gnu<<"t"<<theta<<"_L"<<L<<"_0_M"<<k;
    chdir(gnu.str().c_str());
    std::cout<<system("pwd");
    system("echo \"load \'anim-cluster.gp\'\"|gnuplot");
    chdir("..");	
} 
theta=theta+25;
 }     
   
    
    
}

std::string addpoint(std::string name){
    stringstream ss;
    ss<<name<<".";
    return ss.str();
}
