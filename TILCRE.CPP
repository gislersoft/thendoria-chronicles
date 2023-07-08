#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>

/*===========================[Archivos Cabecera Custom]============================*/
#include "vgapoo.h"
#include "fnt13h.h"
#include "spr13h.h"
#include "ratoli.h"

/*=======================[Definicion de constantes]=========================*/
#define MAXCHAROBJ 9 // Maximo de caracteres para el nombre de un objeto
#define MAXTILEOBJ 3 // Maximo de tiles por objeto
#define MAXARCHOBJ 8 // Maximo de caracteres para el nombre del archivo asociado al objeto
#define MAXMAP 40	 // Tama?o maximo del mapa
#define SCROLL_VEL 2 // Velocidad del Scroll
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

typedef unsigned char byte;

/*====================[Definicion de la estructura objeto]==================*/
typedef struct objeto
{

	boolean importante; // Si el objeto es importante
	boolean solido;		// Si el objeto es solido
	boolean ciclico;	// Si el objeto es ciclico

	tipoAccion accion; // Tipo de accion asociada al objeto
	tipoEstado estado; // El tipo de estado del objeto

	char nombre[MAXCHAROBJ];  // Contiene el nombre del objeto
	char archivo[MAXARCHOBJ]; // Contiene el nombre del archivo asociado al objeto
	int tiles[MAXTILEOBJ];	  // Los tiles asociados del objeto
	byte x, y;				  // Posicion del tile que debe modificar si es de tipo TELEACCION
};

objeto mapa[MAXMAP][MAXMAP]; // Variable que contiene todos los objetos

class Creador
{
public:
	boolean salir;
	boolean copiar;
	boolean poner;
	objeto actual;
	objeto pegado;
	objeto modificado;
	int xmouse;
	int ymouse;

	Sprite recursos;

