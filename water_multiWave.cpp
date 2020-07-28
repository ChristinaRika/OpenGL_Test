#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#define PI 3.14
#define RESOLUTION 100

//GLUT
#include<GL/glut.h>

//SOIL
#include <SOIL.h>

bool LoadGLTextures(const char* textureName);
bool wire_frame = false;
bool normals = false;
//light
GLfloat LightAmbient[] = { 0.9f,0.9f,0.9f,1.0f };
GLfloat LightDiffuse[] = { 0.9f,0.9f,0.9f,1.0f };
GLfloat LightPosition[] = { 1.0f,1.0f,-0.5f,0.0f };


//wave source
struct waveSource {
	float amplitude;
	float waveLength;
	float speed;
	float center[1][2];

};
waveSource wave[] = 
{ 
	{0.01,0.3,-0.2,0.0,0.0}, 
	{0.01,0.3,0.1,0.0,0.5},
	{0.015,0.23,0.3,0.1,0.2},
	{0.01,0.1,0.12,0.4,-0.1}
};//////waves
int numWaves = sizeof(wave) / sizeof(waveSource);

struct Vector3 {
	float x;
	float y;
	float z;
	Vector3(void) {
		x = 0;
		y = 0;
		z = 0;
	}
	Vector3(float x0, float y0, float z0)
	{
		x = x0;
		y = y0;
		z = z0;
	}
	~Vector3() {};
};
float dot(int i, float x, float y) {
	float cx = x - wave[i].center[0][0];
	float cy = y - wave[i].center[0][1];
	return sqrt(cx * cx + cy * cy);
}
float subWaveHeight(int i, float x, float y, float time) {
	float frequency = 2 * PI / wave[i].waveLength;
	float phase = wave[i].speed * frequency;
	float theta = dot(i, x, y);
	return wave[i].amplitude * sin(theta * frequency + time * phase);
}
float waveHeight(float x, float y, float time) {
	float height = 0.0f;
	for (int i = 0; i < numWaves; i++) {
		height += subWaveHeight(i, x, y, time);
	}
	return height;
}
float dWavedx(int i, float x, float y, float time) {
	float frequency = 2 * PI / wave[i].waveLength;
	float phase = wave[i].speed * frequency;
	float theta = dot(i, x, y);
	float A = wave[i].amplitude * x * frequency / theta;
	return A * cos(theta * frequency + time * phase);
}
float dWavedy(int i, float x, float y, float time) {
	float frequency = 2 * PI / wave[i].waveLength;
	float phase = wave[i].speed * frequency;
	float theta = dot(i, x, y);
	float A = wave[i].amplitude * y * frequency / theta;
	return A * cos(theta * frequency + time * phase);
}
Vector3 waveNormal(float x, float y, float time) {
	float dx = 0;
	float dy = 0;
	for (int i = 0; i < numWaves; i++) {
		dx += dWavedx(i, x, y, time);
		dy += dWavedy(i, x, y, time);
	}
	
	Vector3 n;
	n.x = -dx;
	n.y = 1.0;
	n.z = -dy;
	float l = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
	if (l != 0) {
		n.x /= 1;
		n.y /= 1;
		n.z /= 1;
	}
	else {
		n.x = 0;
		n.y = 1;
		n.z = 0;
	}
	return n;
}
//water momdel
static float surface[6 * RESOLUTION * (RESOLUTION + 1)];
static float normal[6 * RESOLUTION * (RESOLUTION + 1)];

