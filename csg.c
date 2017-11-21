#include <GL/glut.h>
#include <math.h>

GLfloat ASPECTO, ANGULO;
GLfloat obsX, obsY, obsZ, rotX, rotY;
GLfloat obsX_ini, obsY_ini, obsZ_ini, rotX_ini, rotY_ini;
GLfloat escalaX, escalaY, escalaZ;
int x_ini,y_ini, bot;
#define SENS_ROT 10.0
#define SENS_OBS 10.0
#define SENS_TRANS 10.0

enum {
		CYL,
		CON,
		CUB,
		SPH,
		CYL_OR_CON,
		CYL_OR_CUB,
		CYL_OR_SPH,
		CON_OR_CUB,
		CON_OR_SPH,
		CUB_OR_SPH,
		CYL_AND_CON,
		CYL_AND_CUB,
		CYL_AND_SPH,
		CON_AND_CUB,
		CON_AND_SPH,
		CUB_AND_SPH,
		CYL_SUB_CON,
		CYL_SUB_CUB,
		CYL_SUB_SPH,
		CON_SUB_CUB,
		CON_SUB_SPH,
		CUB_SUB_SPH	
	  };

enum {SPHERE = 1, CONE, CUBE, CYLINDER};
int csg_op = CYL;
bool reverse = true; 
GLfloat viewangle;

/* Draw a cone */
GLfloat coneX = 0.f, coneY = 0.f, coneZ = 0.f;
void cone(void)
{
  glPushMatrix();
  glTranslatef(coneX, coneY, coneZ);
  glRotatef(-90.0, 1.f, 0.f, 0.f);
  glCallList(CONE);
  glPopMatrix();
}

/* Draw a sphere */
GLfloat sphereX = 0.f, sphereY = 0.f, sphereZ = 0.f;
void sphere(void)
{
  glPushMatrix();
  glTranslatef(sphereX, sphereY, sphereZ);
  glTranslatef(0, 20, 0);
  glRotatef(-90.0, 1.f, 0.f, 0.f);
  glCallList(SPHERE);
  glPopMatrix();

}

/* Draw a cube */
GLfloat cubeX = 0.f, cubeY = 0.f, cubeZ = 0.f;
void cube() {
  glPushMatrix();
  glTranslatef(cubeX, cubeY, cubeZ);
  glTranslatef(0, 20, 0);
  glCallList(CUBE);
  glPopMatrix();
}

GLfloat cylX = 0.f, cylY = 0.f, cylZ = 0.f;
void cylinder() {
  glPushMatrix();
  glTranslatef(cylX, cylY, cylZ);
  glRotatef(-90.0, 1.f, 0.f, 0.f);
  glCallList(CYLINDER);
  glPopMatrix();
}

/* just draw single objct */
void one(void(*a)(void))
{
  glEnable(GL_DEPTH_TEST);
  a();
  glDisable(GL_DEPTH_TEST);
}

/* Utilizando stencil func, stencil op*/
void firstInsideSecond(void(*a)(void), void(*b)(void), GLenum face, GLenum test)
{
  glEnable(GL_DEPTH_TEST);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glCullFace(face); /* controls which face of a to use*/
  a(); /* draw a face of a into depth buffer */

  /* use stencil plane to find parts of a in b */
  glDepthMask(GL_FALSE);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_ALWAYS, 0, 0);
  glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
  glCullFace(GL_BACK);
  b(); /* increment the stencil where the front face of b is drawn */
  glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
  glCullFace(GL_FRONT);
  b(); /* decrement the stencil buffer where the back face of b is drawn */
  glDepthMask(GL_TRUE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glStencilFunc(test, 0, 1);
  glDisable(GL_DEPTH_TEST);

  glCullFace(face);
  a(); /* draw the part of a that's in b */
}

void fixDepth(void(*a)(void))
{
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glDepthFunc(GL_ALWAYS);
  a(); /* draw the front face of a, fixing the depth buffer */
  glDepthFunc(GL_LESS);
}

