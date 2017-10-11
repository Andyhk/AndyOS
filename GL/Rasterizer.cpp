#include "Rasterizer.h"
#include "System.h"
#include "GL.h"
#include "math.h"

namespace gl
{
	float EdgeFunction(const Vector4& a, const Vector4& b, const Vector4& c)
	{
		return (c.y - a.y) * (b.x - a.x) - (c.x - a.x) * (b.y - a.y);
	}

	float EdgeFunction(const Vector4& a, const Vector4& b, float cx, float cy)
	{
		return (cy - a.y) * (b.x - a.x) - (cx - a.x) * (b.y - a.y);
	}

	float* Rasterizer::depth_buffer;

	STATUS Rasterizer::Init()
	{
		uint32 pixels = GL::m_height * GL::m_width;
		uint32 blocks = pixels * sizeof(uint32) / BLOCK_SIZE;

		depth_buffer = (float*)Memory::AllocBlocks(blocks);
		return STATUS_SUCCESS;
	}

	void Rasterizer::DrawTriangle(Vertex& v0, Vertex& v1, Vertex& v2, BMP* texture)
	{
		//Calculate values
		v0.tmpPos.w = 1 / v0.tmpPos.w;
		v1.tmpPos.w = 1 / v1.tmpPos.w;
		v2.tmpPos.w = 1 / v2.tmpPos.w;

		v0.tmpPos.x = v0.tmpPos.x * GL::m_width * v0.tmpPos.w + GL::m_width * 0.5;
		v1.tmpPos.x = v1.tmpPos.x * GL::m_width * v1.tmpPos.w + GL::m_width * 0.5;
		v2.tmpPos.x = v2.tmpPos.x * GL::m_width * v2.tmpPos.w + GL::m_width * 0.5;

		v0.tmpPos.y = v0.tmpPos.y * GL::m_height * v0.tmpPos.w + GL::m_height * 0.5;
		v1.tmpPos.y = v1.tmpPos.y * GL::m_height * v1.tmpPos.w + GL::m_height * 0.5;
		v2.tmpPos.y = v2.tmpPos.y * GL::m_height * v2.tmpPos.w + GL::m_height * 0.5;

		//Backface culling
		Vector4 pa(v0.tmpPos.x, v0.tmpPos.y, v0.tmpPos.z, 0);
		Vector4 pb(v1.tmpPos.x, v1.tmpPos.y, v1.tmpPos.z, 0);
		Vector4 pc(v2.tmpPos.x, v2.tmpPos.y, v2.tmpPos.z, 0);
		Vector4 normal = Vector4::Cross(pb - pa, pc - pa);

		if (normal.z < 0)
			return;

		DrawTriangle2(v0, v1, v2, texture);

		VBE::DrawLine((int)v0.tmpPos.x, (int)v0.tmpPos.y, (int)v1.tmpPos.x, (int)v1.tmpPos.y, 0xFF);
		VBE::DrawLine((int)v1.tmpPos.x, (int)v1.tmpPos.y, (int)v2.tmpPos.x, (int)v2.tmpPos.y, 0xFF00);
		VBE::DrawLine((int)v2.tmpPos.x, (int)v2.tmpPos.y, (int)v0.tmpPos.x, (int)v0.tmpPos.y, 0xFF0000);
	}

