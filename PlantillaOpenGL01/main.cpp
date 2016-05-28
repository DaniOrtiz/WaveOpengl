#include <math.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <GL\glew.h>
#include <GL\freeglut.h>

/* 

- M_PI es la constante que viene en la libreria de math.h, avisame si te funciona
- Cambie las variables a arreglos para que fuera mas sencillo mostrar el texto
- agregue la funcion aumentar y disminuir para que influyera menos la resta de punto flotante 
cuando esta cerca del 0

*/
using namespace std;

#define DEF_floorGridScale  1.0
#define DEF_floorGridXSteps 10.0
#define DEF_floorGridZSteps 10.0

GLfloat L[2] = {8.0,4.0};
GLfloat A[2] = {0.4,0.0};
GLfloat S[2] = {2.0,0.0};
GLfloat D[2][2] = {{0.0,-1.0},{1.0,1.0}}; // 0 coord x, 1 coord z
GLfloat knots[25]; 
GLfloat supKnots = 0.05;      // valor auxiliar para los knots del 4 al 20 
GLfloat ctlpoints[21][21][3]; // 21x21 puntos de control; 0 coord x, 1 coord y, 2 coord z

float posText;      //posicion del texto
float interlineado; //interlineado del texto
GLvoid *font_style = GLUT_BITMAP_9_BY_15; //tipo de letra

bool wave  = true;            // true = ola 1; false = ola 2
bool mover = false;           // variable que indica si las olas estan en movimeinto o paradas
bool ctlpointsActive = false; // activar puntos de control

GLfloat waves[2]; // arreglo para manejar las cosas de cada ola

// Variables para la funcion
GLfloat frecuency[2]; // w de la funcion
GLfloat phase[2];     // phase-constant de la funcion
GLfloat wtime = 0.0;  // tiempo de la funcion
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

// -------------------------------TEXTO-------------------------------
const char* textos[7] = {
    "Ola ",
    "wL = ",  
    "aP = ",
    "sP = ",
    "dirX = ",
    "dirY = ",
    "-> Ola "
  };

void imprimir_bitmap_string(void* font, const char* s){
    bool continuar = true;
    if (s && strlen(s)) {
        while (*s && continuar) {
            glutBitmapCharacter(font, *s);
            if(*s == '.'){
                s++;
                glutBitmapCharacter(font, *s);
                continuar = false;
            }
            s++;
        }
    }
}

void convertirTexto(const char* s, float i){
    glRasterPos3f(0, posText, 0);
    posText -= interlineado;

    std::stringstream ss;
    ss << i;
    std::string num(ss.str());
    string c = s + num;
    const char *C = c.c_str();
    imprimir_bitmap_string(font_style, C);
}

void dibujarVariables(int i, int s){
  glColor3f(0.5f,0.0f,0.8f);
  convertirTexto(textos[s],i);

  glColor3f(1.0,1.0,1.0);
  convertirTexto(textos[1],L[i-1]);
  convertirTexto(textos[2],A[i-1]);
  convertirTexto(textos[3],S[i-1]);
  convertirTexto(textos[4],D[i-1][0]);
  convertirTexto(textos[5],D[i-1][1]);
}

void dibujarTexto() {
  posText = 0.0;

  if(wave) dibujarVariables(1, 6);
  else dibujarVariables(1, 0);

  glColor3f(0.7,0.7,0.7);
  glRasterPos3f(0, posText, 0);
  posText -= interlineado;
  imprimir_bitmap_string(font_style, "==========");

  if(wave) dibujarVariables(2, 0);
  else dibujarVariables(2, 6);
}
// ----------------------------FIN TEXTO----------------------------

void changeViewport(int w, int h) {
  float aspectratio;

  if (h==0) h=1;

  glViewport (0, 0, (GLsizei) w, (GLsizei) h); 

  if(h <320) interlineado = 0.6;
  else if(h < 430) interlineado = 0.5;
  else interlineado = 0.4;

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

    frecuency[0] = 2*M_PI/L[0];
    frecuency[1] = 2*M_PI/L[1];

    phase[0] = S[0]*frecuency[0];
    phase[1] = S[1]*frecuency[1];

    D1normalizado[0] = D[0][0]/sqrt(pow (D[0][0],2) + pow (D[0][1],2));
    D1normalizado[1] = D[0][1]/sqrt(pow (D[0][0],2) + pow (D[0][1],2));

    D2normalizado[0] = D[1][0]/sqrt(pow (D[1][0],2) + pow (D[1][1],2));
    D2normalizado[1] = D[1][1]/sqrt(pow (D[1][0],2) + pow (D[1][1],2));
    
    waves[0] = A[0]*sin((D1normalizado[0]*cx + D1normalizado[1]*cz)*frecuency[0]+ tiempo*phase[0]);
    waves[1] = A[1]*sin((D2normalizado[0]*cx + D2normalizado[1]*cz)*frecuency[1]+ tiempo*phase[1]);

    return (waves[0] + waves[1]);
    
}

