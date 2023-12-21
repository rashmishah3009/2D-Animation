#include <string.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <iostream>

#include <math.h>

#include <SOIL.h>

using namespace std;

bool WireFrame= false;
GLuint spriteSheetTexture;

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

GLuint texture[1];      //Texture array
float ar;

//Variables for texture coordinates
float Xmin = -10.0f;
float Xmax = 10.0f;
float aspectRatio = 800.0f / 600.0f;
float Ymax = 8.0f / aspectRatio;
float Ymin = -7.0f / aspectRatio;

//Define start and end points attached to bottom corners of the image
float start_x = Xmin;
float start_y = Ymin;
float end_x = Xmax;
float end_y = Ymin;

//Calculate intermediate control points
float mid_x = (Xmin + Xmax) / 2.0f;
float mid_y = (Ymax + Ymin) / 2.0f;
float control1_x = (start_x + mid_x) / 2.0f;
float control1_y = (start_y + mid_y) / 2.0f;
float control2_x = (end_x + mid_x) / 2.0f;
float control2_y = (end_y + mid_y) / 2.0f;

//Define additional control points for degree 4 bezier curve
float control3_x = (start_x + 3 * mid_x) / 4.0f;
float control3_y = (start_y + 3 * mid_y) / 4.0f;
float control4_x = (end_x + 3 * mid_x) / 4.0f;
float control4_y = (end_y + 3 * mid_y) / 4.0f;

//Variables for animation
float animationTime = 0.0f;
float animationSpeed;

//Add a global variable to control the animation
bool animateSprite = false;

//Define the number of frames in the sprite sheet
const int numFramesInSpriteSheet = 8;

/* GLUT callback Handlers */