static float	rotate_x = 30;
static float	rotate_y = 15;
static float	translate_z = 4;
void render()
{
	glEnable(GL_TEXTURE_2D);
	const float t = glutGet(GLUT_ELAPSED_TIME) / 1000.;
	const float delta = 2. / RESOLUTION;
	const unsigned int length = 2 * (RESOLUTION + 1);
	const float xn = (RESOLUTION + 1) * delta + 1;
	unsigned int i;
	unsigned int j;
	float x;
	float y;
	unsigned int indice;
	unsigned int preindice;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -translate_z);
	glRotatef(rotate_y, 1, 0, 0);
	glRotatef(rotate_x, 0, 1, 0);

	/* Vertices */
	for (j = 0; j < RESOLUTION; j++)
	{
		y = (j + 1) * delta - 1;
		for (i = 0; i <= RESOLUTION; i++)
		{
			indice = 6 * (i + j * (RESOLUTION + 1));

			x = i * delta - 1;
			surface[indice + 3] = x;
			surface[indice + 4] = waveHeight(x, y, t);
			surface[indice + 5] = y;
			if (j != 0)
			{
				/* Values were computed during the previous loop */
				preindice = 6 * (i + (j - 1) * (RESOLUTION + 1));
				surface[indice] = surface[preindice + 3];
				surface[indice + 1] = surface[preindice + 4];
				surface[indice + 2] = surface[preindice + 5];
			}
			else
			{
				surface[indice] = x;
				surface[indice + 1] = waveHeight(x, -1, t);
				surface[indice + 2] = -1;
			}
			/* Normals */
			Vector3 n = waveNormal(surface[indice], surface[indice + 2], t);
			normal[indice] = n.x;
			normal[indice + 1] = n.y;
			normal[indice + 2] = n.z;

			n = waveNormal(surface[indice + 3], surface[indice + 5], t);
			normal[indice + 3] = n.x;
			normal[indice + 4] = n.y;
			normal[indice + 5] = n.z;

		}
	}


	/* The ground */
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4f(1.0f, 0.9f, 0.7f, 1.0f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, 0.0f, -1.0f);
	glVertex3f(-1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 0.0f, -1.0f);
	glEnd();

	glTranslatef(0, 0.2, 0);

	/* Render wireframe? */
	if (wire_frame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


	/* The water */
	//glEnable(GL_TEXTURE_2D);
	glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glNormalPointer(GL_FLOAT, 0, normal);
	glVertexPointer(3, GL_FLOAT, 0, surface);
	for (i = 0; i < RESOLUTION; i++)
		glDrawArrays(GL_TRIANGLE_STRIP, i * length, length);

	/* Draw normals? */
	if (normals)
	{
		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		for (j = 0; j < RESOLUTION; j++)
			for (i = 0; i <= RESOLUTION; i++)
			{
				indice = 6 * (i + j * (RESOLUTION + 1));
				glVertex3fv(&(surface[indice]));
				glVertex3f(surface[indice] + normal[indice] / 50,
					surface[indice + 1] + normal[indice + 1] / 50,
					surface[indice + 2] + normal[indice + 2] / 50);
			}
		glEnd();
	}

	/* End */
	glutSwapBuffers();
	glutPostRedisplay();
}


bool initParam()
{
	if (!LoadGLTextures("reflection.jpg")) {
		return false;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	return true;
}
void key_callback(unsigned char key, int x, int y)
{
	//key evernt
	switch (key) {
	case 'q':case 27:exit(0); break;
	case'1':wire_frame = !wire_frame; break;
	case'n':normals = !normals;
		break;
	}

}

void windowResize_callback(int width, int height)
{
	if (width == 0) width = 1;
	float ratio = 1.0 * width / height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluPerspective(20, ratio, 0.1, 15);
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();

}
int xold;
int yold;
int left_click;
int right_click;
void mouse_callback(int button, int state, int x, int y){
	if(GLUT_LEFT_BUTTON == button)
		left_click = state;
	if(GLUT_RIGHT_BUTTON == button)
		right_click = state;
	xold = x;
	yold = y;
}
void mouseMotion(int x, int y){
	if(GLUT_DOWN == left_click){
		rotate_x = rotate_x+(x-xold)/5.0;
		rotate_y = rotate_y+(y-yold)/5.0;
		if(rotate_x>90)rotate_x=90;
		if(rotate_y>90)rotate_y=90;
		glutPostRedisplay();
	}
	if(GLUT_DOWN == right_click){
		translate_z = translate_z+(yold-y)/50;
		if(translate_z<0.5)translate_z=0.5;
		if(translate_z>10)translate_z=10;
		glutPostRedisplay();
	}
	xold=x;
	yold=y;
}

bool LoadGLTextures(const char* textureName) {
	GLuint texture = SOIL_load_OGL_texture(
		textureName,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);
	if (texture == 0) {
		return false;
	}
	glBindTexture(GL_TEXTURE_2D, texture);
	//near/far filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	return true;
}

int main(int argc, char** argv)
{
	//glfwInit();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("Water");

	initParam();

	glutDisplayFunc(render);
	glutReshapeFunc(windowResize_callback);
	glutKeyboardFunc(key_callback);
	glutMouseFunc(mouse_callback);
	glutMotionFunc(mouseMotion);

	// Game loop    
	glutMainLoop();

	return 0;
}