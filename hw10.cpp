// HW10 for CS 637
// Shangqi Wu

#include "Angel.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

using namespace std;

typedef vec4 color4;

// Height and width of main window. 
const int h = 500;
const int w = 500;

const int verNum = 3;
const int triFaceNum = 12;

// True for perspective projection, false for parallel projection. 
bool perspective = true;

// Mode 0 for modifying base, mode 1 for modifying lower arm, mode 2 for modifying upper arm. 
int mode = 0;
// Translate matrix for lower arm.
const mat4 lowerArmTranlate(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.65,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);
// Translate matrix for upper arm. 
const mat4 upperArmTranslate(
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.45,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);
// Theta values for base rotation.
float baseThetaX = 0;
float baseThetaY = 0;
float baseThetaZ = 0;
// Theta values for lower arm rotation.
float lowerArmThetaX = 0;
float lowerArmThetaY = 0;
float lowerArmThetaZ = 0;
// Theta values for upper arm rotation.
float upperArmThetaX = 0;
float upperArmThetaY = 0;
float upperArmThetaZ = 0;

// RGBA color for background of main window.
float Red = 0;
float Green = 0;
float Blue = 0;
float Alpha = 1;

// Radius of camera rotation and its delta.
float cameraRadius = 1;
float dr = 0.1;
// Height of camera and its delta.
float cameraHeight = 0.5;
float dh = 0.25;
// Current position and its delta of a circling camera
float t = 0;
float dt = 0.005;

// Initial position of look-at, camera position, and up vector for projection.
vec4 at(0, 0.5, 0, 1);
vec4 eye(0, 0, 0, 1);
vec4 up(0, 1, 0, 1);

// Phong shading model parameters of light source 1, which rotates around the object.
float Idr1 = 0.4;
float Idg1 = 0.4;
float Idb1 = 0.4;
float Isr1 = 0.2;
float Isg1 = 0.2;
float Isb1 = 0.2;
float Iar1 = 0.05;
float Iag1 = 0.05;
float Iab1 = 0.05;

// Phong shading model parameters of light source 2, which moves with camera.
float Idr2 = 0.3;
float Idg2 = 0.3;
float Idb2 = 0.3;
float Isr2 = 0.2;
float Isg2 = 0.2;
float Isb2 = 0.2;
float Iar2 = 0.05;
float Iag2 = 0.05;
float Iab2 = 0.05;

// Shininess parameter for phong shading.
float shininess = 100;

// Phong shading model parameters of material property. 
float kdr = 157.0 / 255.0;
float kdg = 217.0 / 255.0;
float kdb = 234.0 / 255.0;
float ksr = 157.0 / 255.0;
float ksg = 217.0 / 255.0;
float ksb = 234.0 / 255.0;
float kar = 157.0 / 255.0;
float kag = 217.0 / 255.0;
float kab = 234.0 / 255.0;

// Position parameters of light source 2. 
float lightHeight = 1;
float lightRadius = 1;
float dhlight2 = 0.2;
float drlight2 = 0.2;
float tlight = 1;
float dtlight = 0.1;

// IDs for main window. 
int MainWindow;

// ID for shaders program.
GLuint program;

// IDs for FBO components.
GLuint frameBuffer, colorBuffer, depthBuffer;
GLint samples;

//--------------------------------------------------------------------------

vec4
product(const vec4 &a, const vec4 &b)
{
	return vec4(a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3]);
}

//--------------------------------------------------------------------------

