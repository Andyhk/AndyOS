#pragma once
#include "sys/types.h"
#include "math.h"
#include "font.h"
#include "color.h"
#include "bmp.h"

struct Point
{
	int x;
	int y;

	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};

struct Rect
{
	int x;
	int y;
	int width;
	int height;

	Rect()
	{
		this->x = 0;
		this->y = 0;
		this->width = 0;
		this->height = 0;
	}

	Rect(int x, int y, int w, int h)
	{
		this->x = x;
		this->y = y;
		this->width = w;
		this->height = h;
	}

	bool Contains(int x, int y)
	{
		return ((x >= this->x)
			&& x <= (this->x + this->width)
			&& (y >= this->y)
			&& (y <= this->y + this->height));
	}

	int Right()
	{
		return this->x + this->width;
	}

	int Top()
	{
		return this->y + this->height;
	}
};

struct GC
{
	int x;
	int y;
	int width;
	int height;

	int stride;
	uint32* framebuffer;

	GC();
	GC(int width, int height, uint32* framebuffer);
	GC(int width, int height);
	GC(GC& gc, int x, int y, int width, int height);
	GC(GC& gc, Rect bounds);

	void Resize(int width, int height);

	inline int memsize()
	{
		return width * height;
	}

	inline int pixels()
	{
		return width * height;
	}

	void Draw();
	void Clear(Color& col);
	void CopyTo(int x0, int y0, int w0, int h0, GC& dst, int x1, int y1, bool alpha = false);
	void SetPixel(int x, int y, Color& col);
	Color GetPixel(int x, int y);

	void DrawLine(int x0, int y0, int x1, int y1, Color& col);
	void DrawBezierQuad(Point* points, int count, Color& col);
	void DrawBezierCube(Point* points, int count, Color& col);

	void DrawRect(Rect& bounds, int width, Color& col);
	void DrawRect(int x, int y, int w, int h, int width, Color& col);
	void FillRect(Rect& bounds, Color& col);
	void FillRect(int x, int y, int w, int h, Color& col);

	void DrawImage(Rect& bounds, BMP* bmp);
	void DrawImage(int x, int y, int w, int h, BMP* bmp);

	void DrawText(int x, int y, char* c, Color& fg);
	void DrawText(int x, int y, char* c, Color& fg, Color& bg);

private:
	inline uint32 BlendAlpha(uint32 src, uint32 dst);
};

namespace Drawing
{
	void Init();
}