#include <GL/glut.h>
#include <cmath>
#include "edge_function.cpp"
#include <string>
#include <random>


// Number of segments for tessellation
int slices = 100;
int stacks = 100;

// Rotation angles
float rotX = 0.0f, rotY = 0.0f;
int lastMouseX, lastMouseY;
bool rotating = false;

// Light position (modifiable dynamically)
GLfloat lightPosition[] = {2.0f, 2.0f, 2.0f, 1.0f};

GLuint texture;
GLuint floorTexture;  // Texture for the floor
bool textureEnabled = true; // Flag to track texture state

bool darkBackground = true;  // Flag for background color
GLfloat textColor[3] = {1.0f, 1.0f, 1.0f}; // Initial text color (white)

// Color components (modifiable)
float textureR = 0.5f; // Initial grey (approximately 127/255)
float textureG = 0.5f;
float textureB = 0.5f;

enum TextureMode { NO_TEXTURE, DYNAMIC_TEXTURE, RGB_TEXTURE, RAINBOW_TEXTURE };
TextureMode currentTextureMode = NO_TEXTURE; // Start with no texture

// Textures
GLuint dynamicTexture; // Texture ID for the dynamically colored texture
GLuint rgbTexture;
GLuint rainbowTexture;


// Profile function for the bishop's shape
double bishopProfile(double y) {
  return edge_function(y);
}


// Modified function to accept a color parameter
void createCheckerboardTexture(int texSize, int checkSize, unsigned char* data) {

    for (int i = 0; i < texSize; ++i) {
        for (int j = 0; j < texSize; ++j) {
            int color = (((i / checkSize) % 2) ^ ((j / checkSize) % 2)) * 255;
            data[(i * texSize + j) * 3] = color;
            data[(i * texSize + j) * 3 + 1] = color;
            data[(i * texSize + j) * 3 + 2] = color;
        }
    }
}


void createCheckerboardTexture(int texSize, int checkSize) {
    unsigned char* data = new unsigned char[texSize * texSize * 3];  // RGB texture

    for (int i = 0; i < texSize; ++i) {
        for (int j = 0; j < texSize; ++j) {
            int color = (((i / checkSize) % 2) ^ ((j / checkSize) % 2)) * 255; // Checkerboard pattern (0 or 255)
            data[(i * texSize + j) * 3] = color;       // Red
            data[(i * texSize + j) * 3 + 1] = color;   // Green
            data[(i * texSize + j) * 3 + 2] = color;   // Blue
        }
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;
}


void createRandomTexture(int texSize) {
    std::random_device rd;  // Obtain a random seed from the OS
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(150, 220);

    unsigned char* data = new unsigned char[texSize * texSize * 3];
    for (int i = 0; i < texSize; ++i) {
        for (int j = 0; j < texSize; ++j) {
             // Random values for the random effect
            int r = distrib(gen);
            int g = distrib(gen);
            int b = distrib(gen);


            data[(i * texSize + j) * 3] = r;
            data[(i * texSize + j) * 3 + 1] = g;
            data[(i * texSize + j) * 3 + 2] = b;
        }
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;

}


void createDynamicTexture(int texSize) {
    unsigned char* data = new unsigned char[texSize * texSize * 3];

    for (int i = 0; i < texSize; ++i) {
        for (int j = 0; j < texSize; ++j) {
            data[(i * texSize + j) * 3] = static_cast<unsigned char>(textureR * 255);
            data[(i * texSize + j) * 3 + 1] = static_cast<unsigned char>(textureG * 255);
            data[(i * texSize + j) * 3 + 2] = static_cast<unsigned char>(textureB * 255);
        }
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    delete[] data;
}



void drawLightSource() {
    glPushMatrix();
    glTranslatef(lightPosition[0], lightPosition[1], lightPosition[2]);
    glColor3f(0.0f, 0.0f, 0.0f); // Yellow color for the light
    glutSolidSphere(0.1f, 20, 20); // Draw a small sphere
    glPopMatrix();
}


void renderBitmapString(float x, float y, void *font, const char *string) {
    const char *c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}


// Lighting setup
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f }; // Low ambient
    GLfloat lightDiffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f }; // Golden diffuse color
    GLfloat lightSpecular[] = { 0.8f, 0.7f, 0.2f, 1.0f }; // Golden specular for highlights


    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);


    glEnable(GL_NORMALIZE);
}


// Material setup
void setupMaterial() {
    GLfloat matAmbient[]  = { 0.8f, 0.8f, 0.8f, 1.0f }; // White ambient
    GLfloat matDiffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f }; // White diffuse
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // White specular (for highlights)
    GLfloat shininess[]   = { 50.0f };                   // Shininess remains the same

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}