void
init(void)
{
	// FBO for rendering. 
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// Get capatibility for MSAA.
	glGetIntegerv(GL_MAX_SAMPLES, &samples);

	// Generate color and depth render buffers. 
	glGenRenderbuffers(1, &colorBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);

	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT24, w, h);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		cerr << "FBO incomplete." << endl;
		exit(1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create vertices for base.
	vec4 base[triFaceNum][verNum] = {
		vec4(0.2, 0.7, 0.2, 1), vec4(-0.2, 0.7, 0.2, 1), vec4(0.2, 0, 0.2, 1),
		vec4(-0.2, 0.7, 0.2, 1), vec4(-0.2, 0, 0.2, 1), vec4(0.2, 0, 0.2, 1),
		vec4(0.2, 0.7, 0.2, 1), vec4(0.2, 0, -0.2, 1), vec4(0.2, 0.7, -0.2, 1),
		vec4(0.2, 0.7, 0.2, 1), vec4(0.2, 0, 0.2, 1), vec4(0.2, 0, -0.2, 1),
		vec4(-0.2, 0.7, -0.2, 1), vec4(0.2, 0, -0.2, 1), vec4(0.2, 0.7, -0.2, 1),
		vec4(-0.2, 0.7, -0.2, 1), vec4(-0.2, 0, -0.2, 1), vec4(0.2, 0, -0.2, 1),
		vec4(-0.2, 0.7, -0.2, 1), vec4(-0.2, 0.7, 0.2, 1), vec4(-0.2, 0, 0.2, 1),
		vec4(-0.2, 0.7, -0.2, 1), vec4(-0.2, 0, -0.2, 1), vec4(-0.2, 0, 0.2, 1),
		vec4(-0.2, 0.7, -0.2, 1), vec4(-0.2, 0.7, 0.2, 1), vec4(0.2, 0.7, 0.2, 1),
		vec4(-0.2, 0.7, -0.2, 1), vec4(0.2, 0.7, 0.2, 1), vec4(0.2, 0.7, -0.2, 1),
		vec4(-0.2, 0, 0.2, 1), vec4(-0.2, 0, -0.2, 1), vec4(0.2, 0, -0.2, 1),
		vec4(-0.2, 0, 0.2, 1), vec4(0.2, 0, -0.2, 1), vec4(0.2, 0, 0.2, 1)
	};

	// Create vertices for lower arm. 
	vec4 lowerArm[triFaceNum][verNum] = {
		vec4(0.1, 0.5, 0.1, 1), vec4(-0.1, 0.5, 0.1, 1), vec4(0.1, 0, 0.1, 1),
		vec4(-0.1, 0.5, 0.1, 1), vec4(-0.1, 0, 0.1, 1), vec4(0.1, 0, 0.1, 1),
		vec4(0.1, 0.5, 0.1, 1), vec4(0.1, 0, -0.1, 1), vec4(0.1, 0.5, -0.1, 1),
		vec4(0.1, 0.5, 0.1, 1), vec4(0.1, 0, 0.1, 1), vec4(0.1, 0, -0.1, 1),
		vec4(-0.1, 0.5, -0.1, 1), vec4(0.1, 0, -0.1, 1), vec4(0.1, 0.5, -0.1, 1),
		vec4(-0.1, 0.5, -0.1, 1), vec4(-0.1, 0, -0.1, 1), vec4(0.1, 0, -0.1, 1),
		vec4(-0.1, 0.5, -0.1, 1), vec4(-0.1, 0.5, 0.1, 1), vec4(-0.1, 0, 0.1, 1),
		vec4(-0.1, 0.5, -0.1, 1), vec4(-0.1, 0, -0.1, 1), vec4(-0.1, 0, 0.1, 1),
		vec4(-0.1, 0.5, -0.1, 1), vec4(-0.1, 0.5, 0.1, 1), vec4(0.1, 0.5, 0.1, 1),
		vec4(-0.1, 0.5, -0.1, 1), vec4(0.1, 0.5, 0.1, 1), vec4(0.1, 0.5, -0.1, 1),
		vec4(-0.1, 0, 0.1, 1), vec4(-0.1, 0, -0.1, 1), vec4(0.1, 0, -0.1, 1),
		vec4(-0.1, 0, 0.1, 1), vec4(0.1, 0, -0.1, 1), vec4(0.1, 0, 0.1, 1)
	};

	// Create vertices for upper arm. 
	vec4 upperArm[triFaceNum][verNum] = {
		vec4(0.05, 0.4, 0.05, 1), vec4(-0.05, 0.4, 0.05, 1), vec4(0.05, 0, 0.05, 1),
		vec4(-0.05, 0.4, 0.05, 1), vec4(-0.05, 0, 0.05, 1), vec4(0.05, 0, 0.05, 1),
		vec4(0.05, 0.4, 0.05, 1), vec4(0.05, 0, -0.05, 1), vec4(0.05, 0.4, -0.05, 1),
		vec4(0.05, 0.4, 0.05, 1), vec4(0.05, 0, 0.05, 1), vec4(0.05, 0, -0.05, 1),
		vec4(-0.05, 0.4, -0.05, 1), vec4(0.05, 0, -0.05, 1), vec4(0.05, 0.4, -0.05, 1),
		vec4(-0.05, 0.4, -0.05, 1), vec4(-0.05, 0, -0.05, 1), vec4(0.05, 0, -0.05, 1),
		vec4(-0.05, 0.4, -0.05, 1), vec4(-0.05, 0.4, 0.05, 1), vec4(-0.05, 0, 0.05, 1),
		vec4(-0.05, 0.4, -0.05, 1), vec4(-0.05, 0, -0.05, 1), vec4(-0.05, 0, 0.05, 1),
		vec4(-0.05, 0.4, -0.05, 1), vec4(-0.05, 0.4, 0.05, 1), vec4(0.05, 0.4, 0.05, 1),
		vec4(-0.05, 0.4, -0.05, 1), vec4(0.05, 0.4, 0.05, 1), vec4(0.05, 0.4, -0.05, 1),
		vec4(-0.05, 0, 0.05, 1), vec4(-0.05, 0, -0.05, 1), vec4(0.05, 0, -0.05, 1),
		vec4(-0.05, 0, 0.05, 1), vec4(0.05, 0, -0.05, 1), vec4(0.05, 0, 0.05, 1)
	};

	// Create normals for bse. 
	vec4 baseNormals[triFaceNum][verNum] = {
		vec4(0, 0, 1, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 1),
		vec4(0, 0, 1, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 1),
		vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 1),
		vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 1),
		vec4(0, 0, -1, 1), vec4(0, 0, -1, 1), vec4(0, 0, -1, 1),
		vec4(0, 0, -1, 1), vec4(0, 0, -1, 1), vec4(0, 0, -1, 1),
		vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1),
		vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1),
		vec4(0, 1, 0, 1), vec4(0, 1, 0, 1), vec4(0, 1, 0, 1),
		vec4(0, 1, 0, 1), vec4(0, 1, 0, 1), vec4(0, 1, 0, 1),
		vec4(0, -1, 0, 1), vec4(0, -1, 0, 1), vec4(0, -1, 0, 1),
		vec4(0, -1, 0, 1), vec4(0, -1, 0, 1), vec4(0, -1, 0, 1),
	};

	// Create normals for lower arm. 
	vec4 lowerArmNormals[triFaceNum][verNum] = {
		vec4(0, 0, 1, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 1),
		vec4(0, 0, 1, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 1),
		vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 1),
		vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 1),
		vec4(0, 0, -1, 1), vec4(0, 0, -1, 1), vec4(0, 0, -1, 1),
		vec4(0, 0, -1, 1), vec4(0, 0, -1, 1), vec4(0, 0, -1, 1),
		vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1),
		vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1),
		vec4(0, 1, 0, 1), vec4(0, 1, 0, 1), vec4(0, 1, 0, 1),
		vec4(0, 1, 0, 1), vec4(0, 1, 0, 1), vec4(0, 1, 0, 1),
		vec4(0, -1, 0, 1), vec4(0, -1, 0, 1), vec4(0, -1, 0, 1),
		vec4(0, -1, 0, 1), vec4(0, -1, 0, 1), vec4(0, -1, 0, 1),
	};

	// Create normals for upper arm. 
	vec4 upperArmNormals[triFaceNum][verNum] = {
		vec4(0, 0, 1, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 1),
		vec4(0, 0, 1, 1), vec4(0, 0, 1, 1), vec4(0, 0, 1, 1),
		vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 1),
		vec4(1, 0, 0, 1), vec4(1, 0, 0, 1), vec4(1, 0, 0, 1),
		vec4(0, 0, -1, 1), vec4(0, 0, -1, 1), vec4(0, 0, -1, 1),
		vec4(0, 0, -1, 1), vec4(0, 0, -1, 1), vec4(0, 0, -1, 1),
		vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1),
		vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1), vec4(-1, 0, 0, 1),
		vec4(0, 1, 0, 1), vec4(0, 1, 0, 1), vec4(0, 1, 0, 1),
		vec4(0, 1, 0, 1), vec4(0, 1, 0, 1), vec4(0, 1, 0, 1),
		vec4(0, -1, 0, 1), vec4(0, -1, 0, 1), vec4(0, -1, 0, 1),
		vec4(0, -1, 0, 1), vec4(0, -1, 0, 1), vec4(0, -1, 0, 1),
	};

	// Create a vertex array object.
	GLuint vao[1];
	glGenVertexArrays(1, vao);
	glBindVertexArray(vao[0]);
	cout << "glGenVertexArrays(), glBindVertexArray() for main window initialization." << endl;

	// Create and initialize a buffer object.
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	cout << "glGenBuffer(), glBindBuffer() for main window initialization." << endl;
	glBufferData(GL_ARRAY_BUFFER, sizeof(base)+sizeof(lowerArm)+sizeof(upperArm)+sizeof(baseNormals)+sizeof(lowerArmNormals)+sizeof(upperArmNormals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(base), base);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(base), sizeof(lowerArm), lowerArm);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(base) + sizeof(lowerArm), sizeof(upperArm), upperArm);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(base) + sizeof(lowerArm) + sizeof(upperArm), sizeof(baseNormals), baseNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(base) + sizeof(lowerArm) + sizeof(upperArm) + sizeof(baseNormals), sizeof(lowerArmNormals), lowerArmNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(base) + sizeof(lowerArm) + sizeof(upperArm) + sizeof(baseNormals) + sizeof(lowerArmNormals), sizeof(upperArmNormals), upperArmNormals);
	cout << "glBufferData(), glBufferSubData() for main window initialization." << endl;

	// Load shaders and use the resulting shader program.
	program = InitShader("vshader.glsl", "fshader.glsl");
	LinkShader(program);
	cout << "InitShader(), glUseProgram() for main window initialization." << endl;

	// Initialize the vertex position attribute from the vertex shader.
	GLuint loc_ver = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc_ver);
	glVertexAttribPointer(loc_ver, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	// Pass normal vectors of each triangle to vertex shader
	GLuint loc_col = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(loc_col);
	glVertexAttribPointer(loc_col, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(base)+sizeof(lowerArm)+sizeof(upperArm)));
	cout << "glEnableVertexAttribArray(), glVertexAttribPointer() for main window initialization." << endl;
}

