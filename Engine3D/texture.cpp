//#define GLEW_STATIC
// #define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "stb_image.h"
#include "../res/includes/glad/include/glad/glad.h"
#include <iostream>
#include "../res/includes/glm/glm.hpp"

//wrote this one, it simply clamps our values between 0 - 255
//unsigned char clamp(unsigned char val)
//{
//    if (val < 0) return 0;
//    else if (val > 255) return 255;
//    else return val;
//}


uint32_t PerPixel(glm::vec2 coord) //essentially cherno's raytracing vid #3 as linked in the assignment
{ 
    //uint8_t r = (uint8_t)(coord.x * 255.0f);
    //uint8_t g = (uint8_t)(coord.y * 255.0f);

    // a+bt - ray formula. a = ray origin, b = ray direction , t = hit distance

    //------------TEMPORARY VALUES, WILL LIKELY CHANGE------------------
    glm::vec3 rayOrigin(0.0f,0.0f,+2.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    glm::vec3 rayDirection(coord.x, coord.y, -1.0f); // currently Z is hardcoded, will change
    float radius = 0.5f;
    //rayDirection = glm::normalize(rayDirection);
    //------------------------------------------------------------------
    

    float a = glm::dot(rayDirection,rayDirection); //(bx^2 + by^2 + bz^2)
    float b = 2.0f * glm::dot(rayOrigin, rayDirection); //2(a.x * b.x + a.y * b.y + a.z * b.z)
    float c = glm::dot(rayOrigin, rayOrigin) - radius * radius; //(a.x^2 + a.y^2 + a.z^2 - r^2)

    //Quatratic formula discriminant:
    // b^2 - 4ac

    float discriminant = b * b - 4.0f * a * c; //calculating ray intersection with sphere
    if (discriminant >= 0.0f) 
    {
        return 0xffff00ff; //returning the spheres colour, temporarily hardcoded
    }
    return 0xff000000 | 0x00a00000 ;
}

Texture::Texture(int width, int height) //added by me
{
    uint32_t* image_data = new uint32_t[width * height]; //right-to-left -> 2bytes(R) -> 2bytes(G) -> 2bytes(B) -> 2bytes(alpha) -> 0x meaning we write in hexadecimal
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            glm::vec2 coord = { (float)x / (float)width , (float)y / (float)height };
            coord = coord * 2.0f - 1.0f; //normalizing the coordinates from -1 to 1
            image_data[x + y * width] = PerPixel(coord);  
        }
    }

    glGenTextures(1, &m_texture);
    Bind(m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.4f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
}

Texture::Texture(const std::string& fileName)
{
	int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);
	
    if(data == NULL)
		std::cerr << "Unable to load texture: " << fileName << std::endl;
        
    glGenTextures(1, &m_texture);
    Bind(m_texture);
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_LOD_BIAS,-0.4f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}

Texture::Texture(int width,int height,unsigned char *data)
{
    glGenTextures(1, &m_texture);
    Bind(m_texture);
        
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

