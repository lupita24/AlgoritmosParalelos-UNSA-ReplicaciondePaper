#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <thread>
#include <sys/time.h>

using namespace std;

#define MAX 100000000  //maximo numero de vértices

int V , E;      //numero de vertices y aristas
int intervalo;
int pos;
const int nHilos = 16;//numero de hilos que se usan para detectar los ciclos
thread hilos[nHilos];//vector de los hilos 

int aH=0;//numero de ciclos detectados por helper threads
int aM=0;//numero de ciclos detectados por main thread

///UNION-FIND
int padre[ MAX ]; //Este arreglo contiene el padre del i-esimo nodo

//Método de inicialización
void MakeSet( int n ){
    for( int i = 1 ; i <= n ; ++i ) padre[ i ] = i;
}

//Método para encontrar la raiz del vértice actual X
int Find( int x ){
    return ( x == padre[ x ] ) ? x : padre[ x ] = Find( padre[ x ] );
}

//Método para unir 2 componentes conexas
void Union( int x , int y ){
    padre[ Find( x ) ] = Find( y );
}

//Método que me determina si 2 vértices estan o no en la misma componente conexa
bool sameComponent( int x , int y ){
    if( Find( x ) == Find( y ) ) return true;
    return false;
}
///FIN UNION-FIND


//Estructura arista( edge )
struct Edge{
    int origen;     //Vértice origen
    int destino;    //Vértice destino
    int peso;       //Peso entre el vértice origen y destino
    bool ciclo;
    Edge(){}
    //Comparador por peso, me servira al momento de ordenar lo realizara en orden ascendente
    //Cambiar signo a > para obtener el arbol de expansion maxima
    bool operator<( const Edge &e ) const {
        return peso < e.peso;
    }
}arista[ MAX ];      //Arreglo de aristas para el uso en kruskal
Edge MST[ MAX ];     //Arreglo de aristas del MST encontrado


//funcion que realizan los helper threads para evaluar los ciclos
void ejecutar(int ini, int fin){
	while(1){
		if(pos > fin-10) break;
		for(int i = ini; i < fin;++i){
			if(arista[i].ciclo) continue;
			if(sameComponent(arista[i].origen, arista[i].destino)){
				arista[i].ciclo = true;	
                aH++;
            }
		}
	}
}

void Kruskal(){
    int origen , destino , peso;
    int total = 0;          //Peso total del MST
    int numAristas = 0;     //Numero de Aristas del MST

    cout << "# de Hilos: "<<nHilos << endl;

    MakeSet( V );           //Inicializamos cada componente
    std::sort( arista , arista + E );    //Ordenamos las aristas por su comparador

    cout<< "Se ordeno todo..."<<endl;

    intervalo = E / (nHilos + 1);//realiza la particion de datos para evaluarlos con los hilos ayudantes
    //condicion: si el numero de hilos es igual a 0 no entra caso contrario si lo hace
    if(nHilos > 0){
        for(int i=1; i <= nHilos; ++i ){
        	cout << i*intervalo << " " << (i+1)*intervalo << endl;
        	hilos[i-1] = thread(ejecutar,(i*intervalo), ((i+1)*intervalo));
        }
    }

    time_t iniT, finT;//variables para medir el tiempo
    struct timeval  tv;//estructura para medir el tiempo

    gettimeofday(&tv, NULL);//funcion para emdir el tiempo
    long long ms = tv.tv_sec*1000L + tv.tv_usec/1000;
    time(&iniT); //funcion que mide el inicio de la funcion
    for( pos = 0 ; pos < E ; ++pos ){     //Recorremos las aristas ya ordenadas por peso
        //Verificamos si estan o no en la misma componente conexa
        if(arista[pos].ciclo) continue;
  
        origen = arista[ pos ].origen;    //Vértice origen de la arista actual
	    destino = arista[ pos ].destino;  //Vértice destino de la arista actual
  	    peso = arista[ pos ].peso;        //Peso de la arista actual
  
        if( !sameComponent( origen , destino ) ){  //Evito ciclos 
            total += peso;              //Incremento el peso total del MST
            MST[ numAristas++ ] = arista[ pos ];  //Agrego al MST la arista actual
            Union( origen , destino );  //Union de ambas componentes en una sola
            //if( V - 1 == numAristas ) break;
        }
        else {
            aM++;
            arista[pos].ciclo = true;
        }
    }
    time(&finT); //Funcion que mide tiempo de fin de ejecucion
    gettimeofday(&tv, NULL);//funcion que mide tiempo
    ms = (tv.tv_sec*1000L + tv.tv_usec/1000) - ms;//tiempo de demora en milisegundos

    double se = difftime(finT, iniT);//tiempo de demora en segundos
    cout << "Tiempo de ejecucion(s): " << se <<endl;
    cout << "Tiempo de ejecucion(ms): " << ms <<endl;

    //Si el MST encontrado no posee todos los vértices mostramos mensaje de error
    //Para saber si contiene o no todos los vértices basta con que el numero
    //de aristas sea igual al numero de vertices - 1
    if( V - 1 != numAristas ){
        puts("No existe MST valido para el grafo ingresado, el grafo debe ser conexo.");
        return;
    }

    printf( "El costo minimo de todas las aristas del MST es : %d\n" , total );

    if(nHilos > 0){for(int i=0; i < nHilos; ++i )    hilos[i].detach();}
    
}

int main(){

    int mst;//costo de arbol de expansion minima

    ifstream lee("archivo_prueba.txt");//leemos el grafo de un archivo

    string p, q;//variables para omitir datos en el grafo
    int ini, fin, peso;//variables para crear el grafo
    
    for(int i=0;i<7;++i) getline(lee,p);//leemos info que no sirve
    lee >>  p >> q >> V >> E ;//lee los valores para el grafo, p y q no sirve 

    for(int i = 0;i < E; ++i){ //lee os datos para ingresar al grafo
        lee >>  p >> ini >> fin >> peso ;
        arista[ i ].origen = ini-1;
        arista[ i ].destino = fin-1;
        arista[ i ].peso = peso + 1;//se le aumenta unno porque el generador de grafos colcoa a las aristas desde 0
        arista[ i ].ciclo = false;

    }
    lee.close();

    cout << "Grafo cargado" <<endl;

    time_t iniT, finT;//declaro las variables para medir el tiempo
    time(&iniT); 
    Kruskal();
    time(&finT); 

    double se = difftime(finT, iniT);

    cout << "Tiempo con ordenamiento: " << se <<endl;
    
    cout << "Vertices: "<<V << " Aristas: " <<  E << endl;
    cout << "Hilo Principal(# ciclos detectados): " << aM<< endl;
    cout << "Hilos Secundarios(# ciclos detectados):" << aH <<endl;
    
    


    return 0;
}