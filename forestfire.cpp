//_______________________________________________________________
// 		Projektpraktikum 2011
// 	    Projekt: Selbstorganisierte Kritikalitaet 
// 	    Beispiel: Waldbraende
//	    Abgabe:   30.Juni.2011
//________________________________________________________________

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath> 
#include <stack>
#include <sys/stat.h> 
#include <sys/types.h>
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////////////
//                               Einstellbare Parameter                            
//////////////////////////////////////////////////////////////////////////////////////

int L=200;
int N=L*L+1;
int steps=200000;
int equilibriumsteps=100000;
double theta=500; //=p/f
int L_max_funkenflug=500; //maximale Funkenflugdistanz
int massstab=4;//wieviel felder sind 20ha

//////////////////////////////////////////////////////////////////////////////////////
//                               Symbolische Konstanten                             
//////////////////////////////////////////////////////////////////////////////////////

int perimeter=-N;//perimetersite
int outerperimeter=(-N-1);//äußerer perimetersite
int temp=-2*N-1;//tempwert für int countarea(int);

//////////////////////////////////////////////////////////////////////////////////////
//                        Benötigte Variablen / Messgrößen                          
//////////////////////////////////////////////////////////////////////////////////////

const double PI=3.1415926535898;
std::stack<int> perimeterverteilung;//stack mit perimetergrößen
int *parent;
int *cluster;
int baumanzahl=0;
double baumdichte=0;
double gesamtdichte=0;
double funkenflugcount=0;
int feuergenerationen=0;
int feueranzahl=0;

void initialise(std::ostream &fireshape);
std::string create_dir();
std::string create_dir_sub(std::string name);
std::string create_file(std::string name);
void create_gnuscript(std::ostream &gnuscript);
std::string foldername;
inline int randomspot()
{return (1+(N-1)*(double)rand()/(RAND_MAX+1.0));}


//////////////////////////////////////////////////////////////////////////////////////
//                             Methoden zur Simulation                              
//////////////////////////////////////////////////////////////////////////////////////

int find(int index);//findet root von index
int merge(int x,int y);//mergt cluster bei x,y und gibt root index zurück
void grow(int x);//growt Baum bei x, wenn x leer
void fire(int x,std::ostream &fireshape,bool statistik);//brennt cluster von Baum x ab
void funkenflug(int x,std::ostream &fireshape,bool statistik);//berechnet Flugweg eines Funkens und ruft wieder fire auf

//////////////////////////////////////////////////////////////////////////////////////
//                            geometrische Clusterstatistiken                       
// Nur findperimeter wird direkt aufgerufen und ruft selbst die anderen Methoden auf
//////////////////////////////////////////////////////////////////////////////////////

void findperimeter(int x,std::ostream &fireshape);//sucht und markiert perimeterfelder für andere methoden
void identify_outerperimeter();//sucht und markiert den aeußeren perimeter nach aufruf von findperimeter
int countarea(int root);//flaecheninhalt eines clusters zurückliefern
int sumofneighbors(int x,int root);//berechnet anzahl der nachbarn von x mit root
void countperimeter(int x,int root);//zaehlt laenge von perimeter, zu dem x gehört

//////////////////////////////////////////////////////////////////////////////////////
//                                 Ausgabemethoden                                  
//////////////////////////////////////////////////////////////////////////////////////

void print_snapshot(int t);//gibt teil des feldes aus
void print_clusterstat(int t);//erstellt clusterstatistik


