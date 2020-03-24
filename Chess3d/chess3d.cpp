// Kenneth Adamson
// Chess animation starter kit.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <ctime>
using namespace std;
#include "GL/freeglut.h"
#include "camera.h"
//#include "graphics.h"


// Global Variables
// Some colors you can use, or make your own and add them
// here and in graphics.h
GLdouble redMaterial[] = { 0.7, 0.1, 0.2, 1.0 };
GLdouble greenMaterial[] = { 0.1, 0.7, 0.4, 1.0 };
GLdouble brightGreenMaterial[] = { 0.1, 0.9, 0.1, 1.0 };
GLdouble blueMaterial[] = { 0.1, 0.2, 0.7, 1.0 };
GLdouble whiteMaterial[] = { 1.0, 1.0, 1.0, 1.0 };

double screen_x = 800;
double screen_y = 600;
double t = 0.0;
bool loopExit = false;
int timeSinceStart;
int oldTimeSinceStart = 0;
int deltaTime;
Camera* camera;

enum piece_numbers { pawn = 100, king, queen, rook, bishop, knight };

double GetTime()
{
    static clock_t start_time = clock();
    clock_t current_time = clock();
    double total_time = double(current_time - start_time) / CLOCKS_PER_SEC;
    return total_time;
}

// Outputs a string of text at the specified location.
void text_output(double x, double y, const char* string)
{
    void* font = GLUT_BITMAP_9_BY_15;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    int len, i;
    glRasterPos2d(x, y);
    len = (int)strlen(string);
    for (i = 0; i < len; i++)
    {
        glutBitmapCharacter(font, string[i]);
    }

    glDisable(GL_BLEND);
}

// Given the three triangle points x[0],y[0],z[0],
//		x[1],y[1],z[1], and x[2],y[2],z[2],
//		Finds the normal vector n[0], n[1], n[2].
void FindTriangleNormal(double x[], double y[], double z[], double n[])
{
    // Convert the 3 input points to 2 vectors, v1 and v2.
    double v1[3], v2[3];
    v1[0] = x[1] - x[0];
    v1[1] = y[1] - y[0];
    v1[2] = z[1] - z[0];
    v2[0] = x[2] - x[0];
    v2[1] = y[2] - y[0];
    v2[2] = z[2] - z[0];

    // Take the cross product of v1 and v2, to find the vector perpendicular to both.
    n[0] = v1[1] * v2[2] - v1[2] * v2[1];
    n[1] = -(v1[0] * v2[2] - v1[2] * v2[0]);
    n[2] = v1[0] * v2[1] - v1[1] * v2[0];

    double size = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
    n[0] /= -size;
    n[1] /= -size;
    n[2] /= -size;
}

// Loads the given data file and draws it at its default position.
// Call glTranslate before calling this to get it in the right place.
void DrawPiece(const char filename[])
{
    // Try to open the given file.
    char buffer[200];
    ifstream in(filename);
    if (!in)
    {
        cerr << "Error. Could not open " << filename << endl;
        exit(1);
    }

    double x[100], y[100], z[100]; // stores a single polygon up to 100 vertices.
    int done = false;
    int verts = 0; // vertices in the current polygon
    int polygons = 0; // total polygons in this file.
    do
    {
        in.getline(buffer, 200); // get one line (point) from the file.
        int count = sscanf(buffer, "%lf, %lf, %lf", &(x[verts]), &(y[verts]), &(z[verts]));
        done = in.eof();
        if (!done)
        {
            if (count == 3) // if this line had an x,y,z point.
            {
                verts++;
            }
            else // the line was empty. Finish current polygon and start a new one.
            {
                if (verts >= 3)
                {
                    glBegin(GL_POLYGON);
                    double n[3];
                    FindTriangleNormal(x, y, z, n);
                    glNormal3dv(n);
                    for (int i = 0; i < verts; i++)
                    {
                        glVertex3d(x[i], y[i], z[i]);
                    }
                    glEnd(); // end previous polygon
                    polygons++;
                    verts = 0;
                }
            }
        }
    } while (!done);

    if (verts > 0)
    {
        cerr << "Error. Extra vertices in file " << filename << endl;
        exit(1);
    }

}