void drawBishop() {
    if (textureEnabled) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture only ONCE if enabled
    }

    float totalHeight = 1.2f;
    for (int i = 0; i < stacks; ++i) {
        float h1 = totalHeight * i / stacks;
        float h2 = totalHeight * (i + 1) / stacks;
        float r1 = bishopProfile(h1) / 4.2;
        float r2 = bishopProfile(h2) / 4.2;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; ++j) {
            float angle = 2.0f * M_PI * j / slices;
            float x1 = r1 * cos(angle), y1 = r1 * sin(angle);
            float x2 = r2 * cos(angle), y2 = r2 * sin(angle);

            float nx1 = cos(angle), ny1 = sin(angle);
            float nx2 = cos(angle), ny2 = sin(angle);


            if (textureEnabled) {
                float s = (float)j / slices;
                float t = (float)i / stacks;
                glTexCoord2f(s, t);
            }

            glNormal3f(nx1, ny1, 0.0f);
            glVertex3f(x1, y1, h1);

            if (textureEnabled) {
                float s = (float)j / slices;
                float t1 = (float)(i + 1) / stacks;
                glTexCoord2f(s, t1);
            }

            glNormal3f(nx2, ny2, 0.0f);
            glVertex3f(x2, y2, h2);
        }
        glEnd();
    }


    if (textureEnabled) {
        glDisable(GL_TEXTURE_2D);
    }
}


void drawFloor(float size) {

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);  // Normal pointing upwards
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-size, -0.6f, -size);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(size, -0.6f, -size);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(size, -0.6f, size);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-size, -0.6f, size);

    glEnd();
    glDisable(GL_TEXTURE_2D);

}


// Mouse button handler
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        rotating = true;
        lastMouseX = x;
        lastMouseY = y;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        rotating = false;
    }
}


// Mouse motion handler
void motion(int x, int y) {
    if (rotating) {
        rotX += (y - lastMouseY) * 0.2f;
        rotY += (x - lastMouseX) * 0.2f;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay(); // Trigger redraw
    }
}


float lightDistance = 2.0f; // Initial distance from origin
float lightSpinX = 0.0f;    // Rotation around X-axis
float lightSpinY = 0.0f;    // Rotation around Y-axis


void keyboard(int key, int x, int y) {

    if (key == GLUT_KEY_LEFT) {
        lightSpinY -= 5.0f;
    }
    if (key == GLUT_KEY_RIGHT) {
        lightSpinY += 5.0f;
    }
    if (key == GLUT_KEY_UP) {
        lightSpinX += 5.0f;
    }
    if (key == GLUT_KEY_DOWN) {
        lightSpinX -= 5.0f;
    }
    if (key == '1') {      // Move light forward (positive Z)
      lightDistance -= 0.1f;
    }
    if (key == '2') {     // Move light backward (negative Z)
        lightDistance += 0.1f;
    }

     // Calculate the light's position based on its spin and distance from origin.
    lightPosition[0] = lightDistance * sin(lightSpinY * M_PI / 180.0) * cos(lightSpinX * M_PI/180.0);
    lightPosition[1] = lightDistance * sin(lightSpinX * M_PI/180.0);
    lightPosition[2] = lightDistance * cos(lightSpinY * M_PI / 180.0) * cos(lightSpinX * M_PI/180.0);

    glutPostRedisplay();
}


static void key(unsigned char key, int x, int y)
{
    // Get the state of keyboard modifiers
    int modifier = glutGetModifiers();
    float colorChange = 0.05f; // Amount to change color by

    switch (key) {
        case 't': // Toggle background and text color
            darkBackground = !darkBackground;
            if (darkBackground) {
                textColor[0] = 1.0f; // White text on dark background
                textColor[1] = 1.0f;
                textColor[2] = 1.0f;
            } else {
                textColor[0] = 0.0f; // Black text on light background
                textColor[1] = 0.0f;
                textColor[2] = 0.0f;
            }
            glutPostRedisplay();
            break;

        case '1':
            if (!textureEnabled) break;
            if (modifier == GLUT_ACTIVE_ALT) {
                textureR = std::max(0.0f, textureR - colorChange); // Decrease and clamp
            } else {
                textureR = std::min(1.0f, textureR + colorChange); // Increase and clamp
            }
            createDynamicTexture(256); // Recreate texture with new color
            glutPostRedisplay();
            break;

        case '2':
            if (!textureEnabled) break;
            if (modifier == GLUT_ACTIVE_ALT) {
                textureG = std::max(0.0f, textureG - colorChange);
            } else {
                textureG = std::min(1.0f, textureG + colorChange);
            }
            createDynamicTexture(256);
            glutPostRedisplay();
            break;

        case '3':
            if (!textureEnabled) break;
            if (modifier == GLUT_ACTIVE_ALT) {
                textureB = std::max(0.0f, textureB - colorChange);
            } else {
                textureB = std::min(1.0f, textureB + colorChange);
            }
            createDynamicTexture(256);
            glutPostRedisplay();
            break;

        case 27:  // ESC key
        case 'q': // Quit
            exit(0);
            break;

        case 'e':  // Toggle texture
            textureEnabled = !textureEnabled;
            glutPostRedisplay();
            break;

        case '=':
            if (modifier == GLUT_ACTIVE_ALT) {
                // Increase slices when Alt + '='
                slices++;
            } else {
                // Increase stacks when just '='
                stacks++;
            }
            break;

        case '-':
            if (modifier == GLUT_ACTIVE_ALT) {
                // Decrease slices when Alt + '-'
                if (slices > 3) slices--;
            } else {
                // Decrease stacks when just '-'
                if (stacks > 3) stacks--;
            }
            break;

        default:
            break;
    }

    // Redisplay the scene after modification
    glutPostRedisplay();
}