int main(int argc, char* argv[])
{
    switch(argc){
    default: 
	parent = new int[N];
	cluster = new int[N];
	break;
    case 2: theta=atoi(argv[1]);
	parent = new int[N];
	cluster = new int[N];
	perimeter=-N;
	outerperimeter=(-N-1);
	temp=-2*N-1;
	std::cout<<"Theta = "<<theta<<std::endl;break;
    case 3: theta=atoi(argv[1]);
	equilibriumsteps=atoi(argv[2]);
	perimeter=-N;
	outerperimeter=(-N-1);
	temp=-2*N-1;
	parent = new int[N];
	cluster = new int[N];
	std::cout<<"Theta = "<<theta<<"\t"<<"equilibriumsteps = "<<equilibriumsteps<<std::endl;break;
    case 7: theta=atoi(argv[1]);
	L=atoi(argv[2]);
	L_max_funkenflug=atoi(argv[3]);
	equilibriumsteps=atoi(argv[4]);
	steps=atoi(argv[5]);
	massstab=atoi(argv[6]);
	N=L*L+1;
	perimeter=-N;
	outerperimeter=(-N-1);
	temp=-2*N-1;
	parent = new int[N];
	cluster = new int[N];
	std::cout<<"Theta = "<<theta<<"\t"<<"L = "<<L<<"\t";
	std::cout<<"FFlaenge = "<<L_max_funkenflug<<std::endl;
	std::cout<<"Equisteps = "<<equilibriumsteps<<"\t"<<"Steps = "<<steps<<"\t";
	std::cout<<"Massstab = "<<massstab<<std::endl;break;
    }
    

    foldername=create_dir();
    int zufall;
    std::ofstream dichte((create_file("dichte.dat")).c_str());
    std::ofstream fireshape((create_file("shape.dat")).c_str());
    std::ofstream stuff((create_file("stuff.dat")).c_str());
    std::ofstream gnuscript((create_file("anim-dichte.gp")).c_str());
    initialise(fireshape);
    
    clock_t start, ende, zwischenzeit; 
    start=clock(); 
    for(int t=0;t<equilibriumsteps;t++)
	{
	    for(int i=0;i<theta;i++)
		{
		    zufall=randomspot();
		    grow(zufall);
		}
	    zufall=randomspot();
	    fire(zufall,fireshape,false);
	    dichte<<t<<"\t"<<(double) baumanzahl/(N-1)<<std::endl;
	    if(t%10000==0)
		{
		    std::cout<<t<<" equilibriumsteps done"<<std::endl;
		}
	}
    zwischenzeit=clock();
    std::cout<<"Zwischenzeit nach Equilibrierung "<<((double)zwischenzeit-start)/CLOCKS_PER_SEC<<" s"<<std::endl;
    for(int t=0;t<steps;t++)
	{
	    for(int i=0;i<theta;i++)
		{
		    zufall=randomspot();
		    grow(zufall);
		}
	    zufall=randomspot();
	    if(parent[zufall]!=0)
		{
		    fire(zufall,fireshape,true);  
		}
	    
	    //print_snapshot(t);
	    if(t%400==0){
		print_clusterstat(t);
	    }
	    gesamtdichte+=(double) baumanzahl/(N-1);
	    dichte<<t+equilibriumsteps<<"\t"<<(double) baumanzahl/(N-1)<<std::endl;
	    if(t%10000==0){
		std::cout<<"Schritt "<<t<<" "<<"erfolgreich !"<<"\t";
		std::cout<<"Funkenfluganzahl "<<funkenflugcount<<std::endl;
		funkenflugcount=0;
	    }
	}
    stuff<<(double)feuergenerationen/feueranzahl<<"\t"<<gesamtdichte/steps<<std::endl;
    std::cout<<(double)feuergenerationen/feueranzahl<<"\t"<<gesamtdichte/steps<<std::endl;
    create_gnuscript(gnuscript);
    ende=clock();
    std::cout<<"Simulation erfolgreich beendet. Laufzeit: "<<((double)(ende-start))/CLOCKS_PER_SEC<<" s"<<std::endl;
    std::cout<<std::endl;
    std::cout<<"Laufzeit für Equilibrium: "<<((double)zwischenzeit-start)/CLOCKS_PER_SEC<<" s"<<std::endl;
    std::cout<<"Laufzeit für steps: "<<((double)ende-zwischenzeit)/CLOCKS_PER_SEC<<" s"<<std::endl;
    return 0;
}	

std::string create_dir() 
{ 
    std::stringstream ss;
    ss<<"t"<<theta<<"_L"<<L<<"_FF"<<L_max_funkenflug<<"_M"<<massstab; 
    
    if((mkdir(ss.str().c_str(),0755))){
	std::string name=ss.str();
	ss.str(create_dir_sub(name));
    }
    return ss.str();
}

std::string create_dir_sub(std::string name) 
{ 
    std::stringstream ss;
    ss<<name<<".";
    if((mkdir(ss.str().c_str(),0755))){
	std::string name=ss.str();
	ss.str(create_dir_sub(name));
    }
    return ss.str();
}

std::string create_file(std::string name) 
{
    std::stringstream ss;
    ss<<foldername<<"/"<<name;
    return ss.str();
}

void create_gnuscript(std::ostream &gnuscript){
    gnuscript<<"reset"<<std::endl;
    gnuscript<<"set terminal postscript enhanced color solid"<<std::endl;
    gnuscript<<"set output \"dichte.eps\""<<std::endl;
    gnuscript<<"set title \"equi: "<<equilibriumsteps<<", steps: "<<steps<<"\""<<std::endl;
    gnuscript<<"set xlabel \"Schritt\""<<std::endl;
    gnuscript<<"set ylabel \"Baumdichte\""<<std::endl;
    gnuscript<<"plot \"dichte.dat\" using 1:2 notitle"<<std::endl;

    chdir(foldername.c_str());
    system("echo \"load \'anim-dichte.gp\'\"|gnuplot");
    chdir("..");
    
}

void initialise(std::ostream &fireshape)
{
    srand(time(NULL));
    for(int i=0;i<N;i++)
	{
	    parent[i]=cluster[i]=0;
	}
    //fireshape<<"#Clusterkategorie "<<"Total islad area "<<"shape index ";
    //fireshape<<"edge index "<<"island size "<<"number of islands"<<std::endl;
}


//////////////////////////////////////////////////////////////////////////////////////
// Bekommt indizes übergeben und sucht rekursiv den root-index. Im Fall 0 wird      
// einfach 0 zurückgegeben.                                                         
//////////////////////////////////////////////////////////////////////////////////////

int find(int index)//bekommt index, sucht root-index
{
    if(parent[index]==0)
	{
	    return 0;
	}
    while(parent[index]>0)
	{
	    index=parent[index];
	}
    return index;
}


