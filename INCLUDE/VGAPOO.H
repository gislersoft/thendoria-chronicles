//Programado por Gisler Garcs Vargas
//Junio 2002
//Libreria para usar el modo 13h
#include <dos.h>
#include <alloc.h>
#include <math.h>
#include <mem.h>
#include <iostream.h>
#include <stdio.h>
/*
Se crea un Objeto de tipo graphics
asi:  Graphics g(x);
-donde si x=1 entonces se crean dos buffer adicionales para utilizar
dos pantallas virtuales adicionales llamadas pvm(pv mascara) y pvf(pv frontal).
-Si x=0 entonces solo utilizaresmos las pantallas virtuales pv1 y pv2, teniendo en
cuenta que cuando cargamos un pcx con loadPCX siempre se carga sobre la pv2.
*/

#define GRAFICO 0X0013
#define TEXTO 0X0003

class Graph{
/*Los OBJETOS creados de esta clase necesitan mas o menos 256 k en heap sin contar
otros Bitmaps*/
public:
     unsigned char far *vga;
     unsigned char far *pv1;
     unsigned char far *pv2;
     unsigned char far *pvm;
     unsigned char far *pvf;
     int utilizar_pvm_pvf;

     Graph(int p);
     ~Graph();

     void modo_video(int modo);
     void volcar(unsigned char far *Pdes,unsigned char far *Pori);
     void volcarT(unsigned char far *Pdes,unsigned char far *Pori);
     unsigned char loadPCX( char *nombre );
     unsigned char loadPCXmask(char *nombre);//Carga sobre la pvm una mascara
     int status(void);
     void destruir(void);
     void pal(void);
     void clr(unsigned char far *px,unsigned char color);
     void getframe(unsigned char far *px,int x,int y,int a,unsigned char far *img);
     void putframe(unsigned char far *px,int x,int y,int a,unsigned char far *img,unsigned char colort);
     //Primitivas
     void putpixel(unsigned char far *px,int x,int y,unsigned char color);
	 void circle( unsigned char far *px,int x,int y,int r, unsigned char color);
	 void ball(unsigned char far *px,int x,int y,int r,unsigned char color);
	 void line(unsigned char far *px,int a,int b,int c,int d,unsigned char col);
	 void rayos(int a,int b,int c,int d,int xs[],int ys[]);
	 void linep(int a,int b,int c,int d,int &x,int &y,int &l);
	 void hline(unsigned char far *px,int x1, int x2, int y,unsigned char color );
	 void vline(unsigned char far *px,int x, int y1, int y2,unsigned char color );
	 void fillbox(unsigned char far *px,int x1, int y1, int x2, int y2,unsigned char color );
	 void box(unsigned char far *px,int x1, int y1, int x2, int y2,unsigned char color );
	 unsigned char getpixel(unsigned char far *px,int x,int y);
	 void wait_retrace();//Espera el retrazado vertical
	 void pal2();
	 void copiar();
	 void fadeout(int demora);
	 void fade12(int demora);
	 void fade21(int demora);
	 void clrpal2();

private:

	 unsigned long ancho;
	 unsigned long alto;
	 unsigned char paleta[256][3];
	 unsigned char paleta2[256][3];

	 int sgn(long a);
	 int round(double a);
	 void correr_paleta();
	 void putpal(unsigned char rojo,unsigned char verde,unsigned char azul);

};


Graph::Graph(int p){
	 utilizar_pvm_pvf=p;
	 vga=(unsigned char far*) MK_FP(0xA000,0);
	 pv1=(unsigned char far*) farmalloc(64000L);
	 pv2=(unsigned char far*) farmalloc(64000L);
	 if(utilizar_pvm_pvf==1){
	pvm=(unsigned char far*) farmalloc(64000L);
	pvf=(unsigned char far*) farmalloc(64000L);
	 }
	 alto=ancho=0;
}
Graph::~Graph(){
}
void Graph::destruir(){
	 farfree(pv1);
	 farfree(pv2);
	 farfree(vga);
	 if(utilizar_pvm_pvf==1){
		 farfree(pvm);
		 farfree(pvf);
	 }
}


int Graph::status(){
	  if(vga==NULL) return(0);
		if(pv1==NULL) return(0);
		if(pv2==NULL) return(0);
	  if(utilizar_pvm_pvf==1){
		if(pvm==NULL) return(0);
		if(pvf==NULL) return(0);
	  }
	  return(1);
}