// NOTE: Y is the UP direction for the chess pieces.
double eye[3] = { 4500, 8000, -4000 }; // pick a nice vantage point.
double at[3] = { 4500, 0,     4000 };
//
// GLUT callback functions
//

// As t goes from t0 to t1, set v between v0 and v1 accordingly.
void Interpolate(double t, double t0, double t1, double& v, double v0, double v1)
{
    double ratio = (t - t0) / (t1 - t0);
    if (ratio < 0)
        ratio = 0;
    if (ratio > 1)
        ratio = 1;
    v = v0 + (v1 - v0) * ratio;
}

void drawBoardSquare(int i, int j, bool isDark)
{
    if (isDark)
    {
        GLfloat gray[]{ 0.5, 0.5, 0.5, 1.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
    }
    else
    {
        GLfloat white[]{ 3.0, 3.0, 3.0, 1.0 };
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
    }
    glBegin(GL_QUADS);
    glVertex3d(500.0 + (i * 1000.0), 0, 500 + (j * 1000.0));
    glVertex3d(500.0 + (i * 1000.0), 0, 500 + ((j + 1.0) * 1000.0));
    glVertex3d(500.0 + ((i + 1.0) * 1000.0), 0, 500 + ((j + 1.0) * 1000.0));
    glVertex3d(500.0 + ((i + 1.0) * 1000.0), 0, 500 + (j * 1000.0));
    glEnd();
}

// This callback function gets called by the Glut
// system whenever it decides things need to be redrawn.
void display(void)
{
    double t = GetTime();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    //gluLookAt(eye[0], eye[1], eye[2],  at[0], at[1], at[2],  0,1,0); // Y is up!
    gluLookAt(
        camera->Position.x, camera->Position.y, camera->Position.z,
        camera->Position.x + camera->Front.x, camera->Position.y + camera->Front.y, camera->Position.z + camera->Front.z,
        //at[0], at[1], at[2],
        0, 1, 0
        );

    // Set the color for one side (white), and draw its 16 pieces.
    GLfloat mat_amb_diff1[] = { 0.8f, 0.9f, 0.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff1);

    glPushMatrix();
    glTranslatef(3000, 0, 1000);
    glCallList(bishop);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(4000, 0, 1000);
    glCallList(king);
    glPopMatrix();

    double z;
    Interpolate(t, 1.0, 3.0, z, 1000, 5000);
    glPushMatrix();
    glTranslatef(5000, 0, z);
    glCallList(queen);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(6000, 0, 1000);
    glCallList(bishop);
    glPopMatrix();

    // Set the color for one side (black), and draw its 16 pieces.
    GLfloat mat_amb_diff2[] = { 0.1f, 0.5f, 0.8f, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff2);

    double x;
    Interpolate(t, 4.0, 6.0, x, 4000, 2000);
    glPushMatrix();
    glTranslatef(x, 0, 8000);
    glCallList(king);
    glPopMatrix();

    for (int x = 1000; x <= 8000; x += 1000)
    {
        glPushMatrix();
        glTranslatef(x, 0, 7000);
        glCallList(pawn);
        glPopMatrix();
    }



    // Outline of board
    GLfloat gray[]{ 1.9, 1.9, 1.9, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);
    glBegin(GL_LINES);
    glVertex3d(500, 0, 500);
    glVertex3d(500, 0, 8500);

    glVertex3d(500, 0, 8500);
    glVertex3d(8500, 0, 8500);

    glVertex3d(8500, 0, 8500);
    glVertex3d(8500, 0, 500);

    glVertex3d(8500, 0, 500);
    glVertex3d(500, 0, 500);

    glEnd();

    // 3d ness



    // lines
    GLfloat black[]{ 0.2, 0.2, 0.2, 1.0 };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, black);

    glBegin(GL_LINES);
    glVertex3d(500, 0, 500);
    glVertex3d(0, -200, 0);

    glVertex3d(500, 0, 8500);
    glVertex3d(0, -200, 9000);

    glVertex3d(8500, 0, 8500);
    glVertex3d(9000, -200, 9000);

    glVertex3d(8500, 0, 500);
    glVertex3d(9000, -200, 0);
    glEnd();

    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gray);

    // bottom
    glBegin(GL_QUADS);
    glVertex3d(0, -200, 0);
    glVertex3d(0, -200, 9000);
    glVertex3d(9000, -200, 9000);
    glVertex3d(9000, -200, 0);
    glEnd();

    // edge 1
    glBegin(GL_QUADS);
    glVertex3d(500, 0, 500);
    glVertex3d(0, -200, 0);
    glVertex3d(0, -200, 9000);
    glVertex3d(500, 0, 8500);
    glEnd();

    // edge 2
    glBegin(GL_QUADS);
    glVertex3d(500, 0, 8500);
    glVertex3d(0, -200, 9000);
    glVertex3d(9000, -200, 9000);
    glVertex3d(8500, 0, 8500);
    glEnd();


    // edge 3
    glBegin(GL_QUADS);
    glVertex3d(8500, 0, 8500);
    glVertex3d(9000, -200, 9000);
    glVertex3d(9000, -200, 0);
    glVertex3d(8500, 0, 500);
    glEnd();

    // edge 4
    glBegin(GL_QUADS);
    glVertex3d(9000, -200, 0);
    glVertex3d(8500, 0, 500);
    glVertex3d(500, 0, 500);
    glVertex3d(0, -200, 0);
    glEnd();

    // fill in squares
    // starts as white

    bool fill = false;
    for (size_t i = 0; i < 8; i++)
    {
        for (size_t j = 0; j < 8; j++)
        {
            drawBoardSquare(i, j, fill);
            fill = !fill;
        }
        fill = !fill;
    }

    GLfloat light_position[] = { 1,2,-.1f, 0 }; // light comes FROM this vector direction.
    glLightfv(GL_LIGHT0, GL_POSITION, light_position); // position first light

    glutSwapBuffers();
    glutPostRedisplay();
}


