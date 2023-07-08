//Gislersoft(R)
//Libreria para el uso del mouse en cualquier tipo de modo
//grafico ¢ texto
#include <dos.h>

/* NOTA:    Valores que retorna la funci¢ boton():
			   BOTONES DEL MOUSE
	-----------------------------------------------
	|  Valor:  |  Izquierda: |  medio  |  Derecha|
	-----------------------------------------------
	|    0     |     NO      |    NO    |   NO    |
	|    1     |     SI      |    NO    |   NO    |
	|    2     |     NO      |    NO    |   SI    |
	|    3     |     SI      |    NO    |   SI    |
	|    4     |     NO      |    SI    |   NO    |
	|    5     |     SI      |    SI    |   NO    |
	|    6     |     NO      |    SI    |   SI    |
	|    7     |     SI      |    SI    |   SI    |
	-----------------------------------------------
*/

union REGS rin,rout;

/*
-----------------------------------------------------------------------------
  INITMOUSE: Inicialitza el mouse.
-----------------------------------------------------------------------------
*/
int initmouse(void)
{
  rin.x.ax=0;
  int86(0x33,&rin,&rout);
  return (rout.x.ax);            /* Retorna -1 si el mouse se ha instalado */																 /*         0 si no instalado*/
}

/*
-----------------------------------------------------------------------------
  MOUSEOFF: Apaga el cursor del mouse.
-----------------------------------------------------------------------------
*/
void mouseoff(void)
{
  rin.x.ax=2;
  int86(0x33,&rin,&rout);
}

/*
-----------------------------------------------------------------------------
  MOUSEON: Para aparecer el cursor del mouse.
-----------------------------------------------------------------------------
*/
void mouseon(void)
{
  rin.x.ax=1;
  int86(0x33,&rin,&rout);
}

/*
-----------------------------------------------------------------------------
  pos_X: Retorna la posici¢n horizontal del mouse (depende de la resoluci¢n del
    texto de los gr ficos).
-----------------------------------------------------------------------------
*/
int pos_x(void)
{
  rin.x.ax=3;
  int86(0x33,&rin,&rout);
  return(rout.x.cx);
}

/*
-----------------------------------------------------------------------------
  pos_Y: Retorna la posici¢n vertical del mouse.
-----------------------------------------------------------------------------
*/
int pos_y(void)
{
  rin.x.ax=3;
  int86(0x33,&rin,&rout);
  return(rout.x.dx);
}

/*
-----------------------------------------------------------------------------
  BOTON: Retorna los valores de los botones. Ver la tabla del principio para saber
    los valores exactos.
-----------------------------------------------------------------------------
*/
int boton(void)
{
  rin.x.ax=3;
  int86(0x33,&rin,&rout);
  return(rout.x.bx);
}

/*
-----------------------------------------------------------------------------
  mouse_xy: Coloca el cursor del mouse en la posici¢ (x,y).
-----------------------------------------------------------------------------
*/
void mouse_xy(int x, int y)
{
  rin.x.ax=4;
  rin.x.cx=x;
  rin.x.dx=y;
  int86(0x33,&rin,&rout);
}