void Graph::modo_video(int modo){
	_AX=modo;
	geninterrupt(0x10);
}

void Graph::putpixel(unsigned char far *px,int x,int y,unsigned char color){
	   if((x>=0)&&(x<320))
	 if((y>=0)&&(y<200))
		_fmemset(px+x+(y<<8)+(y<<6),color,1L);
}
unsigned char Graph::getpixel(unsigned char far *px,int x,int y){
	return(*(px+x+(y<<8)+(y<<6)));
}
void Graph::volcar(unsigned char far *Pdes,unsigned char far *Pori){
	_fmemcpy(Pdes,Pori,64000L);
}
void Graph::volcarT(unsigned char far *Pdes,unsigned char far *Pori){
	   for(long i=0;i<64000;i++)
	   if(Pori[i]!=0) Pdes[i]=Pori[i];
}
void Graph::getframe(unsigned char far *px,int x,int y,int a,unsigned char far *img){
	 for(int j=0;j<a;j++)
	   for(int i=0;i<a;i++)
	   img[i+(j*a)]=getpixel(px,x+i,y+j);

}

void Graph::putframe(unsigned char far *px,int x,int y,int a,unsigned char  far *img,unsigned char colort){
  for(int j=0;j<a;j++)
	   for(int i=0;i<a;i++)
		 if(img[i+(j*a)]!=colort){
		 putpixel(px,x+i,y+j,img[i+(j*a)]);
		 }

}
void Graph::clr(unsigned char far *px,unsigned char color){
	_fmemset(px,color,64000L);
}

void Graph::wait_retrace(){
   asm mov dx,0x3DA
l1:
   asm in al,dx
	 asm and al,0x08
   asm jnz l1
l2:
   asm in al,dx
	 asm and al,0x08
   asm jz  l2
}

void Graph::circle(unsigned char far *px,int x,int y,int r, unsigned char color)
	 {
	   float grad=0;
	   int c, f;
	 do
	 {
	   c=round(r*cos(grad));
	   f=round(r*sin(grad));
	   putpixel(px, c+x, f+y, color );
	   grad=grad+0.005;
	 }while (!(grad>6.4));
     }
void Graph::ball(unsigned char far *px,int x,int y,int r,unsigned char color){
     for(int i=0;i<r;i++)
		 {
       circle(px,x,y,i,color);
     }
}
int Graph::sgn (long a)
{
   if (a > 0) return +1;
   else if (a < 0) return -1;
	else return 0;
}

int Graph::round (double a)
{
 int temp2;

 temp2 = (int)a;

 if ( (a - temp2) < 0.5) return temp2;
 else return (temp2 + 1);
}


void Graph::line (unsigned char far *px,int a, int b, int c, int d, unsigned char col)

{
 long u,s,v,d1x,d1y,d2x,d2y,m,n;
 int i;

 u = c-a;
 v= d-b;
 d1x = sgn(u);
 d1y = sgn(v);
 d2x = sgn(u);
 d2y = 0;
 m= abs(u);
 n = abs(v);

 if (m<=n)
 {
  d2x = 0;
  d2y = sgn(v);
  m = abs(v);
  n= abs(u);
 }

 s = (int)(m / 2);

 for (i=0;i<=round(m);i++)
 {
  putpixel(px,a,b,col);
  s += n;

  if (s >= m)
	{
   s -= m;
	 a += d1x;
   b += d1y;
  }
	else
  {
   a += d2x;
   b += d2y;
  }
 }
}

void Graph::rayos(int a, int b, int c, int d,int xs[],int ys[])

{
 long u,s,v,d1x,d1y,d2x,d2y,m,n;
 int i,j=0;

 u = c-a;
 v= d-b;
 d1x = sgn(u);
 d1y = sgn(v);
 d2x = sgn(u);
 d2y = 0;
 m= abs(u);
 n = abs(v);

 if (m<=n)
 {
  d2x = 0;
  d2y = sgn(v);
  m = abs(v);
  n= abs(u);
 }

 s = (int)(m / 2);

 for (i=0;i<=round(m);i++)
 {
  xs[j]=a;
  ys[j]=b;
  j++;
  s += n;

  if (s >= m)
	{
   s -= m;
   a += d1x;
	 b += d1y;
  }
  else
  {
   a += d2x;
   b += d2y;
  }
 }
}