//////////////////////////////////////////////////////////////////////////////////////
// Bekommt root(!)-index übergeben und verbindet die zugehörigen cluster. Ist einer 
// der beiden überlieferten Werte ==0, so wird nichts getan. Der Rückgabewert ist   
// immer die root des größten entstandenen clusters, oder 0, wenn x==y==0.          
//////////////////////////////////////////////////////////////////////////////////////

int merge(int x,int y)
{
    if((x==0)&&(y==0))
	{
	    return 0;
	}
    if(x==0)
	{
	    return y;
	}
    if(y==0)
	{
	    return x;
	}
    if(x==y)
	{
	    return x; 
	}
    
    if(parent[x]<parent[y])
	{
	    cluster[-parent[y]]--; //Alte cluster aus der Statistik entfernen
	    cluster[-parent[x]]--;
	    parent[x]+=parent[y];
	    parent[y]=x;
	    cluster[-parent[x]]++; //Neuer cluster wird aufgenommen
	    return x;
	    
	}else
	{
	    cluster[-parent[y]]--;
	    cluster[-parent[x]]--;
	    parent[y]+=parent[x];
	    parent[x]=y;
	    cluster[-parent[y]]++;
	    return y;
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// Soll einen Baum bei x pflanzen, falls x leer. Dazu müssen die roots aller        
// Nachbarn gesucht werden. Es wird merge mit den jeweiligen roots gegen den        
// Uhrzeigersinn aufgerufen, da diese cluster durch den neuen Baum verbunden werden.
// Dabei wird gleichzeitig die clusterstatistik (cluster[]) aktualisiert. Nachdem   
// alle Nachbarn korrigiert wurden "pflanzen" wir den Baum und müssen erneut        
// cluster[] aktualisieren.                                                         
//////////////////////////////////////////////////////////////////////////////////////

void grow(int x)
{
    int nachbar1,nachbar2;
    if(parent[x]==0)
	{
	    baumanzahl++;
	    if(x<=L)                           //oben
		{
		    nachbar1=0;
		}else
		{
		    nachbar1=find(x-L);
		}
	    if((x%L==1)||(x<=L))               //obenlinks
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x-1-L);
		}
	    nachbar1=merge(nachbar1,nachbar2); //speichere die größere root

	    if(x%L==1)                         //...
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x-1);
		}
	    nachbar1=merge(nachbar1,nachbar2);

	    if((x>(N-L-1))||(x%L==1))
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x+L-1);
		}
	    nachbar1=merge(nachbar1,nachbar2);

	    if(x>(N-L-1))
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x+L);
		}
	    nachbar1=merge(nachbar1,nachbar2);
	    
	    if((x%L==0)||(x>(N-L-1)))
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x+1+L);
		}
	    nachbar1=merge(nachbar1,nachbar2);
	    
	    if(x%L==0)
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x+1);
		}
	    nachbar1=merge(nachbar1,nachbar2);
	    
	    if((x%L==0)||(x<=L))                  //oben rechts   
		{
		    nachbar2=0;
		}else
		{
		    nachbar2=find(x+1-L);
		}
	    parent[x]=merge(nachbar1,nachbar2);
	    	    
	    if(parent[x]>0)
		{
		    cluster[-parent[parent[x]]]--;//Neuen cluster in Statistik eintragen
		    parent[parent[x]]--;          //und seine masse erhöhen (negativ)
		    cluster[-parent[parent[x]]]++;
		}else
		{
		    parent[x]--;                  //x ist neuer cluster!
		    cluster[1]++;
		}	    
	    
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// Soll gesamten cluster bei x abbrennen, wenn x nicht leer. Dazu werden zwei stacks
// benutzt. Der Baum x wird auf den current-stack gelegt und auf 0 gesetzt          
// (abgebrannt). In jedem Schritt wird jetzt einer der stacks abgearbeitet. Der     
// oberste Eintrag wird abgenommen, seine Nachbarn werden auf den next-stack gelegt 
// und auf 0 gesetzt. In der zweiten while-schleife wird jetzt umgekehrt der nun    
// gefüllte next-stack nach demselbsen Prinzip abgearbeitet (kopieren der stacks mit
// current=next; würde mehr Laufzeit kosten). Wenn nach einer solchen iteration der 
// current-stack leer ist, sind wir fertig. Funkenflug kann optional am Ende        
// aufgerufen werden.                                                                
//////////////////////////////////////////////////////////////////////////////////////

