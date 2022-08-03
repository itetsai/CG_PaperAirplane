// openGL_Test.cpp : Defines the entry point for the console application.


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../../include/GL/glew.h"
#include "../../include/GL/freeglut.h"


#include "../../include/assimp/Importer.hpp"
#include "../../include/assimp/scene.h"
#include "../../include/assimp/postprocess.h"

#include "../../include/glm/glm.hpp"
#include "../../include/glm/gtc/type_ptr.hpp"
#include "../../include/glm/gtc/matrix_transform.hpp"

#if defined(_WIN32)
#pragma comment(lib,"../../lib/x64/freeglut.lib")
#pragma comment(lib,"../../lib/x64/glew32.lib")
#pragma comment(lib,"../../lib/x64/assimp-vc140-mt.lib")
#pragma comment(lib,"../../lib/x64/glm_static.lib")	
#endif

#define WINDOW_TITLE_PREFIX "Midterm Project - Press m to switch view mode"

#include <vector>
using namespace std;

int CurrentWidth = 800, CurrentHeight = 600, WindowHandle = 0;

unsigned FrameCount = 0;
int deg = 0;

const aiScene* scene;
const aiScene* airplane;

using namespace glm;

void ResizeFunction(int, int);
void RenderFunction(void);

void recursive_render(const aiScene* sc, const aiNode* nd);

vector<vector<float>> route;

bool mode = true;

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'm':
		mode = !mode;
	}
}


int main(int argc, char* argv[])
{
	//初始化 glut
	glutInit(&argc, argv);

	//設定 glut 畫布尺寸 與color / depth模式
	glutInitWindowSize(CurrentWidth, CurrentHeight);
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA);
	
	//根據已設定好的 glut (如尺寸,color,depth) 向window要求建立一個視窗，接著若失敗則退出程式
	WindowHandle = glutCreateWindow(WINDOW_TITLE_PREFIX);
	if(WindowHandle < 1) {	fprintf(stderr,"ERROR: Could not create a new rendering window.\n");exit(EXIT_FAILURE);	}
	
	glutReshapeFunc(ResizeFunction); //設定視窗 大小若改變，則跳到"AResizeFunction"這個函數處理應變事項
	glutDisplayFunc(RenderFunction);  //設定視窗 如果要畫圖 則執行"RenderFunction"

	GLenum GlewInitResult = glewInit();
	if (GlewInitResult != GLEW_OK ) {	fprintf(stderr,"ERROR: %s\n",glewGetErrorString(GlewInitResult)	);	exit(EXIT_FAILURE);	}

	//背景顏色黑
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat MaterialAmbient[] = {0.4,0.4,0.4,1.0f};
	GLfloat MaterialDiffuse[] = {0.7,0.7,0.7,1.0f};
	GLfloat MaterialSpecular[] =  { 1.2,1.2,1.2, 1.0f};
	GLfloat AmbientLightPosition[] = {-250,-250,1000,1.0f};

	glLightfv(GL_LIGHT0, GL_AMBIENT, MaterialAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, MaterialDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, MaterialSpecular);
	glLightfv(GL_LIGHT0, GL_POSITION, AmbientLightPosition);

	glutKeyboardFunc(keyboard);

	//透過Assimp讀檔
	Assimp::Importer importer;
	scene = importer.ReadFile("City.stl", aiProcess_Triangulate | aiProcess_FlipUVs);
	Assimp::Importer importer2;
	airplane = importer2.ReadFile("PaperPlane.stl", aiProcess_Triangulate | aiProcess_FlipUVs);

	//讀取路徑檔案
	FILE* fp;
	char ch;
	fp = fopen("Route.xyz", "r");
	if (fp != NULL)
	{
		while (true)
		{
			float p;
			vector<float> p_vector;
			if (fscanf(fp, "%f", &p) == EOF)
				break;
			p_vector.push_back(p);
			fscanf(fp, "%f", &p);
			p_vector.push_back(p);
			fscanf(fp, "%f", &p);
			p_vector.push_back(p);
			route.push_back(p_vector); 
		}
	}
	else
		printf("------Read file ERROR------\n");

	glutMainLoop();

	exit(EXIT_SUCCESS);
}

