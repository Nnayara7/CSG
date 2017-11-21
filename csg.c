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


enum {CSG_A, CSG_B, CSG_C,CSG_A_OR_B, CSG_A_OR_C, CSG_B_OR_C, CSG_A_AND_B, CSG_A_SUB_B, CSG_B_SUB_A, CSG_B_SUB_C, CSG_A_SUB_C, CSG_A_AND_C};

enum {SPHERE = 1, CONE, CUBE, CYLINDER};

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

int csg_op = CSG_A;

/* add menu callback */

GLfloat viewangle;
GLfloat viewangley;

void redraw()
{
  /* clear stencil each time */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

    switch(csg_op) {
    case CSG_A:
      one(cylinder);
      // e(cone, sphere);  
      break;
    case CSG_B:
      one(sphere);
      break;
      case CSG_C:
      one(cube);
      break;
    case CSG_A_OR_B:
      ou(cylinder, sphere);
      break;
    case CSG_A_OR_C:
      ou(cone, cube);
      break;
    case CSG_B_OR_C:
      ou(sphere, cube);
      break;
    case CSG_A_AND_B:
      e(cone, sphere);
      break;
    case CSG_A_AND_C:
      e(cone, cube);
      break;
    case CSG_A_SUB_B:
      sub(cone, sphere);
      break;
    case CSG_A_SUB_C:
      sub(cone, cube);
      break;
    case CSG_B_SUB_A:
      sub(sphere, cone);
      break;
    case CSG_B_SUB_C:
      sub(sphere, cube);
      break;
    }
    glPopMatrix();
	glFlush();
	
	//ativaIluminacao();
	glutPostRedisplay();
    glutSwapBuffers();
}

void menu(int csgop)
{
  csg_op = csgop;
  glutPostRedisplay();
}


void key(unsigned char key, int x, int y)
{
  switch(key) {
  case 'a':
    viewangle -= 10.f;
    glutPostRedisplay();
    break;
  case 's':
    viewangle += 10.f;
    glutPostRedisplay();
    break;
  case '\033':
    exit(0);
  }
}

void init(){	 
        glClearColor (1.0, 1.0, 1.0, 1.0);
        ANGULO = 45;
        rotX = rotY = 0;
        obsX = obsY = 0;
        obsZ = 20;//Voltar para 10
        escalaX = escalaY = escalaZ = 1;
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
  //   if (botao == GLUT_RIGHT_BUTTON){
		// if(estado == GLUT_UP) {
		// 	picked_object = botao;
		// 	xpos += newxpos;
		// 	ypos += newypos;
		// 	newxpos = 0;
		// 	newypos = 0;
		// 	bot=botao;
		// } else { /* GLUT_DOWN */
		// 	startx = x;
		// 	starty = y;
		// 	bot = 2;
		// }
  //  	}
  //  	else{
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
// 	}
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
		GLfloat r, objx, objy, objz;
		
		newxpos = x - startx;
		newypos = starty - y;
		
		r = (newxpos + xpos) * 50.f/512.f;
		objx = r * (float)cos(viewangle * DEGTORAD);
		objy = (newypos + ypos) * 50.f/512.f;
		objz = r * (float)sin(viewangle * DEGTORAD);
		
		switch(picked_object) {
			case CSG_A:
				coneX = objx;
				coneY = objy;
				coneZ = objz;
			break;
			case CSG_B:
				sphereX = objx;
				sphereY = objy;
				sphereZ = objz;
			break;
			case CSG_C:
				cubeX = objx;
				cubeY = objy;
				cubeZ = objz;
			break;
		}
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

void stop()
{
	int a = 0;
}
    
int main(int argc, char **argv)
{
	
    static GLfloat lightpos[] = {25.f, 50.f, -50.f, 1.f};
    static GLfloat sphere_mat[] = {1.f, .5f, 0.f, 1.f};
    static GLfloat cone_mat[] = {0.f, .5f, 1.f, 1.f};
    static GLfloat cube_mat[] = {1.f, .0f, 0.f, 1.f};
    GLUquadricObj *sphere, *cone, *base;

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

    glutCreateMenu(menu);
    glutAddMenuEntry("Desenha cone", CSG_A);
    glutAddMenuEntry("Desenha Esfera", CSG_B);
    glutAddMenuEntry("Denha Cubo", CSG_C);
    glutAddMenuEntry("Cone Uniao Esfera", CSG_A_OR_B);
    glutAddMenuEntry("Cone Uniao Cubo", CSG_A_OR_C);
    glutAddMenuEntry("Esfera Uniao Cubo", CSG_B_OR_C);
    glutAddMenuEntry("Cone + Esfera", CSG_A_AND_B);
    glutAddMenuEntry("Cone + Cubo", CSG_A_AND_C);
    glutAddMenuEntry("Cone - Esfera", CSG_A_SUB_B);
    glutAddMenuEntry("Cone - Cubo", CSG_A_SUB_C);
    glutAddMenuEntry("Esfera - Cone", CSG_B_SUB_A);
    glutAddMenuEntry("Esfera - Cubo", CSG_B_SUB_C);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
	
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
    glutMainLoop();
    
	return 0;
}