void fire(int x,std::ostream &fireshape,bool statistik)
{
    int y;
    int generation=0;
    if(parent[x]==0){
	return;
    }
    //if(statistik){
	//findperimeter(x,fireshape);
    //}
    baumanzahl+=parent[find(x)];//Statistik führen
    cluster[-parent[find(x)]]--;//cluster aus der Statistik entfernen
    std::stack<int> current;
    std::stack<int> next;
    current.push(x);
    parent[x]=0;
    do{
	while(!current.empty())
	    {
		y=current.top();
		current.pop();
		if((y>L)&&(parent[y-L]!=0))
		    {
			next.push(y-L);
			parent[y-L]=0;
		    }
		if((y>L)&&(y%L!=1)&&(parent[y-L-1]!=0))
		    {
			next.push(y-L-1);
			parent[y-L-1]=0;
		    }
		if((y%L!=1)&&(parent[y-1]!=0))
		    {
			next.push(y-1);
			parent[y-1]=0;
		    }	
		if((y%L!=1)&&(y<(N-L))&&(parent[y-1+L]!=0))
		    {
			next.push(y-1+L);
			parent[y-1+L]=0;
		    }	
		if((y<(N-L))&&(parent[y+L]!=0))
		    {
			next.push(y+L);
			parent[y+L]=0;
		    }
		if((y<(N-L))&&(y%L!=0)&&(parent[y+L+1]!=0))
		    {
			next.push(y+L+1);
			parent[y+L+1]=0;
		    }
		if((y%L!=0)&&(parent[y+1]!=0))
		    {	
			next.push(y+1);
			parent[y+1]=0;
		    }
		if((y%L!=0)&&(y>L)&&(parent[y+1-L]!=0))
		    {	
			next.push(y+1-L);
			parent[y+1-L]=0;
		    }
	    }
	if(statistik){
	    generation++;
	    if(!next.empty()){
	    	generation++;
	    }
	}
	while(!next.empty())
	    {
		y=next.top();
		next.pop();
		if((y>L)&&(parent[y-L]!=0))
		    {
			current.push(y-L);
			parent[y-L]=0;
		    }
		if((y>L)&&(y%L!=1)&&(parent[y-L-1]!=0))
		    {
			current.push(y-L-1);
			parent[y-L-1]=0;
		    }
		if((y%L!=1)&&(parent[y-1]!=0))
		    {
			current.push(y-1);
			parent[y-1]=0;
		    }	
		if((y%L!=1)&&(y<(N-L))&&(parent[y-1+L]!=0))
		    {
			current.push(y-1+L);
			parent[y-1+L]=0;
		    }	
		if((y<(N-L))&&(parent[y+L]!=0))
		    {
			current.push(y+L);
			parent[y+L]=0;
		    }
		if((y<(N-L))&&(y%L!=0)&&(parent[y+L+1]!=0))
		    {
			current.push(y+L+1);
			parent[y+L+1]=0;
		    }
		if((y%L!=0)&&(parent[y+1]!=0))
		    {	
			current.push(y+1);
			parent[y+1]=0;
		    }
		if((y%L!=0)&&(y>L)&&(parent[y+1-L]!=0))
		    {	
			current.push(y+1-L);
			parent[y+1-L]=0;
		    }
	    }
    }while(!current.empty());
    if(statistik){
	feueranzahl++;
	feuergenerationen+=generation;}
    funkenflug(x,fireshape,statistik);//funkenflug :)
}


//////////////////////////////////////////////////////////////////////////////////////
// Snapshot aus Systemmitte wird in 'config'+t'.dat' geschrieben                    
//////////////////////////////////////////////////////////////////////////////////////

void print_snapshot(int t)
{
    std::ostringstream dateiname; 	
    std::string name;                   
    dateiname.str("");                  
    dateiname<<"conf"<<t<<".dat";			
    std::ofstream config(create_file(dateiname.str().c_str()).c_str(),std::ios::out); 
    if(L<1500)
	{
	    for(int i=1;i<N;i++)
		{
		    if(parent[i]!=0)
			{
			    config<<(i-1)%L+1<<"\t"<<(i-1)/L+1<<"\t"<<1<<std::endl;
			}else{
			config<<(i-1)%L+1<<"\t"<<(i-1)/L+1<<"\t"<<0<<std::endl;
		    }
		}
	    config.close();
	    return;
	}
    
    for(int j=(L-1000)/2;j<=(L+1000)/2;j++)
	{
	    for(int i=(L-1000)/2+L*j;i<=(L+1000)/2+L*j;i++)
		{
		    if(parent[i+1]!=0)
			{
			    config<<(i-1)%L+1<<"\t"<<(i-1)/L+1<<"\t"<<1<<std::endl;
			}else{
			config<<(i-1)%L+1<<"\t"<<(i-1)/L+1<<"\t"<<0<<std::endl;
		    }
		}
	    config.close();
	    
	}
}


//////////////////////////////////////////////////////////////////////////////////////
// Ausgabe der Clusterstatistik in 'clusterstat' +t '.dat'                         
//////////////////////////////////////////////////////////////////////////////////////

void print_clusterstat(int t)
{
    std::ostringstream dateiname;
    dateiname.str("");                         
    dateiname<<"clusterstat"<<t<<".dat";       
    std::ofstream clusterstat((create_file(dateiname.str().c_str())).c_str()); 
    for(int i=1;i<N;i++)
	{
	    if(cluster[i]!=0)
		{
		    clusterstat<<i<<"\t"<<cluster[i]<<std::endl;
		}
	}
    clusterstat.close();
}



//////////////////////////////////////////////////////////////////////////////////////
// Erstellt mit Hilfe der anderen Clusterstat. Methoden geometrische                
// Clusterstatistik. Wenn der cluster der x enthaelt den Rand berührt, betrachten wir
// ihn nicht. Falls doch werden alle Randfelder gesucht, also leere Felder, von     
// mindestens ein Nachbar auf root(x) zeigt. Diese werden markiert (=perimeter).   
// Dann wird identify_outerperimeter() aufgerufen, um den aeußeren Rand mit          
// outer_perimeter zu markieren, damit countarea(x) den Flaecheninhalt zaehlen kann.
// Die Werte werden danach ausgegeben.                                              
//////////////////////////////////////////////////////////////////////////////////////