/* "or" is easy; simply draw both objects with depth buffering on */
void ou(void(*a)(void), void(*b)())
{
  glEnable(GL_DEPTH_TEST);
  a();
  b();
  glDisable(GL_DEPTH_TEST);
}

/* "and" two objects together */
void e(void(*a)(void), void(*b)(void))
{
  firstInsideSecond(a, b, GL_BACK, GL_NOTEQUAL);

  fixDepth(b);

  firstInsideSecond(b, a, GL_BACK, GL_NOTEQUAL);

  glDisable(GL_STENCIL_TEST); /* reset things */
}

/* subtract b from a */
void sub(void(*a)(void), void(*b)(void))
{
  firstInsideSecond(a, b, GL_FRONT, GL_NOTEQUAL);

  fixDepth(b);

  firstInsideSecond(b, a, GL_BACK, GL_EQUAL);

  glDisable(GL_STENCIL_TEST); /* reset things */
}

void redraw()
{
  /* clear stencil each time */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

    switch(csg_op) {
		case CYL:	one(cylinder);				break;
		case CON:	one(cone);					break;
		case CUB:	one(cube);					break;
		case SPH:	one(sphere);				break;
		case CYL_OR_CON: (reverse) ? ou(cylinder, cone)		: ou(cone, cylinder);	break;
		case CYL_OR_CUB: (reverse) ? ou(cylinder, cube)		: ou(cube, cylinder);	break;
		case CYL_OR_SPH: (reverse) ? ou(cylinder, sphere)	: ou(sphere, cylinder);	break;
		case CON_OR_CUB: (reverse) ? ou(cone, cube)			: ou(cube, cone);		break;
		case CON_OR_SPH: (reverse) ? ou(cone, sphere)		: ou(sphere, cone);		break;
		case CUB_OR_SPH: (reverse) ? ou(cube, sphere)		: ou(sphere, cube);		break;
		case CYL_AND_CON: (reverse) ? e(cylinder, cone)	: e(cone, cylinder);		break;
		case CYL_AND_CUB: (reverse) ? e(cylinder, cube)	: e(cube, cylinder);		break;
		case CYL_AND_SPH: (reverse) ? e(cylinder, sphere)	: e(sphere, cylinder);	break;
		case CON_AND_CUB: (reverse) ? e(cone, cube)		: e(cube, cone);			break;
		case CON_AND_SPH: (reverse) ? e(cone, sphere)		: e(sphere, cone);		break;
		case CUB_AND_SPH: (reverse) ? e(cube, sphere)		: e(sphere, cube);		break;
		case CYL_SUB_CON: (reverse) ? sub(cylinder, cone)	: sub(cone, cylinder);	break;
		case CYL_SUB_CUB: (reverse) ? sub(cylinder, cube)	: sub(cube, cylinder);	break;
		case CYL_SUB_SPH: (reverse) ? sub(cylinder, sphere): sub(sphere, cylinder);	break;
		case CON_SUB_CUB: (reverse) ? sub(cone, cube)		: sub(cube, cone);		break;
		case CON_SUB_SPH: (reverse) ? sub(cone, sphere)	: sub(sphere, cone);		break;
		case CUB_SUB_SPH: (reverse) ? sub(cube, sphere)	: sub(sphere, cube);		break;
    }
    glPopMatrix();
	glFlush();
	
	glutPostRedisplay();
    glutSwapBuffers();
}

