#ifdef WINDOWS

#include "sim_local.h"
#include "Texture.h"
#include "Shape.h"
#include "Coord.h"
#include "Simulator.h"
#include "Simulation.h"
#include "Wire.h"
#include "Junction.h"
#include "Text.h"
#include "Line.h"
#include "Rect.h"
#include "Circle.h"
#include "Tool_Base.h"
#include "Tool_Wire.h"
#include "Tool_Use.h"
#include "Tool_Move.h"
#include "Tool_Delete.h"
#include "CursorManager.h"
#include "Controller_Button.h"

#pragma comment (lib, "SDL2.lib")
#pragma comment (lib, "Opengl32.lib")
#pragma comment (lib, "freeglut.lib")

int WinWidth = 800;
int WinHeight = 600;
#undef main

CStyle g_style_shapes(CColor(165, 75, 75), 3.0f);
CStyle g_style_wires(CColor(75, 165, 75), 3.0f);
CStyle g_style_text(CColor(131, 131, 131), 3.0f);
CStyle g_style_text_red(CColor(255, 131, 131), 3.0f);

int drawTextInternal(float x, float y, const char *buffer) {
	glRasterPos2f(x, y);
	const char *p = buffer;
	while (*p) {
		if (*p == '\n') {
			y += 15;
			glRasterPos2f(x, y);
		}
		else {
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)*p);
		}
		p++;
	}
	y += 15;
	return y;
}

int drawText(class CStyle *style, int x, int y, const char* fmt, ...) {
	va_list argList;
	char buffer2[4096];
	char buffer[4096];
	va_start(argList, fmt);
	vsnprintf(buffer2, sizeof(buffer2), fmt, argList);
	va_end(argList);
	CMD_ExpandConstantsWithinString(buffer2, buffer, sizeof(buffer));
	if (style == 0) {
		g_style_text.apply();
	}
	else {
		style->apply();
	}
	float ret = drawTextInternal(x, y, buffer);
	drawTextInternal(x + 0.5f, y + 0.5f, buffer);
	drawTextInternal(x - 0.5f, y - 0.5f, buffer);
	return ret;
}
int gridSize = 20;
float roundToGrid(float f) {
	float g = f / gridSize;
	g += 0.5;
	g = floor(g);
	return g * gridSize;
}
Coord roundToGrid(Coord c) {
	return Coord(roundToGrid(c.getX()), roundToGrid(c.getY()));
}

Coord GetMousePos() {
	Coord r;
	int mx, my;
	//SDL_GetGlobalMouseState(&mx, &my);
	SDL_GetMouseState(&mx, &my);
	// No longer needed after resize event was introduced
	// BUGFIX FOR MENUBAR OFFSET
	//my += WINDOWS_MOUSE_MENUBAR_OFFSET;
	r.set(mx, my);
	return r;
}



float CEdge::drawInformation2D(float x, float h) {
	h = CShape::drawInformation2D(x, h);
	h = a->drawInformation2D(x + 20, h);
	h = b->drawInformation2D(x + 20, h);
	return h;
}
void CEdge::translate(const Coord &o) {
	a->translate(o);
	b->translate(o);
}
const Coord &CEdge::getPositionA() const {
	return a->getPosition();
}
const Coord &CEdge::getPositionB() const {
	return b->getPosition();
}

const char *find_first_of_s(const char *s, const char *ss) {
	while (*s) {
		const char *p = ss;
		while (*p) {
			if (*p == *s)
				return s;
			p++;
		}
		s++;
	}
	return 0;
}

// Given a file path, create all constituent directories if missing
void FS_CreateDirectoriesForPath(const char *file_path) {
	char *dir_path = (char *)malloc(strlen(file_path) + 1);
	const char *next_sep = find_first_of_s(file_path, "/\\");
	while (next_sep != NULL) {
		int dir_path_len = next_sep - file_path;
		memcpy(dir_path, file_path, dir_path_len);
		dir_path[dir_path_len] = '\0';
#if WINDOWS
		CreateDirectory(dir_path, 0);
#else
		mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH);
#endif
		next_sep = find_first_of_s(next_sep + 1, "/\\");
	}
	free(dir_path);
}
bool FS_Exists(const char *fname) {
	if (fname == 0)
		return false;

	FILE *f = fopen(fname, "rb");
	if (f == 0)
		return false;
	fclose(f);
	return true;
}
char *FS_ReadTextFile(const char *fname) {
	if (fname == 0)
		return 0;
	FILE *f = fopen(fname, "r");
	if (f == 0)
		return 0;
	fseek(f, 0, SEEK_END);
	int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *r = (char*)malloc(len + 1);
	fread(r, 1, len, f);
	r[len] = 0;
	fclose(f);
	return r;
}
bool FS_WriteTextFile(const char *data, const char *fname) {
	FILE *f = fopen(fname, "w");
	if (f == 0)
		return true;
	fprintf(f, data);
	fclose(f);
	return false;
}
CSimulator *sim;
extern "C" int SIM_CreateWindow(int argc, char **argv)
{
	glutInit(&argc, argv);
	sim = new CSimulator();
	sim->createWindow();
	sim->loadRecentProject();
	return 0;
}
extern "C" void SIM_RunWindow() {
	sim->drawWindow();
}
#endif