void findperimeter(int x,std::ostream &fireshape)
{
    int root=find(x);
    int groesse=-parent[root];
    if(groesse<massstab){ //zu klein für unsere Statistik
	return;}
    if(groesse>massstab*1000){ //zu groß
	return;}
    
    for(int i=1;i<=L;i++)//Randcluster betrachten wir nicht - diese nur abbrennen !!
	{
	    if(find(i)==root){
		return;}
	}
    for(int i=L+1;i<=(N-L);i=i+L)
	{
	    if(find(i)==root){
		return;}
	}
    for(int i=L;i<N;i=i+L)
	{
	    if(find(i)==root){
		return;}
	}
    for(int i=(N-L);i<N;i++)
	{
	    if(find(i)==root){
		return;}
	}

    for(int i=1;i<N;i++)//Randfelder suchen und auf perimeter setzen
	{
	    if(parent[i]==0)
		{
		    if(i>L)
			{
			    if((find(i-L)==root))
				{
				    parent[i]=perimeter;
				    continue;
				}
			    if((i%L!=1)&&(find(i-1-L)==root))
				{
				    parent[i]=perimeter;
				    continue;
				}
			    if((i%L!=0)&&(find(i+1-L)==root))
				{
				    parent[i]=perimeter;
				    continue;
				}
			}
		    if((i%L!=1)&&(find(i-1)==root))
			{
			    parent[i]=perimeter;
			    continue;
			}
		    if((i%L!=0)&&(find(i+1)==root))
			{
			    parent[i]=perimeter;
			    continue;
			}
		    if(i<(N-L))
			{
			    if(find(i+L)==root)
				{
				    parent[i]=perimeter;
				    continue;
				}
			    if((i%L!=1)&&(find(i-1+L)==root))
				{
				    parent[i]=perimeter;
				    continue;
				}
			    if((i%L!=0)&&(find(i+1+L)==root))
				{
				    parent[i]=perimeter;
				    continue;
				}
			}
		}
	}
    
    identify_outerperimeter();//nach diesen beiden Schritten sind im Feld wieder nur
    int area=countarea(root); //die perimetersites markiert

    for(int i=1;i<N;i++)
	{	    
	    if(parent[i]==perimeter)//wenn Randfeld, zaehle Laenge!
		{
		    countperimeter(i,root);
		}
	} //nach dieser Schleife ist das Feld wieder "normal"


    int size=(int) perimeterverteilung.size();//Anzahl der inseln(+1 wegen aeußerem perimeter)
    int inner_perimeter=0;
    //std::cout<<"Die Inselanzahl ist "<<size-1<<std::endl;
    
    //Alle bis auf den letzten Eintrag im Stack sind innere Perimeter
    //->Berechne Summe der inneren Perimeterlaengen.

    for(int i=0;i<size-1;i++)
	{
	    inner_perimeter+=perimeterverteilung.top();
	    perimeterverteilung.pop();
	}
    
    
    if(groesse<2*massstab)
	{
	    fireshape<<"1"<<"\t";
	}else{
	if(groesse<10*massstab)
	    {
		fireshape<<"2"<<"\t";
	    }else{
	    if(groesse<20*massstab)
		{
		    fireshape<<"3"<<"\t";
		}else{
		if(groesse<100*massstab)
		    {
			fireshape<<"4"<<"\t";
		    }else{
		    fireshape<<"5"<<"\t";
		}
	    }
	}
    }
    //Clusterkategorie << total island area << shape index << edge index
    //<<median island size<< number of islands
    
    fireshape<<1-(double)groesse/area<<"\t"<<perimeterverteilung.top()/(2*sqrt(PI*area))<<"\t";
    fireshape<<(perimeterverteilung.top()+inner_perimeter)/(2*sqrt(PI*groesse))<<"\t";
    if((size-1)==0){
	 fireshape<<0<<"\t"<<0<<std::endl;
    }else{
	fireshape<<(double)((area-groesse)/(size-1))*20.0/massstab<<"\t";
	fireshape<<(double)(size-1)/(-parent[root])*5.0*massstab<<std::endl;
    }

    //Größe des clusters << äußerer Perimeter << innerer << anzahl inseln << area
    //fireshape<<-parent[root]<<"\t"<<perimeterverteilung.top()<<"\t";
    //fireshape<<inner_perimeter<<"\t"<<size-1<<"\t"<<area<<std::endl; //ALTE AUSGABE
    perimeterverteilung.pop();
}


//////////////////////////////////////////////////////////////////////////////////////
// Hilfsmethode für findperimeter. Geht von links oben angefangen durchs Feld und   
// sucht nach Perimeterfeldern (diese sind vorher markiert worden). Das erste das   
// gefunden wird, gehört auf jeden Fall zum aeußeren Perimeter. Dieser soll markiert 
// werden. Analog zu fire(int) werden stacks angelegt und der (eindeutig markierte) 
// perimeter "abgebrannt", d.h. auf outerperimeter gesetzt.                         
//////////////////////////////////////////////////////////////////////////////////////