void key(unsigned char key, int x, int y)
{
  switch(key) {
	case 'q': reverse = true; csg_op = CYL;			break;
	case 'w': reverse = true; csg_op = CON;			break;
	case 'e': reverse = true; csg_op = CUB;			break;
	case 'r': reverse = true; csg_op = SPH;			break;
	case 't': reverse = true; csg_op = CYL_OR_CON;	break;
	case 'y': reverse = true; csg_op = CYL_OR_CUB;	break;
	case 'u': reverse = true; csg_op = CYL_OR_SPH;	break;
	case 'i': reverse = true; csg_op = CON_OR_CUB;	break;
	case 'o': reverse = true; csg_op = CON_OR_SPH;	break;
	case 'p': reverse = true; csg_op = CUB_OR_SPH;	break;
	case 'a': reverse = true; csg_op = CYL_AND_CON;	break;
	case 's': reverse = true; csg_op = CYL_AND_CUB;	break;
	case 'd': reverse = true; csg_op = CYL_AND_SPH;	break;
	case 'f': reverse = true; csg_op = CON_AND_CUB;	break;
	case 'g': reverse = true; csg_op = CON_AND_SPH;	break;
	case 'h': reverse = true; csg_op = CUB_AND_SPH;	break;
	case 'z': reverse = true; csg_op = CYL_SUB_CON;	break;
	case 'x': reverse = true; csg_op = CYL_SUB_CUB;	break;
	case 'c': reverse = true; csg_op = CYL_SUB_SPH;	break;
	case 'v': reverse = true; csg_op = CON_SUB_CUB;	break;
	case 'b': reverse = true; csg_op = CON_SUB_SPH;	break;
	case 'n': reverse = true; csg_op = CUB_SUB_SPH;	break;
	case 'T': reverse = false; csg_op = CYL_OR_CON;	break;
	case 'Y': reverse = false; csg_op = CYL_OR_CUB;	break;
	case 'U': reverse = false; csg_op = CYL_OR_SPH;	break;
	case 'I': reverse = false; csg_op = CON_OR_CUB;	break;
	case 'O': reverse = false; csg_op = CON_OR_SPH;	break;
	case 'P': reverse = false; csg_op = CUB_OR_SPH;	break;
	case 'A': reverse = false; csg_op = CYL_AND_CON;	break;
	case 'S': reverse = false; csg_op = CYL_AND_CUB;	break;
	case 'D': reverse = false; csg_op = CYL_AND_SPH;	break;
	case 'F': reverse = false; csg_op = CON_AND_CUB;	break;
	case 'G': reverse = false; csg_op = CON_AND_SPH;	break;
	case 'H': reverse = false; csg_op = CUB_AND_SPH;	break;
	case 'Z': reverse = false; csg_op = CYL_SUB_CON;	break;
	case 'X': reverse = false; csg_op = CYL_SUB_CUB;	break;
	case 'C': reverse = false; csg_op = CYL_SUB_SPH;	break;
	case 'V': reverse = false; csg_op = CON_SUB_CUB;	break;
	case 'B': reverse = false; csg_op = CON_SUB_SPH;	break;
	case 'N': reverse = false; csg_op = CUB_SUB_SPH;	break;
	default: break;
  }
}

void init(){	 
    glClearColor (1.0, 1.0, 1.0, 1.0);
    ANGULO = 45;
    rotX = rotY = 0;
    obsX = obsY = 0;
    obsZ = 20;//Voltar para 10
    escalaX = escalaY = escalaZ = 1;
	
    static GLfloat lightpos[] = {25.f, 50.f, -50.f, 1.f};
    static GLfloat sphere_mat[] = {1.f, .5f, 0.f, 1.f};
    static GLfloat cone_mat[] = {0.f, .5f, 1.f, 1.f};
    static GLfloat cube_mat[] = {1.f, .0f, 0.f, 1.f};
    GLUquadricObj *sphere, *cone, *base;

    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
	/* make display lists for sphere,cone and cube */
    glNewList(SPHERE, GL_COMPILE);
    sphere = gluNewQuadric();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere_mat);
    gluSphere(sphere, 20.f, 64, 64);
    gluDeleteQuadric(sphere);
    glEndList();
    
    glNewList(CUBE, GL_COMPILE);
    //defineLightConfiguration();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cube_mat);
    glutSolidCube(30);
    glEndList();

    glNewList(CONE, GL_COMPILE);
    cone = gluNewQuadric();
    base = gluNewQuadric();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cone_mat);
    gluQuadricOrientation(base, GLU_INSIDE);
    gluDisk(base, 0., 15., 64, 1);
    gluCylinder(cone, 15., 0., 60., 64, 64);
    gluDeleteQuadric(cone);
    gluDeleteQuadric(base);
    glEndList();
    
    glNewList(CYLINDER, GL_COMPILE);
    cone = gluNewQuadric();
    base = gluNewQuadric();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cone_mat);
    gluQuadricOrientation(base, GLU_INSIDE);
    gluDisk(base, 0., 10., 64, 1);
    gluCylinder(cone, 10., 10., 60., 64, 64);
    glRotatef(180.0, 1.f, 0.f, 0.f);
    glTranslatef(0, 0, -60);
    gluDisk(base, 0., 10., 64, 1);
    gluDeleteQuadric(cone);
    gluDeleteQuadric(base);
    glEndList();

    glMatrixMode(GL_PROJECTION);
    glOrtho(-50., 50., -50., 50., -50., 50.);
    glMatrixMode(GL_MODELVIEW); 

}
    