	void Rasterizer::DrawTriangle2(Vertex& v0, Vertex& v1, Vertex& v2, BMP* texture)
	{
		//Clamping
		int minx = min(floor(v0.tmpPos.x), floor(v1.tmpPos.x), floor(v2.tmpPos.x));
		int maxx = max(floor(v0.tmpPos.x), floor(v1.tmpPos.x), floor(v2.tmpPos.x));
		int miny = min(floor(v0.tmpPos.y), floor(v1.tmpPos.y), floor(v2.tmpPos.y));
		int maxy = max(floor(v0.tmpPos.y), floor(v1.tmpPos.y), floor(v2.tmpPos.y));

		minx = clamp(minx, 0, (int)GL::m_width - 1);
		maxx = clamp(maxx, 0, (int)GL::m_width - 1);
		miny = clamp(miny, 0, (int)GL::m_height - 1);
		maxy = clamp(maxy, 0, (int)GL::m_height - 1);

		float area = EdgeFunction(v0.tmpPos, v1.tmpPos, v2.tmpPos);
		float inv_area = 1 / area;

		int line_delta = GL::m_width - (maxx - minx) - 1;

		uint32* color_ptr = (uint32*)(VBE::buffer + miny * GL::m_width + minx);
		float* depth_ptr = (float*)(depth_buffer + miny * GL::m_width + minx);

		float co[3][3] = {
			{ v0.builtColor.r * v0.tmpPos.w, v1.builtColor.r * v1.tmpPos.w , v2.builtColor.r * v2.tmpPos.w },
			{ v0.builtColor.g * v0.tmpPos.w, v1.builtColor.g * v1.tmpPos.w , v2.builtColor.g * v2.tmpPos.w },
			{ v0.builtColor.b * v0.tmpPos.w, v1.builtColor.b * v1.tmpPos.w , v2.builtColor.b * v2.tmpPos.w }
		};

		float A01 = (v0.tmpPos.y - v1.tmpPos.y) * inv_area;
		float B01 = (v1.tmpPos.x - v0.tmpPos.x) * inv_area;
		float A12 = (v1.tmpPos.y - v2.tmpPos.y) * inv_area;
		float B12 = (v2.tmpPos.x - v1.tmpPos.x) * inv_area;
		float A20 = (v2.tmpPos.y - v0.tmpPos.y) * inv_area;
		float B20 = (v0.tmpPos.x - v2.tmpPos.x) * inv_area;

		float w0_row = EdgeFunction(v1.tmpPos, v2.tmpPos, minx, miny) * inv_area;
		float w1_row = EdgeFunction(v2.tmpPos, v0.tmpPos, minx, miny) * inv_area;
		float w2_row = EdgeFunction(v0.tmpPos, v1.tmpPos, minx, miny) * inv_area;

		//Drawing loop
		for (int y = miny; y <= maxy; y++)
		{
			float w0 = w0_row;
			float w1 = w1_row;
			float w2 = w2_row;

			for (int x = minx; x <= maxx; x++)
			{
				if (w0 >= 0 && w1 >= 0 && w2 >= 0)
				{
					float Z = 1 / (v0.tmpPos.w * w0 + v1.tmpPos.w * w1 + v2.tmpPos.w * w2);

					if (Z < *depth_ptr)
					{
						float U = ((w0 * v0.tex_u * v0.tmpPos.w) + (w1 * v1.tex_u * v1.tmpPos.w) + (w2 * v2.tex_u * v2.tmpPos.w)) * Z;
						float V = ((w0 * v0.tex_v * v0.tmpPos.w) + (w1 * v1.tex_v * v1.tmpPos.w) + (w2 * v2.tex_v * v2.tmpPos.w)) * Z;

						float r = (co[0][0] * w0 + co[0][1] * w1 + co[0][2] * w2);
						float g = (co[1][0] * w0 + co[1][1] * w1 + co[1][2] * w2);
						float b = (co[2][0] * w0 + co[2][1] * w1 + co[2][2] * w2);

						r *= Z;
						g *= Z;
						b *= Z;

						float lum = 0.21 * r + 0.72 * g + 0.07 * b;

						int X = texture->width * U;
						int Y = texture->height * V;

						uint32 color = texture->pixels[(int)(Y * texture->width + X)];
						r = lum * (uint8)(color >> 16);
						g = lum * (uint8)(color >> 8);
						b = lum * (uint8)(color >> 0);

						int col = ((int)(r) << 16) | ((int)(g) << 8) | (int)(b);

						*color_ptr = col;
						*depth_ptr = Z;
					}
				}

				w0 += A12;
				w1 += A20;
				w2 += A01;

				color_ptr++;
				depth_ptr++;
			}

			w0_row += B12;
			w1_row += B20;
			w2_row += B01;

			color_ptr += line_delta;
			depth_ptr += line_delta;
		}
	}
}