	Creador();
	~Creador(){};
	void dibujarMapa(Graph &h);
	void dibujarMenu(Graph &h, Font &l);
	void dibujarMenu2(Graph &h, Font &l);
	void dibujarMenu3(Graph &h, Sprite &s);
	void puntero(Graph &h, Sprite &s);			 // dibuja el puntero
	int comando();								 // Devuelve el comando que se clickeo
	void ejecutar(Graph &h, Sprite &s, int com); // Ejecuta el comando com
	void over(Graph &h, Font &l);				 // Da el efecto de roll over
	void imprimirAccion(Graph &h, Font &l, int op, int x, int y);
	void imprimirEstado(Graph &h, Font &l, int op, int x, int y);
	void imprimirBooleano(Graph &h, Font &l, int op, int x, int y);
	void dibujarCelda(Graph &h, int x, int y, int til);
	int guardarArchivo(char *nombre);
	int abrirArchivo(char *nombre);
	int til_i();
	int til_j();
	int tilx();
	int tily();
	void leerMouse();
	void dibujarCuadro(Graph &h, int x, int y);
};
void Creador::Creador()
{
	salir = FALSE;
	copiar = FALSE;
	poner = TRUE;
	recursos = Sprite(240, 16);
	modificado.accion = NEUTRO;
	modificado.estado = ESTATICO;
	modificado.solido = FALSE;
	modificado.importante = FALSE;
	modificado.ciclico = FALSE;
	strcpy(modificado.nombre, "NULL");
	strcpy(modificado.archivo, "NULL");
	for (int i = 0; i < 3; i++)
		modificado.tiles[i] = 0;
	modificado.x = 0;
	modificado.y = 0;
	actual = modificado;
	pegado = modificado;
	for (int j = 0; j < 40; j++)
		for (i = 0; i < 40; i++)
			mapa[i][j] = modificado;
}
void Creador::leerMouse()
{
	xmouse = pos_x() / 2;
	ymouse = pos_y();
}
void Creador::dibujarMapa(Graph &h)
{
	int aux1, aux2, aux3;
	for (int j = 0; j < 40; j++)
	{
		for (int i = 0; i < 40; i++)
		{
			aux1 = i << 2;
			aux2 = j << 2;
			aux3 = mapa[i][j].tiles[0] - 1;
			if (aux3 != -1)
				dibujarCelda(h, aux1, aux2, aux3);
			else
				h.box(h.pv1, aux1, aux2, aux1 + 4, aux2 + 4, 15);
		}
	}
}
void Creador::dibujarCelda(Graph &h, int x, int y, int til)
{
	int aux1, aux2;
	aux1 = x;
	aux2 = y;
	for (int j = 0; j < 16; j = j + 4)
	{
		for (int i = 0; i < 16; i = i + 4)
		{
			h.putpixel(h.pv1, aux1, aux2, recursos.frm[til].img[i + (j << 4)]);
			aux1++;
		}
		aux1 = x;
		aux2++;
	}
}
void Creador::dibujarMenu(Graph &h, Font &l)
{
	l.putstr(h.pv1, 197, 10, "ACCION", h, 3, 15);
	h.box(h.pv1, 195, 8, 237, 16, 15);
	l.putstr(h.pv1, 197, 20, "ESTADO", h, 3, 15);
	h.box(h.pv1, 195, 18, 237, 26, 15);
	l.putstr(h.pv1, 197, 30, "IMPORTANTE", h, 3, 15);
	h.box(h.pv1, 195, 28, 237, 36, 15);
	l.putstr(h.pv1, 197, 40, "SOLIDO", h, 3, 15);
	h.box(h.pv1, 195, 38, 237, 46, 15);
	l.putstr(h.pv1, 197, 50, "CICLICO", h, 3, 15);
	h.box(h.pv1, 195, 48, 237, 56, 15);
	l.putstr(h.pv1, 197, 60, "TILES", h, 3, 15);
	h.box(h.pv1, 195, 58, 237, 66, 15);
	l.putstr(h.pv1, 197, 70, "POSICION", h, 3, 15);
	h.box(h.pv1, 195, 68, 237, 76, 15);
	l.putstr(h.pv1, 197, 80, "NOMBRE", h, 3, 15);
	h.box(h.pv1, 195, 78, 237, 86, 15);
	l.putstr(h.pv1, 197, 90, "ARCHIVO", h, 3, 15);
	h.box(h.pv1, 195, 88, 237, 96, 15);
}
void Creador::dibujarMenu2(Graph &h, Font &l)
{
	int aux;

	l.putstr(h.pv1, 10, 180, "COPIAR", h, 3, 15);
	if (copiar == FALSE)
		h.box(h.pv1, 9, 178, 34, 186, 15);
	else
		h.box(h.pv1, 9, 178, 34, 186, 67);

	l.putstr(h.pv1, 100, 180, "LLENAR", h, 3, 15);
	h.box(h.pv1, 98, 178, 123, 186, 15);

	l.putstr(h.pv1, 130, 180, "CLONAR", h, 3, 15);
	h.box(h.pv1, 128, 178, 153, 186, 15);

	l.putstr(h.pv1, 165, 18, "ACTUAL", h, 3, 15);
	aux = actual.tiles[0] - 1; // El primer til
	if (aux != -1)
		h.putframe(h.pv1, 165, 0, 16, recursos.frm[aux].img, 0);
	else
		h.box(h.pv1, 165, 0, 181, 16, 15);

	//-----------------------------------------------
	l.putstr(h.pv1, 165, 46, "MODIF", h, 3, 15);
	aux = modificado.tiles[0] - 1; // El primer til
	if (aux != -1)
		h.putframe(h.pv1, 165, 28, 16, recursos.frm[aux].img, 0);
	else
		h.box(h.pv1, 165, 28, 181, 44, 15);

	l.putstr(h.pv1, 165, 76, "PEGAR", h, 3, 15);
	aux = pegado.tiles[0] - 1; // El primer til
	if (aux != -1)
		h.putframe(h.pv1, 165, 58, 16, recursos.frm[aux].img, 0);
	else
		h.box(h.pv1, 165, 58, 181, 74, 15);

	//-----------------------------------------------
	int x_center = til_i() - 1;
	int y_center = til_j() - 1;
	int start_x = 239;

	if ((x_center >= 0) && (y_center >= 0))
	{
		if ((xmouse >= 0 && xmouse <= 160) && (ymouse >= 0 && ymouse <= 160))
		{
			if ((x_center >= 0) && (y_center >= 0))
			{
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 5; j++)
					{
						int tile_x_index = x_center + (1 * i) - 2;
						int tile_y_index = y_center + (1 * j) - 2;

						if ((tile_x_index >= 0 && tile_x_index < 40) &&
							(tile_y_index >= 0 && tile_y_index < 40))
						{
							aux = mapa[tile_x_index][tile_y_index].tiles[0] - 1; // El primer til
							if (aux != -1 && recursos.frm[aux].img != NULL)
							{
								h.putframe(h.pv1, start_x + (i * 16), 0 + (j * 16), 16, recursos.frm[aux].img, 0);
								// h.box(h.pv1, start_x + (i*16), 0 + (j*16), 271 + (i*16), 16 + (j*16), 109);
							}
							else
							{
								h.fillbox(h.pv1, start_x + (i * 16), 0 + (j * 16), 271 + (i * 16), 16 + (j * 16), 0);
							}
						}
						else
						{
							h.fillbox(h.pv1, start_x + (i * 16), 0 + (j * 16), 271 + (i * 16), 16 + (j * 16), 0);
						}
					}
				}
			}
			// Muestre el til seleccionado resaltado
			h.box(h.pv1, start_x + 32, 32, start_x + 32 + 16, 16 + 32, 67);
		}
	}

	//-----------------------------------------------
}
/*------------------------------*/
void Creador::dibujarMenu3(Graph &h, Sprite &s)
{
	h.putframe(h.pv1, 200, 170, 20, s.frm[4].img, 0);
	h.putframe(h.pv1, 225, 170, 20, s.frm[5].img, 0);
	h.putframe(h.pv1, 250, 170, 20, s.frm[7].img, 0);
}
void Creador::puntero(Graph &h, Sprite &s)
{
	h.putframe(h.pv1, pos_x() / 2, pos_y(), 20, s.frm[17].img, 0);
}
/*------------------------------*/
/*------------------------------*/
int Creador::comando()
{
	int x;
	int y;

	if (boton() == 1)
	{
		x = xmouse;
		y = ymouse;

		if ((x >= 200) && (x <= 220) && (y >= 170) && (y <= 190))
			return 1;
		if ((x >= 225) && (x <= 245) && (y >= 170) && (y <= 190))
			return 2;
		if ((x >= 250) && (x <= 270) && (y >= 170) && (y <= 190))
			return 3;
		if ((x >= 195) && (x <= 237) && (y >= 8) && (y <= 16))
			return 4;
		if ((x >= 195) && (x <= 237) && (y >= 18) && (y <= 26))
			return 5;
		if ((x >= 195) && (x <= 237) && (y >= 28) && (y <= 36))
			return 6;
		if ((x >= 195) && (x <= 237) && (y >= 38) && (y <= 46))
			return 7;
		if ((x >= 195) && (x <= 237) && (y >= 48) && (y <= 56))
			return 8;
		if ((x >= 195) && (x <= 237) && (y >= 58) && (y <= 66))
			return 9;
		if ((x >= 195) && (x <= 237) && (y >= 68) && (y <= 76))
			return 10;
		if ((x >= 195) && (x <= 237) && (y >= 78) && (y <= 86))
			return 11;
		if ((x >= 195) && (x <= 237) && (y >= 88) && (y <= 96))
			return 12;
		if ((x >= 9) && (x <= 34) && (y >= 178) && (y <= 186))
			return 13;
		if ((x >= 165) && (x <= 181) && (y >= 58) && (y <= 74))
			return 14;
		if ((x >= 165) && (x <= 181) && (y >= 28) && (y <= 44))
			return 15;
		if ((x >= 0) && (x <= 160) && (y >= 0) && (y <= 160))
			return 16;
		if ((x >= 98) && (x <= 123) && (y >= 178) && (y <= 186))
			return 17;
		if ((x >= 128) && (x <= 153) && (y >= 178) && (y <= 186))
			return 18;
	}

	return 0;
}
void Creador::over(Graph &h, Font &l)
{
	int x, y;
	int aux1, aux2, aux3, aux4;
	int aux6, aux7, aux8;
	char *num1 = "**", *num2 = "**";

	x = xmouse;
	y = ymouse;

	if ((x >= 200) && (x <= 220) && (y >= 170) && (y <= 190))
		h.box(h.pv1, 200, 170, 220, 190, 109);
	if ((x >= 225) && (x <= 245) && (y >= 170) && (y <= 190))
		h.box(h.pv1, 225, 170, 245, 190, 109);
	if ((x >= 250) && (x <= 270) && (y >= 170) && (y <= 190))
		h.box(h.pv1, 250, 170, 270, 190, 109);

	if ((x >= 195) && (x <= 237) && (y >= 8) && (y <= 16))
		h.box(h.pv1, 195, 8, 237, 16, 109);
	if ((x >= 195) && (x <= 237) && (y >= 18) && (y <= 26))
		h.box(h.pv1, 195, 18, 237, 26, 109);
	if ((x >= 195) && (x <= 237) && (y >= 28) && (y <= 36))
		h.box(h.pv1, 195, 28, 237, 36, 109);
	if ((x >= 195) && (x <= 237) && (y >= 38) && (y <= 46))
		h.box(h.pv1, 195, 38, 237, 46, 109);
	if ((x >= 195) && (x <= 237) && (y >= 48) && (y <= 56))
		h.box(h.pv1, 195, 48, 237, 56, 109);
	if ((x >= 195) && (x <= 237) && (y >= 58) && (y <= 66))
		h.box(h.pv1, 195, 58, 237, 66, 109);
	if ((x >= 195) && (x <= 237) && (y >= 68) && (y <= 76))
		h.box(h.pv1, 195, 68, 237, 76, 109);
	if ((x >= 195) && (x <= 237) && (y >= 78) && (y <= 86))
		h.box(h.pv1, 195, 78, 237, 86, 109);
	if ((x >= 195) && (x <= 237) && (y >= 88) && (y <= 96))
		h.box(h.pv1, 195, 88, 237, 96, 109);

	if ((x >= 9) && (x <= 34) && (y >= 178) && (y <= 186))
		h.box(h.pv1, 9, 178, 34, 186, 109);

	if ((x >= 98) && (x <= 123) && (y >= 178) && (y <= 186))
		h.box(h.pv1, 98, 178, 123, 186, 109);
	if ((x >= 128) && (x <= 153) && (y >= 178) && (y <= 186))
		h.box(h.pv1, 128, 178, 153, 186, 109);

	if ((x >= 165) && (x <= 181) && (y >= 0) && (y <= 17))
		h.box(h.pv1, 165, 0, 181, 16, 100);

	if ((x >= 165) && (x <= 181) && (y >= 28) && (y <= 44))
		h.box(h.pv1, 165, 28, 181, 44, 109);
	if ((x >= 165) && (x <= 181) && (y >= 58) && (y <= 74))
		h.box(h.pv1, 165, 58, 181, 74, 109);

	if ((x >= 165) && (x <= 181) && (y >= 58) && (y <= 74))
	{
		l.putstr(h.pv1, 180, 104, "ACCION:", h, 0, 15);
		imprimirAccion(h, l, (int)pegado.accion, 230, 104);
		l.putstr(h.pv1, 180, 110, "ESTADO:", h, 0, 15);
		imprimirEstado(h, l, (int)pegado.estado, 230, 110);
		l.putstr(h.pv1, 180, 116, "IMPORTANTE:", h, 0, 15);
		imprimirBooleano(h, l, (int)pegado.importante, 230, 116);
		l.putstr(h.pv1, 180, 122, "SOLIDO:", h, 0, 15);
		imprimirBooleano(h, l, (int)pegado.solido, 230, 122);
		l.putstr(h.pv1, 180, 128, "CICLICO:", h, 0, 15);
		imprimirBooleano(h, l, (int)pegado.ciclico, 230, 128);
		l.putstr(h.pv1, 180, 134, "NOMBRE:", h, 0, 15);
		l.putstr(h.pv1, 230, 134, pegado.nombre, h, 1, 15);
		l.putstr(h.pv1, 180, 140, "ARCHIVO:", h, 0, 15);
		l.putstr(h.pv1, 230, 140, pegado.archivo, h, 1, 15);
		l.putstr(h.pv1, 180, 146, "POSICION:", h, 0, 15);
		l.toString(pegado.x, num1);
		l.toString(pegado.y, num2);
		l.putstr(h.pv1, 230, 146, num1, h, 3, 15);
		l.putstr(h.pv1, 237, 146, ",*", h, 0, 15);
		l.putstr(h.pv1, 242, 146, num2, h, 3, 15);
		l.putstr(h.pv1, 180, 152, "PARRAFOS:", h, 0, 15);
		l.putstr(h.pv1, 230, 152, num1, h, 3, 15);
		l.putstr(h.pv1, 180, 158, "PORTAPAPELES", h, 0, 4);

		aux6 = pegado.tiles[0] - 1;
		aux7 = pegado.tiles[1] - 1;
		aux8 = pegado.tiles[2] - 1;

		l.putstr(h.pv1, 268, 90, "TILES", h, 0, 15);
		h.box(h.pv1, 270, 100, 285, 115, 109);
		h.box(h.pv1, 270, 120, 285, 135, 109);
		h.box(h.pv1, 270, 140, 285, 155, 109);

		if (aux6 != -1)
			h.putframe(h.pv1, 270, 100, 16, recursos.frm[aux6].img, 0);
		if (aux7 != -1)
			h.putframe(h.pv1, 270, 120, 16, recursos.frm[aux7].img, 0);
		if (aux8 != -1)
			h.putframe(h.pv1, 270, 140, 16, recursos.frm[aux8].img, 0);
	}

	if (((x >= 128) && (x <= 153) && (y >= 178) && (y <= 186)) ||
		((x >= 165) && (x <= 181) && (y >= 28) && (y <= 44)))
	{
		l.putstr(h.pv1, 180, 104, "ACCION:", h, 0, 15);
		imprimirAccion(h, l, (int)modificado.accion, 230, 104);
		l.putstr(h.pv1, 180, 110, "ESTADO:", h, 0, 15);
		imprimirEstado(h, l, (int)modificado.estado, 230, 110);
		l.putstr(h.pv1, 180, 116, "IMPORTANTE:", h, 0, 15);
		imprimirBooleano(h, l, (int)modificado.importante, 230, 116);
		l.putstr(h.pv1, 180, 122, "SOLIDO:", h, 0, 15);
		imprimirBooleano(h, l, (int)modificado.solido, 230, 122);
		l.putstr(h.pv1, 180, 128, "CICLICO:", h, 0, 15);
		imprimirBooleano(h, l, (int)modificado.ciclico, 230, 128);
		l.putstr(h.pv1, 180, 134, "NOMBRE:", h, 0, 15);
		l.putstr(h.pv1, 230, 134, modificado.nombre, h, 1, 15);
		l.putstr(h.pv1, 180, 140, "ARCHIVO:", h, 0, 15);
		l.putstr(h.pv1, 230, 140, modificado.archivo, h, 1, 15);
		l.putstr(h.pv1, 180, 146, "POSICION:", h, 0, 15);
		l.toString(modificado.x, num1);
		l.toString(modificado.y, num2);
		l.putstr(h.pv1, 230, 146, num1, h, 3, 15);
		l.putstr(h.pv1, 237, 146, ",*", h, 0, 15);
		l.putstr(h.pv1, 242, 146, num2, h, 3, 15);
		l.putstr(h.pv1, 180, 152, "PARRAFOS:", h, 0, 15);
		l.putstr(h.pv1, 230, 152, num1, h, 3, 15);
		l.putstr(h.pv1, 180, 158, "MODIFICADO", h, 0, 32);

		// Mostramos los tiles asociados
		aux6 = modificado.tiles[0] - 1;
		aux7 = modificado.tiles[1] - 1;
		aux8 = modificado.tiles[2] - 1;

		l.putstr(h.pv1, 268, 90, "TILES", h, 0, 15);
		h.box(h.pv1, 270, 100, 285, 115, 109);
		h.box(h.pv1, 270, 120, 285, 135, 109);
		h.box(h.pv1, 270, 140, 285, 155, 109);

		if (aux6 != -1)
			h.putframe(h.pv1, 270, 100, 16, recursos.frm[aux6].img, 0);
		if (aux7 != -1)
			h.putframe(h.pv1, 270, 120, 16, recursos.frm[aux7].img, 0);
		if (aux8 != -1)
			h.putframe(h.pv1, 270, 140, 16, recursos.frm[aux8].img, 0);
	}
	if (((x >= 165) && (x <= 181) && (y >= 0) && (y <= 17)) ||
		((x >= 98) && (x <= 123) && (y >= 178) && (y <= 186)))
	{
		l.putstr(h.pv1, 180, 104, "ACCION:", h, 0, 15);
		imprimirAccion(h, l, (int)actual.accion, 230, 104);
		l.putstr(h.pv1, 180, 110, "ESTADO:", h, 0, 15);
		imprimirEstado(h, l, (int)actual.estado, 230, 110);
		l.putstr(h.pv1, 180, 116, "IMPORTANTE:", h, 0, 15);
		imprimirBooleano(h, l, (int)actual.importante, 230, 116);
		l.putstr(h.pv1, 180, 122, "SOLIDO:", h, 0, 15);
		imprimirBooleano(h, l, (int)actual.solido, 230, 122);
		l.putstr(h.pv1, 180, 128, "CICLICO:", h, 0, 15);
		imprimirBooleano(h, l, (int)actual.ciclico, 230, 128);
		l.putstr(h.pv1, 180, 134, "NOMBRE:", h, 0, 15);
		l.putstr(h.pv1, 230, 134, actual.nombre, h, 1, 15);
		l.putstr(h.pv1, 180, 140, "ARCHIVO:", h, 0, 15);
		l.putstr(h.pv1, 230, 140, actual.archivo, h, 1, 15);
		l.putstr(h.pv1, 180, 146, "POSICION:", h, 0, 15);
		l.toString(actual.x, num1);
		l.toString(actual.y, num2);
		l.putstr(h.pv1, 230, 146, num1, h, 3, 15);
		l.putstr(h.pv1, 237, 146, ",*", h, 0, 15);
		l.putstr(h.pv1, 242, 146, num2, h, 3, 15);
		l.putstr(h.pv1, 180, 152, "PARRAFOS:", h, 0, 15);
		l.putstr(h.pv1, 230, 152, num1, h, 3, 15);
		l.putstr(h.pv1, 180, 158, "ACTUAL", h, 0, 9);

		// Mostramos los tiles asociados
		aux6 = actual.tiles[0] - 1;
		aux7 = actual.tiles[1] - 1;
		aux8 = actual.tiles[2] - 1;

		l.putstr(h.pv1, 268, 90, "TILES", h, 0, 15);
		h.box(h.pv1, 270, 100, 285, 115, 109);
		h.box(h.pv1, 270, 120, 285, 135, 109);
		h.box(h.pv1, 270, 140, 285, 155, 109);

		if (aux6 != -1)
			h.putframe(h.pv1, 270, 100, 16, recursos.frm[aux6].img, 0);
		if (aux7 != -1)
			h.putframe(h.pv1, 270, 120, 16, recursos.frm[aux7].img, 0);
		if (aux8 != -1)
			h.putframe(h.pv1, 270, 140, 16, recursos.frm[aux8].img, 0);
	}

	aux1 = til_i();
	aux2 = til_j();
	if ((aux1 != 0) && (aux2 != 0))
	{
		aux3 = (aux1 - 1) * 4;
		aux4 = (aux2 - 1) * 4;
		h.box(h.pv1, aux3, aux4, aux3 + 4, aux4 + 4, 67);
		l.toString(aux1, num1);
		l.toString(aux2, num2);
		l.putstr(h.pv1, 110, 164, "X:", h, 0, 15);
		l.putstr(h.pv1, 120, 164, num1, h, 2, 15);
		l.putstr(h.pv1, 135, 164, "Y:", h, 0, 15);
		l.putstr(h.pv1, 145, 164, num2, h, 2, 15);
		//******MUESTRA EL OBJETO
		l.putstr(h.pv1, 180, 104, "ACCION:", h, 0, 15);
		imprimirAccion(h, l, (int)mapa[aux1 - 1][aux2 - 1].accion, 230, 104);
		l.putstr(h.pv1, 180, 110, "ESTADO:", h, 0, 15);
		imprimirEstado(h, l, (int)mapa[aux1 - 1][aux2 - 1].estado, 230, 110);
		l.putstr(h.pv1, 180, 116, "IMPORTANTE:", h, 0, 15);
		imprimirBooleano(h, l, (int)mapa[aux1 - 1][aux2 - 1].importante, 230, 116);
		l.putstr(h.pv1, 180, 122, "SOLIDO:", h, 0, 15);
		imprimirBooleano(h, l, (int)mapa[aux1 - 1][aux2 - 1].solido, 230, 122);
		l.putstr(h.pv1, 180, 128, "CICLICO:", h, 0, 15);
		imprimirBooleano(h, l, (int)mapa[aux1 - 1][aux2 - 1].ciclico, 230, 128);
		l.putstr(h.pv1, 180, 134, "NOMBRE:", h, 0, 15);
		l.putstr(h.pv1, 230, 134, mapa[aux1 - 1][aux2 - 1].nombre, h, 1, 15);
		l.putstr(h.pv1, 180, 140, "ARCHIVO:", h, 0, 15);
		l.putstr(h.pv1, 230, 140, mapa[aux1 - 1][aux2 - 1].archivo, h, 1, 15);
		l.putstr(h.pv1, 180, 146, "POSICION:", h, 0, 15);
		l.toString(mapa[aux1 - 1][aux2 - 1].x, num1);
		l.toString(mapa[aux1 - 1][aux2 - 1].y, num2);
		l.putstr(h.pv1, 230, 146, num1, h, 3, 15);
		l.putstr(h.pv1, 237, 146, ",*", h, 0, 15);
		l.putstr(h.pv1, 242, 146, num2, h, 3, 15);
		l.putstr(h.pv1, 180, 152, "PARRAFOS:", h, 0, 15);
		l.putstr(h.pv1, 230, 152, num1, h, 3, 15);
		l.putstr(h.pv1, 180, 158, "MOUSE*OVER", h, 0, 23);

		aux6 = mapa[aux1 - 1][aux2 - 1].tiles[0] - 1;
		aux7 = mapa[aux1 - 1][aux2 - 1].tiles[1] - 1;
		aux8 = mapa[aux1 - 1][aux2 - 1].tiles[2] - 1;

		l.putstr(h.pv1, 268, 90, "TILES", h, 0, 15);
		h.box(h.pv1, 270, 100, 285, 115, 109);
		h.box(h.pv1, 270, 120, 285, 135, 109);
		h.box(h.pv1, 270, 140, 285, 155, 109);

		if (aux6 != -1)
			h.putframe(h.pv1, 270, 100, 16, recursos.frm[aux6].img, 0);
		if (aux7 != -1)
			h.putframe(h.pv1, 270, 120, 16, recursos.frm[aux7].img, 0);
		if (aux8 != -1)
			h.putframe(h.pv1, 270, 140, 16, recursos.frm[aux8].img, 0);
	}
}
void Creador::imprimirAccion(Graph &h, Font &l, int op, int x, int y)
{
	switch (op)
	{
	case 0:
		l.putstr(h.pv1, x, y, "PREGUNTA", h, 2, 15);
		break;
	case 1:
		l.putstr(h.pv1, x, y, "ACCION", h, 2, 15);
		break;
	case 2:
		l.putstr(h.pv1, x, y, "NEUTRO", h, 2, 15);
		break;
	case 3:
		l.putstr(h.pv1, x, y, "TELEACCION", h, 2, 15);
		break;
	case 4:
		l.putstr(h.pv1, x, y, "SALIDA", h, 2, 15);
		break;
	}
}
void Creador::imprimirEstado(Graph &h, Font &l, int op, int x, int y)
{
	switch (op)
	{
	case 0:
		l.putstr(h.pv1, x, y, "ESTATICO", h, 2, 15);
		break;
	case 1:
		l.putstr(h.pv1, x, y, "ANIMADO", h, 2, 15);
		break;
	case 2:
		l.putstr(h.pv1, x, y, "SUICHE", h, 2, 15);
		break;
	}
}
void Creador::imprimirBooleano(Graph &h, Font &l, int op, int x, int y)
{
	switch (op)
	{
	case 0:
		l.putstr(h.pv1, x, y, "FALSE", h, 2, 15);
		break;
	case 1:
		l.putstr(h.pv1, x, y, "TRUE", h, 2, 15);
		break;
	}
}
void Creador::ejecutar(Graph &h, Sprite &s, int com)
{
	int op, aux1, aux2;
	char aux3[30];
	switch (com)
	{
	case 1:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[GUARDAR ARCHIVO]";
		cout << "\n\n			Nombre del archivo:";
		cin >> aux3;
		if (guardarArchivo(strcat("MAPS/", aux3)) == 0)
		{
			cout << "No se pudo guardar!";
			getch();
		}
		else
		{
			cout << "Archivo guardado!";
			getch();
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 2:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[ABRIR ARCHIVO]";
		cout << "\n\n			Nombre del archivo:";
		cin >> aux3;
		if (abrirArchivo(strcat("MAPS/", aux3)) == 0)
		{
			cout << "No se pudo abrir!";
			getch();
		}
		else
		{
			cout << "Archivo abierto satisfactoriamente!";
			getch();
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 3:
		salir = TRUE;
		break;
	case 4:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[TIPO ACCION]";
		cout << "\n\n	[0] PREGUNTA";
		cout << "\n	[1] ACCION";
		cout << "\n	[2] NEUTRO";
		cout << "\n	[3] TELEACCION";
		cout << "\n	[4] SALIDA";
		cout << "\n\n			Opcion:";
		cin >> op;
		if (op < 0 && op > 4)
		{
			cout << "\nOpcion no valida! ...";
			getch();
		}
		else
		{
			modificado.accion = (tipoAccion)op;
			if (op == 0)
			{
				cout << "\n\n	Digite el numero de parrafos del guion (maximo 255):";
				cin >> op;
				modificado.x = op;
			}
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 5:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[TIPO ESTADO]";
		printf("\n\n	[0] ESTATICO");
		cout << "\n	[1] ANIMADO";
		cout << "\n	[2] SUICHE";
		cout << "\n\n			Opcion:";
		cin >> op;
		if (op < 0 && op > 3)
		{
			cout << "\nOpcion no valida! ...";
			getch();
		}
		else
		{
			modificado.estado = (tipoEstado)op;
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 6:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[IMPORTANTE]";
		cout << "\n\n	[0] FALSE";
		cout << "\n	[1] TRUE";
		cout << "\n\n			Opcion:";
		cin >> op;
		if (op < 0 && op > 1)
		{
			cout << "\nOpcion no valida! ...";
			getch();
		}
		else
		{
			modificado.importante = (boolean)op;
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 7:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[SOLIDO]";
		cout << "\n\n	[0] FALSE";
		cout << "\n	[1] TRUE";
		cout << "\n\n			Opcion:";
		cin >> op;
		if (op < 0 && op > 1)
		{
			cout << "\nOpcion no valida! ...";
			getch();
		}
		else
		{
			modificado.solido = (boolean)op;
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;

	case 8:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[CICLICO]";
		cout << "\n\n	[0] FALSE";
		cout << "\n	[1] TRUE";
		cout << "\n\n			Opcion:";
		cin >> op;
		if (op < 0 && op > 1)
		{
			cout << "\nOpcion no valida! ...";
			getch();
		}
		else
		{
			modificado.ciclico = (boolean)op;
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 9:
		h.modo_video(TEXTO);
		int nveces;
		cout << "\n	[TILES]";
		cout << "\n\n	(Max 3 tiles por objeto)";
		cout << "\n	Digite el numero de tiles a ingresar:";
		cin >> nveces;
		for (int i = 0; i < 3; i++)
			modificado.tiles[i] = 0;
		for (i = 0; i < nveces; i++)
		{
			h.modo_video(TEXTO);
			cout << "Escoja el til numero " << i + 1 << " con el mouse.";
			cout << "\nPresione una tecla para continuar...";
			getch();
			h.modo_video(GRAFICO);
			h.pal();
			while (boton() != 1)
			{
				h.volcar(h.pv1, h.pv2);
				aux1 = tilx();
				aux2 = tily();
				dibujarCuadro(h, tilx() << 4, tily() << 4);
				puntero(h, s);
				h.volcar(h.vga, h.pv1);
			}
			modificado.tiles[i] = aux1 + aux2 * 20 + 1;
			h.modo_video(TEXTO);
			cout << aux1 + aux2 * 20 + 1;
			getch();
			h.modo_video(GRAFICO);
			h.pal();
		}

		break;
	case 10:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[POSICION]";
		cout << "\n\n	Digite x:";
		cin >> aux1;
		cout << "\n	Digite y:";
		cin >> aux2;
		if (aux1 >= 0 && aux1 < 40)
		{
			if (aux2 >= 0 && aux2 < 40)
			{
				modificado.x = aux1;
				modificado.y = aux2;
			}
			else
			{
				cout << "Punto fuera del rango.";
				getch();
			}
		}
		else
		{
			cout << "Punto fuera del rango.";
			getch();
		}

		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 11:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[NOMBRE]";
		cout << "\n\n	Digite el nombre del objeto (Max 8 Caracteres):";
		cin >> aux3;
		if (strlen(aux3) > 8)
		{
			cout << "\nDigito mas de 8 caracteres.";
			getch();
		}
		else
		{
			for (i = 0; i < strlen(aux3); i++)
			{
				aux3[i] = toupper(aux3[i]);
			}
			strcpy(modificado.nombre, aux3);
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 12:
		h.modo_video(TEXTO);
		clrscr();
		cout << "\n	[ARCHIVO ASOCIADO]";
		cout << "\n\n	(Max 9 caracteres)";
		cout << "\n	Digite el archivo sin extensiones:";
		cin >> aux3;
		if (strlen(aux3) > 9)
		{
			cout << "\nDigito mas de 9 caracteres.";
			getch();
		}
		else
		{
			for (i = 0; i < strlen(aux3); i++)
			{
				aux3[i] = toupper(aux3[i]);
			}
			strcpy(modificado.archivo, aux3);
		}
		h.modo_video(GRAFICO);
		h.pal();
		break;
	case 13:
		copiar = TRUE;
		break;
	case 14:
		actual = pegado;
		poner = FALSE;
		break;
	case 15:
		actual = modificado;
		poner = TRUE;
		break;
	case 16:
		if (copiar == FALSE)
			mapa[til_i() - 1][til_j() - 1] = actual;
		else
		{
			pegado = mapa[til_i() - 1][til_j() - 1];
			actual = pegado; // DIRECTO DEL PORTAPAPELES AL ACTUAL
			copiar = FALSE;
			delay(100);
		}
		break;
	case 17:
		for (int j = 0; j < 40; j++)
		{
			for (int i = 0; i < 40; i++)
			{
				mapa[j][i] = actual;
			}
		}
		delay(100);
		break;
	case 18:
		modificado = actual;
		delay(100);
		break;
	}
}
/*------------------------------*/
int Creador::til_i()
{
	int x, aux;
	x = xmouse;
	for (int i = 0; i < 40; i++)
	{
		aux = i * 4;
		if ((x >= aux) && (x <= aux + 4))
			return (i + 1);
	}
	return 0;
}
/*------------------------------*/
int Creador::til_j()
{
	int y, aux;
	y = ymouse;
	for (int i = 0; i < 40; i++)
	{
		aux = i * 4;
		if ((y >= aux) && (y < aux + 4))
			return (i + 1);
	}
	return 0;
}
/*------------------------------*/
int Creador::tilx()
{
	int x, aux;
	x = pos_x() / 2;
	for (int i = 0; i < 20; i++)
	{
		aux = i * 16;
		if ((x >= aux) && (x <= aux + 16))
			return (i);
	}
	return 0;
}
int Creador::tily()
{
	int y, aux;
	y = pos_y();
	for (int i = 0; i < 12; i++)
	{
		aux = i * 16;
		if ((y >= aux) && (y < aux + 16))
			return (i);
	}
	return 0;
}
void Creador::dibujarCuadro(Graph &h, int x, int y)
{
	h.box(h.pv1, x, y, x + 16, y + 16, 109);
}
int Creador::guardarArchivo(char *nombre)
{
	FILE *archivo;
	if ((archivo = fopen(nombre, "w")) == NULL)
		return 0;

	for (int j = 0; j < 40; j++)
	{
		for (int i = 0; i < 40; i++)
		{
			fprintf(archivo, "%d %d %d %d %d %s %s %d %d %d %d %d\n", mapa[i][j].importante, mapa[i][j].solido, mapa[i][j].ciclico, mapa[i][j].accion, mapa[i][j].estado, mapa[i][j].nombre, mapa[i][j].archivo, mapa[i][j].tiles[0], mapa[i][j].tiles[1], mapa[i][j].tiles[2], mapa[i][j].x, mapa[i][j].y);
		}
	}
	fclose(archivo);
	return 1;
}
int Creador::abrirArchivo(char *nombre)
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
/*
//----------------------------------------------------------------------------
//	Primitivas
//----------------------------------------------------------------------------
void	putpixel( unsigned char far *px, int x, int y,  byte color);
void	circle	( unsigned char far *px, int x, int y,  int r, byte color);
void	ball	( unsigned char far *px, int x, int y,  int r, byte color);
void	line	( unsigned char far *px, int a, int b,  int c, int d,  byte col);
void	hline	( unsigned char far *px, int x1,int x2, int y, byte color );
void	vline	( unsigned char far *px, int x, int y1, int y2,byte color );
void	fillbox	( unsigned char far *px, int x1,int y1, int x2,int y2, byte color );
void	box	( unsigned char far *px, int x1,int y1, int x2,int y2, byte color );
byte	getpixel( unsigned char far *px, int x, int y);
*/
void Error(char *n)
{
	cout << n;
	exit(1);
}
void main()
{

	Graph g(0);
	Font f;
	Sprite iconos = Sprite(21, 20); // Creamos el espacio en memoria para los iconos
	Creador c;
	if (g.status() == 0)
		Error("No hay suficiente memoria.");
	if ((iconos.status() == 0))
		Error("No hay suficiente memoria.");
	if (iconos.cargarSprite("IMG/ICO_TIL.PCX", g) == 0)
		Error("No se pudo abrir ICO_TIL.PCX");

	if (c.recursos.status() == 0)
	{
		Error("No hay suficiente memoria!");
	}
	if (c.recursos.cargarSprite("IMG/16x16.pcx", g) == 0)
	{
		Error("No se pudo cargar el archivo!");
	}
	g.modo_video(GRAFICO);
	initmouse();
	g.pal();
	mouseoff();
	mouse_xy(154, 96);
	while (!c.salir)
	{
		c.leerMouse();
		g.clr(g.pv1, 0);
		c.dibujarMapa(g);
		c.dibujarMenu(g, f);
		c.dibujarMenu2(g, f);
		c.dibujarMenu3(g, iconos);
		c.over(g, f);
		c.puntero(g, iconos);
		g.volcar(g.vga, g.pv1);
		c.ejecutar(g, iconos, c.comando());
	}
	g.modo_video(TEXTO);
}
