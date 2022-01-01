#include <assimp/Importer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "model.h"

Model::Model()
{

}

Model::Model(std::string& name)
{
	sName = name;
}