void posicionaObservador (void) {
    glMatrixMode (GL_MODELVIEW);/*Coordenadas na matrix de visualização*/  
    glLoadIdentity();
    glTranslatef(-obsX, -obsY, -obsZ-200);/*Translata a câmera para essas variáveis*/
    glRotatef(rotX,1,0,0);/*Rotacionar a câmera para essas coordenadas*/
    glRotatef(rotY,0,1,0);   
}

void especificaParametrosVisuais (void){
    glMatrixMode(GL_PROJECTION);/*Modo de visualização da matriz, Projeção*/
    glLoadIdentity();
    gluPerspective (ANGULO, ASPECTO, 0.5, 500);
    posicionaObservador();
}

void redesenhaPrimitivas(GLsizei largura, GLsizei altura){ 
    if (altura == 0)
    altura = 1;
    glViewport (0, 0, largura, altura);/*Dimensiona o ViewPort*/
    ASPECTO = (GLfloat) largura/ (GLfloat) altura;/*Calcula a correção de aspecto*/
    especificaParametrosVisuais();        
}

int picked_object;
int xpos = 0, ypos = 0;
int newxpos, newypos;
int startx, starty;

void mouse(int botao, int estado, int x, int y){
		if(estado == GLUT_DOWN){
	        x_ini = x;
	        y_ini = y;
	        obsX_ini = obsX;
	        obsY_ini = obsY;
	        obsZ_ini = obsZ;
	        rotX_ini = rotX;
	        rotY_ini = rotY;
	        bot=botao;
	    }
    	else
	        bot = -1;
}

#define DEGTORAD (2 * 3.1415 / 360)
void motion(int x, int y){
    if(bot == GLUT_LEFT_BUTTON){//Rotação
        int deltaX = x_ini - x;
        int deltaY = y_ini - y; 
        rotX = rotX_ini - deltaY/ SENS_ROT;
        rotY = rotY_ini - deltaX/ SENS_ROT;
     }
     else if (bot == GLUT_RIGHT_BUTTON){//Zoom
         int deltaZ = y_ini - y;
         obsZ = obsZ_ini + deltaZ/ SENS_OBS;
     }
     else if (bot == GLUT_MIDDLE_BUTTON){//Correr
         int deltaX = x_ini - x;
         int deltaY = y_ini - y;
         obsX = obsX_ini + deltaX/ SENS_TRANS;
         obsY = obsY_ini + deltaY/ SENS_TRANS;
     }
     posicionaObservador();
     glutPostRedisplay();
}
    
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_DOUBLE);
    glutInitWindowSize(1000,1000);
    glutInitWindowPosition(0,0);
    glutCreateWindow("simple");
    glutDisplayFunc(redraw);
    glutReshapeFunc(redesenhaPrimitivas);/*Redesenho na tela*/
    glutMouseFunc(mouse);/*Rotina do mouse*/
    glutMotionFunc(motion);/*Rotina do movimento*/ 
    glutKeyboardFunc(key); /*Rotina de teclado*/
    init();
    glutMainLoop();
    
	return 0;
}