void Graph::linep(int a, int b, int c, int d, int &x,int &y,int &l)

{
 long u,s,v,d1x,d1y,d2x,d2y,m,n;
 int i,j;

 v = -(c-a);
 u=  (d-b);
 d1x = sgn(u);
 d1y = sgn(v);
 d2x = sgn(u);
 d2y = 0;
 m= abs(u);
 n = abs(v);

 if (m<=n)
 {
  d2x = 0;
  d2y = sgn(v);
	m = abs(v);
  n= abs(u);
 }

 s = (int)(m / 2);

 for (i=0;i<=round(m);i++)
 {
  x=a;
	y=b;
  l++;
	s += n;

  if (s >= m)
	{
   s -= m;
   a += d1x;
   b += d1y;
  }
  else
  {
   a += d2x;
	 b += d2y;
  }
 }
}

void Graph::hline(unsigned char far *px,int x1, int x2, int y,unsigned char color )
{
  for(int i=x1; i<=x2; i++ )
    putpixel(px,i, y, color );
}
void Graph::vline(unsigned char far *px,int x, int y1, int y2,unsigned char color )
{
	for(int i=y1; i<y2; i++ )
    putpixel(px,x,i, color );
}

void Graph::fillbox(unsigned char far *px,int x1, int y1, int x2, int y2,unsigned char color )
{
int var;
  for( var=y1; var<=y2; var++ )
    hline(px,x1, x2, var, color );
}

void Graph::box(unsigned char far *px,int x1, int y1, int x2, int y2,unsigned char color )
{
  hline(px,x1, x2, y1, color );
	hline(px,x1, x2, y2, color );
	vline(px,x1, y1, y2, color );
	vline(px,x2, y1, y2, color );
}

void Graph::correr_paleta()
{
	unsigned char cont1,c;
	int cont;

	for (cont=0;cont<256;cont++)
		for (cont1=0;cont1<3;cont1++)
			paleta[cont][cont1]=(paleta[cont][cont1]>>2); /* Movemos 2 bits
					hacia la derecha todos los bytes le¡dos en la paleta del PCX.
					Los componentes RGB de la paleta se encuentran en los 6 bits
					de mayor peso de los valores le¡dos del fichero */
}