//----------------------------------------------------------------------------

void
recal(void)
{
	// Calculate renewed camera position. 
	eye = vec4(cameraRadius*sin(t), cameraHeight, cameraRadius*cos(t), 1);

	// Light 1 is within camera coordinate.
	vec4 light1_pos = vec4(0, 0, 1, 1);
	color4 light1_diffuse = color4(Idr1, Idg1, Idb1, 1);
	color4 light1_specular = color4(Isr1, Isg1, Isb1, 1);
	color4 light1_ambient = color4(Iar1, Iag1, Iab1, 1);

	// Light 2 is within world coordinate.
	vec4 light2_pos = vec4(lightRadius*sin(tlight), lightHeight, lightRadius*cos(tlight), 1);
	color4 light2_diffuse = color4(Idr2, Idg2, Idb2, 1);
	color4 light2_specular = color4(Isr2, Isg2, Isb2, 1);
	color4 light2_ambient = color4(Iar2, Iag2, Iab2, 1);

	// Material property.
	color4 material_diffuse(kdr, kdg, kdb, 1);
	color4 material_ambient(kar, kag, kab, 1);
	color4 material_specular(ksr, ksg, ksb, 1);

	// Create model and projection matrix.
	mat4 modelview;
	mat4 projection;

	// Implementing projection.
	if (perspective) projection *= Perspective(90, 1, 1e-10, 1e10);
	else projection *= Ortho(-10, 10, -10, 10, -100, 100);

	// Implementing modelview. 
	modelview *= LookAt(eye, at, up);

	// Create rotation matrix for each direction of each part of robot. 
	mat4 baseRotateX = RotateX(baseThetaX);
	mat4 baseRotateY = RotateY(baseThetaY);
	mat4 baseRotateZ = RotateZ(baseThetaZ);
	mat4 lowerArmRotateX = RotateX(lowerArmThetaX);
	mat4 lowerArmRotateY = RotateY(lowerArmThetaY);
	mat4 lowerArmRotateZ = RotateZ(lowerArmThetaZ);
	mat4 upperArmRotateX = RotateX(upperArmThetaX);
	mat4 upperArmRotateY = RotateY(upperArmThetaY);
	mat4 upperArmRotateZ = RotateZ(upperArmThetaZ);

	// Passing matrix to vertex shaders. 
	GLuint loc_lowertrans = glGetUniformLocation(program, "lowerArmTranslation");
	glUniformMatrix4fv(loc_lowertrans, 1, GL_FALSE, lowerArmTranlate);
	GLuint loc_uppertrans = glGetUniformLocation(program, "upperArmTranslation");
	glUniformMatrix4fv(loc_uppertrans, 1, GL_FALSE, upperArmTranslate);
	GLuint loc_baseRX = glGetUniformLocation(program, "baseRotateX");
	glUniformMatrix4fv(loc_baseRX, 1, GL_TRUE, baseRotateX);
	GLuint loc_baseRY = glGetUniformLocation(program, "baseRotateY");
	glUniformMatrix4fv(loc_baseRY, 1, GL_TRUE, baseRotateY);
	GLuint loc_baseRZ = glGetUniformLocation(program, "baseRotateZ");
	glUniformMatrix4fv(loc_baseRZ, 1, GL_TRUE, baseRotateZ);
	GLuint loc_lowerRX = glGetUniformLocation(program, "lowerArmRotateX");
	glUniformMatrix4fv(loc_lowerRX, 1, GL_TRUE, lowerArmRotateX);
	GLuint loc_lowerRY = glGetUniformLocation(program, "lowerArmRotateY");
	glUniformMatrix4fv(loc_lowerRY, 1, GL_TRUE, lowerArmRotateY);
	GLuint loc_lowerRZ = glGetUniformLocation(program, "lowerArmRotateZ");
	glUniformMatrix4fv(loc_lowerRZ, 1, GL_TRUE, lowerArmRotateZ);
	GLuint loc_upperRX = glGetUniformLocation(program, "upperArmRotateX");
	glUniformMatrix4fv(loc_upperRX, 1, GL_TRUE, upperArmRotateX);
	GLuint loc_upperRY = glGetUniformLocation(program, "upperArmRotateY");
	glUniformMatrix4fv(loc_upperRY, 1, GL_TRUE, upperArmRotateY);
	GLuint loc_upperRZ = glGetUniformLocation(program, "upperArmRotateZ");
	glUniformMatrix4fv(loc_upperRZ, 1, GL_TRUE, upperArmRotateZ);

	// Pass modelview and projection matrix to vertex shader. 
	GLuint loc_modelview = glGetUniformLocation(program, "modelview");
	glUniformMatrix4fv(loc_modelview, 1, GL_TRUE, modelview);
	GLuint loc_projection = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(loc_projection, 1, GL_TRUE, projection);
	GLint loc_eyeposition = glGetUniformLocation(program, "eyeposition");
	glUniform4f(loc_eyeposition, eye.x, eye.y, eye.z, eye.w);

	// Pass positions of light sources to vertex shader.
	GLint loc_light1_pos = glGetUniformLocation(program, "light1_pos");
	GLint loc_light2_pos = glGetUniformLocation(program, "light2_pos");
	glUniform4f(loc_light1_pos, light1_pos.x, light1_pos.y, light1_pos.z, light1_pos.w);
	glUniform4f(loc_light2_pos, light2_pos.x, light2_pos.y, light2_pos.z, light2_pos.w);

	// Calculate and pass color products of each light source to vertex shader.
	vec4 d_pro1 = product(light1_diffuse, material_diffuse);
	vec4 d_pro2 = product(light2_diffuse, material_diffuse);
	vec4 a_pro1 = product(light1_ambient, material_ambient);
	vec4 a_pro2 = product(light2_ambient, material_ambient);
	vec4 s_pro1 = product(light1_specular, material_specular);
	vec4 s_pro2 = product(light2_specular, material_specular);
	GLint loc_diffuse_product1 = glGetUniformLocation(program, "light1_diffuse_product");
	GLint loc_diffuse_product2 = glGetUniformLocation(program, "light2_diffuse_product");
	GLint loc_specular_product1 = glGetUniformLocation(program, "light1_specular_product");
	GLint loc_specular_product2 = glGetUniformLocation(program, "light2_specular_product");
	GLint loc_ambient_product1 = glGetUniformLocation(program, "light1_ambient_product");
	GLint loc_ambient_product2 = glGetUniformLocation(program, "light2_ambient_product");
	glUniform4f(loc_diffuse_product1, d_pro1.x, d_pro1.y, d_pro1.z, d_pro1.w);
	glUniform4f(loc_diffuse_product2, d_pro2.x, d_pro2.y, d_pro2.z, d_pro2.w);
	glUniform4f(loc_specular_product1, s_pro1.x, s_pro1.y, s_pro1.z, s_pro1.w);
	glUniform4f(loc_specular_product2, s_pro2.x, s_pro2.y, s_pro2.z, s_pro2.w);
	glUniform4f(loc_ambient_product1, a_pro1.x, a_pro1.y, a_pro1.z, a_pro1.w);
	glUniform4f(loc_ambient_product2, a_pro2.x, a_pro2.y, a_pro2.z, a_pro2.w);

	GLint loc_shininess = glGetUniformLocation(program, "shininess");
	glUniform1f(loc_shininess, shininess);
	cout << "glGetUniformLocation(), glUniformMatrix4fv() for transformation matrix." << endl;
}

