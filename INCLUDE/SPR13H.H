/*
	GISLERSOFT(R) 2004
	CLASE SPRITE para el manejo de graficos
	Programado por Gisler Garc�s Vargas
	Cali-Colombia
*/
#define MAX_FRAMES 256

struct frame
{
	unsigned char far *img;
};
class Sprite
{
public:
	frame frm[MAX_FRAMES];
	int tam;

	Sprite(int n, int t); // n=numero de frames y t el tama�o
	Sprite(){};
	~Sprite(){};
	void destruir();					 // Destruimos todos los frames
	int status();						 // Si hay suficiente espacio en memoria
	int cargarSprite(char *n, Graph &h); // Carga todos los frames

    // Atributos migrados de SPRPOO2
	int numframes;
	int x1, y1, x2, y2;
	int inicia, inicia2, animacion;

	// Metodos migrados de SPRPOO2
	void crear(int n, int t, float tm);
	void dibujar(int f, int c, Graph &h);
	void dibujart(int ini, int fin, int c, clock_t r, Graph &h);
	void animar(int ini, int fin, int c, clock_t r, Graph &h);
	void posicionar(int x, int y);
	int lim_x(int xpos, int prelativo);
	int lim_y(int ypos, int prelativo);
	int lim(int a, int b, int a2, int b2, Graph &h);

public:
	int reloj, reloj2, cf;
	clock_t start, start2;
	float tiempo;
};

void Sprite::crear(int n, int t, float tm)
{
	tam = t;
	reloj = 1;
	reloj2 = 1;
	animacion = 0;
	inicia = 0;
	inicia2 = 0;
	tiempo = tm;
	numframes = n;
	for (int i = 0; i < numframes; i++)
		frm[i].img = (unsigned char far *)farmalloc(t * t);
}

void Sprite::destruir()
{
	for (int i = 0; i < numframes; i++)
		farfree(frm[i].img);
}

void Sprite::~Sprite()
{
	destruir();
}

Sprite::Sprite(int n, int t)
{
	numframes = n;
	tam = t;
	for (int i = 0; i < numframes; i++)
		frm[i].img = (unsigned char far *)farmalloc(t * t);
}

int Sprite::status()
{
	for (int i = 0; i < numframes; i++)
		if (frm[i].img == NULL)
			return (0);
	return (1);
}

void Sprite::posicionar(int x, int y)
{
	x1 = x;
	y1 = y;
	x2 = x + tam;
	y2 = y + tam;
}
int Sprite::lim_x(int xpos, int prelativo)
{
	if (xpos + prelativo < 0)
		return -prelativo;
	if (xpos + (tam - prelativo) > 320)
		return (320 - (tam - prelativo));
	return xpos;
}
int Sprite::lim_y(int ypos, int prelativo)
{
	if (ypos + prelativo < 0)
		return -prelativo;
	if (ypos + (tam - prelativo) > 200)
		return (200 - (tam - prelativo));
	return ypos;
}

void Sprite::dibujar(int f, int c, Graph &h)
{
	h.putframe(h.pv1, x1, y1, tam, frm[f].img, c);
}

void Sprite::dibujart(int ini, int fin, int c, clock_t r, Graph &h)
{
	if ((cf < ini) || (cf > fin))
		inicia = 0;
	if (inicia == 0)
	{
		cf = ini;
		inicia = 1;
	}
	if (reloj == 1)
	{
		start = r;
		reloj = 0;
	}
	if (((r - start) / CLK_TCK) > tiempo)
	{
		cf++;
		if (cf > fin)
			cf = ini;
		reloj = 1;
	}
	dibujar(cf, c, h);
}
void Sprite::animar(int ini, int fin, int c, clock_t r, Graph &h)
{
	if (animacion == 1)
	{
		if ((cf < ini) || (cf > fin))
			inicia2 = 0;
		if (inicia2 == 0)
		{
			cf = ini;
			inicia2 = 1;
		}
		if (reloj2 == 1)
		{
			start2 = r;
			reloj2 = 0;
		}
		if (((r - start2) / CLK_TCK) > tiempo)
		{
			cf++;
			if (cf > fin)
				animacion = 0;
			reloj2 = 1;
		}
		if (animacion == 1)
			dibujar(cf, c, h);
	}
}

int Sprite::lim(int a, int b, int a2, int b2, Graph &h)
{
	if (h.getpixel(h.pvm, x1 + a, y1 + b) == 0)
		return 1;
	if (h.getpixel(h.pvm, x1 + a2, y1 + b2) == 0)
		return 1;
	if (h.getpixel(h.pvm, x1 + a, y1 + b2) == 0)
		return 1;
	if (h.getpixel(h.pvm, x1 + a2, y1 + b) == 0)
		return 1;
	return (0);
}

int Sprite::cargarSprite(char *n, Graph &h)
{

	if (h.loadPCX(n) == 0)
		return (0);
	int o = 0;
	int u = 0;
	for (int k = 0; k < numframes; k++)
	{
		h.getframe(h.pv2, o, u, tam, frm[k].img);
		o = o + tam;
		if ((o + tam) > 320)
		{
			o = 0;
			u = u + tam;
		}
	}
	return (1);
}
