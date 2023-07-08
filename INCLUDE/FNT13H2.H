//----------------------------------------------------------------------------
//	GISLERSOFT (R)
//	Programado por: Gisler Garc‚s Vargas
//	Esta version 2003
//
//	Para poder utilizar esta libreria antes debe incluirse como cabecera
//	la libreria vga13h.h
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>



//----------------------------------------------------------------------------
//	Matriz que contiene todas los caracteres a representar de 3x5 pixeles
//----------------------------------------------------------------------------
int matriz[3*235]={
0,1,0,1,0,1,1,1,1,1,0,1,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,0,0,1,1,1,0,0,1,0,0,
1,0,0,0,1,1,1,1,0,1,0,1,1,0,1,1,0,1,1,1,0,1,1,1,1,0,0,1,1,0,1,0,0,1,1,1,1,1,1,
1,0,0,1,1,0,1,0,0,1,0,0,1,1,1,1,0,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,1,1,1,0,1,
1,0,1,1,1,1,0,1,0,0,1,0,0,1,0,1,1,1,1,1,1,0,1,0,0,1,0,0,1,0,1,1,0,1,0,1,1,0,1,
1,1,0,1,0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,0,0,1,1,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,
1,1,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,1,0,1,0,1,0,1,1,0,1,0,1,1,1,0,
1,0,0,1,0,0,0,1,0,1,0,1,1,0,1,1,1,0,0,0,1,1,1,0,1,0,1,1,1,0,1,0,1,1,0,1,0,1,1,
1,0,0,0,1,0,0,0,1,1,1,0,1,1,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,1,1,0,1,1,0,1,1,0,1,
1,1,1,1,0,1,1,0,1,1,0,1,1,0,1,0,1,0,1,0,1,1,0,1,1,0,1,1,1,1,1,0,1,1,0,1,1,0,1,
0,1,0,1,0,1,1,0,1,1,0,1,1,0,1,0,1,0,0,1,0,0,1,0,1,1,1,0,0,1,0,1,0,1,0,0,1,1,1,
0,0,0,0,1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,1,1,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,1,0,0,1,0,0,0,1,0,
1,0,1,1,1,1,1,0,1,0,1,0,0,1,0,1,1,0,0,1,0,0,1,0,1,1,1,1,1,0,0,0,1,0,1,0,1,0,0,
1,1,1,1,1,0,0,0,1,1,1,0,0,0,1,1,1,0,1,0,1,1,0,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,0,
1,1,1,0,0,1,1,1,1,0,1,1,1,0,0,1,1,1,1,0,1,1,1,1,1,1,1,0,0,1,0,1,0,0,1,0,0,1,0,
1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1,0,1,0,1,0,1,1,0,0,1,0,0,1,0,0,0,0,1,0,0,0,0,
0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,1,1,0,0,0,1,1,1,1,0,1,1,0,1,1,1,0,
0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0
};

//----------------------------------------------------------------------------
//	Clase Font para el manejo de las fuentes en modo13h
//----------------------------------------------------------------------------
class Font{
public:

	void showletter(int f);
	void putletter(unsigned char far *px,int x,int y,int f,Graph &h,int col);
	void cputletter(unsigned char far *px,int x,int y,int f,Graph &h,int tx);
	void putstr(unsigned char far *px,int x,int y,char *s,Graph &h,int tx,int col);
	void toString(int n,char *numero);

	Font();
	~Font();
private:

	int subrayar;
	unsigned char textura[3][5];
	unsigned char textura2[3][5];
	unsigned char textura3[3][5];

};
Font::Font(){
	subrayar=0;
	for(int i=0;i<3;i++) textura[i][0]=82;
	for(i=0;i<3;i++) textura[i][1]=83;
	for(i=0;i<3;i++) textura[i][2]=84;
	for(i=0;i<3;i++) textura[i][3]=83;
	for(i=0;i<3;i++) textura[i][4]=82;

	for(i=0;i<3;i++) textura2[i][0]=22;
	for(i=0;i<3;i++) textura2[i][1]=190;
	for(i=0;i<3;i++) textura2[i][2]=127;
	for(i=0;i<3;i++) textura2[i][3]=190;
	for(i=0;i<3;i++) textura2[i][4]=22;

	for(i=0;i<3;i++) textura3[i][0]=186;
	for(i=0;i<3;i++) textura3[i][1]=141;
	for(i=0;i<3;i++) textura3[i][2]=140;
	for(i=0;i<3;i++) textura3[i][3]=235;//235
	for(i=0;i<3;i++) textura3[i][4]=186; //42
}
Font::~Font(){
}
//----------------------------------------------------------------------------
//	Muestra la letra especificada en f en el modo texto
//----------------------------------------------------------------------------
void Font::showletter(int f){
     f=f*5;
     for(int j=f;j<f+5;j++)
     {
      for(int i=0;i<3;i++) cout<<matriz[i+(j*3)];
      cout<<"\n";
     }
}
//----------------------------------------------------------------------------
//	Imprime en la px la letra f especificada en f en la posicion
//	x,y de color "col".(Se debe pasar el objeto Graph como referencia)
//----------------------------------------------------------------------------
void Font::putletter(unsigned char far *px,int x,int y,int f,Graph &h,int col)
{
	f=f*5;
  for(int j=f;j<f+5;j++)
    for(int i=0;i<3;i++)
			 {
	if(matriz[i+(j*3)]==1) h.putpixel(px,x+i,y+(j-f),col);
       }
}
//----------------------------------------------------------------------------
//	Imprime en la px la letra f en la posicion x,y con la textura tx
//----------------------------------------------------------------------------
void Font::cputletter(unsigned char far *px,int x,int y,int f,Graph &h,int tx)
{
  f=f*5;
  for(int j=f;j<f+5;j++)
    for(int i=0;i<3;i++)
       {
	if(matriz[i+(j*3)]==1)
	switch(tx){
	case 1:h.putpixel(px,x+i,y+(j-f),textura[i][j-f]);break;
	case 2:h.putpixel(px,x+i,y+(j-f),textura2[i][j-f]);break;
	case 3:h.putpixel(px,x+i,y+(j-f),textura3[i][j-f]);break;
	default:h.putpixel(px,x+i,y+(j-f),15);
	}
       }
}
//----------------------------------------------------------------------------
//	Imprime en la px, desde la posicion x,y la cadena de caracteres p
//	"s" con textura "tx" de color "col".
//
//	Ejemplo:
//
//	cadena: HOLA*MUNDO*GRAFICO!
//	imprime: HOLA MUNDO GRAFICO!
//
//	cadena: HOLA+MUNDO+GRAFICO!
//	imprime: HOLA MUNDO GRAFICO! (MUNDO QUEDA DE OTRO COLOR)
//
//	cadena: 5[2*:MUNDO?
//	imprime: 5+2 :MUNDO?
//	
//----------------------------------------------------------------------------
void Font::putstr(unsigned char far *px,int x,int y,char *s,Graph &h,int tx,int col)
{
    int f,i=0;

    while(s[i]!='\0')
    {
			 f= (int) s[i];
       f= f-65;
       if(f<0){
		f=(int) s[i];
		switch(f){
		case 42: f=46;break;
		case 43: f=46;
			 if(subrayar==0) subrayar=1;
			 else subrayar=0;
			 break;
		case 33: f=45;break;
		case 63: f=44;break;
		default: f=f-17;
		}
	      }
       if(subrayar==0){
       if(tx==0) putletter(px,x,y,f,h,col);
			 else cputletter(px,x,y,f,h,tx);
       }
       else cputletter(px,x,y,f,h,2);
       i++;
	   x=x+4;
	}
}
//----------------------------------------------------------------------------
//	Combierte el entero n a una cadena de caracteres numero, se debe
//	tener en cuenta que el tamaño de la cadena numero debe ser como minimo
//	de 5 posiciones.
//
//	ejemplo:
//
//	char cadena[5];
//
//	toString(1235,cadena);
//----------------------------------------------------------------------------
void Font::toString(int n,char *numero){

  int a,b,c;
  int numero_partido[4];

	if(n>=0 && n<=9){
		numero_partido[0]=n;
		numero_partido[1]=256;
		for(int i=0;i<2;i++){
			switch(numero_partido[i]){
				case 1: numero[i]='1';break;
				case 2: numero[i]='2';break;
				case 3: numero[i]='3';break;
				case 4: numero[i]='4';break;
				case 5: numero[i]='5';break;
				case 6: numero[i]='6';break;
				case 7: numero[i]='7';break;
				case 8: numero[i]='8';break;
				case 9: numero[i]='9';break;
				case 0: numero[i]='0';break;
				default: numero[i]='\0';
			}
		 }
	}
	if((n<=99)&&(n>9)){
		 a=n/10;
		 numero_partido[0]=a;
		 numero_partido[1]=n%10;
		 numero_partido[2]=256;//Para decirle que hay termina la cadena
		 for(int i=0;i<3;i++){
			switch(numero_partido[i]){
				case 1: numero[i]='1';break;
				case 2: numero[i]='2';break;
				case 3: numero[i]='3';break;
				case 4: numero[i]='4';break;
				case 5: numero[i]='5';break;
				case 6: numero[i]='6';break;
				case 7: numero[i]='7';break;
				case 8: numero[i]='8';break;
				case 9: numero[i]='9';break;
				case 0: numero[i]='0';break;
				default: numero[i]='\0';
			}
		 }
	}
	if((n<=999)&&(n>99)){
		 a=n/100;
		 numero_partido[0]=a;
		 b=(n%100)/10;
		 numero_partido[1]=b;
		 numero_partido[2]=(n%100)%10;
		 numero_partido[3]=256;
		 for(int i=0;i<4;i++){
			switch(numero_partido[i]){
				case 1: numero[i]='1';break;
				case 2: numero[i]='2';break;
				case 3: numero[i]='3';break;
				case 4: numero[i]='4';break;
				case 5: numero[i]='5';break;
				case 6: numero[i]='6';break;
				case 7: numero[i]='7';break;
				case 8: numero[i]='8';break;
				case 9: numero[i]='9';break;
				case 0: numero[i]='0';break;
				default: numero[i]='\0';
			}
		 }
   }
   if((n<=9999)&&(n>999)){
		 a=n/1000;
		 numero_partido[0]=a;
		 b=(n%1000)/100;
		 numero_partido[1]=b;
		 c=((n%1000)%100)/10;
		 numero_partido[2]=c;
		 numero_partido[3]=((n%1000)%100)%10;
		 numero_partido[4]=256;
		 for(int i=0;i<5;i++){
			switch(numero_partido[i]){
				case 1: numero[i]='1';break;
				case 2: numero[i]='2';break;
				case 3: numero[i]='3';break;
				case 4: numero[i]='4';break;
				case 5: numero[i]='5';break;
				case 6: numero[i]='6';break;
				case 7: numero[i]='7';break;
				case 8: numero[i]='8';break;
				case 9: numero[i]='9';break;
				case 0: numero[i]='0';break;
				default: numero[i]='\0';
			}
		 }
	 }
}