void identify_outerperimeter()
{
    std::stack<int> current;
    std::stack<int> next;
    int i=1;
    while(parent[i]!=perimeter){
	i++;
    }
    current.push(i);
    parent[i]=outerperimeter;
    do{
	while(!current.empty())
	    {
		i=current.top();
		current.pop();
		if((i>L)&&(parent[i-L]==perimeter))
		    {
			next.push(i-L);
			parent[i-L]=outerperimeter;
		    }
		if((i>L)&&(i%L!=1)&&(parent[i-L-1]==perimeter))
		    {
			next.push(i-L-1);
			parent[i-L-1]=outerperimeter;
		    }
		if((i%L!=1)&&(parent[i-1]==perimeter))
		    {
			next.push(i-1);
			parent[i-1]=outerperimeter;
		    }	
		if((i%L!=1)&&(i<(N-L))&&(parent[i-1+L]==perimeter))
		    {
			next.push(i-1+L);
			parent[i-1+L]=outerperimeter;
		    }	
		if((i<(N-L))&&(parent[i+L]==perimeter))
		    {
			next.push(i+L);
			parent[i+L]=outerperimeter;
		    }
		if((i<(N-L))&&(i%L!=0)&&(parent[i+L+1]==perimeter))
		    {
			next.push(i+L+1);
			parent[i+L+1]=outerperimeter;
		    }
		if((i%L!=0)&&(parent[i+1]==perimeter))
		    {	
			next.push(i+1);
			parent[i+1]=outerperimeter;
		    }
		if((i%L!=0)&&(i>L)&&(parent[i+1-L]==perimeter))
		    {	
			next.push(i+1-L);
			parent[i+1-L]=outerperimeter;
		    }
	    }
	
	while(!next.empty())
	    {
		i=next.top();
		next.pop();
		if((i>L)&&(parent[i-L]==perimeter))
		    {
			current.push(i-L);
			parent[i-L]=outerperimeter;
		    }
		if((i>L)&&(i%L!=1)&&(parent[i-L-1]==perimeter))
		    {
			current.push(i-L-1);
			parent[i-L-1]=outerperimeter;
		    }
		if((i%L!=1)&&(parent[i-1]==perimeter))
		    {
			current.push(i-1);
			parent[i-1]=outerperimeter;
		    }	
		if((i%L!=1)&&(i<(N-L))&&(parent[i-1+L]==perimeter))
		    {
			current.push(i-1+L);
			parent[i-1+L]=outerperimeter;
		    }	
		if((i<(N-L))&&(parent[i+L]==perimeter))
		    {
			current.push(i+L);
			parent[i+L]=outerperimeter;
		    }
		if((i<(N-L))&&(i%L!=0)&&(parent[i+L+1]==perimeter))
		    {
			current.push(i+L+1);
			parent[i+L+1]=outerperimeter;
		    }
		if((i%L!=0)&&(parent[i+1]==perimeter))
		    {	
			current.push(i+1);
			parent[i+1]=outerperimeter;
		    }
		if((i%L!=0)&&(i>L)&&(parent[i+1-L]==perimeter))
		    {	
			current.push(i+1-L);
			parent[i+1-L]=outerperimeter;
		    }
	    }
    }while(!current.empty());
    //std::cout<<"Outer perimeter identified"<<std::endl;
}


////////////////////////////////////////////////////////////////////////////////////
// Zaehlt die Flaeche des clusters. Der aeußere Perimeter ist bereits entsprechend  
// markiert von identify_outerperimeter(). countarea() sucht nach der übergebenen   
// root. Wenn gefunden, sind wir im cluster. Jetzt laeuft wieder analog zu fire() das
// System mit den zwei stacks ab und "brennt" den cluster weg. Um keine             
// Informationen zu verlieren addieren wir immer den konstanten Wert temp=-2*N-1.   
// Da outerperimeter den cluster umschließt, kommen wir nie aus dem cluster heraus. 
// Nach dem zaehlen setzen wir die Feld wieder auf den Zustand vor Aufruf von       
// identify_outerperimeter zurück, d.h. die perimeter sind wieder ununterscheidbar. 
////////////////////////////////////////////////////////////////////////////////////