void SetPerspectiveView(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double aspectRatio = (GLdouble)w / (GLdouble)h;
    gluPerspective(
        /* field of view in degree */ 45.0,
        /* aspect ratio */ aspectRatio,
        /* Z near */ 100, /* Z far */ 30000.0);
    glMatrixMode(GL_MODELVIEW);
}

// This callback function gets called by the Glut
// system whenever the window is resized by the user.
void reshape(int w, int h)
{
    screen_x = w;
    screen_y = h;

    // Set the pixel resolution of the final picture (Screen coordinates).
    glViewport(0, 0, w, h);

    SetPerspectiveView(w, h);

}

#pragma region Keyboard

// This callback function gets called by the Glut
// system whenever a key is pressed.
void asciiKeyboardDown(unsigned char c, int x, int y)
{
    switch (c)
    {
    case 27: // escape character means to quit the program
        exit(0);
        break;
    case 'w':
        CamMove.forward = true;
        break;
    case 's':
        CamMove.back = true;
        break;
    case 'a':
        CamMove.left = true;
        break;
    case 'd':
        CamMove.right = true;
        break;
    case 'c':
        std::cout << camera->Position.x << ", " << camera->Position.y << ", " << camera->Position.z << "\n";
        std::cout << camera->Front.x << ", " << camera->Front.y << ", " << camera->Front.z << "\n";
        std::cout << "P: " << camera->Pitch << ", Y: " << camera->Yaw << "\n";
        break;
    default:
        return; // if we don't care, return without glutPostRedisplay()
    }
    //camera->ProcessKeyboard(deltaTime);
    //glutPostRedisplay();
}

void asciiKeyboardUp(unsigned char c, int x, int y)
{
    switch (c)
    {
    case 'w':
        CamMove.forward = false;
        break;
    case 's':
        CamMove.back = false;
        break;
    case 'a':
        CamMove.left = false;
        break;
    case 'd':
        CamMove.right = false;
        break;
    default:
        return; // if we don't care, return without glutPostRedisplay()
    }
    //camera->ProcessKeyboard(deltaTime);
    //glutPostRedisplay();
}

#pragma endregion Keyboard

