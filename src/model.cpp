#include <assimp/Importer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "model.h"

Color Texture::read(float u, float v) 
{
// 	int mode = 0;
// 	if (mode == 0)
// 	{
		int idx = channels * ((int)round(v * height) * width + (int)round(u * width));
		return Color(data[idx], data[idx+1], data[idx+2]);
//	}

// 	Color* t;
// 	int w, h;
// 	int width;	//w+1
// 	t = texture->pixels;
// 	w = texture->u_max;
// 	h = texture->v_max;
// 	width = texture->width;
// 
// 	Color c00, c01, c10, c11;
// 	float r, g, b;
// 
// 	u = u * w;
// 	v = v * h;
// 
// 	int iu = min((int)u, w), iv = min((int)v, h);
// 
// 	if (iv == h && iu == w) return t[iv * width + iu];
// 
// 	u = u - iu;
// 	v = v - iv;
// 
// 	if (iv == h) {
// 		c00 = t[iv * width + iu];
// 		c01 = t[iv * width + iu + 1];
// 		r = (1 - u) * (BYTE)(c00 >> 16) + u * (BYTE)(c01 >> 16) + 0.5f;
// 		g = (1 - u) * (BYTE)(c00 >> 8) + u * (BYTE)(c01 >> 8) + 0.5f;
// 		b = (1 - u) * (BYTE)(c00)+u * (BYTE)(c01)+0.5f;
// 		return ((int)r << 16) | ((int)g << 8) | (int)b;
// 	}
// 
// 	if (iu == w) {
// 		c00 = t[iv * width + iu];
// 		c10 = t[(iv + 1) * width + iu];
// 		r = (1 - v) * (BYTE)(c00 >> 16) + v * (BYTE)(c10 >> 16) + 0.5f;
// 		g = (1 - v) * (BYTE)(c00 >> 8) + v * (BYTE)(c10 >> 8) + 0.5f;
// 		b = (1 - v) * (BYTE)(c00)+v * (BYTE)(c10)+0.5f;
// 		return ((int)r << 16) | ((int)g << 8) | (int)b;
// 	}
// 
// 	c00 = t[iv * width + iu];
// 	c01 = t[iv * width + iu + 1];
// 	c10 = t[(iv + 1) * width + iu];
// 	c11 = t[(iv + 1) * width + iu + 1];
// 
// 	r = (1 - u) * (1 - v) * (BYTE)(c00 >> 16) + u * (1 - v) * (BYTE)(c01 >> 16) + v * (1 - u) * (BYTE)(c10 >> 16) + u * v * (BYTE)(c11 >> 16) + 0.5f;
// 	g = (1 - u) * (1 - v) * (BYTE)(c00 >> 8) + u * (1 - v) * (BYTE)(c01 >> 8) + v * (1 - u) * (BYTE)(c10 >> 8) + u * v * (BYTE)(c11 >> 8) + 0.5f;
// 	b = (1 - u) * (1 - v) * (BYTE)(c00)+u * (1 - v) * (BYTE)(c01)+v * (1 - u) * (BYTE)(c10)+u * v * (BYTE)(c11)+0.5f;
// 
// 	return ((int)r << 16) | ((int)g << 8) | (int)b;
}


Model::Model()
{

}

Model::Model(std::string& name)
{
	sName = name;
}


Model::~Model()
{
	if (albedo)
	{
		stbi_image_free(albedo->data);
		delete albedo;
		albedo = NULL;
	}
}

Color Model::readAlbedo(float u, float v)
{
	if (albedo)
		return albedo->read(u, v);
	else
		return 0x00ff00ff;
}

void Model::loadTexture(std::string path)
{
	assert(albedo == NULL);
	int width, height, nComponents;
	stbi_set_flip_vertically_on_load(1);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nComponents, 0);
	if (!data)
	{
		WARN("error occur when loading texture %s.", path.c_str());
		return;
	}

	albedo = new Texture();

	auto filename_start = path.find_last_of('/');
	albedo->sName = path.substr(filename_start + 1, path.find_last_of('.') - filename_start - 1);
	albedo->channels = nComponents;
	albedo->data = data;
	albedo->width = width;
	albedo->height = height;

	//stbi_image_free(data);
}