void animacionOla(int h) {
    if( h > 0 && mover){
        for (int i=0; i < 21; i++){
            for (int j=0; j < 21; j++){
                ctlpoints[i][j][1] = funcionH(ctlpoints[i][j][0], ctlpoints[i][j][2],wtime); // funcion Y = f(x,z)
            }
        }
        wtime += 0.1;
        glutTimerFunc(20,animacionOla,1);
        glutPostRedisplay();        
    }
}

GLfloat aumentar(GLfloat i){
    if((i + 0.1) < 0.1  && i + 0.1 >= -0.001) i = 0.0;
    else i += 0.1;
    return i;
}

GLfloat disminuir(GLfloat i){
    if((i - 0.1) > -0.1 && (i - 0.1) <= 0.001 ) i = 0.0;
    else i -= 0.1;
    return i;
}

void Keyboard(unsigned char key, int x, int y){
    switch (key){
        case ' ': //activa los putnos de control
            if(ctlpointsActive) ctlpointsActive = false;
            else ctlpointsActive = true;
            if(!mover) glutPostRedisplay(); 
        break;
        case 'r':
            if(!mover){
                mover = true;
                animacionOla(1);// comienza la animacion de las olas.
            }
        break;
        case 'p':
            mover = false;// se pausa la animacion de las olas.
        break;
        case '1': // Wave 1
            wave = true;    
        break;
        case '2': // Wave 2
            wave = false;     
        break;
        case 'a':
            if (wave) L[0] = disminuir(L[0]);
            else L[1] = disminuir(L[1]);
        break;
        case 'z':
            if (wave) L[0] = aumentar(L[0]);
            else L[1] = aumentar(L[1]);
        break;
        case 's':
            if (wave) A[0] = disminuir(A[0]);
            else A[1] = disminuir(A[1]);
        break;
        case 'x':
            if (wave) A[0] = aumentar(A[0]);
            else A[1] = aumentar(A[1]);
        break;
        case 'd':
            if (wave) S[0] = disminuir(S[0]);
            else S[1] = disminuir(S[1]);
        break;
        case 'c':
            if (wave) S[0] = aumentar(S[0]);
            else S[1] = aumentar(S[1]);
        break;
        case 'f':
            if (wave) D[0][0] = disminuir(D[0][0]);
            else D[1][0] = disminuir(D[1][0]);
        break;
        case 'v':
            if (wave) D[0][0] = aumentar(D[0][0]);
            else D[1][0] = aumentar(D[1][0]);
        break;
        case 'g':
            if (wave) D[0][1] = disminuir(D[0][1]);
            else D[1][1] = disminuir(D[1][1]);
        break;
        case 'b':
            if (wave) D[0][1] = aumentar(D[0][1]);
            else D[1][1] = aumentar(D[1][1]);
        break;
        default:
        break;
    }
    if(!mover) glutPostRedisplay();   
}

void render(){
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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
    if(ctlpointsActive){
        int i,j;
        glPointSize(5.0);
        glDisable(GL_LIGHTING);
        glColor3f(1.0, 1.0, 0.0);
        glBegin(GL_POINTS);
        for (i = 0; i <21; i++) {
            for (j = 0; j < 21; j++) {
                glVertex3f(ctlpoints[i][j][0],  ctlpoints[i][j][1], ctlpoints[i][j][2]);
            }
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
    
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);

    //TEXTO
    glPushMatrix();
      glDisable(GL_LIGHTING);
      glRotatef(-10,1.0f,0.0f,0.0f); 
      glTranslatef(0,5.2,11.4); 
      dibujarTexto();
      glEnable(GL_LIGHTING);
    glPopMatrix();

    glutSwapBuffers();
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