void ResizeFunction(int Width, int Height)
{
	CurrentWidth = Width;
	CurrentHeight = Height;
	glViewport(0, 0, CurrentWidth, CurrentHeight);
}


int t = 0;
void RenderFunction(void)
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, CurrentWidth, CurrentHeight);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (mode)
	{
		glOrtho(-float(CurrentWidth), float(CurrentWidth),
			-float(CurrentHeight), float(CurrentHeight),
			-CurrentHeight * 10.0, CurrentHeight * 10.0);
		gluLookAt(-1000, -1000, 500,
			0, 0, 0,
			0, 0, 1);
	}
	else
	{
		gluPerspective(60.0, (GLfloat)CurrentWidth / (GLfloat)CurrentHeight, 25, 1000.0);

		gluLookAt(route[t % route.size()][0], route[t % route.size()][1], route[t % route.size()][2],
			route[(t + 1) % route.size()][0], route[(t + 1) % route.size()][1], route[(t + 1) % route.size()][2],
			0, 0, 1);
	}


	glMatrixMode(GL_MODELVIEW);
	
	glPushMatrix();
	recursive_render(scene, scene->mRootNode);
	glPopMatrix();

	glPushMatrix();	
	glm::mat4 move = glm::translate(glm::mat4(1.0), vec3(route[t%route.size()][0], route[t%route.size()][1], route[t%route.size()][2]));
	glMultMatrixf((const float*)glm::value_ptr(move));

	float z_angle = atan2f(route[(t + 1) % route.size()][0] - route[t % route.size()][0], route[(t + 1) % route.size()][1] - route[t % route.size()][1]);
	glm::mat4 Rmatrix = glm::rotate(glm::mat4(1.0), -z_angle, vec3(0, 0, 1));
	glMultMatrixf((const float*)glm::value_ptr(Rmatrix));

	float xy_distance = sqrtf(
		(route[(t + 1) % route.size()][1] - route[t % route.size()][1]) * (route[(t + 1) % route.size()][1] - route[t % route.size()][1]) +
		(route[(t + 1) % route.size()][0] - route[t % route.size()][0]) * (route[(t + 1) % route.size()][0] - route[t % route.size()][0])
	);
	float x_angle = atan2f(route[(t + 1) % route.size()][2] - route[t % route.size()][2], xy_distance);
	glm::mat4 Rmatrix2 = glm::rotate(glm::mat4(1.0), x_angle, vec3(1, 0, 0));
	glMultMatrixf((const float*)glm::value_ptr(Rmatrix2));
	
	recursive_render(airplane, airplane->mRootNode);
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
	Sleep(15);
	t++;
}


void recursive_render(const aiScene* sc, const aiNode* nd)
{
	unsigned int i;
	unsigned int n = 0, t;
	aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiMatrix4x4 mT = m.Transpose();
	glMultMatrixf((float*)& mT);
	glPushMatrix();
	
	
	// draw all meshes assigned to this node
	for (n=0; n < nd->mNumMeshes; n++) {
		const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

		
		for (t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch (face->mNumIndices) {
				case 1: face_mode = GL_POINTS; break;
				case 2: face_mode = GL_LINES; break;
				case 3: face_mode = GL_TRIANGLES; break;
				default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);
			for (i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				if (mesh->mColors[0] != NULL) glColor4fv((GLfloat*)& mesh->mColors[0][index]);
				if (mesh->mNormals != NULL)
				{
					glNormal3fv(&mesh->mNormals[index].x);
				}
				
				glVertex3fv(&mesh->mVertices[index].x);
			}

			glEnd();
		}

	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		recursive_render(sc, nd->mChildren[n]);
	}
	glPopMatrix();
}
