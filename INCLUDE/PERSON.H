/*
 Un personaje tiene un Nivel de pelea de
 acuerdo a su nivel varian sus condiciones de pelea, cada
 500 puntos de experiencia eleva el nivel y los puntos de exp
 vuelven a ser 0.

 Nota: Todas las siguientes variables cambian de acuerdo al nivel
       de pelea o tambien depende del tipo de armadura, arma o
       amuleto que esten usando.

 HP= Cantidad de vida del personaje si es 0 el personaje
     no puede seguir peleando.
 MP= Cantidad de Magia que puede gastar (Cada tipo de magia cuesta
     cierta cantidad de MP despues de su Uso si el MP es 0 no
     puede seguir utilizando ataques Magicos).
 DEFENSA=Cantidad que le resta un ataque de otro personaje, Ej:

	   ATAQUE     DEFENSA     ATAQUE TOTAL
	     45         5             40
 ATAQUE=Cantidad neta que el personaje puede atacar a otro.
 MAGIA=Cantidad que sumada con la cantidad de ataque de un poder magico
     genera un ataque magico total,Ej:

	    MAGIA  TIPODEMAGIA        ATAQUE TOTAL
	      20        36                 56

 DEF=Porcentaje que se aumente ala DEFENS dependiendo de la armadura.
 ATK=Porcentaje que se aumenta al ATAQUE dependiendo del arma que use.
 MAG=Porcentaje que se aumenta ala MAGIA dependiendo del amuleto,EJ

	    MAGIA      MAG        ATK MAGICO DEL PERSONAJE
	     20         0.4                 28

			     20 * 0.4 = 8
			     20 + 8 = 28

 Nota: Todas las anterior variables dependiendian de un u otra cosa
       pero todas estan sometidas al destino, el destino puede quitar o
       aumentar  un ataque,un ataque magico o la defensa.

       DESTINO PARA UN ATAQUE o ATAQUE MAGICO

       -5%  +2%  +4%  +6%   +8%  +10%  neto

       DESTINO PARA LA DEFENSA
       -10%   neto   +10%

 TABLA GENERAL

 Esta es la tabla de un Personaje que esta en un nivel 4.
 ----------------------------------------
 EXP   212
 NIVEL 4        | CANTIDAD
 -----------------------------------------
 | HP     MP    | DEFENSA  ATAQUE  MAGIA |
 | 140    700   |    5       35       20 |
 ----------------------------------------
		  DEF       ATK      MAG
 PORCENTAJE     | 0.2       0.5      0.4

*/
struct MonDat{
	char *nombre;
	int tam;
};
struct EnemDat{
       int hp,mp,nivel,defensa,ataque,magia;
       int arr_magia[5];
       float def,atk,mag;
};
class Personaje{
public:  char *nombre;
	 int hp,mp,nivel,defensa,ataque,magia;
	 int arr_magia[5];
	 int hpN,mpN;
	 float def,atk,mag,escala1,escala2;
	 int golpe;
	 int vivo;