//Loading background for the 2D animation
void loadGLTextures() {
    texture[0] = SOIL_load_OGL_texture
    ("Images/Background.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if(texture[0] == 0)
        cout << "SOIL loading error: '" << SOIL_last_result() << endl;

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
}

//Loading sprite sheet character
void loadSpriteSheet() {
    spriteSheetTexture = SOIL_load_OGL_texture("Images/Walk.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);

    if (spriteSheetTexture == 0)
        cout << "SOIL loading error for sprite sheet: '" << SOIL_last_result() << endl;

    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void DrawBezierCurveWithAnimatedSprite() {

    if (animateSprite) {                    //Updating animation time for the sprite when right arrow key is pressed
        animationTime += animationSpeed;
        if (animationTime > 1.0f) {         //Stop animation when it reaches the end point of the bezier curve
            animationTime = 1.0f;
            animateSprite = false;
        }
    }

    if (animateSprite) {

        //Calculating sprite position on the curve
        float spriteX = pow(1 - animationTime, 4) * start_x +
                        4 * animationTime * pow(1 - animationTime, 3) * control1_x +
                        6 * pow(animationTime, 2) * pow(1 - animationTime, 2) * control2_x +
                        4 * pow(animationTime, 3) * (1 - animationTime) * control3_x +
                        pow(animationTime, 4) * end_x;

        float spriteY = pow(1 - animationTime, 4) * start_y +
                        4 * animationTime * pow(1 - animationTime, 3) * control1_y +
                        6 * pow(animationTime, 2) * pow(1 - animationTime, 2) * control2_y +
                        4 * pow(animationTime, 3) * (1 - animationTime) * control3_y +
                        pow(animationTime, 4) * end_y;


        //Calculating tangent of the bezier curve at the current animation time
        float tangentX = -4 * pow(1 - animationTime, 3) * start_x +
                          (12 * pow(1 - animationTime, 2) - 24 * animationTime * (1 - animationTime)) * control1_x +
                          (12 * animationTime * (1 - animationTime) - 24 * pow(animationTime, 2)) * control2_x +
                          (4 * pow(animationTime, 3) - 12 * animationTime * (1 - animationTime)) * control3_x +
                          4 * pow(animationTime, 3) * end_x;

        float tangentY = -4 * pow(1 - animationTime, 3) * start_y +
                          (12 * pow(1 - animationTime, 2) - 24 * animationTime * (1 - animationTime)) * control1_y +
                          (12 * animationTime * (1 - animationTime) - 24 * pow(animationTime, 2)) * control2_y +
                          (4 * pow(animationTime, 3) - 12 * animationTime * (1 - animationTime)) * control3_y +
                          4 * pow(animationTime, 3) * end_y;


        float angle = atan2(tangentY, tangentX) * 180.0f / M_PI;   //Calculating angle of rotation based on the tangent

        //Calculating sprite texture coordinates based on animation time
        float spriteTextureX = (1.0f - animationTime) * numFramesInSpriteSheet;
        int currentFrame = static_cast<int>(spriteTextureX) % static_cast<int>(numFramesInSpriteSheet);

        float spriteWidthInSheet = 1.0f / numFramesInSpriteSheet;   //Calculating the width of each sprite in the sprite sheet

        float spriteDirection = 1.0f;     //Determining the direction of the sprite
        if (animationSpeed < 0.0f) {
            spriteDirection = -1.0f;      // Flipping the sprite horizontally
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Drawing image
    glBegin(GL_QUADS);
    float spriteTextureY = 0.0f;

    //Adjusting texture coordinates for horizontal flip
    if (spriteDirection > 0.0f) {
        glTexCoord2f(currentFrame * spriteWidthInSheet, spriteTextureY);
        glVertex3f(spriteX - 1.0f, spriteY, 2.1f);  // Adjust X coordinate to align the sprite with the curve
        glTexCoord2f((currentFrame + 1) * spriteWidthInSheet, spriteTextureY);
        glVertex3f(spriteX + 1.0f, spriteY, 2.1f);  // Adjust X coordinate to align the sprite with the curve
        glTexCoord2f((currentFrame + 1) * spriteWidthInSheet, spriteTextureY + 1.0f);
        glVertex3f(spriteX + 1.0f, spriteY + 3, 2.1f);  // Adjust X coordinate to align the sprite with the curve
        glTexCoord2f(currentFrame * spriteWidthInSheet, spriteTextureY + 1.0f);
        glVertex3f(spriteX - 1.0f, spriteY + 3, 2.1f);  // Adjust X coordinate to align the sprite with the curve
    }
    //Horizontal flipping and swapping the order of texture coordinates
    else {
        glTexCoord2f((currentFrame + 1) * spriteWidthInSheet, spriteTextureY);
        glVertex3f(spriteX - 1.0f, spriteY, 2.1f);
        glTexCoord2f(currentFrame * spriteWidthInSheet, spriteTextureY);
        glVertex3f(spriteX + 1.0f, spriteY, 2.1f);
        glTexCoord2f(currentFrame * spriteWidthInSheet, spriteTextureY + 1.0f);
        glVertex3f(spriteX + 1.0f, spriteY + 3, 2.1f);
        glTexCoord2f((currentFrame + 1) * spriteWidthInSheet, spriteTextureY + 1.0f);
        glVertex3f(spriteX - 1.0f, spriteY + 3, 2.1f);
    }

    glEnd();

    glDisable(GL_TEXTURE_2D);

    glDisable(GL_BLEND);
    }
}

//Resetting the sprite sheet position
void resetSpriteSheet() {
    animationTime = 0.0f;
}

static void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0, 5, 10, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    if (WireFrame)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,  texture[0]);

    //Drawing image with adjusted texture coordinates
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(Xmin, Ymin, 2.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(Xmax, Ymin, 2.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(Xmax, Ymax, 2.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(Xmin, Ymax, 2.0f);
    glEnd();

    //Drawing the bezier curve
    glLineWidth(10.0f); //Setting the line width
    glBegin(GL_LINE_STRIP);
    for (float t = 0; t <= 1; t += 0.01) {

        //Calculating bezier curve points for degree 4 bezier curve
        float x = pow(1 - t, 4) * start_x +
                  4 * t * pow(1 - t, 3) * control1_x +
                  6 * pow(t, 2) * pow(1 - t, 2) * control2_x +
                  4 * pow(t, 3) * (1 - t) * control3_x +
                  pow(t, 4) * end_x;

        float y = pow(1 - t, 4) * start_y +
                  4 * t * pow(1 - t, 3) * control1_y +
                  6 * pow(t, 2) * pow(1 - t, 2) * control2_y +
                  4 * pow(t, 3) * (1 - t) * control3_y +
                  pow(t, 4) * end_y;

        glVertex3f(x, y, 2.1f);
    }
    glEnd();

    //Drawing the animated sprite above the curve if the right arrow key is pressed
    if (animateSprite) {
        DrawBezierCurveWithAnimatedSprite();
    }


    glutSwapBuffers();
}


static void resize(int width, int height)
{
     double Ratio;

   if(width<=height)
            glViewport(0,(GLsizei) (height-width)/2,(GLsizei) width,(GLsizei) width);
    else
          glViewport((GLsizei) (width-height)/2 ,0 ,(GLsizei) height,(GLsizei) height);

    glViewport(0, 0, width, height);
    ar = (float) width / (float) height;

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();
    gluPerspective(50.0, ar, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
	gluPerspective (50.0f,1,0.1f, 100.0f);

 }


void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        //Converting window coordinates to normalized device coordinates
        float ndcX = (x / 800.0f) * 2.0f - 1.0f;
        float ndcY = ((600 - y) / 600.0f) * 2.0f - 1.0f;

        //Mapping normalized device coordinates to world coordinates
        float worldX = ndcX * (Xmax - Xmin) / 2.0f;
        float worldY = ndcY * (Ymax - Ymin) / 2.0f;

        //Updating control points based on mouse position
        control1_x = (start_x + worldX) / 2.0f;
        control1_y = (start_y + worldY) / 2.0f;
        control2_x = (end_x + worldX) / 2.0f;
        control2_y = (end_y + worldY) / 2.0f;
    }
    glutPostRedisplay();
}

static void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;
        case ' ':
            //Resetting sprite sheet position to the start of the curve
            resetSpriteSheet();
            break;
    }
}

void Specialkeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_UP:
        break;
    case GLUT_KEY_RIGHT:
        //Starting animation when right arrow key is pressed
        animateSprite = true;
        animationSpeed = 0.0001f; //Setting the animation speed for forward animation
        break;
    case GLUT_KEY_LEFT:
        //Starting animation when left arrow key is pressed
        animateSprite = true;
        animationSpeed = -0.0001f; //Setting the animation speed for reverse animation
        break;
   }
  glutPostRedisplay();
}

void SpecialUpkeys(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_RIGHT:
        //Stopping animation when right arrow key is released
        animateSprite = false;
        animationSpeed = 0.0f;
        break;
    case GLUT_KEY_LEFT:
        //Stopping animation when left arrow key is released
        animateSprite = false;
        animationSpeed = 0.0f;
        break;
    }
    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

static void init(void)
{
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    loadGLTextures();      //Loading the background texture
    loadSpriteSheet();     //Loadinf the sprite sheet texture

    glEnable(GL_DEPTH_TEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
}


/* Program entry point */

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    glutInitWindowSize(900,600);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("2D Animation");
    init();
    glutReshapeFunc(resize);
    glutDisplayFunc(display);
     glutMouseFunc(mouse);
    glutKeyboardFunc(key);
    glutSpecialFunc(Specialkeys);
    glutSpecialUpFunc(SpecialUpkeys);

    glutIdleFunc(idle);
    glutMainLoop();

    return EXIT_SUCCESS;
}
