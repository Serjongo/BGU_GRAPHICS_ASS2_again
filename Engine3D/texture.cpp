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


//sphere intersection func
//this return the color of the sphere, assuming we hit it, if not, we return 0.
//NOTE: the return process is somewhat convoluted, because when we start checking for both intersections with transparent objects, I might have to restructure the return type, for now it stays like this.
uint32_t sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 sphere_center, float sphere_radius, uint32_t sphere_color, uint32_t original_color) //if -1, no hits
{
    //line vector: x = ray_origin.x + (ray_direction.x * t) , y = ray_origin.y + (ray_direction.y * t) , z = ray_origin.z + (ray_direction.z * t)
    //( x - a )^2 + (y - b)^2 + (z - c)^2 - r^2 = 0
    // (x^2 - 2xa + a^2) + (y^2 - 2yb + b^2) + (z^2 - 2zc + c^2) - r^2 = 0
    //  ( (x_0 + d_x * t)^2 - 2(x_0 + d_x * t)*a + (a^2) ) + ( (y_0 + d_y * t)^2 - 2(y_0 + d_y * t)*b + b^2 ) + ( (z_0 + d_z * t)^2 - 2(z_0 + d_z * t) * c + c^2 ) - r^2 = 0
    //  ... grouping by t (which will appear as sol_1,sol_2 upon solving in func)
    // A + B*t + C*t^2 = 0, 
    float a = glm::dot(ray_direction, ray_direction); //(dirx^2+diry^2+dirz^2)
    float b = 2 * ray_origin.x * ray_direction.x - 2 * ray_direction.x * sphere_center.x + 2 * ray_origin.y * ray_direction.y - 2 * ray_direction.y * sphere_center.y + 2 * ray_origin.z * ray_direction.z - 2 * sphere_center.z; //(2*p0x*dirx-2*dirx*cx+2*p0y*diry-2*diry*cy+2*p0z*dirz-2*dirz*cz)
    float c = glm::dot(ray_origin, ray_origin) + glm::dot(sphere_center, sphere_center) - pow(sphere_radius,2); //p0x^2+p0y^2+p0z^2+cx^2+cy^2+cz^2-R^2

    //Quatratic formula discriminant:
    // b^2 - 4ac

    float discriminant = b * b - 4.0f * a * c; //calculating ray intersection with sphere
    if (discriminant > 0.0f)
    {
        //so if this happens, I have 2 intersections to return, but temporarily I will ignore the 2nd one, as we're not talking transparent objects yet. It will likely come back though, for now I return the smaller t
        float sol_1 = (- b + sqrt(discriminant))/2*a;
        float sol_2 = (- b - sqrt(discriminant))/2*a;
       
        //return the smaller one, as long as it is positive. otherwise, return -1, which means no intersection
        if ((sol_1 <= sol_2)) 
        { 
            if ((sol_1 >= 0)) 
            {
                //return sol_1;
                return sphere_color;
            }
            else if (sol_2 >= 0)
            {
                //return sol_2;
                return sphere_color;
            }
            else 
            {
                return original_color;//no hits
            } 
        }
        else if ((sol_2 <= sol_1))
        {
            if ((sol_2 >= 0))
            {
                //return sol_2;
                return sphere_color;
            }
            else if (sol_1 >= 0)
            {
                //return sol_1;
                return sphere_color;
            }
            else
            {
                return original_color;//no hits
            }
        }
        //return 0xffff00ff; //returning the spheres colour, temporarily hardcoded
    }
    else if (discriminant == 0.0f) 
    {
        float sol_1 = -b / 2 * a;
        return sol_1 >= 0 ? sphere_color : 0;

    }
    else //no intersection
    {
        return original_color;
    }
    
    // 
    
}

uint32_t plane_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec4 plane_coord, uint32_t plane_color, uint32_t original_color)//plane coords (a,b,c,d), TO ADD: the checkers pattern on the plane
{
    //-----------MATH CALCULATIONS
    //ax+by+cz+d = 0
    //(ray_origin.x + t * ray_direction.x) * plane.coord.x + (ray_origin.y + t * ray_direction.y) * plane.coord.y + (ray_origin.z + t * ray_direction.z) * plane.coord.z + plane_coord.d = 0
    //(ray_direction.x * plane_coord.x + ray_direction.y * plane_coord.y + ray_direction.z * plane_coord.z ) * t = - (ray_origin.x * plane_coord.x + ray_origin.y * plane_coord.y + ray_origin.z * plane_coord.z + plane_coord.d)
    //Now we isolate t : t = - (ray_origin.x * plane_coord.x + ray_origin.y * plane_coord.y + ray_origin.z * plane_coord.z + plane_coord.d) / (ray_direction.x * plane_coord.x + ray_direction.y * plane_coord.y + ray_direction.z * plane_coord.z )
    //--------------------------------
    float denominator = (ray_direction.x * plane_coord.x + ray_direction.y * plane_coord.y + ray_direction.z * plane_coord.z);
    float nominator = -(ray_origin.x * plane_coord.x + ray_origin.y * plane_coord.y + ray_origin.z * plane_coord.z + plane_coord.w); //w is the 4th coord
    if (denominator == 0) //either we have infinite results, or none at all 
    {
        if (nominator == 0) 
        {
            //infinite results - we currently don't do anything with it, but we might later down the line, so for now we return same result as a single intersection
            return plane_color;
        }
        else 
        {
            //none at all
            return original_color;
        }
    }
    else // a single intersection
    {
        float reuslt = nominator / denominator; //we currently do nothing with it, but perhaps we might later down the line
        return plane_color;
    }


    
    
    return original_color;
}


uint32_t PerPixel(glm::vec2 coord) //essentially cherno's raytracing vid #3 as linked in the assignment
{ 
    //TO_ADD INPUTS:
    //glm::vec3 eye_position

    //uint8_t r = (uint8_t)(coord.x * 255.0f);
    //uint8_t g = (uint8_t)(coord.y * 255.0f);

    // a+bt - ray formula. a = ray origin, b = ray direction , t = hit distance

    //------------TEMPORARY VALUES, WILL LIKELY CHANGE------------------
    glm::vec3 rayOrigin(0.0f,0.0f,2.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    glm::vec3 rayDirection(coord.x, coord.y, -1.0f); // currently Z is hardcoded, will change
    float radius = 0.5f;
    uint32_t pixel_value = 0; // this will change as it goes through more processes
    pixel_value = plane_intersection(rayOrigin, rayDirection, glm::vec4(0, 0, 2, -100), 0xffaaaaff, pixel_value);
    pixel_value = sphere_intersection(rayOrigin, rayDirection, glm::vec3(6.4, 6.6, 6.4), 0.3, 0xff00a0ff, pixel_value);
    pixel_value = sphere_intersection(rayOrigin, rayDirection, glm::vec3(0.2,0.4,0.4), radius, 0xffaa00ff, pixel_value);
    
    

    return pixel_value;
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