int countarea(int root)
{
    int i=1;
    int area=1;

    //Begebe dich irgendwo ins Innere des clusters
    while(find(i)!=root){
	i++;
    }
    std::stack<int> current;
    std::stack<int> next;
    current.push(i);
    parent[i]+=temp;
    do{
	while(!current.empty())
	    {
		i=current.top();
		current.pop();
		if(parent[i-L]>outerperimeter)
		    {
			next.push(i-L);
			parent[i-L]+=temp;
			area++;
		    }
		if(parent[i-L-1]>outerperimeter)
		    {
			next.push(i-L-1);
			parent[i-L-1]+=temp;
			area++;
		    }
		if(parent[i-1]>outerperimeter)
		    {
			next.push(i-1);
			parent[i-1]+=temp;
			area++;
		    }	
		if(parent[i-1+L]>outerperimeter)
		    {
			next.push(i-1+L);
			parent[i-1+L]+=temp;
			area++;
		    }	
		if(parent[i+L]>outerperimeter)
		    {
			next.push(i+L);
			parent[i+L]+=temp;
			area++;
		    }
		if(parent[i+L+1]>outerperimeter)
		    {
			next.push(i+L+1);
			parent[i+L+1]+=temp;
			area++;
		    }
		if(parent[i+1]>outerperimeter)
		    {	
			next.push(i+1);
			parent[i+1]+=temp;
			area++;
		    }
		if(parent[i+1-L]>outerperimeter)
		    {	
			next.push(i+1-L);
			parent[i+1-L]+=temp;
			area++;
		    }
	    }
	
	while(!next.empty())
	    {
		i=next.top();
		next.pop();
		if(parent[i-L]>outerperimeter)
		    {
			current.push(i-L);
			parent[i-L]+=temp;
			area++;
		    }
		if(parent[i-L-1]>outerperimeter)
		    {
			current.push(i-L-1);
			parent[i-L-1]+=temp;
			area++;
		    }
		if(parent[i-1]>outerperimeter)
		    {
			current.push(i-1);
			parent[i-1]+=temp;
			area++;
		    }	
		if(parent[i-1+L]>outerperimeter)
		    {
			current.push(i-1+L);
			parent[i-1+L]+=temp;
			area++;
		    }	
		if(parent[i+L]>outerperimeter)
		    {
			current.push(i+L);
			parent[i+L]+=temp;
			area++;
		    }
		if(parent[i+L+1]>outerperimeter)
		    {
			current.push(i+L+1);
			parent[i+L+1]+=temp;
			area++;
		    }
		if(parent[i+1]>outerperimeter)
		    {	
			current.push(i+1);
			parent[i+1]+=temp;
			area++;
		    }
		if(parent[i+1-L]>outerperimeter)
		    {	
			current.push(i+1-L);
			parent[i+1-L]+=temp;
			area++;
		    }
	    }
    }while(!current.empty());

    for(int i=1;i<N;i++) //Feld wieder reparieren.
	{
	    if(parent[i]==outerperimeter)
		{
		    parent[i]=perimeter;
		}
	    if(parent[i]<(-N-1))
		{
		    parent[i]-=temp;
		}
	}
    return area;
}


//////////////////////////////////////////////////////////////////////////////////////
// Bekommt von findperimeter einen index eines perimeterstücks und die root des     
// zu untersuchenden clusters übergeben. Mit dem stacksystem wird der gesamte       
// perimeter "abgebrannt". Die einzelnen Stücke müssen wir aber unterschiedlich     
// zaehlen, je nachdem, wie viele "Kanten" sie mit dem cluster haben. Die Zahl der  
// Kanten ermittelt jeweils die Hilfsmethode sumofneighbors(position,root). Die     
// resultierende Laenge wird auf den perimeterverteilung stack gelegt. Hierbei ist  
// klar, dass der erste perimeter der aeußere ist, da wir auf diesem immer zuerst   
// treffen.                                                                         
//////////////////////////////////////////////////////////////////////////////////////

void countperimeter(int x,int root)
{
    std::stack<int> current;
    std::stack<int> next;
    int perimeterlaenge=sumofneighbors(x,root);
    current.push(x);
    parent[x]=0;    
    do{
	while(!current.empty())
	    {
		x=current.top();
		current.pop();
		if((x>L)&&(parent[x-L]==perimeter))
		    {
			next.push(x-L);
			parent[x-L]=0;
			perimeterlaenge+=sumofneighbors(x-L,root);
		    }
		if((x>L)&&(x%L!=1)&&(parent[x-L-1]==perimeter))
		    {
			next.push(x-L-1);
			parent[x-L-1]=0;
			perimeterlaenge+=sumofneighbors(x-L-1,root);
		    }
		if((x%L!=1)&&(parent[x-1]==perimeter))
		    {
			next.push(x-1);
			parent[x-1]=0;
			perimeterlaenge+=sumofneighbors(x-1,root);
		    }	
		if((x%L!=1)&&(x<(N-L))&&(parent[x-1+L]==perimeter))
		    {
			next.push(x-1+L);
			parent[x-1+L]=0;
			perimeterlaenge+=sumofneighbors(x-1+L,root);
		    }	
		if((x<(N-L))&&(parent[x+L]==perimeter))
		    {
			next.push(x+L);
			parent[x+L]=0;
			perimeterlaenge+=sumofneighbors(x+L,root);
		    }
		if((x<(N-L))&&(x%L!=0)&&(parent[x+L+1]==perimeter))
		    {
			next.push(x+L+1);
			parent[x+L+1]=0;
			perimeterlaenge+=sumofneighbors(x+L+1,root);
		    }
		if((x%L!=0)&&(parent[x+1]==perimeter))
		    {	
			next.push(x+1);
			parent[x+1]=0;
			perimeterlaenge+=sumofneighbors(x+1,root);
		    }
		if((x%L!=0)&&(x>L)&&(parent[x+1-L]==perimeter))
		    {	
			next.push(x+1-L);
			parent[x+1-L]=0;
			perimeterlaenge+=sumofneighbors(x+1-L,root);
		    }
	    }
	
	while(!next.empty())
	    {
		x=next.top();
		next.pop();
		if((x>L)&&(parent[x-L]==perimeter))
		    {
			current.push(x-L);
			parent[x-L]=0;
			perimeterlaenge+=sumofneighbors(x-L,root);
		    }
		if((x>L)&&(x%L!=1)&&(parent[x-L-1]==perimeter))
		    {
			current.push(x-L-1);
			parent[x-L-1]=0;
			perimeterlaenge+=sumofneighbors(x-L-1,root);
		    }
		if((x%L!=1)&&(parent[x-1]==perimeter))
		    {
			current.push(x-1);
			parent[x-1]=0;
			perimeterlaenge+=sumofneighbors(x-1,root);
		    }	
		if((x%L!=1)&&(x<(N-L))&&(parent[x-1+L]==perimeter))
		    {
			current.push(x-1+L);
			parent[x-1+L]=0;
			perimeterlaenge+=sumofneighbors(x-1+L,root);
		    }	
		if((x<(N-L))&&(parent[x+L]==perimeter))
		    {
			current.push(x+L);
			parent[x+L]=0;
			perimeterlaenge+=sumofneighbors(x+L,root);
		    }
		if((x<(N-L))&&(x%L!=0)&&(parent[x+L+1]==perimeter))
		    {
			current.push(x+L+1);
			parent[x+L+1]=0;
			perimeterlaenge+=sumofneighbors(x+L+1,root);
		    }
		if((x%L!=0)&&(parent[x+1]==perimeter))
		    {	
			current.push(x+1);
			parent[x+1]=0;
			perimeterlaenge+=sumofneighbors(x+1,root);
		    }
		if((x%L!=0)&&(x>L)&&(parent[x+1-L]==perimeter))
		    {	
			current.push(x+1-L);
			parent[x+1-L]=0;
			perimeterlaenge+=sumofneighbors(x+1-L,root);
		    }
	    }
    }while(!current.empty());
    perimeterverteilung.push(perimeterlaenge);	    
}


