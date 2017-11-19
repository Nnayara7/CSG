#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>

enum {CSG_A, CSG_B, CSG_C,CSG_A_OR_B, CSG_A_OR_C, CSG_B_OR_C, CSG_A_AND_B, CSG_A_SUB_B, CSG_B_SUB_A, CSG_B_SUB_C, CSG_A_SUB_C, CSG_A_AND_C};

enum {SPHERE = 1, CONE, CUBE};

/* Draw a cone */
GLfloat coneX = 0.f, coneY = 0.f, coneZ = 0.f;
void cone(void)
{
  glPushMatrix();
  glTranslatef(coneX, coneY, coneZ);
  glTranslatef(0.f, 0.f, -30.f);
  glCallList(CONE);
  glPopMatrix();
}

/* Draw a sphere */
GLfloat sphereX = 0.f, sphereY = 0.f, sphereZ = 0.f;
void sphere(void)
{
  glPushMatrix();
  glTranslatef(sphereX, sphereY, sphereZ);
  glCallList(SPHERE);
  glPopMatrix();

}

/* Draw a cube */
GLfloat cubeX = 0.f, cubeY = 0.f, cubeZ = 0.f;
void cube() {
  glPushMatrix();
  glTranslatef(cubeX, cubeY, cubeZ);
  glCallList(CUBE);
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

void redraw()
{
  /* clear stencil each time */
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    glPushMatrix();
    glRotatef(viewangle, 0.f, 1.f, 0.f);

    switch(csg_op) {
    case CSG_A:
      one(cone);
      break;
    case CSG_B:
      one(sphere);
      break;
      case CSG_C:
      one(cube);
      break;
    case CSG_A_OR_B:
      ou(cone, sphere);
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
    glutSwapBuffers();
}

void menu(int csgop)
{
  csg_op = csgop;
  glutPostRedisplay();
}


/* animate scene by rotating */
enum {ANIM_LEFT, ANIM_RIGHT};
int animDirection = ANIM_LEFT;

void anim(void)
{
  if(animDirection == ANIM_LEFT)
    viewangle -= 3.f;
  else
    viewangle += 3.f;
   glutPostRedisplay();
}

/* special keys, like array and F keys */
void special(int key, int x, int y)
{
  switch(key) {
  case GLUT_KEY_LEFT:
    glutIdleFunc(anim);
    animDirection = ANIM_LEFT;
    break;
  case GLUT_KEY_RIGHT:
    glutIdleFunc(anim);
    animDirection = ANIM_RIGHT;
    break;
  case GLUT_KEY_UP:
  case GLUT_KEY_DOWN:
    glutIdleFunc(0);
    break;
  }
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

int picked_object;
int xpos = 0, ypos = 0;
int newxpos, newypos;
int startx, starty;

void mouse(int button, int state, int x, int y)
{
  if(state == GLUT_UP) {
      picked_object = button;
      xpos += newxpos;
      ypos += newypos;
      newxpos = 0;
      newypos = 0;
  } else { /* GLUT_DOWN */
    startx = x;
    starty = y;
  }
}

#define DEGTORAD (2 * 3.1415 / 360)
void motion(int x, int y)
{
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
  glutPostRedisplay();
}

GLfloat lightPosition[] = {0.f, 0.f, 0.f, 0.f};
void defineLightConfiguration() {
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_COLOR_MATERIAL);
}

int main(int argc, char **argv)
{
    static GLfloat lightpos[] = {25.f, 50.f, -50.f, 1.f};
    static GLfloat sphere_mat[] = {1.f, .5f, 0.f, 1.f};
    static GLfloat cone_mat[] = {0.f, .5f, 1.f, 1.f};
    static GLfloat cube_mat[] = {1.f, .0f, 0.f, 0.f};
    GLUquadricObj *sphere, *cone, *base, *cube;
    defineLightConfiguration();

    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_STENCIL|GLUT_DEPTH|GLUT_DOUBLE);
    (void)glutCreateWindow("csg");
    glutDisplayFunc(redraw);
    glutKeyboardFunc(key);
    glutSpecialFunc(special);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

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

    glMatrixMode(GL_PROJECTION);
    glOrtho(-50., 50., -50., 50., -50., 50.);
    glMatrixMode(GL_MODELVIEW);
    glutMainLoop();

    return 0;
}








