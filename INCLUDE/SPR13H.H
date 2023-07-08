/*
	GISLERSOFT(R) 2004
	CLASE SPRITE para el manejo de graficos
	Programado por Gisler Garc‚s Vargas
	Cali-Colombia
*/
#define MAX_FRAMES	256

struct frame{
	  unsigned char far *img;
};
class Sprite{
public:
	frame frm[MAX_FRAMES];
	int numframes;
	int tam;

	Sprite(int n,int t);//n=numero de frames y t el tama¤o
	Sprite(){};
	~Sprite(){};
	void destruir();//Destruimos todos los frames
	int status();//Si hay suficiente espacio en memoria
	int cargarSprite(char *n,Graph &h);//Carga todos los frames
};

Sprite::Sprite(int n,int t){
	numframes=n;
	tam=t;
	for(int i=0;i<numframes;i++)
	frm[i].img=(unsigned char far*) farmalloc(t*t);
}

void Sprite::destruir(){
	for(int i=0;i<numframes;i++)
	farfree(frm[i].img);
}

int Sprite::status(){
	   for(int i=0;i<numframes;i++)
	   if(frm[i].img==NULL) return(0);
	   return(1);
}

int Sprite::cargarSprite(char *n,Graph &h){

	  if(h.loadPCX(n)==0) return(0);
	  int o=0;
	  int u=0;
	  for(int k=0;k<numframes;k++)
	  {
		 h.getframe(h.pv2,o,u,tam,frm[k].img);
		 o=o+tam;
		if((o+tam)>320)
		{
		 o=0;
		 u=u+tam;
		}
	  }
	  return(1);
}

