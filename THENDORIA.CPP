/*
	GISLERSOF(R) 2003
	Version Beta
	Proyecto RPG
	Programado por: Gisler Garc�s Vargas
	Cali - Colombia

	Comentarios:

	Juego RPG(Role play game) programado usando la teoria
	de Tiles y la teoria de tableros para juegos RPG.
*/

/*===========================[Archivos Cabecera]============================*/
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <time.h>
/*===========================[Archivos Cabecera Custom]============================*/
#include "vgapoo.h"
#include "fnt13h.h"
#include "spr13h.h"
/*===========================[Archivos Cabecera Custom Modo Batalla]============================*/
#include "person.h"
#include "batalla.h"

/*=======================[Definicion de constantes]=========================*/
#define MAXCHAROBJ 9 // Maximo de caracteres para el nombre de un objeto
#define MAXTILEOBJ 3 // Maximo de tiles por objeto
#define MAXARCHOBJ 8 // Maximo de caracteres para el nombre del archivo asociado al objeto
#define MAXMAP 40	 // Tama�o maximo del mapa

/*** Revision 2023 Musica */

/*
NOTAS MUSICALES POR FREQ
A  220.0000 // Pick up item!
A# 233
B  247 // Talk
C  262
C# 277
D  293
D# 311
E  330
F  350
F# 370
G  392
G# 415
A  440
*/

int mainThemeMusic[18] = {440, 392, 350, 262, 293, 0, 0, 0, 0, 0, 293, 293, 262, 262, 0, 0, 0, 0};
int misteryMusic[18] = {261, 350, 262, 350, 392, 0, 0, 0, 0, 0, 261, 350, 262, 350, 392, 0, 0, 0};
int fightTheme[5] = {293, 330, 350, 392, 293};

int currentMusicIndex = 0;
int musicLenght = 18;

/*=======================[Definicion de valores]============================*/
enum boolean
{
	FALSE,
	TRUE
};
enum tipoAccion
{
	PREGUNTA,
	ACCION,
	NEUTRO,
	TELEACCION,
	SALIDA
};
enum tipoEstado
{
	ESTATICO,
	ANIMADO,
	SUICHE
};
enum tipoTile
{
	TIL1,
	TIL2,
	TIL3
};
enum tipoBrujula
{
	NORTE,
	ESTE,
	SUR,
	OESTE
};
typedef unsigned char byte;

/*====================[Definicion de la estructura objeto]==================*/
typedef struct objeto
{ 
	boolean importante;		  // Si el objeto es importante
	boolean solido;			  // Si el objeto es solido
	boolean ciclico;		  // Si el objeto es ciclico
	boolean tocado;			  // Si el jugador toco el objeto
	tipoAccion accion;		  // Tipo de accion asociada al objeto
	tipoEstado estado;		  // El tipo de estado del objeto
	tipoTile actual;		  // Apunta al til actual el que se va a dibujar, en el arreglo de tiles
	char nombre[MAXCHAROBJ];  // Contiene el nombre del objeto
	char archivo[MAXARCHOBJ]; // Contiene el nombre del archivo asociado al objeto
	int tiles[MAXTILEOBJ];	  // Los tiles asociados del objeto
	byte x, y;				  // Posicion del tile que debe modificar si es de tipo TELEACCION
};
/*==========================[Variables globales]============================*/

objeto mapa[MAXMAP][MAXMAP]; // Variable que contiene todos los objetos

/*==========================[Definicion de la clase Batallador]=============*/

class BattleMode
{
public:
	clock_t start, start2;		 // Controla los tiempos de teclado y animaciones
	Batalla bat;
	Sprite enemigos[6];
	Sprite personajes[3];

	char tecla;
	unsigned char accion;
	int op, op2, op3;
	unsigned char reloj, reloj2;
	int enemigo_actual, personaje_actual;
	unsigned char seleccionar_enemigo, seleccionar_personaje, seleccionar_magia;
	unsigned char turno, control1, control2, seleccionar_opcion;
	unsigned char seleccionar_sub;

	boolean loaded;

	// Constructor y destructor
	BattleMode();
	~BattleMode();
	//-----------------