//////////////////////////////////////////////////////////////////////////////////////
// Hilfsmethode für countperimeter(). Bestimmt die Anzahl der Kanten, die ein       
// perimeterstück mit dem cluster hat.                                              
//////////////////////////////////////////////////////////////////////////////////////

int sumofneighbors(int x,int root)
{
    int sum=0;
    if((x>L)&&(find(x-L)==root))
	{
	    sum++;
	}
    if((x<N-L)&&(find(x+L)==root))
	{
	    sum++;
	}
    if((x%L!=1)&&(find(x-1)==root))
	{
	    sum++;
	}
    if((x%L!=0)&&(find(x+1)==root))
	{
	    sum++;
	}
    return sum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Berechnet zunächst die fluglänge des Funnkens(abhänig von L_max_funkennflug), wobei die
// Wahrscheinlichkeit linear mit der Länge abnimmt. Jetzt wird eine Richtung 		
// gewürfelt in die der Funken fliegt. Ist die Stelle berechnet, wo der Funken landet	
// wird wieder fire aufgerufen um das Cluster abzubrennen.				
//////////////////////////////////////////////////////////////////////////////////////////

void funkenflug(int x,std::ostream &fireshape,bool statistik)
{
    int fluglaenge;
    int landepunkt;
    double a_1 =(double)rand()/(RAND_MAX+1.0);
    double a_2 =(double)rand()/(RAND_MAX+1.0);
    double richtung=(double)rand()/(RAND_MAX+1.0);

                                //Berechnung wie weit der Funken fliegt (linear)
    if(a_1>=a_2){		//a_1 muss kleiner als a_2 sein, damit ein linearer Zusammenhang entsteht,oder umgedreht
	fluglaenge=L_max_funkenflug*a_2; //Flugweite des Funkens (zwischen 1 und L_max)
    } 
    else{
	fluglaenge=L_max_funkenflug*a_1;
    }
    
    switch((int)(richtung*4)){
    case 0: //rechts
	landepunkt=x+fluglaenge;
	if((landepunkt-1)/L==(x-1)/L){
	    if(parent[landepunkt]!=0){
		funkenflugcount++;
		fire(landepunkt,fireshape,statistik);
	    }
	    return;
	}
	funkenflug(x,fireshape,statistik);
	return;
	
    case 1: //links
	landepunkt=x-fluglaenge;
	if((landepunkt-1)<0){
	    funkenflug(x,fireshape,statistik);return;
	}
	if((landepunkt-1)/L==(x-1)/L){
	    if(parent[landepunkt]!=0){
		funkenflugcount++;
		fire(landepunkt,fireshape,statistik);
	    }
	    return;
	}
	funkenflug(x,fireshape,statistik);
	return;
	
    case 2: //oben
	landepunkt=x-L*fluglaenge;
	if(((landepunkt)<N)&&((landepunkt)>0))
	    {
		if(parent[landepunkt]!=0)
		    {
			funkenflugcount++;
			fire(landepunkt,fireshape,statistik);
		    }
		return;
	    }
	funkenflug(x,fireshape,statistik);
	return;
    default: //unten
	landepunkt=x+L*fluglaenge;
	if(((landepunkt)<N)&&((landepunkt)>0)){
	    if(parent[landepunkt]!=0){
		funkenflugcount++;
		fire(landepunkt,fireshape,statistik);
	    }
	    return;
	}
	funkenflug(x,fireshape,statistik);
    }
}