//SE COPIA EL PCX A UN BITMAP PARA LUEGO DIBUJARLO
unsigned char Graph::loadPCX( char *nombre )
{
	FILE *fp;

	unsigned char num_bytes,data;
	unsigned long cont;

	if ((fp=fopen(nombre,"rb"))==NULL) return (0);

	fseek (fp,8,SEEK_SET);
	fread(&ancho,2,1,fp);
	fread(&alto,2,1,fp);
	ancho++;
	alto++;

	fseek (fp,128L,SEEK_SET); // Nos saltamos la cabecera del PCX

	cont=0;
	ancho+=(4-ancho%4)%4;
	unsigned long LONG=alto*ancho;

		while(cont <= LONG)
     {
       data = getc(fp); //Leemos el bitmap byte por byte

     if (data >= 192)      // es un rle?
	{
	 num_bytes = data - 192; // Cuantos veces se repite el color?

	data = getc(fp);  // Leemos el color que se repite

	while(num_bytes-- > 0) pv2[cont++] = data; // copiamos el color en el bitmap num_bytes veces

	}
	 else  //si no hay repeticion de color
	{
	 // copiamos directamente el color en el bitmap
	 pv2[cont++] = data;
	}

     }

      fseek(fp,-768L,SEEK_END);  // Nos situamos en el comienzo de la paleta
			fread(paleta,768L,1,fp);    // Leemos los bytes componentes del RGB
      correr_paleta();
      fclose(fp);   // Cerramos el fichero pcx

	return (1);
}
//CARGA LA MASCARA
unsigned char Graph::loadPCXmask( char *nombre )
{
	FILE *fp;

	unsigned char num_bytes,data;
	unsigned long cont;

	if ((fp=fopen(nombre,"rb"))==NULL) return (0);

	fseek (fp,8,SEEK_SET);
	fread(&ancho,2,1,fp);
	fread(&alto,2,1,fp);
	ancho++;
	alto++;

	fseek (fp,128L,SEEK_SET); // Nos saltamos la cabecera del PCX

	cont=0;
	ancho+=(4-ancho%4)%4;
	unsigned long LONG=alto*ancho;

    while(cont <= LONG)
     {
       data = getc(fp); //Leemos el bitmap byte por byte

     if (data >= 192)      // es un rle?
	{
	 num_bytes = data - 192; // Cuantos veces se repite el color?

	data = getc(fp);  // Leemos el color que se repite

	while(num_bytes-- > 0) pvm[cont++] = data; // copiamos el color en el bitmap num_bytes veces

	}
     else  //si no hay repeticion de color
	{
	 // copiamos directamente el color en el bitmap
	 pvm[cont++] = data;
	}

		 }
			fclose(fp);   // Cerramos el fichero pcx

	return (1);
}
//coloca la paleta en la video ram
void Graph::putpal(unsigned char rojo,unsigned char verde,unsigned char azul)
{
asm{
		mov    dx,0x3c9
		mov    al,[rojo]
		out    dx,al
		mov    al,[verde]
		out    dx,al
		mov    al,[azul]
		out    dx,al
	 }
}
//setea la paleta2
void Graph::pal2()
{
	int i;

	outportb (0x3C8,0);  //Primer color a modificar se incr. automaticamente
	for (i=0;i<256;i++)  //Vuelca paleta en todos los colores
		putpal(paleta2[i][0],paleta2[i][1],paleta2[i][2]);
}
//setea la paleta1
/*void Graph::pal()
{
	int i;

	outportb (0x3C8,0);  //Primer color a modificar se incr. automaticamente
	for (i=0;i<256;i++)  //Vuelca paleta en todos los colores
		putpal(paleta[i][0],paleta[i][1],paleta[i][2]);
} */
void Graph::pal(void)
{
	unsigned char cont1,c;
	int cont;

	cont=0;
	cont1=0;

	/* El c¢digo que sigue a continuaci¢n pasa el contenido del array "Paleta"
	a la paleta de la VGA */
	for (cont=0;cont<256;cont++)
	{
		outportb (0x03c8,cont);
		for (cont1=0;cont1<3;cont1++)
			outportb (0x03c9,paleta[cont][cont1]);
	}
}
//copia la paleta 1 a la 2
void Graph::copiar(){
	for(int i=0;i<256;i++){
		paleta2[i][0]=paleta[i][0];
		paleta2[i][1]=paleta[i][1];
		paleta2[i][2]=paleta[i][2];
	}
}
//hace el fade sobre la paleta1 y la deja en 0
void Graph::fadeout(int demora){
int acabo=0;
int sinterminar=0;
	while(acabo==0){
		for(int i=0;i<256;i++){
			if(paleta[i][0]>0){paleta[i][0]--; sinterminar=1;}
			if(paleta[i][1]>0){paleta[i][1]--; sinterminar=1;}
			if(paleta[i][2]>0){paleta[i][2]--; sinterminar=1;}
		}
		if(sinterminar){
			acabo=0;
			sinterminar=0;
		}else{
			acabo=1;
		}
		delay(demora);
		wait_retrace();
		pal();
	}
}
//hace un fade de la paleta1 hasta la 2
void Graph::fade12(int demora){
int acabo=0;
int sinterminar=0;
	while(acabo==0){
		for(int i=0;i<256;i++){
			if(paleta[i][0]<paleta2[i][0]){paleta[i][0]++; sinterminar=1;}
			if(paleta[i][0]>paleta2[i][0]){paleta[i][0]--; sinterminar=1;}
			if(paleta[i][1]<paleta2[i][1]){paleta[i][1]++; sinterminar=1;}
			if(paleta[i][1]>paleta2[i][1]){paleta[i][1]--; sinterminar=1;}
			if(paleta[i][2]<paleta2[i][2]){paleta[i][2]++; sinterminar=1;}
			if(paleta[i][2]>paleta2[i][2]){paleta[i][2]--; sinterminar=1;}

		}
		if(sinterminar){
			acabo=0;
			sinterminar=0;
		}else{
			acabo=1;
		}
		delay(demora);
		wait_retrace();
		pal();
	}
}
//hace un fade de la paleta2 hasta la 1
void Graph::fade21(int demora){
int acabo=0;
int sinterminar=0;
	while(acabo==0){
		for(int i=0;i<256;i++){
			if(paleta2[i][0]<paleta[i][0]){paleta2[i][0]++; sinterminar=1;}
			if(paleta2[i][0]>paleta[i][0]){paleta2[i][0]--; sinterminar=1;}
			if(paleta2[i][1]<paleta[i][1]){paleta2[i][1]++; sinterminar=1;}
			if(paleta2[i][1]>paleta[i][1]){paleta2[i][1]--; sinterminar=1;}
			if(paleta2[i][2]<paleta[i][2]){paleta2[i][2]++; sinterminar=1;}
			if(paleta2[i][2]>paleta[i][2]){paleta2[i][2]--; sinterminar=1;}

		}
		if(sinterminar){
			acabo=0;
			sinterminar=0;
		}else{
			acabo=1;
		}
		delay(demora);
		wait_retrace();
		pal2();
	}
}
//borra la paleta 2
void Graph::clrpal2(){
	for(int i=0;i<256;i++) {paleta2[i][0]=0;paleta2[i][1]=0;paleta2[i][2]=0;}
}

 /*
struct parrafo{unsigned char linea[235];};
class Font {
public:
	int  loadfont(char *nombre);
	void showletter(int f);
	void putletter(unsigned char far *px,int x,int y,int f,Graphics &h,int col);
	void cputletter(unsigned char far *px,int x,int y,int f,Graphics &h,int tx);
	void putstr(unsigned char far *px,int x,int y,char *s,Graphics &h,int tx,int col);
	Font();
	~Font(){
	cout<<"Entra el destructor";
	}
private:
	parrafo fuente[3];
	unsigned char matriz[3][235];
	unsigned char textura[3][5];
	unsigned char textura2[3][5];
	unsigned char textura3[3][5];

};
Font::Font(){
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

	for(i=0;i<3;i++) textura3[i][0]=142;
	for(i=0;i<3;i++) textura3[i][1]=141;
	for(i=0;i<3;i++) textura3[i][2]=140;
	for(i=0;i<3;i++) textura3[i][3]=235;//235
	for(i=0;i<3;i++) textura3[i][4]=42; //42
}

int Font::loadfont(char *nombre)
{
  FILE *archivo;
  int i,j;
  parrafo lectura[235];

  if((archivo=fopen(nombre,"r+"))==NULL) return(0);
  for(i=0;i<235;i++) //i es el numero de lineas por leer
  {
       fgets(lectura[i].linea,6,archivo);
  }
  fclose(archivo);
  for(j=0;j<235;j++)
  {
   for(i=0;i<3;i++)
      {
       matriz[i][j]=lectura[j].linea[i];
      }
  }
  return(1);
}

void Font::showletter(int f){
     f=f*5;
     for(int j=f;j<f+5;j++)
     {
      for(int i=0;i<3;i++) cout<<matriz[i][j];
      cout<<"\n";
     }
}
void Font::putletter(unsigned char far *px,int x,int y,int f,Graphics &h,int col)
{
  f=f*5;
  for(int j=f;j<f+5;j++)
    for(int i=0;i<3;i++)
       {
	if(matriz[i][j]=='1') h.putpixel(px,x+i,y+(j-f),col);
       }
}
void Font::cputletter(unsigned char far *px,int x,int y,int f,Graphics &h,int tx)
{
  f=f*5;
  for(int j=f;j<f+5;j++)
    for(int i=0;i<3;i++)
       {
	if(matriz[i][j]=='1')
	switch(tx){
	case 1:h.putpixel(px,x+i,y+(j-f),textura[i][j-f]);break;
	case 2:h.putpixel(px,x+i,y+(j-f),textura2[i][j-f]);break;
	case 3:h.putpixel(px,x+i,y+(j-f),textura3[i][j-f]);break;
	default:h.putpixel(px,x+i,y+(j-f),15);
	}
       }
}
void Font::putstr(unsigned char far *px,int x,int y,char *s,Graphics &h,int tx,int col)
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
		case 33: f=45;break;
		case 63: f=44;break;
		default: f=f-17;
		}
	      }

       if(tx==0) putletter(px,x,y,f,h,col);
       else cputletter(px,x,y,f,h,tx);
       i++;
       x=x+4;
    }
}   */