	void initValues();
	void Error(char *texto, int tipo_error);
	int demorar(clock_t s, float t);
	void cargar_sprite_buffer(Sprite &m, char *nombre, Graph &h);
	void loadBatalla(Graph &h);
	void modoBatalla(Graph &g, Font &f);
};
void BattleMode::initValues() {
	op = 1;
	op2 = 1;
	op3 = 1;
	reloj = 1;
	reloj2 = 1;
	enemigo_actual = 0;
	personaje_actual = 0;
	seleccionar_enemigo = 0;
	seleccionar_personaje = 0;
	seleccionar_magia = 0;
	turno = 1;
	control1 = 0;
	control2 = 0;
	seleccionar_opcion = 0;
	seleccionar_sub = 0;
	loaded = FALSE;
}
//----------------------------------------------------------------------------
BattleMode::BattleMode()
{
	initValues();
}
//----------------------------------------------------------------------------
BattleMode::~BattleMode()
{
}
// Escribe un mensaje de erro con el tipo de error generado
void BattleMode::Error(char *texto, int tipo_error)
{
	cout << "Error:" << texto;
	switch (tipo_error)
	{
	case 1:
		cout << "No hay suficiente memoria.";
		break;
	case 2:
		cout << "No se pudo abrir.";
		break;
	default:
		cout << "Se aborto el programa por razones desconocidas.";
		break;
	}
	getch();
	exit(1);
}
// Retorna 1 cuando pasan t segundos
int BattleMode::demorar(clock_t s, float t)
{
	if (((clock() - s) / CLK_TCK) > t)
		return 1;
	return 0;
}
// Carga un sprite desde un archivo
void BattleMode::cargar_sprite_buffer(Sprite &m, char *nombre, Graph &h)
{
	if ((m.status() == 0))
		Error(nombre, 1);
	if (m.cargarSprite(nombre, h) == 0)
		Error(nombre, 2);
}
void BattleMode::loadBatalla(Graph &h) {
	bat = Batalla(6, 3); // Crea una batalla con X monstruos y Y personajes
	bat.CargarPersonajes();
	bat.CargarMonstruos();

	/*
	 Se cargan los monstruos de acuerdo a los archivos
	 especificados para la batalla actual
	*/
	switch (bat.nmonstruos)
	{
	case 6:
		enemigos[5].crear(7, 60, 0.13);
		cargar_sprite_buffer(enemigos[5], "PLANTA.PCX", h);
	case 5:
		enemigos[4].crear(7, 60, 0.13);
		cargar_sprite_buffer(enemigos[4], "PLANTA.PCX", h);
	case 4:
		enemigos[3].crear(7, 60, 0.13);
		cargar_sprite_buffer(enemigos[3], "PLANTA.PCX", h);
	case 3:
		enemigos[2].crear(7, 60, 0.13);
		cargar_sprite_buffer(enemigos[2], "PLANTA.PCX", h);
	case 2:
		enemigos[1].crear(7, 60, 0.13);
		cargar_sprite_buffer(enemigos[1], "PLANTA.PCX", h);
	case 1:
		enemigos[0].crear(7, 60, 0.13);
		cargar_sprite_buffer(enemigos[0], "PLANTA.PCX", h);
	}

	// Se cargan los sprites de los protagonistas
	switch (bat.npersonajes)
	{
		case 3:
			personajes[2].crear(8, 53, 0.2);
			cargar_sprite_buffer(personajes[2], "PROF.PCX", h);
		case 2:
			personajes[1].crear(8, 53, 0.2);
			cargar_sprite_buffer(personajes[1], "PROF.PCX", h);
		case 1:
			personajes[0].crear(8, 53, 0.2);
			cargar_sprite_buffer(personajes[0], "PROF.PCX", h);
	}

	if (h.loadPCX("BOSQUEF.PCX") == 0)
		Error("BOSQUEF.PCX", 2); // Se carga el fondo

			// Se empieza el modo 13h
	h.modo_video(GRAFICO);
	h.clr(h.pv1, 0);
	h.pal();
}
void BattleMode::modoBatalla(Graph &g, Font &f)
{

	g.volcar(g.pv1, g.pv2);

	tecla = inport(0x60);

	// Control de relojes
	if (reloj == 1)
	{
		start = clock();
		reloj = 0;
	}
	if (reloj2 == 1)
	{
		start2 = clock();
		reloj2 = 0;
	}

	// Gestion de teclas
	if (turno == 1)
	{
		switch (tecla)
		{

		case 57:
			if (demorar(start, 0.2) == 1)
			{
				if (seleccionar_sub == 1)
				{
					seleccionar_sub = 0;
					control1 = 3;
					op3 = 1;
					op2 = 1;
					op = 1;
				}
				if (seleccionar_magia == 1)
				{
					op2 = 7;
					seleccionar_magia = 0;
					seleccionar_sub = 1;
				}
				if (seleccionar_enemigo == 1)
				{
					seleccionar_enemigo = 0;
					control1 = 2;
				}
				if (seleccionar_opcion == 1)
				{
					control1 = 1;
					seleccionar_opcion = 0;
				}
				reloj = 1;
			}
			break;
		case 75:
			if (demorar(start, 0.2) == 1)
			{
				if (seleccionar_enemigo == 1)
				{
					if (enemigo_actual > 0)
						enemigo_actual--;
					else
						enemigo_actual = bat.nmonstruos - 1;
				}
				reloj = 1;
			}
			break;
		case 77:
			if (demorar(start, 0.2) == 1)
			{
				if (seleccionar_enemigo == 1)
				{
					if (enemigo_actual < bat.nmonstruos - 1)
						enemigo_actual++;
					else
						enemigo_actual = 0;
				}
				reloj = 1;
			}
			break;
		case 72:
			if (seleccionar_opcion == 1)
			{
				if (demorar(start, 0.2) == 1)
				{
					op--;
					reloj = 1;
				}
			}
			if (seleccionar_magia == 1)
			{
				if (demorar(start, 0.2) == 1)
				{
					op2--;
					reloj = 1;
				}
			}
			if (seleccionar_sub == 1)
			{
				if (demorar(start, 0.2) == 1)
				{
					op3--;
					reloj = 1;
				}
			}
			break;
		case 80:
			if (seleccionar_opcion == 1)
			{
				if (demorar(start, 0.2) == 1)
				{
					op++;
					reloj = 1;
				}
			}
			if (seleccionar_magia == 1)
			{
				if (demorar(start, 0.2) == 1)
				{
					op2++;
					reloj = 1;
				}
			}
			if (seleccionar_sub == 1)
			{
				if (demorar(start, 0.2) == 1)
				{
					op3++;
					reloj = 1;
				}
			}
			break;
		}
	}

	bat.menu(op, f, g);

	// Gestion del turno de los protagonistas
	if (turno == 1)
	{
		switch (control1)
		{
		case 0:
			seleccionar_opcion = 1;
			break;
		case 1:
			seleccionar_enemigo = 1;
			break;
		case 2:
			if (bat.m[enemigo_actual].vivo == 1)
			{
				switch (op)
				{
				case 1:
					bat.contrincantes(personaje_actual, enemigo_actual, 1);
					control1 = 3;
					break;
				case 2:
					if (seleccionar_sub == 0)
						seleccionar_magia = 1;
					else
						seleccionar_magia = 0;
					op = 5;
					break;
				case 3:
					control1 = 0;
					break;
				}
			}
			else
				control1 = 0;
			break;

		case 3:
			char strtemp[5] = "****";
			bat.pro[0].convertir(bat.ataque(), strtemp);
			personajes[personaje_actual].animacion = 1;
			enemigos[enemigo_actual].animacion = 1;
			control1 = 4;
			op = 5;
			break;
		case 4:
			if (enemigos[enemigo_actual].animacion == 1)
			{
				enemigos[enemigo_actual].posicionar(enemigos[enemigo_actual].x1, enemigos[enemigo_actual].y1);
				enemigos[enemigo_actual].animar(0, 4, 0, clock(), g);
			}
			if (personajes[personaje_actual].animacion == 1)
			{
				personajes[personaje_actual].posicionar(enemigos[enemigo_actual].x1 + 30, enemigos[enemigo_actual].y1 + 20);
				personajes[personaje_actual].animar(2, 4, 0, clock(), g);
			}

			bat.mostrar_golpe(strtemp, enemigos[enemigo_actual], f, g);

			if (demorar(start2, 2) == 1 && personajes[personaje_actual].animacion == 0 && enemigos[enemigo_actual].animacion == 0)
			{
				personaje_actual++;
				op = 1;
				if (personaje_actual > (bat.npersonajes - 1))
				{
					personaje_actual = 0;
					enemigo_actual = 0;
					control1 = 0;
					turno = 0;
					op = 5;
				}
				else
					control1 = 0;
				reloj2 = 1;
			}
			break;
		}
	}
	else
	{
		// Gestion del turno de los enemigos
		switch (control2)
		{
		case 0:
			control2 = 1;
			accion = 1;
			break;
		case 1:
			personaje_actual = 0;
			control2 = 2;
			break;
		case 2:
			if (bat.m[enemigo_actual].vivo == 1)
			{
				switch (accion)
				{
				case 1:
					bat.contrincantes(personaje_actual, enemigo_actual, 0);
					control2 = 3;
					break;
				}
			}
			else
			{
				control2 = 0;
				enemigo_actual++;
				if (enemigo_actual > bat.nmonstruos - 1)
				{
					control2 = 0;
					enemigo_actual = 0;
					turno = 1;
				}
			}
			break;
		case 3:
			char strtemp2[5] = "****";
			bat.pro[0].convertir(bat.ataque(), strtemp2);
			personajes[personaje_actual].animacion = 1;
			enemigos[enemigo_actual].animacion = 1;
			control2 = 4;
			break;
		case 4:
			if (personajes[personaje_actual].animacion == 1)
			{
				personajes[personaje_actual].posicionar(personajes[personaje_actual].x1, personajes[personaje_actual].y1);
				personajes[personaje_actual].animar(5, 7, 0, clock(), g);
			}
			if (enemigos[enemigo_actual].animacion == 1)
			{
				enemigos[enemigo_actual].posicionar(personajes[personaje_actual].x1 - 30, personajes[personaje_actual].y1 - 10);
				enemigos[enemigo_actual].animar(5, 6, 0, clock(), g);
			}

			bat.mostrar_golpe(strtemp2, personajes[personaje_actual], f, g);

			if (demorar(start2, 2) == 1 && personajes[personaje_actual].animacion == 0 && enemigos[enemigo_actual].animacion == 0)
			{
				enemigo_actual++;
				if (enemigo_actual > bat.nmonstruos - 1)
				{
					enemigo_actual = 0;
					personaje_actual = 0;
					control2 = 0;
					turno = 1;
					op = 1;
				}
				else
					control2 = 0;
				reloj2 = 1;
			}
			break;
		}
	}

	if (seleccionar_enemigo == 1)
	{
		if (bat.m[enemigo_actual].vivo == 1)
			bat.marcar_personaje(enemigos[enemigo_actual], 157, g);
		else
			bat.marcar_personaje(enemigos[enemigo_actual], 69, g);
	}
	// Gestion de Dibujo de enemigos
	switch (bat.nmonstruos)
	{
	case 6:
		if (bat.m[5].vivo == 1)
		{
			if (enemigos[5].animacion == 0)
			{
				enemigos[5].posicionar(110, 0);
				enemigos[5].dibujart(0, 3, 0, clock(), g);
			}
		}
		else
		{
			enemigos[5].posicionar(110, 0);
			enemigos[5].animar(0, 4, 0, clock(), g);
		}
	case 5:
		if (bat.m[4].vivo == 1)
		{
			if (enemigos[4].animacion == 0)
			{
				enemigos[4].posicionar(70, 30);
				enemigos[4].dibujart(0, 3, 0, clock(), g);
			}
		}
		else
		{
			enemigos[4].posicionar(70, 30);
			enemigos[4].animar(0, 4, 0, clock(), g);
		}
	case 4:
		if (bat.m[3].vivo == 1)
		{
			if (enemigos[3].animacion == 0)
			{
				enemigos[3].posicionar(30, 60);
				enemigos[3].dibujart(0, 3, 0, clock(), g);
			}
		}
		else
		{
			enemigos[3].posicionar(30, 60);
			enemigos[3].animar(0, 4, 0, clock(), g);
		}
	case 3:
		if (bat.m[2].vivo == 1)
		{
			if (enemigos[2].animacion == 0)
			{
				enemigos[2].posicionar(150, 20);
				enemigos[2].dibujart(0, 3, 0, clock(), g);
			}
		}
		else
		{
			enemigos[2].posicionar(150, 20);
			enemigos[2].animar(0, 4, 0, clock(), g);
		}
	case 2:
		if (bat.m[1].vivo == 1)
		{
			if (enemigos[1].animacion == 0)
			{
				enemigos[1].posicionar(110, 50);
				enemigos[1].dibujart(0, 3, 0, clock(), g);
			}
		}
		else
		{
			enemigos[1].posicionar(110, 50);
			enemigos[1].animar(0, 4, 0, clock(), g);
		}
	case 1:
		if (bat.m[0].vivo == 1)
		{
			if (enemigos[0].animacion == 0)
			{
				enemigos[0].posicionar(70, 80);
				enemigos[0].dibujart(0, 3, 0, clock(), g);
			}
		}
		else
		{
			enemigos[0].posicionar(70, 80);
			enemigos[0].animar(0, 4, 0, clock(), g);
		}
	}
	// Gestion de marcado de personajes
	if (seleccionar_personaje == 1)
		bat.marcar_personaje(personajes[personaje_actual], 46, g);
	// Gestion de dibujo de protagonistas
	switch (bat.npersonajes)
	{
	case 3:
		if (bat.pro[2].vivo == 1)
		{
			if (personajes[2].animacion == 0)
			{
				personajes[2].posicionar(270, 66);
				personajes[2].dibujart(0, 2, 0, clock(), g);
			}
		}
	case 2:
		if (bat.pro[1].vivo == 1)
		{
			if (personajes[1].animacion == 0)
			{
				personajes[1].posicionar(240, 86);
				personajes[1].dibujart(0, 2, 0, clock(), g);
			}
		}
	case 1:
		if (bat.pro[0].vivo == 1)
		{
			if (personajes[0].animacion == 0)
			{
				personajes[0].posicionar(210, 106);
				personajes[0].dibujart(0, 2, 0, clock(), g);
			}
		}
	}

	bat.Mostrartabla(f, g); // Se muestra el estatus del usuario
	if (seleccionar_magia == 1 || seleccionar_sub == 1)
		bat.menu_magia(personaje_actual, f, g, op2, op3);
	g.volcar(g.vga, g.pv1);											 // Se dibuja un frame de escena
	g.clr(g.pv1, 0);												 // Se borra el buffer para crear una nueva escena
}
/*==========================[Definicion de la clase explorador]=============*/
class Explorador
{
public:
	int frame;			  // Cuenta la cantidad de frames que han pasado.
	int quantum;		  // Guarda el valor constante del cuanto.
	int scroll_vel;		  // Velocidad del scroll.
	int tope;			  // Valor maximo del scroll.
	int xpos_scroll;	  // Guarda la cantidad horizontal del scroll que se esta haciendo.
	int ypos_scroll;	  // Guarda la cantidad vertical del scroll que se esta haciendo.
	boolean scroll;		  // Guarda T/F si hay que mover la pantalla.
	boolean quanto;		  // Bandera que guarda T/F si paso un quanto.
	boolean salir;		  // Bandera que guarda T/F si se desea salir del juego.
	boolean menu;		  // Bandera que guarda T/F si se desea entrar al menu de config de personaje.
	boolean camino;		  // Bandera que guarda TRUE/FALSE si el personaje camino.
	boolean toco;		  // Bandera que guarda TRUE/FALSE si el personaje toco algun objeto.
	boolean cargar_nuevo; // Bandera que guarda T/F si se debe cargar un nuevo mapa.
	boolean dialogo;	  // Bandera que guarda T/F si estamos en modo dialogo.
	boolean lectura;	  // si estamos leyendo un parrafo.
	boolean reloj;		  // Permite que se asigne un valor inicial ala varible "start".
	boolean leyendo;	  // Si el jugador esta leyendo.
	boolean enter;		  // Muestra el Enter del dialogo.
	boolean toggleSound;
	boolean batalla;
	byte xpos_actual; // Guarda la posicion vertical del personaje dentro del mapa.
	byte ypos_actual; // Guarda la posicion horizontal del personaje dentro del mapa.
	byte xpos_objeto; // Guarda la posicion vertical del objeto interactuado.
	byte ypos_objeto; // Guarda la posicion horizontal del objeto interactuado.
	byte xpos_ref;	  // Guarda la anterior posicion horizontal del personaje para hacer el scroll.
	byte ypos_ref;	  // Guarda la anterior posicion vertical del personaje para hacer el scroll.
	byte tecla;		  // Guarda la tecla presionada.
	byte n_parrafo;	  // Cuenta el parrafo actual.

