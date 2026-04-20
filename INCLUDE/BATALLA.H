
class Batalla{
public: int nmonstruos;
	int npersonajes;
	int magia_escogida;
	int n1,n2,nyn;

	Personaje m[6];
	Personaje pro[3];

	Batalla(int a,int b);
	~Batalla();
	void CargarMonstruos();
	void CargarPersonajes();
	void mostrar_golpe(char *golpe,Sprite &s,Font &f,Graph &h);
	void marcar_personaje(Sprite &s,int c,Graph &h);
	void Mostrartabla(Font &l,Graph &h);
	void menu_magia(int p,Font &l,Graph &h,int &o,int &o2);
	void contrincantes(int p1,int p2,int condicion);
	void menu(int &o,Font &r,Graph &h);
	int ataque();
};
void Batalla::Batalla(int a,int b){
       nmonstruos=a;
       npersonajes=b;
}
void Batalla::~Batalla(){
	cout<<"Se destruyo el Objeto Batalla exitosamente!\n";
}
void Batalla::CargarMonstruos(){
    EnemDat e;
    e.hp=400;
    e.mp=100;
    e.defensa=10;
    e.ataque=30;
    e.magia=10;
    e.def=0.03;
    e.atk=0.01;
    e.mag=0.05;
    e.nivel=1;
    for(int j=0;j<5;j++){
    e.arr_magia[j]=1;
    }
    for(int i=0;i<nmonstruos;i++){
	m[i]=Personaje(e);
    }
}
void Batalla::CargarPersonajes(){
    EnemDat e;
    e.hp=524;
    e.mp=100;
    e.defensa=3;
    e.ataque=100;
    e.magia=15;
    e.def=0.05;
    e.atk=0.03;
    e.mag=0.04;
    e.nivel=3;
    e.arr_magia[0]=1;
    e.arr_magia[1]=2;
    e.arr_magia[2]=0;
    e.arr_magia[3]=2;
    e.arr_magia[4]=3;
    for(int i=0;i<npersonajes;i++){
	pro[i]=Personaje(e);
    }
}
void Batalla::Mostrartabla(Font &l,Graph &h){
  char *str1="****";
  char *str2="****";
  int ancho;

  pro[0].convertir(pro[0].hpN,str2);
  pro[0].convertir(pro[0].hp,str1);

  l.putstr(h.pv1,3,171,"HP",h,0,0);
  l.putstr(h.pv1,3,182,"MP",h,0,186);
  l.putstr(h.pv1,143,164,str2,h,0,0);

  pro[0].convertir(pro[0].mpN,str2);
  l.putstr(h.pv1,143,177,str2,h,0,0);


  l.putstr(h.pv1,124,164,str1,h,2,0);
  ancho=(int) pro[0].hp/pro[0].escala1;
  if(ancho>33) ancho=33;
  h.fillbox(h.pv1,124,170,124+ancho,173,32);

  h.box(h.pv1,124,170,157,173,0);

  pro[0].convertir(pro[0].mp,str1);

  l.putstr(h.pv1,124,177,str1,h,3,0);
  ancho=(int) pro[0].mp/pro[0].escala2;
  if(ancho>33) ancho=33;
  h.fillbox(h.pv1,124,183,124+ancho,186,84);
  h.box(h.pv1,124,183,157,186,0);

  l.putstr(h.pv1,174,177,"VACIO",h,0,32);
  l.putstr(h.pv1,254,177,"VACIO",h,0,32);
}
void Batalla::menu_magia(int p,Font &l,Graph &h,int &o,int &o2){

	h.fillbox(h.pv1,65,105,95,161,230);
	h.box(h.pv1,65,105,95,161,15);
	l.putstr(h.pv1,70,110,"AIRE",h,1,0);
	l.putstr(h.pv1,70,120,"AGUA",h,1,0);
	l.putstr(h.pv1,70,130,"FUEGO",h,1,0);
	l.putstr(h.pv1,70,140,"TIERRA",h,1,0);
	l.putstr(h.pv1,70,150,"VIDA",h,1,0);
	switch(o){
	case 0: o=5; break;
	case 1:	l.putstr(h.pv1,70,110,"AIRE",h,2,0); magia_escogida=0;break;
	case 2:	l.putstr(h.pv1,70,120,"AGUA",h,2,0); magia_escogida=1;break;
	case 3:	l.putstr(h.pv1,70,130,"FUEGO",h,2,0); magia_escogida=2; break;
	case 4:	l.putstr(h.pv1,70,140,"TIERRA",h,2,0); magia_escogida=3;break;
	case 5:	l.putstr(h.pv1,70,150,"VIDA",h,2,0); magia_escogida=4;break;
	case 6: o=1; break;
	case 7: h.fillbox(h.pv1,95,125,125,161,230);
		h.box(h.pv1,95,125,125,161,15);

		switch(pro[p].arr_magia[magia_escogida]){
		case 3:l.putstr(h.pv1,100,150,"NIVEL3",h,1,0);
		case 2:l.putstr(h.pv1,100,140,"NIVEL2",h,1,0);
		case 1:l.putstr(h.pv1,100,130,"NIVEL1",h,1,0);
		}

		switch(o2){
		case 0: o2=4; break;
		case 1: if(pro[p].arr_magia[magia_escogida]>0){
			   l.putstr(h.pv1,100,130,"NIVEL1",h,2,0);
			}
			else l.putstr(h.pv1,100,130,"VACIO",h,0,0);
			break;
		case 2: if(pro[p].arr_magia[magia_escogida]>1){
			   l.putstr(h.pv1,100,140,"NIVEL2",h,2,0);
			}
			else l.putstr(h.pv1,100,140,"VACIO",h,0,0);
			break;
		case 3: if(pro[p].arr_magia[magia_escogida]>2){
			   l.putstr(h.pv1,100,150,"NIVEL3",h,2,0);
			}
			else l.putstr(h.pv1,100,150,"VACIO",h,0,0);
			break;
		case 4: o2=1; break;
		}
		break;
	}
}
void Batalla::contrincantes(int p1,int p2,int condicion){
       n1=p1;
       n2=p2;
       nyn=condicion;
}
int Batalla::ataque(){
     if(nyn==1) return m[n2].defender(pro[n1].atacar());
     else return pro[n1].defender(m[n2].atacar());
}
void Batalla::menu(int &o,Font &r,Graph &h){
      r.putstr(h.pv1,40,170,"ATACAR",h,1,0);
      r.putstr(h.pv1,40,180,"MAGIA",h,1,0);
      r.putstr(h.pv1,40,190,"ITEM",h,1,0);
      switch(o){
	  case 0: o=3;break;
	  case 1: r.putstr(h.pv1,40,170,"ATACAR",h,2,0);break;
	  case 2: r.putstr(h.pv1,40,180,"MAGIA",h,2,0); break;
	  case 3: r.putstr(h.pv1,40,190,"ITEM",h,2,0); break;
	  case 4: o=1;
      }
}
void Batalla::marcar_personaje(Sprite &s,int c,Graph &h){
    h.box(h.pv1,s.x1,s.y1,s.x2,s.y2,c);
}

void Batalla::mostrar_golpe(char *golpe,Sprite &s,Font &f,Graph &h){
    h.fillbox(h.pv1,s.x1-1,s.y1-1,s.x1+16,s.y1+6,0);
    f.putstr(h.pv1,s.x1,s.y1,golpe,h,0,15);
}