	 Personaje(EnemDat &e);
	 Personaje(){};
	 ~Personaje(){
		cout<<"Se destruyo el objeto Personaje exitosamente!\n";
	 }
	 int atacar(void);
	 int magiatk(int n);
	 float dado6(int n);
	 float dado3(float n);
	 int defender(int n);
	 void convertir(int n,char *numero);
};
//llena los datos del personaje
Personaje::Personaje(EnemDat &e){
	 hp=e.hp;
	 mp=e.mp;
	 if(hp<100) hp=100;
	 if(mp<100) mp=100;
	 nivel=e.nivel;
	 defensa=e.defensa;
	 ataque=e.ataque;
	 magia=e.magia;
	 def=e.def;
	 atk=e.atk;
	 mag=e.mag;
	 hpN=hp;
	 mpN=mp;
	 escala1=hpN/33;
	 escala2=mpN/33;
	 for(int i=0;i<5;i++) arr_magia[i]=e.arr_magia[i];
	 vivo=1;
}
//Define el destino de un ataque y retorna un valor aleatorio
float Personaje::dado6(int n){
      float aux=0;
	  switch(random(7)){
	   case 0:   aux=n*-0.05; break;
	   case 1:   aux=n*0.02; break;
	   case 2:   aux=n*0.04; break;
	   case 3:   aux=n*0.06; break;
	   case 4:   aux=n*0.08; break;
	   case 5:   aux=n*0.1; break;
	  }
	  return(n+aux);
}
//Define el destino de la defensa y retorna el valor aleatorio
float Personaje::dado3(float n){
     float aux=0;
	  switch(random(3)){
	   case 1:   aux=n*0.1; break;
	   case 2:   aux=(n*0.1)*-1; break;
	  }
	  return(n+aux);
}
//Devuelve la cantidad real que el personaje ataca
int Personaje::atacar(){
     int aux=0;
     aux=(int) (ataque*atk);
     aux=aux+ataque;
     return((int) dado6(aux));
}
//Devuelve la catidad real del ataque magico
int Personaje::magiatk(int n){
     int aux=0;
     aux=(int) (magia*mag);
     aux=aux+magia;
     aux=aux+n;
     return((int) dado6(aux));
}
//recive el ataque de otro personaje y se defiende
//devuelve la catidad real que se mermo de su hp
int Personaje::defender(int n){
     float aux=n;
     aux=aux-(defensa+(defensa*def));
     aux=(int) dado3(aux);
     if(aux<hp) hp=hp-aux;
     else {hp=0;
	   vivo=0;
     }
     return(aux);
}
//Convierte un valor menor de 10000 a String
void Personaje::convertir(int n,char *numero){

  int a,b,c;
  int ndigitos=0;

  int numero_partido[4];
    if((n<=999)&&(n>99)){
		 a=n/100;
		 numero_partido[0]=a;
		 b=(n%100)/10;
		 numero_partido[1]=b;
		 numero_partido[2]=(n%100)%10;
		 ndigitos=3;
		 }
    if((n<=9999)&&(n>999)){
		 a=n/1000;
		 numero_partido[0]=a;
		 b=(n%1000)/100;
		 numero_partido[1]=b;
		 c=((n%1000)%100)/10;
		 numero_partido[2]=c;
		 numero_partido[3]=((n%1000)%100)%10;
		 ndigitos=4;
    }
    if((n<=99)&&(n>9)){
		 a=n/10;
		 numero_partido[0]=a;
		 numero_partido[1]=n%10;
		 ndigitos=2;
	}
    if((n<=9)&&(n>=0)) {numero_partido[0]=n; ndigitos=1;}

   for(int i=0;i<4;i++)
   {
       switch(numero_partido[i])
       {
	   case 1: if(i<ndigitos) numero[i]='1';
		   else numero[i]='*';
		   break;
	   case 2: if(i<ndigitos) numero[i]='2';
		   else numero[i]='*';
		   break;
	   case 3: if(i<ndigitos) numero[i]='3';
		   else numero[i]='*';
		   break;
	   case 4: if(i<ndigitos) numero[i]='4';
		   else numero[i]='*';
		   break;
	   case 5: if(i<ndigitos) numero[i]='5';
		   else numero[i]='*';
		   break;
	   case 6: if(i<ndigitos) numero[i]='6';
		   else numero[i]='*';
		   break;
	   case 7: if(i<ndigitos) numero[i]='7';
		   else numero[i]='*';
		   break;
	   case 8: if(i<ndigitos) numero[i]='8';
		   else numero[i]='*';
		   break;
	   case 9: if(i<ndigitos) numero[i]='9';
		   else numero[i]='*';
		   break;
	   case 0: if(i<ndigitos) numero[i]='0';
		   else numero[i]='*';
		   break;
	   default: numero[i]='*';
       }
   }
	numero[4]='\0';
}