	tipoBrujula brujula; // Guarda la orientaci�n actual del personaje.
	char t_archivo[32];	 // Guarda temporalmente el nombre del archivo asociado al objeto
	Sprite personaje;	 // Contiene los frames del personaje
	Sprite recursos;	 // Contiene los tiles [0-239] 240 tiles en total
	clock_t start;		 // Almacena el tiempo transcurrido para la lectura de un dialogo

	// Constructor y destructor
	Explorador();
	~Explorador();
	//------------------------

	int cargarMapa(char *nombre);									  // Lee el archivo "nombre" y carga sobre la variable mapa[][].
	int cargarRecursos(char *n_recurso1, char *n_recurso2, Graph &h); // Carga los sprites retorna 0 si fall�.
	boolean importante();											  // Devuelve T si el objeto con que se interactuo es importante.
	boolean solido();												  // Devuel T, prende la bandera scroll y modifica la posicion del personaje.
	boolean neutro();												  // Devuel T, si el objeto de la interaccion es neutro.
	boolean demorar(clock_t s, float t);							  // Devuelve T si ha pasado t segundos.
	void actualizarObjeto();										  // Reviza la accion del objeto y cambia banderas de acuerdo al tipo de accion.
	void leerTeclado();												  // Lee el puerto y prende banderas deacuerdo a lo tecleado
	void analizadorAccion();										  // Analiza la accion de un objeto y efectua un procedimiento de acuerdo a la accion.
	void analizadorEstado(byte, byte);								  // Analiza el estado de todos los objetos y los cambia de acuerdo a su estado.
	void dibujarMapa(Graph &h, Font &l);							  // Dibuja el Mapa sobre la memoria de video.
	void dibujarNormal(Graph &h);									  // Dibuja el mapa
	void borrarMapa();												  // Deja el mapa en negro
	void dibujarPersonaje(Graph &h);								  // Coloca el personaje en el centro de la pantalla
	void dialogoCarga(Graph &h, Font &l, byte n1, byte n2);			  // Carga el dialogo asociado al objeto
	void actualizarPosicion(byte x, byte y);						  // Reposiciona el personaje modificando sus atributos de posicion.
	void actualizarBrujula(tipoBrujula direc);						  // Coloca el personaje mirando hacia la direccion especificada.
	void logoGislersoft(Graph &g);
	void introScreen(Graph &g, Font &f);
	void aboutScreen(Graph &g, Font &f);
	void historiaParte1(Graph &g, Font &f);
	void historiaParte2(Graph &g, Font &f);
};
//----------------------------------------------------------------------------
Explorador::Explorador()
{
	// Atributos enteros
	frame = 0;
	quantum = 4;
	scroll_vel = 4;
	tope = 16 - scroll_vel; // 16 es el ancho y alto de los tiles, el scroll acaba en el tope
	xpos_scroll = 0;		// Aun no ha empezado el scroll
	ypos_scroll = 0;		// Aun no ha empezado el scroll

	// Atributos banderas
	scroll = FALSE;		  // Aun no ha empezado el scroll
	quanto = FALSE;		  // Aun no ha pasado el primer quanto
	salir = FALSE;		  // No se ha presionado escape.
	menu = FALSE;		  // No se ha presionado la tecla para ver el menu.
	camino = FALSE;		  // No ha caminado en ninguna direcci�n.
	toco = FALSE;		  // No ha tocado ning�n objeto.
	cargar_nuevo = FALSE; /*
							  Aun no se puede cargar un mapa nuevo, para el primer mapa
							  se debe cargar manualmente, por medio de cargarMapa(char *nombre).
						  */
	dialogo = FALSE;	  // No ha interactuado todavia con ning�n objeto de tipoAccion PREGUNTA.
	lectura = FALSE;	  // No ha interactuado todavia con ning�n objeto de tipoAccion PREGUNTA.
	reloj = FALSE;		  // Aun no se debe iniciar el temporizador para el dialogo.
	leyendo = FALSE;	  // Cuando se lee un ENTER para el dialogo se activa, y 2 segundos mas tarde se vuelve a activar.
	enter = FALSE;		  // Cuando pasan 2 segundos se prende esta bandera, para leer un ENTER en el modo dialogo.

	batalla = TRUE;

	// Atributos tipo byte
	/*
		Por defecto el personaje empieza en la posicion 0,0 y el objeto de interacci�n es 0,0
		la posicion anterior tambien es 0,0; Si se desea modificar esta posici�n debe utilizar el metodo
		actualizarPosicion(byte x,byte y);
	*/
	xpos_actual = 0;
	ypos_actual = 0;
	xpos_objeto = 0;
	ypos_objeto = 0;
	xpos_ref = xpos_actual;
	ypos_ref = ypos_actual;
	tecla = 0;	   // Aun no hay teclas presionadas.
	n_parrafo = 0; // Inicializamos para luego poder contar desde 0 los parrafos que se van leyendo.

	// Otros atributos
	brujula = SUR;				// por defecto el personaje mira hacia el sur, para cambiar utilice actualizarBrujula(tipoBrujula direc);
	strcpy(t_archivo, "NULL");	// Coloca "Null" como archivo en transito este archivo no existe dentro de la carpeta.
	personaje = Sprite(8, 16);	// El personaje tendra 8 frames y ser� de tama�o 16x16pixeles
	recursos = Sprite(240, 16); // La variable recursos tendra 240 tiles de 16x16 pixeles
	start = clock();			// Se toma este instante de tiempo que no se tomar� en cuenta.

	// Verificamos que haya suficiente memoria para el personaje y los recursos.
	if (personaje.status() == 0)
	{
		cout << "No hay suficiente memoria!";
		exit(1);
	}
	if (recursos.status() == 0)
	{
		cout << "No hay suficiente memoria!";
		exit(1);
	}
}
//----------------------------------------------------------------------------
Explorador::~Explorador()
{
}
//----------------------------------------------------------------------------
void Explorador::actualizarPosicion(byte x, byte y)
{
	xpos_actual = x;
	ypos_actual = y;
	xpos_ref = x;
	ypos_ref = y;
	xpos_objeto = x;
	ypos_objeto = y;
}
//----------------------------------------------------------------------------
void Explorador::actualizarBrujula(tipoBrujula direc)
{
	brujula = direc;
}
//----------------------------------------------------------------------------
void Explorador::leerTeclado()
{

	if (!scroll)
	{						  // Solo si no hay scroll
		tecla = inport(0x60); // Leemos del puerto la tecla presionada
		switch (tecla)
		{
		// Si la tecla presionada es "flecha arriba"
		case 72:
			if (!dialogo)
			{
				camino = TRUE;
				brujula = NORTE;
			}
			break;
		// Si la tecla presionada es "flecha derecha"
		case 77:
			if (!dialogo)
			{
				camino = TRUE;
				brujula = ESTE;
			}
			break;
		// Si la tecla presionada es "flecha abajo"
		case 80:
			if (!dialogo)
			{
				camino = TRUE;
				brujula = SUR;
			}
			break;
		// Si la tecla presionada es "flecha izquierda"
		case 75:
			if (!dialogo)
			{
				camino = TRUE;
				brujula = OESTE;
			}
			break;
		// Si se presiona la "barra espaciadora"
		case 57:
			if (!dialogo)
			{
				toco = TRUE;
			}
			break;
		// Si se presiona "Esc"
		case 1:
			salir = TRUE;
			nosound();
			break;
		// Si se presiona "Enter"
		case 28:
			if (dialogo)
				leyendo = TRUE;
			break;
		// Sin especificar todavia!!!!!!!!!!!<<<<<<<<<<<<<<<<<<<<<<<<<<<-SIN ESPECIFICAR!
		case 13:
			menu = TRUE;
			break;
		}
	}
}
//----------------------------------------------------------------------------
boolean Explorador::importante()
{
	if (mapa[xpos_objeto][ypos_objeto].importante == TRUE)
		return TRUE;
	else
		return FALSE;
}
//----------------------------------------------------------------------------
boolean Explorador::solido()
{
	switch (brujula)
	{
	case NORTE: // Si este dentro de las dimensiones del mapa
		if (ypos_actual - 1 >= 0)
		{
			// Si el objeto en esa direccion es solido o no
			if (mapa[xpos_objeto][ypos_objeto].solido == FALSE)
			{
				ypos_actual--;			   // Si no es solido puede moverse a esa casilla
				ypos_objeto = ypos_actual; // Actualizamos el objeto de interaccion
				scroll = TRUE;			   // Hay scroll
				return FALSE;
			}
			return TRUE;
		}
		return TRUE;

	case ESTE:
		if (xpos_actual + 1 < MAXMAP)
		{
			if (mapa[xpos_objeto][ypos_objeto].solido == FALSE)
			{
				xpos_actual++;
				xpos_objeto = xpos_actual;
				scroll = TRUE;
				return FALSE;
			}
			return TRUE;
		}
		return TRUE;

	case SUR:
		if (ypos_actual + 1 < MAXMAP)
		{
			if (mapa[xpos_objeto][ypos_objeto].solido == FALSE)
			{
				ypos_actual++;
				ypos_objeto = ypos_actual;
				scroll = TRUE;
				return FALSE;
			}
			return TRUE;
		}
		return TRUE;

	case OESTE:
		if (xpos_actual - 1 >= 0)
		{
			if (mapa[xpos_objeto][ypos_objeto].solido == FALSE)
			{
				xpos_actual--;
				xpos_objeto = xpos_actual;
				scroll = TRUE;
				return FALSE;
			}
			return TRUE;
		}
		return TRUE;
	}

	return TRUE;
}
//----------------------------------------------------------------------------
void Explorador::actualizarObjeto()
{
	switch (brujula)
	{
	//	De acuerdo a la direccion evaluamos el objeto "en frente de"
	case NORTE:
		xpos_objeto = xpos_actual;
		ypos_objeto = ypos_actual - 1;
		break;
	case ESTE:
		xpos_objeto = xpos_actual + 1;
		ypos_objeto = ypos_actual;
		break;
	case SUR:
		xpos_objeto = xpos_actual;
		ypos_objeto = ypos_actual + 1;
		break;
	case OESTE:
		xpos_objeto = xpos_actual - 1;
		ypos_objeto = ypos_actual;
		break;
	}
}
//----------------------------------------------------------------------------
boolean Explorador::neutro()
{
	if (mapa[xpos_objeto][ypos_objeto].accion == NEUTRO)
		return TRUE;
	return FALSE;
}
//----------------------------------------------------------------------------
void Explorador::analizadorAccion()
{
	switch (mapa[xpos_objeto][ypos_objeto].accion)
	{
	// Si es de tipo pregunta prendemos la bandera de dialogo y tocamos el objeto.
	case PREGUNTA:
		dialogo = TRUE;
		sound(247);
		mapa[xpos_objeto][ypos_objeto].tocado = TRUE;
		break;
	// Si es de tipo accion tocamos el objeto.
	case ACCION:
		mapa[xpos_objeto][ypos_objeto].tocado = TRUE;
		sound(220);
		break;
		// SIN ESPECIFICAR TODAVIA<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<SIN ESPEFICIAR!
		/*		case TELEACCION : //Revizamos que el objeto a distancia este dentro del mapa
							if (xpos_objeto.x >=0 && xpos_objeto.x < MAXMAP){
								if (xpos_objeto.y >=0 && ypos_objeto.y < MAXMAP){
									//El objeto a distancia debe ser ESTATICO
									if (mapa[xpos_objeto.x][ypos_objeto.y].estado == ESTATICO){
										//cambiamos la solides del objeto a distancia
										if (mapa[xpos_objeto.x][ypos_objeto.y].solido == TRUE)
											mapa[xpos_objeto.x][ypos_objeto.y].solido = FALSE;
										else mapa[xpos_objeto.x][ypos_objeto.y].solido = TRUE;
										//Cambiamos el frame
										if (mapa[xpos_objeto.x][ypos_objeto.y].actual == TIL1)
											mapa[xpos_objeto.x][ypos_objeto.y].actual = TIL2;
										else mapa[xpos_objeto.x][ypos_objeto.y].actual = TIL1;
									}
								}
							}
							mapa[xpos_objeto][ypos_objeto].tocar = TRUE;
							break;
		*/
	// Si es de tipo salida prendemos la bandera de cargar_nuevo y tocamos el objeto.
	case SALIDA:
		cargar_nuevo = TRUE;
		nosound();
		mapa[xpos_objeto][ypos_objeto].tocado = TRUE;
		break;
	}
}
//----------------------------------------------------------------------------
void Explorador::analizadorEstado(byte x, byte y)
{
	// Recibimosla posicion del objeto a analizar
	switch (mapa[x][y].estado)
	{
	// Si el objeto es estatico no hacemos nada.
	case ESTATICO:
		break;
	// Si es animado cambiamos sus frames para dar el efecto, en este caso solo 3 frames
	case ANIMADO: // Cada vez que pase un cuanto se cambiara un frame
		if (quanto)
		{
			// Miramos si se deben repetir sus frames
			if (mapa[x][y].ciclico && mapa[x][y].actual == TIL3)
				mapa[x][y].actual = TIL1;
			else
				mapa[x][y].actual = (tipoTile)(mapa[x][y].actual + 1);
		}
		else
			return;
		break;
	// Si es suiche cambiamos sus frames para dar el efecto si fue tocado, en este caso solo 2 frames
	case SUICHE: // Cada vez que pase un cuanro se cambiara un frame
		if (quanto)
		{
			if (mapa[x][y].tocado == TRUE)
			{ // Se toco?
				if (mapa[x][y].ciclico == TRUE)
				{ // Miramos si se deben repetir los frames
					if (mapa[x][y].actual == TIL2)
						mapa[x][y].actual = TIL1;
					else
						mapa[x][y].actual = TIL2;
				}
				else
				{
					if (mapa[x][y].actual == TIL1)
						mapa[x][y].actual = TIL2;
				}
			}
			else
				return;
		}
		else
			return;
		break;
	}
}
//---------------------------------------------------------------------------
void Explorador::dibujarNormal(Graph &h)
{
	// Los auxiliares los utilizamos para no repetir calculos.
	int aux1 = 0, aux2 = 0, aux3 = 0, aux4 = 0, aux5 = 0;
	int aux6 = 0;

	// El ciclo empieza para una matriz hipotetica de 22,15 tiles, para que no se presente problemas en el scroll.
	// Nota:Algunos tiles se calculan pero no se dibujan del todo por la misma raz�n.
	for (int j = -1; j < 14; j++)
	{
		for (int i = -1; i < 21; i++)
		{
			// Calculamos la posicion de cada til a dibujar de la matriz hipotetica.
			aux4 = xpos_ref - 10 + i;
			aux5 = ypos_ref - 6 + j;
			aux1 = mapa[aux4][aux5].actual; // Obtenemos el frame a dibujar;
			aux6 = mapa[aux4][aux5].tiles[aux1] - 1;
			// Si el tile esta dentro de los limites y no es vacio lo dibujamos
			if ((aux4 >= 0 && aux6 != -1 && aux4 < MAXMAP) && (aux5 >= 0 && aux5 < MAXMAP))
			{
				aux2 = i << 4;			   // es lo mismo que i*16, pero mas rapido;
				aux3 = j << 4;			   // es lo mismo que j*16, pero mas rapido;
				aux2 = aux2 + xpos_scroll; // lo pintamos sumamdole el corrimiento horizontal.
				aux3 = aux3 + ypos_scroll; // lo pintamos sumandole el corrimiento vertical.
				// Lo dibujamos sobre la pantalla virtual 1.
				h.putframe(h.pv1, aux2, aux3, 16, recursos.frm[aux6].img, 0);
			}
			else
			{
				// Si no esta dentro de los limites � es vacio dibujamos un cuadro en negro
				aux2 = i << 4; // i*16;
				aux3 = j << 4; // j*16;
				aux2 = aux2 + xpos_scroll;
				aux3 = aux3 + ypos_scroll;
				h.fillbox(h.pv1, aux2, aux3, aux2 + 15, aux3 + 15, 0);
			}
		}
	}

	// Si hay scroll calculamos el corrimiento para los tiles
	if (scroll)
	{
		switch (brujula)
		{
		/*
			Dependiendo de la direcci�n sumamos � restamos el corrimiento de acuerdo a su velocidad
			teniendo en cuenta de no sobrepasar el tope, si el tope
			se alcanza termina el scroll, y el personaje toma una nueva posici�n de referencia
			para dibujarlo.
		*/
		case NORTE:
			if (ypos_scroll >= tope)
			{
				ypos_scroll = 0;
				scroll = FALSE;
				xpos_ref = xpos_actual;
				ypos_ref = ypos_actual;
			}
			else
			{
				ypos_scroll = ypos_scroll + scroll_vel;
				xpos_scroll = 0;
			}
			break;
		case ESTE:
			if (xpos_scroll <= -tope)
			{
				xpos_scroll = 0;
				scroll = FALSE;
				xpos_ref = xpos_actual;
				ypos_ref = ypos_actual;
			}
			else
			{
				xpos_scroll = xpos_scroll - scroll_vel;
				ypos_scroll = 0;
			}
			break;
		case SUR:
			if (ypos_scroll <= -tope)
			{
				ypos_scroll = 0;
				scroll = FALSE;
				xpos_ref = xpos_actual;
				ypos_ref = ypos_actual;
			}
			else
			{
				ypos_scroll = ypos_scroll - scroll_vel;
				xpos_scroll = 0;
			}
			break;

		case OESTE:
			if (xpos_scroll >= tope)
			{
				xpos_scroll = 0;
				scroll = FALSE;
				xpos_ref = xpos_actual;
				ypos_ref = ypos_actual;
			}
			else
			{
				xpos_scroll = xpos_scroll + scroll_vel;
				ypos_scroll = 0;
			}
			break;
		}
	}
	else
	{
		// Si no hay scroll la posicion de refrencia para dibujarlo es la misma en donde esta.
		xpos_ref = xpos_actual;
		ypos_ref = ypos_actual;
	}
}
//-----------------------------------------------------------------------------
void Explorador::dibujarPersonaje(Graph &h)
{
	/*
		De acuerdo como nos lo indique la brujula dibujames el frame adecuado para que el personaje mire en esa direccion,
		se tiene en cuenta que cuando el corrimiento el scroll este a la mitad de un tile (8), cambie el frame para dar
		el efecto de caminado, reemplazando la imagen por otra.
	*/
	switch (brujula)
	{
	case NORTE:
		if (ypos_scroll >= 8 && scroll == TRUE)
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[4].img, 32);
		else
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[0].img, 32);
		break;
	case ESTE:
		if (xpos_scroll <= -8 && scroll == TRUE)
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[5].img, 32);
		else
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[1].img, 32);
		break;
	case SUR:
		if (ypos_scroll <= -8 && scroll == TRUE)
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[6].img, 32);
		else
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[2].img, 32);
		break;
	case OESTE:
		if (xpos_scroll >= 8 && scroll == TRUE)
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[7].img, 32);
		else
			h.putframe(h.pv1, 160, 94, 16, personaje.frm[3].img, 32);
		break;
	}
}
//----------------------------------------------------------------------------
int Explorador::cargarRecursos(char *n_recurso1, char *n_recurso2, Graph &h)
{
	if (personaje.cargarSprite(n_recurso1, h) == 0)
		return 0;
	if (recursos.cargarSprite(n_recurso2, h) == 0)
		return 0;
	return 1;
}
//----------------------------------------------------------------------------
void Explorador::dibujarMapa(Graph &h, Font &l)
{
	/*
		Variando el valor del quantum se obtiene el tiempo para la animaci�n de los tiles
		se debe variar para diferentes tipos de maquina.
	*/
	frame++; // cada ciclo equivale a un frame
	if (frame == quantum)
	{ // Si hay quantum ciclos el quanto se activa solamente por este ciclo.
		quanto = TRUE;
		frame = 0; // Vuelve a contar
		if (toggleSound == TRUE)
		{
			currentMusicIndex++;
			if (currentMusicIndex >= musicLenght)
			{
				currentMusicIndex = 0;
			}
			sound(misteryMusic[currentMusicIndex]);
			toggleSound = FALSE;
		}
		else
		{
			nosound();
			toggleSound = TRUE;
		}
	}
	else
	{
		quanto = FALSE;
	}

	// Le pasamos cada objeto del mapa para que el analizador de Estado lo modifique de acuerdo a su tipo.
	for (byte j = 0; j < MAXMAP; j++)
		for (byte i = 0; i < MAXMAP; i++)
		{
			analizadorEstado(i, j);
		}

	// dibujamos el Mapa sobre la pv1
	dibujarNormal(h);
	// Luego encima el personaje
	dibujarPersonaje(h);

	// Si en este ciclo se esta leyendo un dialogo lo dibujamos en pantalla
	if (lectura == TRUE)
	{
		l.dialbox(82, 155, t_archivo, n_parrafo, n_parrafo + 4, h, enter);
	}

	// Esperamos que el haz de electrones de la pantalla se devuelva
	h.wait_retrace();
	// Copiamos de la pv1 a la memoria de video
	h.volcar(h.vga, h.pv1);
	// limpiamos la pv1 por aseo de los datos.
	h.clr(h.pv1, 0);
}
//----------------------------------------------------------------------------------------
int Explorador::cargarMapa(char *nombre)
{
	FILE *archivo;
	if ((archivo = fopen(nombre, "r")) == NULL)
		return 0;

	// Cargamos de un archivo en el orden que muestra la linea, este archivo es generado por otro programa
	// Se recomienda el -Tile Creator de Gislersoft(R)-
	for (int j = 0; j < 40; j++)
	{
		for (int i = 0; i < 40; i++)
		{
			fscanf(archivo, "%d %d %d %d %d %s %s %d %d %d %d %d\n", &mapa[i][j].importante, &mapa[i][j].solido, &mapa[i][j].ciclico, &mapa[i][j].accion, &mapa[i][j].estado, &mapa[i][j].nombre, &mapa[i][j].archivo, &mapa[i][j].tiles[0], &mapa[i][j].tiles[1], &mapa[i][j].tiles[2], &mapa[i][j].x, &mapa[i][j].y);
		}
	}
	// cerramos el archivo
	fclose(archivo);
	return 1;
}
//---------------------------------------------------------------------------------------
void Explorador::borrarMapa()
{
	// Por aseo de los datos, colocamos todos los objetos a este tipo de objeto hipotetico que no produce nada sobre los analizadores.
	for (int j = 0; j < 40; j++)
	{
		for (int i = 0; i < 40; i++)
		{
			mapa[i][j].importante = FALSE;
			mapa[i][j].solido = FALSE;
			mapa[i][j].ciclico = FALSE;
			mapa[i][j].tocado = FALSE;
			mapa[i][j].accion = NEUTRO;
			mapa[i][j].estado = ESTATICO;
			mapa[i][j].actual = TIL1;
			mapa[i][j].nombre[0] = '/0';  // No hay nombres
			mapa[i][j].archivo[0] = '/0'; // No hay archivos
			mapa[i][j].x = 0;
			mapa[i][j].y = 0;
			mapa[i][j].tiles[0] = 0;
			mapa[i][j].tiles[1] = 0;
			mapa[i][j].tiles[2] = 0;
		}
	}
}
//-------------------------------------------------------------------------------------
void Explorador::dialogoCarga(Graph &h, Font &l, byte n1, byte n2)
{
	// Carga el dialogo que se debe imprimir sobre pantalla, y prende las banderas para su correcta visualizaci�n.
	if (lectura == FALSE)
	{					// Si no ha empezado la lectura
		n_parrafo = n1; // el primer parrafo se define
		lectura = TRUE; // empezo la lectura
	}

	if (reloj == FALSE)
	{ // Tomamos el tiempo desde la aparicion del parrafo que se esta leyendo.
		start = clock();
		reloj = TRUE; // Se empieza a contar el tiempo.
	}

	if (demorar(start, 2))
	{				   // Si han pasado dos segundos desde que empezo la lectura.
		leerTeclado(); // leemos el teclado
		enter = TRUE;  /*
				   El usuario ahora puede teclear enter, teniendo encuenta que la quinta linea del parrafo sea "ENTER".
				   Se debe tener cuidado de no colocar otra cosa diferente cada 5 lineas, pues no se mostrar� sino hasta
				   hasta dentro de 2 segundos.
				   */
	}

	if (leyendo)
	{							   // Si se presiono la tecla ENTER.
		n_parrafo = n_parrafo + 5; // corremos hasta el otro parrafo
		leyendo = FALSE;		   // apagamos la bandera, para esperar otro ENTER.
		reloj = FALSE;			   // volvemos a contar 2 segundos
		enter = FALSE;			   // esperamos hasta otro enter.
	}

	if (n_parrafo > n2)
	{ // Si el parrafo no esta dentro de las especificaciones se acabo la lectura.
		// apagamos todas las banderas.
		dialogo = FALSE;
		lectura = FALSE;
		leyendo = FALSE;
		n_parrafo = 0;
	}
	// dibujamos en pantalla.
	dibujarMapa(h, l);
}
//----------------------------------------------------------------------------
boolean Explorador::demorar(clock_t s, float t)
{
	if (((clock() - s) / CLK_TCK) > t)
		return TRUE;
	return FALSE;
}
//----------------------------------------------------------------------------
void Explorador::logoGislersoft(Graph &g)
{
	if (g.loadPCX("IMG/gsoft.pcx") == 0)
	{
		cout << "No se pudo cargar el pcx.";
		getch();
		exit(1);
	}
	g.clrpal2();
	g.modo_video(GRAFICO);
	g.pal2();
	g.volcar(g.vga, g.pv2);
	g.fade21(50); // de 1 hacia 2
	delay(2000);
	sound(440);
	delay(500);
	sound(350);
	delay(300);
	nosound();
	delay(2000);
	sound(440);
	delay(500);
	sound(293);
	delay(300);
	nosound();
	g.fadeout(50);
	// GISLERSOFT
}
//----------------------------------------------------------------------------
void Explorador::introScreen(Graph &g, Font &f)
{
	if (g.loadPCX("IMG/intro.pcx") == 0)
	{
		cout << "No se pudo cargar el pcx.";
		getch();
		exit(1);
	}
	g.clrpal2();
	g.modo_video(GRAFICO);
	g.pal2();
	g.volcar(g.vga, g.pv2);
	// Play musica parte 1
	sound(262);
	delay(500);
	sound(247);
	g.fade21(25); // de 1 hacia 2
	// Play musica parte 2
	delay(2000);
	sound(350);
	delay(500);
	sound(247);
	delay(500);
	nosound();
	sound(220);
	delay(1000);
	nosound();
	f.putstr(g.vga, 80, 180, "PRESIONE*CUALQUIER*TECLA*PARA*EMPEZAR...", g, 0, 15);
	getch();
	g.fadeout(50);
}
//----------------------------------------------------------------------------
void Explorador::aboutScreen(Graph &g, Font &f)
{
	if (g.loadPCX("IMG/lostgame.pcx") == 0)
	{
		cout << "No se pudo cargar el pcx.";
		getch();
		exit(1);
	}
	g.clrpal2();
	g.modo_video(GRAFICO);
	g.pal2();
	g.volcar(g.vga, g.pv2);
	sound(262);
	delay(300);
	nosound();
	g.fade21(25); // de 1 hacia 2
	g.volcar(g.pv1, g.pv2);
	f.dialbox(82, 155, "DIALOGS/LIN02.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	g.fadeout(50);
}
//----------------------------------------------------------------------------
void Explorador::historiaParte1(Graph &g, Font &f)
{
	if (g.loadPCX("IMG/histo1.pcx") == 0)
	{
		cout << "No se pudo cargar el pcx.";
		getch();
		exit(1);
	}
	g.clrpal2();
	g.modo_video(GRAFICO);
	g.pal2();
	g.volcar(g.vga, g.pv2);
	g.fade21(25); // de 1 hacia 2
	g.volcar(g.pv1, g.pv2);
	f.dialbox(82, 155, "DIALOGS/LIN03.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	sound(262);
	delay(300);
	nosound();
	f.dialbox(82, 155, "DIALOGS/LIN04.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	sound(262);
	delay(300);
	nosound();
	g.fadeout(50);
}
//----------------------------------------------------------------------------
void Explorador::historiaParte2(Graph &g, Font &f)
{
	if (g.loadPCX("IMG/histo2.pcx") == 0)
	{
		cout << "No se pudo cargar el pcx.";
		getch();
		exit(1);
	}
	g.clrpal2();
	g.modo_video(GRAFICO);
	g.pal2();
	g.volcar(g.vga, g.pv2);
	sound(262);
	delay(300);
	nosound();
	g.fade21(25); // de 1 hacia 2
	g.volcar(g.pv1, g.pv2);
	f.dialbox(82, 155, "DIALOGS/LIN05.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	sound(247);
	delay(300);
	nosound();
	f.dialbox(82, 155, "DIALOGS/LIN06.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	sound(262);
	delay(300);
	nosound();
	f.dialbox(82, 155, "DIALOGS/LIN07.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	sound(350);
	delay(300);
	nosound();
	f.dialbox(82, 155, "DIALOGS/LIN08.TXT", 1, 5, g, FALSE);
	g.volcar(g.vga, g.pv1);
	getch();
	sound(247);
	delay(300);
	nosound();
	g.fadeout(50);
}
//----------------------------------------------------------------------------
int main()
{

	Graph g(0);
	char numero[5] = "****";
	char numero2[5] = "****";
	Font f = Font();
	Explorador explo;
	BattleMode *battle = new BattleMode;
	if (battle == NULL)
	{
		printf("No hay suficiente Memoria");
		getch();
		return 1;
	}
	if (g.status() == 0)
	{
		printf("No hay suficiente Memoria");
		getch();
		return 1;
	}

	// GISLERSOFT
	explo.logoGislersoft(g);
	// Intro logo
	explo.introScreen(g, f);
	// About game
	explo.aboutScreen(g, f);
	// Historia Parte 1
	explo.historiaParte1(g, f);
	// Historia Parte 2
	explo.historiaParte2(g, f);

	if (explo.cargarRecursos("IMG/b16.pcx", "IMG/16x16.pcx", g) == 0)
	{
		printf("No hay suficiente memoria");
		exit(1);
	}
	g.copiar(); // copiamos la paleta 1 a la 2;
	g.clr(g.vga, 0);
	g.clr(g.pv1, 0);
	explo.actualizarPosicion(20, 18); // Cuando empieza debe estar paradosobre un objeto neutro y estatico
	explo.actualizarBrujula(SUR);
	if (explo.cargarMapa("MAPS/casa.txt") == 0)
	{
		printf("Error");
		exit(1);
	}

	// g.modo_video(GRAFICO);
	g.pal();
	explo.dibujarMapa(g, f);
	while (explo.salir != TRUE)
	{
		if (explo.dialogo == FALSE)
		{
			if (explo.batalla == TRUE) {
				if (battle->loaded == FALSE) {
					battle->loadBatalla(g);
					battle->loaded = TRUE;
				}
				battle->modoBatalla(g,f);
			} else {
				explo.leerTeclado();
				if (explo.toco == TRUE)
				{
					explo.toco = FALSE;
					explo.actualizarObjeto();
					if (explo.importante())
					{
						// Guardamos el nombre en el archivo mochila
					}
					if (explo.neutro())
					{
						explo.dibujarMapa(g, f);
					}
					else
					{
						explo.analizadorAccion();
					}
				}
				else
				{
					if (explo.camino == TRUE)
					{
						explo.camino = FALSE;
						explo.actualizarObjeto();
						if (explo.solido() == TRUE)
						{
							explo.dibujarMapa(g, f);
						}
						else
						{
							if (explo.importante())
							{
								// Guardamos el nombre en el archivo mochila
							}
							if (explo.neutro())
							{
								explo.dibujarMapa(g, f);
							}
							else
							{
								explo.analizadorAccion();
							}
						}
					}
					else
					{
						explo.dibujarMapa(g, f);
					}
				}

				if (explo.cargar_nuevo && explo.scroll == FALSE)
				{
					strcpy(explo.t_archivo, "");
					strcat(explo.t_archivo, "MAPS/");
					strcat(explo.t_archivo, mapa[explo.xpos_objeto][explo.ypos_objeto].archivo);
					strcat(explo.t_archivo, ".TXT");
					explo.actualizarPosicion(mapa[explo.xpos_objeto][explo.ypos_objeto].x - 1, mapa[explo.xpos_objeto][explo.ypos_objeto].y - 1);
					explo.borrarMapa();
					if (explo.cargarMapa(explo.t_archivo) == 0)
					{
						printf("Error: no se pudo abrir %s", explo.t_archivo);
						getch();
						exit(0);
					}
					g.fadeout(10);
					explo.dibujarMapa(g, f);
					g.fade12(10);
					explo.actualizarObjeto();
					explo.cargar_nuevo = FALSE;
				}
				/*
				f.toString(explo.xpos_objeto,numero);
				f.toString(explo.ypos_objeto,numero2);
				*/
				/*
				f.toString(explo.xpos_actual,numero);
				f.toString(explo.ypos_actual,numero2);

				//	explo.dibujarMapa(g);

				//dialbox(82,155,"DIALOGS/lineas.txt",1,5,g,f);
				f.putstr(g.vga,225,190,"GISLERSOFT",g,0,15);
				f.putstr(g.vga,275,190,"X:",g,0,15);
				f.putstr(g.vga,295,190,"Y:",g,0,15);
				*/

				f.putstr(g.vga, 284, 190, numero, g, 0, 15);
				f.putstr(g.vga, 304, 190, numero2, g, 0, 15);

				/*
				if(mapa[explo.xpos_objeto][explo.ypos_objeto].solido==TRUE)f.putstr(g.vga,195,190,"TRUE",g,0,15);
				else f.putstr(g.vga,195,190,"FALSE",g,0,15);
				*/
			}
		}
		else
		{
			strcpy(explo.t_archivo, "");
			strcat(explo.t_archivo, "DIALOGS/");
			strcat(explo.t_archivo, mapa[explo.xpos_objeto][explo.ypos_objeto].archivo);
			strcat(explo.t_archivo, ".TXT");
			explo.dialogoCarga(g, f, mapa[explo.xpos_objeto][explo.ypos_objeto].x, mapa[explo.xpos_objeto][explo.ypos_objeto].y);
		}
		*(char far *)MK_FP(0x40, 0x1a) = *(char far *)MK_FP(0x40, 0x1c); // Evita el retrazado y el chilleo de teclas
	}
	g.modo_video(TEXTO);
	delete battle;
	g.destruir();
	exit(1);
	return 0;
}
