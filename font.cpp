#include "font.h"
#include "video.h"
#include <iostream>
#include <fstream>
#include "wowmapview.h"

using namespace std;

Font::Font(unsigned int tex, int tw, int th, int size, const char* infofile) : tex(tex), size(size), tw(tw), th(th)
{
    gLog("Attempting to load font info from: %s\n", infofile);

    // Try multiple paths for the info file just like we did for TGA
    const char* paths[] = {
        "./",           // Current directory
        "fonts/",      // Fonts subdirectory
        "../fonts/",   // Up one level fonts directory
        "../Debug/",   // Debug directory
        "../Release/"  // Release directory
    };

    ifstream in;
    char filepath[512];

    for (const char* path : paths) {
        sprintf(filepath, "%s%s", path, infofile);
        gLog("Trying to open font info: %s\n", filepath);
        in.open(filepath);
        if (in.is_open()) {
            gLog("Successfully opened font info: %s\n", filepath);
            break;
        }
    }

    if (!in.is_open()) {
        gLog("Error: Could not open font info file %s\n", infofile);
        throw runtime_error("Failed to open font info file");
    }

    // Initialize the chars array
    memset(chars, 0, 256 * sizeof(charinfo));

    gLog("Reading font info...\n");

    // Read the font info file
    while (!in.eof()) {
        int line[7];
        for (int i = 0; i < 7; i++) {
            if (!(in >> line[i])) {
                if (in.eof()) break;  // Normal end of file
                gLog("Error reading font info at parameter %d\n", i);
                in.close();
                throw runtime_error("Failed to read font info data");
            }
        }

        if (in.eof()) break;

        if (line[1] != size) continue;

        if (line[0] >= 0 && line[0] < 256) {  // Validate character index
            charinfo ci;
            ci.baseline = line[2];
            ci.x = line[3];
            ci.y = line[4];
            ci.w = line[5];
            ci.h = line[6];

            ci.tx1 = ci.x / (float)tw;
            ci.tx2 = (ci.x + ci.w) / (float)tw;
            ci.ty1 = ci.y / (float)th;
            ci.ty2 = (ci.y + ci.h) / (float)th;

            chars[line[0]] = ci;
        }
    }

    // Setup space character
    charinfo space;
    space.baseline = 0;
    space.h = size;
    space.w = size / 3;
    space.x = space.y = 0;
    space.tx1 = space.tx2 = space.ty1 = space.ty2 = 0;
    chars[32] = space;

    in.close();
    gLog("Font loaded successfully\n");
}

Font::~Font()
{
}

void Font::drawchar(int x, int y, const char ch)
{
	if (ch==32) return;

	charinfo *c = &chars[ch];

    glBegin(GL_QUADS);

	glTexCoord2f(c->tx1,c->ty1);
	glVertex2f((float)x, (float)y);

	glTexCoord2f(c->tx2,c->ty1);
	glVertex2f((float)(x+c->w), (float)y);

	glTexCoord2f(c->tx2,c->ty2);
	glVertex2f((float)(x+c->w), (float)(y+c->h));

	glTexCoord2f(c->tx1,c->ty2);
	glVertex2f((float)x, (float)(y+c->h));

	glEnd();

}

// ASSUME: ortho mode
void Font::drawtext(int x, int y, const char *text)
{
	glBindTexture(GL_TEXTURE_2D, tex);

    int base = y + size;
	int xpos = x;
    const char *c = text;
	while (*c!=0) {

		if (*c != '\n' && *c != '\r') {
			drawchar(xpos, base - chars[*c].baseline, *c);
			xpos += chars[*c].w + 2;
		} else {
			base += size;
			xpos = x;
		}
		c++;
	}
}

void Font::shdrawtext(int x, int y, const char *text)
{
	GLfloat col[4];
	glGetFloatv(GL_CURRENT_COLOR, col);
	glColor4f(0,0,0,1);

	drawtext(x+2,y+2,text);

	glColor4fv(col);
	drawtext(x,y,text);
}


void Font::print(int x, int y, const char *str, ...)
{
	char buf[1024];

	va_list ap;

	va_start (ap, str);
	vsprintf (buf, str, ap);
	va_end (ap);

	drawtext(x,y,buf);
}

void Font::shprint(int x, int y, const char *str, ...)
{
	char buf[1024];

	va_list ap;

	va_start (ap, str);
	vsprintf (buf, str, ap);
	va_end (ap);

	shdrawtext(x,y,buf);
}



int Font::textwidth(const char *text)
{
	int w = 0,maxw=0;
    const char *c = text;
	while (*c!=0) {

		if (*c != '\n' && *c != '\r') {
			w += chars[*c].w +2;
		} else {
			if (w>maxw) maxw = w;
			w = 0;
		}
		c++;
	}
	if (w>maxw) maxw = w;
	return maxw;
}