static void idle(void)
{
    glutPostRedisplay();
}


// Display function
void display() {
    glClearColor(darkBackground ? 0.1f : 0.8f, darkBackground ? 0.1f : 0.8f, darkBackground ? 0.1f : 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.5f, -3.0f);
    glRotatef(rotX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    setupMaterial();
    setupLighting();
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    drawBishop();
    drawLightSource();



    // Text Rendering Section
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);

    // Set text color based on background
    if (darkBackground) {
        glColor3f(1.0f, 1.0f, 1.0f); // White text on dark background
    } else {
        glColor3f(0.0f, 0.0f, 0.0f); // Black text on light background
    }


    // Text lines for instructions 1-5
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 20, GLUT_BITMAP_HELVETICA_12,  "+-----------------------------------------------+                           ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 40, GLUT_BITMAP_HELVETICA_12,  "|     1. Press [ q ] to quit.                                               ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 60, GLUT_BITMAP_HELVETICA_12,  "|     2. Press [ e ] to toggle textures.                                    ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 80, GLUT_BITMAP_HELVETICA_12,  "|     3. Press [ t ] to toggle background.                                  ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 100, GLUT_BITMAP_HELVETICA_12, "|     4. Use arrow keys to move the light source.                           ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 120, GLUT_BITMAP_HELVETICA_12, "|     5. Click and drag mouse to rotate the object.                         ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 140, GLUT_BITMAP_HELVETICA_12, "|     6. Use [ = / - ] keys to increase / decrease stacks.                  ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 160, GLUT_BITMAP_HELVETICA_12, "|     7. Use [ Alt + [ = / - ] ] keys to increase / decrease of slices.     ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 180, GLUT_BITMAP_HELVETICA_12, "|     8. Use [ 1, 2, 3 ] / [ Alt + [ 1, 2, 3 ] ] to change bishop color.    ");
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 200, GLUT_BITMAP_HELVETICA_12, "+-----------------------------------------------+                           ");


    // Text lines for dynamic info (slices/stacks)
    char info[100];
    char color_info[100];
    snprintf(info, sizeof(info), "|      =>     Slices: %d  |  Stacks: %d                             ", slices, stacks);
    snprintf(color_info, sizeof(color_info), "|      =>     Red: %d  |  Green: %d  |  Blue: %d                             ", int(textureR * 255), int(textureG * 255), int(textureB * 255));
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 220, GLUT_BITMAP_HELVETICA_12, info);
    renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 240, GLUT_BITMAP_HELVETICA_12, "+-----------------------------------------------+");
    if (textureEnabled){
        renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 260, GLUT_BITMAP_HELVETICA_12, color_info);
        renderBitmapString(10, glutGet(GLUT_WINDOW_HEIGHT) - 280, GLUT_BITMAP_HELVETICA_12, "+-----------------------------------------------+");
    }

    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}


// Main function
int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800*2, 600*2);
    glutCreateWindow("Interactive Bishop Chess Piece");

    //==========================================================================================
    // --- createCheckerboardTexture(256, 32);  // Create a 256x256 texture, 32 pixel check size
    //==========================================================================================
    // createMattTexture(256);
    createDynamicTexture(256);

    //==========================================================================================
    /*
    unsigned char* bishopTexData = new unsigned char[256 * 256 * 3];
    //createMattTexture(256, bishopTexData); //Generate matt texture.


    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, bishopTexData);
        delete[] bishopTexData;


    unsigned char* floorTexData = new unsigned char[256 * 256 * 3];
    createCheckerboardTexture(256, 32, floorTexData);

    glGenTextures(1, &floorTexture);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, floorTexData);

    delete[] floorTexData;
    */
    //==========================================================================================

    // Set up projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.33, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 0.1f);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Setup lighting
    setupLighting();

    // Register callbacks
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSpecialFunc(keyboard);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);

    // Start rendering loop
    glutMainLoop();

    return 0;
}
