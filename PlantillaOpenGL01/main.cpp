#include <math.h>
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>


/* 

- Se mueve raro, hay que revisarlo.
- Hay que colocar la introduccion por teclado de los valores
para los L, A, S y D de cada ola.
- No se para, hay que ver como hacer para que se reinicie.

*/
using namespace std;

#define DEF_floorGridScale	1.0
#define DEF_floorGridXSteps	10.0
#define DEF_floorGridZSteps	10.0

#define PI 3.14159265358979f

GLfloat L1 = 8.0,L2 = 4.0,A1 = 0.4,A2 = 0.0,S1 = 2.0,S2 = 0.0; /* inicializo en los valores del caso de prueba 1 */
GLfloat D1[2] = {0.0,-1.0},D2[2]={1.0,1.0}; // 0 coord x, 1 coord y


GLfloat knots[25]; 
GLfloat supKnots = 0.05; // valor auxiliar para los knots del 4 al 20 
GLfloat ctlpoints[21][21][3]; // 21x21 puntos de control; 0 coord x, 1 coord y, 2 coord z

bool wave; // true = ola 1; false = ola 1
int mover; // variable para saber si la animacion debe iniciar o parar

GLfloat waves[2]; // arreglo para manejar las cosas de cada ola

// Variables para la funcion
GLfloat frecuency[2]; // w de la funcion
GLfloat phase[2]; // phase-constant de la funcion
GLfloat time = 0.0; // tiempo de la funcion
GLfloat D1normalizado[2],D2normalizado[2]; 

GLUnurbsObj *theNurb;

void ejesCoordenada() {
	
	glLineWidth(2.5);
	glBegin(GL_LINES);
		glColor3f(1.0,0.0,0.0);
		glVertex2f(0,10);
		glVertex2f(0,-10);
		glColor3f(0.0,0.0,1.0);
		glVertex2f(10,0);
		glVertex2f(-10,0);
	glEnd();

	glLineWidth(1.5);
	int i;
	glColor3f(0.0,1.0,0.0);
	glBegin(GL_LINES);
		for(i = -10; i <=10; i++){
			if (i!=0) {		
				if ((i%2)==0){	
					glVertex2f(i,0.4);
					glVertex2f(i,-0.4);

					glVertex2f(0.4,i);
					glVertex2f(-0.4,i);
				}else{
					glVertex2f(i,0.2);
					glVertex2f(i,-0.2);

					glVertex2f(0.2,i);
					glVertex2f(-0.2,i);

				}
			}
		}
		
	glEnd();

	glLineWidth(1.0);
}

void changeViewport(int w, int h) {
	
	float aspectratio;

	if (h==0)
		h=1;

	
   glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity ();
   gluPerspective(30, (GLfloat) w/(GLfloat) h, 1.0, 200.0);
   glMatrixMode (GL_MODELVIEW);

}

void init_surface() {

	float cx = 10.0;
	float cy = 0.0;
	float cz = 10.0;

	// Dibujar puntos de control
	for (int i = 0; i <21; i++) {
			cx = 10.0;
			for (int j = 0; j < 21; j++) {
	            ctlpoints[i][j][0] = cx; 	
				ctlpoints[i][j][1] = cy; 
				ctlpoints[i][j][2] = cz;
				cx -= 1.0;
			}
			cz -= 1.0;
	}

	// Superficie Knots
	
	for (int i = 0; i < 25; i++) {
		if (i < 4) {
			knots[i] = 0.0;
		}
		else if (i > 20) {
			knots[i] = 1.0;
		}
		else {
			knots[i] += supKnots;
			supKnots += 0.05; 
		}
	}
	
}

void init(){

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_AUTO_NORMAL);
   glEnable(GL_NORMALIZE);

   init_surface();

   theNurb = gluNewNurbsRenderer();
   gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 15.0);
   gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
	

}

float funcionH(GLfloat cx, GLfloat cz, GLfloat tiempo){

	frecuency[0] = 2*PI/L1;
	frecuency[1] = 2*PI/L2;

	phase[0] = S1*frecuency[0];
	phase[1] = S2*frecuency[1];

	D1normalizado[0] = D1[0]/sqrt(pow (D1[0],2) + pow (D1[1],2));
	D1normalizado[1] = D1[1]/sqrt(pow (D1[0],2) + pow (D1[1],2));

	D2normalizado[0] = D2[0]/sqrt(pow (D2[0],2) + pow (D2[1],2));
	D2normalizado[1] = D2[1]/sqrt(pow (D2[0],2) + pow (D2[1],2));
	
	waves[0] = A1*sin((D1normalizado[0] + D1normalizado[1])*frecuency[0]+ tiempo*phase[0]);
	waves[1] = A2*sin((D2normalizado[0] + D2normalizado[1])*frecuency[1]+ tiempo*phase[1]);

	return (waves[0] + waves[1]);
	
}


void animacionOla(int movimiento) {
	if (movimiento > 0)	{
		for (int i=0; i < 21; i++){
			for (int j=0; j < 21; j++){
				ctlpoints[i][j][1] = funcionH(ctlpoints[i][j][0], ctlpoints[i][j][2],time); // funcion Y = f(x,z)
			}
		}
		time += 0.1;
		glutTimerFunc(20,animacionOla,1);
		glutPostRedisplay();
	}
}