//----------------------------------------------------------------------------

void
display(void)
{
	recal(); // Calculates vertices & colors for objects in main window. 

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glClearColor(Red, Green, Blue, Alpha); // Set background color of main window.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear main window.
	glViewport(0, 0, w, h);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	GLuint loc_mode = glGetUniformLocation(program, "mode");
	glUniform1i(loc_mode, 0);
	glDrawArrays(GL_TRIANGLES, 0, triFaceNum*verNum); // Draw the points by one triangle.
	glUniform1i(loc_mode, 1);
	glDrawArrays(GL_TRIANGLES, triFaceNum*verNum, triFaceNum*verNum);
	glUniform1i(loc_mode, 2);
	glDrawArrays(GL_TRIANGLES, 2 * triFaceNum*verNum, triFaceNum*verNum);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlitFramebuffer(0, 0, w - 1, h - 1, 0, 0, w - 1, h - 1, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glutSwapBuffers(); // Double buffer swapping. 
	glFlush(); // Flush. 
	cout << "glClearColor(), glClear(), glDrawArrays(), glutSwapBuffers(), glFlush() for main window display function." << endl;
}

//----------------------------------------------------------------------------

void
RotationFunc(void)
{
	t += dt; // Camera rotation animation.
	glutPostRedisplay(); // Redisplay function.
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: exit(EXIT_SUCCESS); break; // "Esc": exit the program.
	case (int)'w': cameraHeight += dh; break; // Increasing camera height.
	case (int)'s': cameraHeight -= dh; break; // Decreasing camera height.
	case (int)'a': cameraRadius += dr; break;// Incresing camera radius, the object looks smaller under perspective projection.
	case (int)'d': if (cameraRadius > dr) cameraRadius -= dr; break; // Decreasing camera radius, the object looks larger under perspective projection.
	case (int)'e': dt += 0.0025; break; // Increase camera rotation speed.
	case (int)'q': dt -= 0.0025; break; // Decrease camera rotation speed.
	case (int)'x': t += dt; break; // Allows you rotate camera by one step. 
	case (int)'z': t -= dt; break;
	case (int)'t': lightHeight += dhlight2; break; // Increasing light height.
	case (int)'g': lightHeight -= dhlight2; break; // Decreasing light height.
	case (int)'h': lightRadius += drlight2; break; // Increasing light orbit radius, the light source becomes farther to the object.
	case (int)'f': lightRadius -= drlight2; break; // Decreasing light orbit radius, the loght source becomes closer to the object. 
	case (int)'y': tlight += dtlight; break; // Rotate light 2 counter-clockwise.
	case (int)'r': tlight -= dtlight; break; // Rotate light 2 clockwise.
	case (int)'v': dtlight *= 2; break; // Make light 2 rotate 2x faster.
	case (int)'c': dtlight /= 2; break; // Make light 2 rotate half speed.
	case (int)'1': mode = 0; break;
	case (int)'2': mode = 1; break;
	case (int)'3': mode = 2; break;
	case (int)'i': if (mode == 1) lowerArmThetaX -= 15; else if (mode == 2) upperArmThetaX -= 15; else baseThetaX -= 15; break;
	case (int)'k': if (mode == 1) lowerArmThetaX += 15; else if (mode == 2) upperArmThetaX += 15; else baseThetaX += 15; break;
	case (int)'j': if (mode == 1) lowerArmThetaZ += 15; else if (mode == 2) upperArmThetaZ += 15; else baseThetaZ += 15; break;
	case (int)'l': if (mode == 1) lowerArmThetaZ -= 15; else if (mode == 2) upperArmThetaZ -= 15; else baseThetaZ -= 15; break;
	case (int)'o': if (mode == 1) lowerArmThetaY += 15; else if (mode == 2) upperArmThetaY += 15; else baseThetaY += 15; break;
	case (int)'u': if (mode == 1) lowerArmThetaY -= 15; else if (mode == 2) upperArmThetaY -= 15; else baseThetaY -= 15; break;
	}
	glutPostRedisplay();
	cout << "glutPostRedisplay() for keyboard function." << endl;
}