#pragma region Mouse
// This callback function gets called by the Glut
// system whenever any mouse button goes up or down.
void mouseButton(int mouse_button, int state, int x, int y)
{
    if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
    }
    if (mouse_button == GLUT_LEFT_BUTTON && state == GLUT_UP)
    {
    }
    if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
    {
    }
    if (mouse_button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
    {
    }
    glutPostRedisplay();
}

void mouseMove(int x, int y)
{

}

void mousePassiveMove(int x, int y)
{
    static bool startMouse = true;

    float xoffset = x - (screen_x / 2);
    float yoffset = (screen_y / 2) - y; // reversed since y coordinates go from top to bottom

    // warping the pointer causes a mousePassiveMove to fire again ... even if we didn't actually move.
    // So, let's not warp the pointer if we didn't move to avoid unnecessary processing.
    if (x != screen_x / 2 || y != screen_y / 2)
        glutWarpPointer(screen_x / 2, screen_y / 2);

    //std::cout << "At: " << x << ", " << y << "\n";
    CamMove.mouseXOffset = xoffset;
    CamMove.mouseYOffset = yoffset;
}

void mouseWheel(int wheel, int direction, int x, int y)
{
    //wheel: the wheel number, if the mouse has only a wheel this will be zero.
    //direction : a + / -1 value indicating the wheel movement direction
    //x, y : the window mouse coordinates
}


#pragma endregion Mouse

// Your initialization code goes here.
void InitializeMyStuff()
{
    // set material's specular properties
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // set light properties
    GLfloat light_position[] = { (float)eye[0], (float)eye[1], (float)eye[2],1 };
    GLfloat white_light[] = { 1,1,1,1 };
    GLfloat low_light[] = { .3f,.3f,.3f,1 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position); // position first light
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light); // specify first light's color
    glLightfv(GL_LIGHT0, GL_SPECULAR, low_light);

    glEnable(GL_DEPTH_TEST); // turn on depth buffering
    glEnable(GL_LIGHTING);	// enable general lighting
    glEnable(GL_LIGHT0);	// enable the first light.

    // Make the display lists for speed
    glNewList(pawn, GL_COMPILE);
    DrawPiece("PAWN.POL");
    glEndList();

    // Make the display lists for speed
    glNewList(king, GL_COMPILE);
    DrawPiece("KING.POL");
    glEndList();

    // Make the display lists for speed
    glNewList(queen, GL_COMPILE);
    DrawPiece("QUEEN.POL");
    glEndList();

    // Make the display lists for speed
    glNewList(rook, GL_COMPILE);
    DrawPiece("ROOK.POL");
    glEndList();

    // Make the display lists for speed
    glNewList(bishop, GL_COMPILE);
    DrawPiece("BISHOP.POL");
    glEndList();

    // Make the display lists for speed
    glNewList(knight, GL_COMPILE);
    DrawPiece("KNIGHT.POL");
    glEndList();

    // Create our camera
    camera = new Camera(glm::vec3(0, 1000, 0), glm::vec3(0, 1, 0), 0.0f, 0.0f);

}

void update(int deltaTime)
{
    camera->Update(deltaTime);

}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_x, screen_y);
    glutInitWindowPosition(10, 10);

    int fullscreen = 0;
    if (fullscreen)
    {
        glutGameModeString("800x600:32");
        glutEnterGameMode();
    }
    else
    {
        glutCreateWindow("Shapes");
    }

    // callbacks for display
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    // callbacks for input

    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(asciiKeyboardDown);
    glutKeyboardUpFunc(asciiKeyboardUp);
    //glutSpecialFunc(pressKey);
    //glutSpecialUpFunc(releaseKey);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);
    glutPassiveMotionFunc(mousePassiveMove);
    glutMouseWheelFunc(mouseWheel);

    glClearColor(1, 1, 1, 1);
    InitializeMyStuff();
    glutSetCursor(GLUT_CURSOR_NONE);
    glutWarpPointer(screen_x / 2, screen_y / 2);

    while (!loopExit)
    {
        // Get Delta Time for calculations
        timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
        deltaTime = timeSinceStart - oldTimeSinceStart;
        oldTimeSinceStart = timeSinceStart;

        glutMainLoopEvent();
        update(deltaTime);
        glutPostRedisplay();
    }

    return 0;
}