void Keyboard(unsigned char key, int x, int y)
{
	
	if (key == 'r'){
		mover = 1;
		animacionOla(mover);// comienza la animacion de las olas.
	}
	else if (key == 'p'){
		mover = 0;// se pausa la animacion de las olas.
	}
	else if (key == '1'){ // Wave 1
		wave = true;
	}
	else if (key == '2'){ // Wave 2
		wave = false;
	}
	else if (key == 'a'){
		if (wave) L1 = L1 - 0.1;
		else L2 = L2 - 0.1;
	}
	else if (key == 'z'){
		if (wave) L1 = L1 + 0.1;
		else L2 = L2 + 0.1;
	}
	else if (key == 's'){
		if (wave) A1 = A1 - 0.1;
		else A2 = A2 - 0.1;
	}
	else if (key == 'x'){
		if (wave) A1 = A1 + 0.1;
		else A2 = A2 + 0.1;
	}
	else if (key == 'd'){
		if (wave) S1 = S1 - 0.1;
		else S2 = S2 - 0.1;
	}
	else if (key == 'c'){
		if (wave) S1 = S1 + 0.1;
		else S2 = S2 + 0.1;
	}
	else if (key == 'f'){
		if (wave) D1[0] = D1[0] - 0.1;
		else D2[0] = D2[0] - 0.1;
	}
	else if (key == 'v'){
		if (wave) D1[0] = D1[0] + 0.1;
		else D2[0] = D2[0] + 0.1;
	}
	else if (key == 'g'){
		if (wave) D1[1] = D1[1] - 0.1;
		else D2[1] = D2[1] - 0.1;
	}
	else if (key == 'b'){
		if (wave) D1[1] = D1[1] + 0.1;
		else D2[1] = D2[1] - 0.1;
	}
	switch (key)
	{
		case 27:             
			exit (0);
			break;
	
	}
}



void render(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat zExtent, xExtent, xLocal, zLocal;
    int loopX, loopZ;

	glLoadIdentity ();                       
	gluLookAt (25.0, 12.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	

	// Luz y material

	GLfloat mat_diffuse[] = { 0.6, 0.6, 0.9, 1.0 };
	GLfloat mat_specular[] = { 0.8, 0.8, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 60.0 };
	
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	

    GLfloat light_ambient[] = { 0.0, 0.0, 0.2, 1.0 };
	GLfloat light_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat light_specular[] = { 0.6, 0.6, 0.6, 1.0 };
	GLfloat light_position[] = { -10.0, 5.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);   


	/*
	// Render Grid 
	glDisable(GL_LIGHTING);
	glLineWidth(1.0);
	glPushMatrix();
	glRotatef(90,1.0,0.0,0.0);
    glColor3f( 0.0, 0.7, 0.7 );
    glBegin( GL_LINES );
    zExtent = DEF_floorGridScale * DEF_floorGridZSteps;
    for(loopX = -DEF_floorGridXSteps; loopX <= DEF_floorGridXSteps; loopX++ )
	{
	xLocal = DEF_floorGridScale * loopX;
	glVertex3f( xLocal, -zExtent, 0.0 );
	glVertex3f( xLocal, zExtent,  0.0 );
	}
    xExtent = DEF_floorGridScale * DEF_floorGridXSteps;
    for(loopZ = -DEF_floorGridZSteps; loopZ <= DEF_floorGridZSteps; loopZ++ )
	{
	zLocal = DEF_floorGridScale * loopZ;
	glVertex3f( -xExtent, zLocal, 0.0 );
	glVertex3f(  xExtent, zLocal, 0.0 );
	}
    glEnd();
	ejesCoordenada();
    glPopMatrix();
	glEnable(GL_LIGHTING);
	// Fin Grid
	*/


	//Suaviza las lineas
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_LINE_SMOOTH );	

	
	glPushMatrix();

	gluBeginSurface(theNurb);
    
	gluNurbsSurface(theNurb, 
                   25, knots, 25, knots,
                   21 * 3, 3, &ctlpoints[0][0][0], 
                   4, 4, GL_MAP2_VERTEX_3);
	/*

		No cambien los numeros de la funcion, solo deben de poner los nombres de las variables correspondiente.
		
	*/


	gluEndSurface(theNurb);
	
	glPopMatrix();
	
	
	/* Muestra los puntos de control */
	/*
		int i,j;
		glPointSize(5.0);
		glDisable(GL_LIGHTING);
		glColor3f(1.0, 1.0, 0.0);
		glBegin(GL_POINTS);
		for (i = 0; i <21; i++) {
			for (j = 0; j < 21; j++) {
	            glVertex3f(ctlpoints[i][j][0], 	ctlpoints[i][j][1], ctlpoints[i][j][2]);
			}
		}
		glEnd();
		glEnable(GL_LIGHTING);
	*/
		

	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);

	glutSwapBuffers();
}

void animacion(int value) {
	
	glutTimerFunc(10,animacion,1);
    glutPostRedisplay();
	
}

int main (int argc, char** argv) {

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	glutInitWindowSize(960,540);

	glutCreateWindow("Nurbs Proyecto - Ola");

	init ();

	glutReshapeFunc(changeViewport);
	glutDisplayFunc(render);
	glutKeyboardFunc (Keyboard);
		
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW error");
		return 1;
	}
	

	glutMainLoop();
	return 0;

}