//----------------------------------------------------------------------------

void
MainSubMenuRotation(int id)
{
	switch (id) {
	case 1: glutIdleFunc(RotationFunc); break; // Start or stop camera rotation.
	case 2: glutIdleFunc(NULL); break; // Start or stop light rotation.
	}
	glutPostRedisplay();
	cout << "glutPostRedisplay() for idle rotation." << endl;
}

//----------------------------------------------------------------------------

void
MainSubMenuPerspective(int id)
{
	switch (id) {
	case 1: perspective = true; break; // Switch to persepctive projection.
	case 2: perspective = false; break; // Switch to parallel projection.
	}
	glutPostRedisplay();
	cout << "glutPostRedisplay() for projection mode changing." << endl;
}

//----------------------------------------------------------------------------

void
setMainWinMenu(void)
{
	int submenu_id_r, submenu_id_p;
	// Create submenu for rotating animation.
	submenu_id_r = glutCreateMenu(MainSubMenuRotation);
	glutAddMenuEntry("Start Camera Rotation", 1);
	glutAddMenuEntry("Stop Camera Rotation", 2);

	// Create submenu for projection changing.
	submenu_id_p = glutCreateMenu(MainSubMenuPerspective);
	glutAddMenuEntry("Perspective Projection", 1);
	glutAddMenuEntry("Parallel Projection", 2);

	glutCreateMenu(NULL); // Set menu in main window. 
	cout << "glutCreateMenu() for main window menu." << endl;
	glutAddSubMenu("Camera Rotation", submenu_id_r);
	glutAddSubMenu("Projection", submenu_id_p);
	cout << "glutAddMenuEntry() for main window menu." << endl;
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	cout << "glutAttachMenu() for main window menu." << endl;
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv); // Initializing environment.
	cout << "glutInit(&argc,argv) called." << endl;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE); // Enable depth.
	cout << "glutInitDisplayMode() called." << endl;
	glutInitWindowSize(w, h);
	cout << "glutInitWindowSize() called." << endl;
	glutInitWindowPosition(50, 50);
	cout << "glutInitWindowPosition() called." << endl;

	MainWindow = glutCreateWindow("ICG_hw10"); // Initializing & setting main window.
	cout << "glutCreateWindow() for main window." << endl;
	glewExperimental = GL_TRUE;
	glewInit();
	cout << "glewInit() for main window." << endl;
	init(); // Create buffers.
	glutDisplayFunc(display); // Setting display function for main window.
	cout << "glutDisplayFunc() for main window." << endl;
	glutKeyboardFunc(keyboard); // Setting keyboard function for main window.
	cout << "glutKeyboardFunc() for main window." << endl;
	setMainWinMenu(); // Setting menu for main window. 
	glutIdleFunc(NULL); // Start animation by default.
	cout << "glutIdleFunc() for main window." << endl;

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	cout << "glEnable( GL_DEPTH_TEST ) called." << endl;
	cout << "glutMainLoop() called." << endl;

	glutMainLoop(); // Start main loop. 
	return 0;
}


