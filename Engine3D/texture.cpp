//#define GLEW_STATIC
// #define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "texture.h"
#include "stb_image.h"
#include "../res/includes/glad/include/glad/glad.h"
#include <iostream>
#include "../res/includes/glm/glm.hpp"
#include <fstream> //added by me
#include <vector> //added by me
#include <typeinfo>//added by me

//wrote this one, it simply clamps our values between 0 - 255
//unsigned char clamp(unsigned char val)
//{
//    if (val < 0) return 0;
//    else if (val > 255) return 255;
//    else return val;
//}

glm::vec4 set_coords(const std::vector<std::string>& temp_coords) {
    glm::vec4 float_coords;

    for (size_t i = 0; i < temp_coords.size() && i < 4; ++i) {
        float val = std::atof(temp_coords[i].c_str()); // Convert string to float
        float_coords[i] = val; // Set the float value into the vec4 components
    }

    return float_coords;
}


std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}


float zero_approximation = -0.000001f;

//for input from files

//custom clamp function to not go above 255
float clamp(float input)
{
    return input <= 255 ? input : 255;
}


uint32_t convert_vec4_rgba_to_uint32_t(glm::vec4 color, int input_as_decimal) //this returns a proper rgba pixel
{

    if (input_as_decimal == 1) // meaning we get input as 0-1.0
    {
        unsigned char r = clamp(color.x * 255);
        unsigned char g = clamp(color.y * 255);
        unsigned char b = clamp(color.z * 255);
        unsigned char alpha = 255;
        uint32_t final_result = r | g << 8 | b << 16 | alpha << 24;
        return final_result;
    }
    else // DECLARE IT 0. meaning meaning we get input as actual color values 0-255
    {
        unsigned char r = color.x;
        unsigned char g = color.y;
        unsigned char b = color.z;
        unsigned char alpha = color.w;
        uint32_t final_result = r | g << 8 | b << 16 | alpha << 24;
        return final_result;
    }
}

glm::vec4 convert_uint32_t_to_vec4_rgba(uint32_t color, int output_as_decimal) // 1 means return decimal percentage of 255 (0.0-1.0), everything else means 0-255
{
    uint32_t temp_color = color;
    unsigned char r = (unsigned char)(color & 0x000000ff);
    unsigned char g = (unsigned char)((color & 0x0000ff00) >> 8);
    unsigned char b = (unsigned char)((color & 0x00ff0000) >> 16);
    unsigned char alpha = (unsigned char)((color & 0xff000000) >> 24);
    if (output_as_decimal == 1) {

        glm::vec4 final_result = glm::vec4((float)(r) / 255, (float)(g) / 255, (float)(b) / 255, (float)(alpha) / 255);
        return final_result; //this will give us an example vector of (0.3,0.6,0.8,1.0)
    }
    else
    {
        glm::vec4 final_result = glm::vec4((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)alpha);
        return final_result; //this will give us an example vector of (64,146,167,255)
    }
}









//glm::vec4 multiply_vector_by_vec_scalars(glm::vec4 input, glm::vec4 scalars) //input vec4 0-255 vals, scalars vec4 0.0-1.0 vals
//{
//    return glm::vec4(input.r * (float)(scalars.r), input.g * (float)(scalars.g), input.b * (float)(scalars.b), input.a * (float)(scalars.a));
//}


glm::vec4 multiply_vector_by_vec_scalars(glm::vec4 input, glm::vec4 scalars) //input vec4 0-255 vals, scalars vec4 0.0-1.0 vals
{
    return glm::vec4(clamp((float)input.r * (float)(scalars.r)), clamp((float)input.g * (float)(scalars.g)), clamp(input.b * (float)(scalars.b)), 255);
}



glm::vec3 manual_refraction(glm::vec3 original_direction_vector, glm::vec3 normal, float density)
{
    float cos_angle = glm::dot(glm::normalize(original_direction_vector), glm::normalize(normal));
    float angle = acos(cos_angle);
    float resulting_sin_angle = sin(angle) / density;
    float resulting_angle = asin(resulting_sin_angle);

    glm::vec3 result = sin(resulting_angle) * (normal * cos_angle - original_direction_vector) - cos(resulting_angle) * normal; // based on snell formula in https://stackoverflow.com/questions/20801561/glsl-refract-function-explanation-available
    return result;
}

//for multiplying our rgba value by a decimal without fudging up our channels
//uint32_t rgba_uint32_t_scalar_multiplication(uint32_t original_color, float decimal_percentage_scalar) //decimal percentage must be between 0-1, I'm not checking for correctness because this is for internal use only
//{
//    return convert_vec4_rgba_to_uint32_t(multiply_vector_by_vec_scalars(convert_uint32_t_to_vec4_rgba(original_color, 0), glm::vec4(decimal_percentage_scalar)), 0);
//}


//algebraic functions for vectors

float distance(glm::vec3 origin, glm::vec3 destination)
{
    return sqrt(pow((destination.x - origin.x), 2) + pow((destination.y - origin.y), 2) + pow((destination.z - origin.z), 2));
}

glm::vec3 calculate_hitpoint_from_distance(glm::vec3 origin, glm::vec3 direction, float dist)
{
    float t_squared = pow(dist, 2) / glm::dot(direction, direction);
    //we want to return the point with the positive of t, not the negative option
    float t_res = sqrt(t_squared);
    glm::vec3 vec_result = glm::vec3(origin.x + direction.x * t_res, origin.y + direction.y * t_res, origin.z + direction.z * t_res);
    return vec_result;

}

glm::vec3 calculate_vector_direction(glm::vec3 origin, glm::vec3 destination)
{
    return destination - origin;
}






// my yee-yee ass shapes classes, it currently stays like this because I don't want to break it off into seperate files
/*class Shape_custom
{
public:
    virtual float intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, uint32_t original_color) const = 0;
    virtual uint32_t getColor(glm::vec3 hitPoint) const = 0;
    virtual glm::vec3 calculate_normal_direction(glm::vec3 hitpoint) const = 0;
    virtual int get_reflecting_status() const = 0;
    virtual int get_transparency_status() const = 0;
    virtual float get_shininess_value() const = 0;
    virtual void setColor(uint32_t color) = 0;
};*/

class Shape_custom
{
protected:
    int transparent;
    uint32_t color;
    float shininess;
public:
    virtual float intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, uint32_t original_color) const = 0;
    virtual uint32_t getColor(glm::vec3 hitPoint) const = 0;
    virtual glm::vec3 calculate_normal_direction(glm::vec3 hitpoint) const = 0;
    virtual int get_reflecting_status() const = 0;
    virtual int get_transparency_status() const = 0;
    virtual float get_shininess_value() const = 0;
    //virtual void setColor(uint32_t color) = 0;
    void setColor(uint32_t color) {
        this->color = color;
    }
    void setShininess(float shininess) {
        this->shininess = shininess;
    }
};

/*class Sphere : public Shape_custom
{
private:
    float radius;
    glm::vec3 center;
    uint32_t color;
    float shininess;
    int reflects_light;
    int transparent;
public:
    //setters
    void setColor(uint32_t color)
    {
        this->color = color;
    }
    //getters
    float getRadius()
    {
        return this->radius;
    }
    glm::vec3 getCenter()
    {
        return this->center;
    }
    uint32_t getColor(glm::vec3 hitPoint) const
    {
        return this->color;
    }
    float get_shininess_value() const override
    {
        return this->shininess;
    }
    int get_reflecting_status() const override
    {
        return this->reflects_light;
    }
    int get_transparency_status() const override
    {
        return this->transparent;
    }

    //constructors
    Sphere(float radius, glm::vec3 center, uint32_t color, int reflects_light, float shininess,int transparent)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->shininess = shininess;
        this->transparent = transparent;
    }

    Sphere(float radius, glm::vec3 center, uint32_t color, int reflects_light, float shininess)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->shininess = shininess;
        this->transparent = 0;
    }
    Sphere(float radius, glm::vec3 center, uint32_t color,int reflects_light)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Sphere(float radius, glm::vec3 center, uint32_t color)
    {
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Sphere(float radius, glm::vec3 center) //we use this if we don't know the current color, and reflecting status (meaning its not)
    {
        this->radius = radius;
        this->center = center;
        this->color = 0xff000000; //fully opaque, and black.
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    */
class Sphere : public Shape_custom
{
private:
    float radius;
    glm::vec3 center;
    //uint32_t color;
    // float shininess;
    int reflects_light;
    //int transparent;
public:
    //setters
  /*  void setColor(uint32_t color)
    {
        this->color = color;
    }*/
    void setTransparent(int transparent)
    {
        this->transparent = transparent;
    }

    /* void setShininess(int shininess)
     {
         this->shininess = shininess;
     }*/
     //getters
    float getRadius()
    {
        return this->radius;
    }
    glm::vec3 getCenter()
    {
        return this->center;
    }
    uint32_t getColor(glm::vec3 hitPoint) const
    {
        return this->color;
    }
    float get_shininess_value() const override
    {
        return this->shininess;
    }
    int get_reflecting_status() const override
    {
        return this->reflects_light;
    }
    int get_transparency_status() const override
    {
        return this->transparent;
    }

    //constructors
    Sphere(float radius, glm::vec3 center, uint32_t color, int reflects_light, float shininess, int transparent)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->shininess = shininess;
        this->transparent = transparent;
    }

    Sphere(float radius, glm::vec3 center, uint32_t color, int reflects_light, float shininess)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->shininess = shininess;
        this->transparent = 0;
    }
    Sphere(float radius, glm::vec3 center, uint32_t color, int reflects_light)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Sphere(float radius, glm::vec3 center, uint32_t color)
    {
        this->radius = radius;
        this->center = center;
        this->color = color;
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Sphere(float radius, glm::vec3 center) //we use this if we don't know the current color, and reflecting status (meaning its not)
    {
        this->radius = radius;
        this->center = center;
        this->color = 0xff000000; //fully opaque, and black.
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }

    Sphere() //we use this if we don't know the current color, and reflecting status (meaning its not)
    {
        this->radius = 0;
        this->center = { 0, 0, 0 };
        this->color = 0xff000000; //fully opaque, and black.
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }

    Sphere(glm::vec3 center, float radius, int reflects_light, int transparent)
    {
        this->reflects_light = reflects_light;
        this->radius = radius;
        this->center = center;
        this->color = 0xff000000;
        this->shininess = 10.0;
        this->transparent = transparent;
    }
    glm::vec3 calculate_normal_direction(glm::vec3 hitpoint) const //this returns a direction vector which points outside of the sphere
    {
        glm::vec3 result = glm::vec3(hitpoint - this->center);
        return -result;
    }


    //do note, that I'm only calculating the shortest intersection distance now and returning it, I will likely change this once I start working with transparent spheres and such
    float intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, uint32_t original_color) const override
    {
        // Calculate coefficients for the quadratic equation
        float a = glm::dot(ray_direction, ray_direction);
        glm::vec3 oc = ray_origin - this->center;
        float b = 2.0f * glm::dot(oc, ray_direction);
        float c = glm::dot(oc, oc) - this->radius * this->radius;

        // Calculate discriminant
        float discriminant = b * b - 4.0f * a * c;

        // Check if there is an intersection
        if (discriminant > 0.0f)
        {
            // Means there are two intersections - potentially, let's return the shortest
            float t1 = (-b + sqrt(discriminant)) / (2.0f * a);
            float t2 = (-b - sqrt(discriminant)) / (2.0f * a);

            // Check if the intersection points are in front of the viewer
            if (t1 >= 0 || t2 >= 0)
            {
                //new --- we return the shorter result
                if (t1 >= 0 && t2 >= 0)
                {
                    if (t1 <= t2)
                    {
                        glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * t1, ray_origin.y + ray_direction.y * t1, ray_origin.z + ray_direction.z * t1);
                        float distance_result = distance(ray_origin, hitpoint);
                        return distance_result;
                    }
                    else
                    {
                        glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * t2, ray_origin.y + ray_direction.y * t2, ray_origin.z + ray_direction.z * t2);
                        float distance_result = distance(ray_origin, hitpoint);
                        return distance_result;
                    }
                }
                else //meaning either t1 or t2 is negative
                {
                    if (t1 >= 0) //if this one is positive, we deal with it, otherwise that leaves the 2nd one
                    {
                        glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * t1, ray_origin.y + ray_direction.y * t1, ray_origin.z + ray_direction.z * t1);
                        float distance_result = distance(ray_origin, hitpoint);
                        return distance_result;
                    }
                    else
                    {
                        glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * t2, ray_origin.y + ray_direction.y * t2, ray_origin.z + ray_direction.z * t2);
                        float distance_result = distance(ray_origin, hitpoint);
                        return distance_result;
                    }
                }

                //old--- Return the sphere's color
                //return this->color;
            }
        }
        else if (discriminant == 0.0f)
        {
            // There is a single intersection point
            float t = -b / (2.0f * a);


            // Check if the intersection point is in front of the viewer
            if (t >= 0)
            {
                glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * t, ray_origin.y + ray_direction.y * t, ray_origin.z + ray_direction.z * t);
                float distance_result = distance(ray_origin, hitpoint);
                return distance_result;

                // old--- Return the sphere's color
                //return this->color;
            }
        }
        else //discriminant is negative, no intersection
        {
            return -1; //negative distance, no intersection
        }
    }


};

/*class Plane : public Shape_custom
{
private:
    glm::vec4 plane_coord;
    uint32_t color;
    int reflects_light;
    float shininess;
    int transparent;
public:
    //constructors
    Plane(glm::vec4 plane_coord, uint32_t color, int reflects_light, float shininess, int transparent)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = reflects_light;
        this->shininess = shininess;
        this->transparent = transparent;
    }
    Plane(glm::vec4 plane_coord, uint32_t color, int reflects_light,float shininess)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = reflects_light;
        this->shininess = shininess;
        this->transparent = 0;
    }
    Plane(glm::vec4 plane_coord, uint32_t color,int reflects_light)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = reflects_light;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Plane(glm::vec4 plane_coord, uint32_t color)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Plane(glm::vec4 plane_coord)
    {
        this->plane_coord = plane_coord;
        this->color = 0xff000000; //fully opaque, and black.
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }

    //setters
    void setColor(uint32_t color) override
    {
        this->color = color;
    } */

class Plane : public Shape_custom
{
private:
    glm::vec4 plane_coord;
    //uint32_t color;
    int reflects_light;
    //float shininess;
    //int transparent;
public:
    //constructors
    Plane(glm::vec4 plane_coord, uint32_t color, int reflects_light, float shininess, int transparent)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = reflects_light;
        this->shininess = shininess;
        this->transparent = transparent;
    }
    Plane(glm::vec4 plane_coord, uint32_t color, int reflects_light, float shininess)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = reflects_light;
        this->shininess = shininess;
        this->transparent = 0;
    }
    Plane(glm::vec4 plane_coord, uint32_t color, int reflects_light)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = reflects_light;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Plane(glm::vec4 plane_coord, uint32_t color)
    {
        this->plane_coord = plane_coord;
        this->color = color;
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }
    Plane(glm::vec4 plane_coord)
    {
        this->plane_coord = plane_coord;
        this->color = 0xff000000; //fully opaque, and black.
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }

    Plane()
    {
        this->plane_coord = { 0, 0, 0, 0 };
        this->color = 0xff000000; //fully opaque, and black.
        this->reflects_light = 0;
        this->shininess = 10.0;
        this->transparent = 0;
    }

    Plane(int reflects_light, int transparent, glm::vec4 plane_coord)
    {
        this->plane_coord = plane_coord;
        this->color = 0xff000000;
        this->reflects_light = reflects_light;
        this->shininess = 10.0;
        this->transparent = transparent;
    }

    //setters
   /* void setColor(uint32_t color) override
    {
        this->color = color;
    }*/

    void setTransparent(int transparent)
    {
        this->transparent = transparent;
    }

    /*void setShininess(int shininess)
    {
        this->shininess = shininess;
    }*/
    //getters
    glm::vec4 getCoord() const
    {
        return this->plane_coord;
    }

    int get_reflecting_status() const override
    {
        return this->reflects_light;
    }
    float get_shininess_value() const override
    {
        return this->shininess;
    }
    int get_transparency_status() const override
    {
        return this->transparent;
    }
    //------------FOR PLANE CHECKERBOARD COLOR PATTERN, COPIED FROM PS5 ------------------------
    uint32_t getColor(glm::vec3 hitPoint) const
    {
        if (reflects_light == 0)
        {
            // Checkerboard pattern
            float scale_parameter = 0.5f;
            float chessboard = 0;

            if (hitPoint.x < 0)
            {
                chessboard += floor((0.5 - hitPoint.x) / scale_parameter);
            }
            else
            {
                chessboard += floor(hitPoint.x / scale_parameter);
            }

            if (hitPoint.y < 0) {
                chessboard += floor((0.5 - hitPoint.y) / scale_parameter);
            }
            else
            {
                chessboard += floor(hitPoint.y / scale_parameter);
            }

            chessboard = (chessboard * 0.5) - int(chessboard * 0.5);
            chessboard *= 2;
            if (chessboard > 0.5) {
                glm::vec4 original_color = convert_uint32_t_to_vec4_rgba(this->color, 0);
                glm::vec4 shaded_color = 0.5f * convert_uint32_t_to_vec4_rgba(this->color, 0);
                uint32_t result = convert_vec4_rgba_to_uint32_t(glm::vec4(shaded_color.x, shaded_color.y, shaded_color.z, shaded_color.w), 0);
                return result;
            }
            return this->color;
        }
        else
        {
            return this->color;
        }

    }

    //------------------------------------------------------------------------------------------

    glm::vec3 calculate_normal_direction(glm::vec3 hitpoint) const //we don't do much here, the definition of a plane contains the normal
    {
        glm::vec4 original_plane_coords = this->getCoord();
        glm::vec3 result = glm::vec3(original_plane_coords.x, original_plane_coords.y, original_plane_coords.z);
        return result;
    }


    float intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, uint32_t original_color) const override
    {
        //first we deal with the checkerboard pattern:


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
                return 0; //this means we are inside the plane, so the distance is 0
            }
            else
            {
                //none at all
                return original_color;
            }
        }
        else // a single intersection
        {
            float result = nominator / denominator; //we currently do nothing with it, but perhaps we might later down the line
            glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * result, ray_origin.y + ray_direction.y * result, ray_origin.z + ray_direction.z * result);
            float distance_result = distance(ray_origin, hitpoint);
            //uint32_t res_color = getColor(hitpoint, this->color);
            if (result >= -0.00001)
            {
                return distance_result;
                //return getColor(hitpoint, this->color);
            }
            else
            {
                return -1;//this means no intersection from our camera forwards
                //return original_color;
            }

        }

        return -1;
        //return original_color;
    }

};

//recursive_ray_cast
uint32_t do_the_intersection_thing_again_re_check(glm::vec3& rayOrigin, glm::vec3& rayDirection, std::vector<float>& distances, std::vector<float>& reflections_distances, int& relevant_re_reflection_shape_index, int& relevant_reflection_shape_index, std::vector<Shape_custom*> shapes_input, uint32_t& pixel_value, float& shortest_distance, int& prev_reflection_shape_index)
{
    //find the reflected shape's color
    //checking for reflections now - we only do 1 hop!
    //we do it again now to grab more info, and fire one more ray, lets do it
    //glm::vec3 rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distances[relevant_reflection_shape_index]);
    for (float distance : distances)
    {
        distances.pop_back();
    }

    //glm::vec3 normal_on_mirror = shapes_input[relevant_reflection_shape_index]->calculate_normal_direction(rayOrigin);
    //glm::vec3 rayDirection_from_mirror = glm::reflect(rayDirection, normal_on_mirror); //may need to negate the normal on mirror

    //distances = reflections_distances;


    std::vector<float> re_reflections_distances;


    //we check intersections with all the shapes from our reflected ray, again
    for (int j = 0; j < shapes_input.size(); j++)
    {
        if (relevant_reflection_shape_index != j) //dont check against ourselves, and don't check against our previous, if our distance is like - nothing
        {
            float reflection_intersection_result = shapes_input[j]->intersection(rayOrigin, rayDirection, pixel_value);

            if ((reflection_intersection_result >= 0 && reflection_intersection_result <= 0.001)) //edge case to avoid corner
            {
                re_reflections_distances.push_back(-1);
            }
            else
            {
                re_reflections_distances.push_back(reflection_intersection_result);
            }



        }
        else
        {
            re_reflections_distances.push_back(-1);

        }

    }

    for (float distance : re_reflections_distances)
    {
        distances.push_back(distance);
    }

    //again, we check for distances, we want to return the color of the closest one - that is not ourselves
    float shortest_reflection_distance = INT_MAX; //arbitrary value, hopefully we don't reach those distances in reality
    relevant_re_reflection_shape_index = -1;
    for (int i = 0; i < re_reflections_distances.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
    {
        if (re_reflections_distances[i] > -1 && re_reflections_distances[i] != -1 && re_reflections_distances[i] < shortest_reflection_distance && i != relevant_reflection_shape_index && i != prev_reflection_shape_index) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
        {
            shortest_reflection_distance = re_reflections_distances[i];
            relevant_re_reflection_shape_index = i;

        }
    }
    if (shortest_reflection_distance == INT_MAX || shortest_reflection_distance > 100) //meaning the reflection doesnt hit anything, so we're technically finished, or it goes so far that it doesn't matter by that point
    {
        pixel_value = 0;
        relevant_reflection_shape_index = -1;
        shortest_distance = -1;
        //doing dumb stuff to not get stuck and avoid adding more pointless variables
    }
    else //meaning we've hit something, don't care what it is - (I mean we kinda do, but we'll just run again if the result doesn't satisfy - until it does)
    {
        //rayOrigin = calculate_hitpoint_from_distance(rayOrigin, rayDirection_from_mirror, shortest_reflection_distance);
        //rayDirection = rayDirection_from_mirror;
        prev_reflection_shape_index = relevant_reflection_shape_index;
        relevant_reflection_shape_index = relevant_re_reflection_shape_index;
        pixel_value = shapes_input[relevant_re_reflection_shape_index]->getColor(rayOrigin);
        shortest_distance = shortest_reflection_distance;


    }


    for (float distance : reflections_distances)
    {
        reflections_distances.pop_back();
    }
    for (float distance : re_reflections_distances)
    {
        reflections_distances.push_back(distance);
    }


    return pixel_value;
}





//will likely need to add: RayOrigin
uint32_t PerPixelLight(int we_are_a_spotlight, glm::vec3 spotlight_pos, std::vector<glm::vec4> light_sources_directions, uint32_t* image_data_input, uint32_t original_object_color, glm::vec3 eye, glm::vec3 from_eye_direction, std::vector<float>& distances_input, std::vector<Shape_custom*>& shapes_input, uint32_t pixel_color, std::vector<glm::vec4>& spotlight_positions, glm::vec4 ambient_light, int relevant_shape_index, int relevant_reflected_shape_index, glm::vec3 hitpoint, glm::vec3 light_ray, float shortest_distance, int spotlight_counter, glm::vec4 light_color_of_source)
{
    //LIGHTING CONSTANTS:

    //specular intensity is set as a hard-coded 0.7. It does not affect the alpha channel
    glm::vec4 specular_intensity_vec = glm::vec4(0.7, 0.7, 0.7, 1.0);
    float specular_intensity_float = 0.7;

    glm::vec3 light_ray_normalized = glm::normalize(light_ray);


    glm::vec3 light_to_surface_vec;
    if (we_are_a_spotlight)
    {
        light_to_surface_vec = calculate_vector_direction(spotlight_pos, hitpoint);
        glm::vec3 light_to_surface_vec_normalized = glm::normalize(light_to_surface_vec);
    }


    glm::vec3 normal_direction_vector;
    //calculating diffusion
    if (relevant_reflected_shape_index != -1)
    {
        normal_direction_vector = shapes_input[relevant_reflected_shape_index]->calculate_normal_direction(hitpoint);
    }
    else
    {
        normal_direction_vector = shapes_input[relevant_shape_index]->calculate_normal_direction(hitpoint);
    }


    glm::vec3 normal_direction_vector_normalized = glm::normalize(normal_direction_vector);
    float diffusion_strength = glm::max(glm::dot(light_ray_normalized, normal_direction_vector_normalized), 0.0f); //1 means they are perpendicular, and the light is maximal, 0 means that the light is parallel to the object and essentially does not hit it
    //---------------------

    //calculating specular
    glm::vec3 from_eye_direction_normalized = glm::normalize(from_eye_direction);
    glm::vec3 reflection_light_ray;

    if (we_are_a_spotlight)
    {
        reflection_light_ray = (glm::reflect(-light_to_surface_vec, normal_direction_vector_normalized));
    }
    else
    {
        reflection_light_ray = (glm::reflect(-light_ray_normalized, normal_direction_vector_normalized));
    }

    float specular_strength = glm::max(glm::dot(glm::normalize(reflection_light_ray), from_eye_direction_normalized), 0.0f);
    if (specular_strength > 1.0)
    {
        pixel_color = pixel_color + 1;
        pixel_color = pixel_color - 1;
    }


    float spec;

    if (relevant_reflected_shape_index == -1)
    {
        spec = (pow(specular_strength, shapes_input[relevant_shape_index]->get_shininess_value())); //in learnopengl.com this is called "spec", we have to multiply it by specularStrength and lightcolor
    }
    else
    {
        spec = (pow(specular_strength, shapes_input[relevant_reflected_shape_index]->get_shininess_value()));
    }


    float spec_strength = spec * specular_intensity_float;

    //--------------------

    //calculating ambient

    //-------------------

    //DOING PHONG
    glm::vec4 diffusion_product = glm::vec4(clamp(light_color_of_source.x * 255 * diffusion_strength), clamp(light_color_of_source.y * 255 * diffusion_strength), clamp(light_color_of_source.z * 255 * diffusion_strength), 255);
    glm::vec4 ambient_product = ambient_light;



    glm::vec4 specular_product = glm::vec4((light_color_of_source.x * spec_strength) * 255, (light_color_of_source.y * spec_strength) * 255, (light_color_of_source.z * spec_strength) * 255, 255);

    glm::vec4 original_object_color_percentage = convert_uint32_t_to_vec4_rgba(original_object_color, 1);
    glm::vec4 original_object_color_values = convert_uint32_t_to_vec4_rgba(original_object_color, 0);

    //glm::vec4 pixel_color_vector_temp = multiply_vector_by_vec_scalars(original_object_color_percentage, (ambient_product+diffusion_product));



    if (original_object_color_percentage.x == 1.0 && original_object_color_percentage.z == 0)
    {

    }


    glm::vec4 pixel_color_vector = multiply_vector_by_vec_scalars(original_object_color_percentage, (diffusion_product)); // DIFFUE DOES OK, SPECULAR EH, SUMMED UP THEY FUCK ME UP

    if (pixel_color_vector.x + specular_product.x > 255)
    {
        //std::cout << "WOAH THERE!";
    }

    pixel_color_vector = glm::vec4(clamp(pixel_color_vector.x + specular_product.x), clamp(pixel_color_vector.y + specular_product.y), clamp(pixel_color_vector.z + specular_product.z), 255); //check clamp


    uint32_t final_result = convert_vec4_rgba_to_uint32_t(glm::vec4(pixel_color_vector.x, pixel_color_vector.y, pixel_color_vector.z, 255), 0);

    if (original_object_color_percentage.x == 1.0 && original_object_color_percentage.z == 0)
    {
        //std::cout << "bingbong\n";
        if (pixel_color_vector.x == 0)
        {

        }
        else if (specular_product.x > 1)
        {

        }

    }

    return final_result;
}




uint32_t PerPixelShadow(std::vector<glm::vec4> light_sources_directions, std::vector<glm::vec4> light_sources_colors, uint32_t* image_data_input, uint32_t original_object_color, glm::vec3 eye, glm::vec3 from_eye_direction, std::vector<float>& distances_input, std::vector<Shape_custom*>& shapes_input, uint32_t pixel_color, std::vector<glm::vec4>& spotlight_positions, glm::vec4 ambient_light)
{
    uint32_t pixel_color_result = pixel_color; //we will be adding values on top of it with each lighting source
    uint32_t pixel_value = 0;
    int light_source_reflection = 0; //turned off by default, produces the results as asked by the assignment - (although ligh ray reflection does make more sense)


    //SETTING UP INFRASTRUCTURE FOR REFLECTIONS-------------------
    //default values, may change if we hit a reflective surface
    glm::vec3 rayOrigin = eye;
    glm::vec3 rayOrigin_before_initial_reflect = eye; //not used currently;
    glm::vec3 rayDirection = from_eye_direction;
    int relevant_reflection_shape_index = -1;
    glm::vec3 normal_on_mirror;
    glm::vec3 rayOrigin_on_mirror;






    //-----------------------------------------------------------





    //specular intensity is set as a hard-coded 0.7. It does not affect the alpha channel
    glm::vec4 specular_intensity_vec = glm::vec4(0.7, 0.7, 0.7, 1.0);
    float specular_intensity_float = 0.7;



    //calculate the shortest distance from the eye, this is how we find a hitpoint, from which we calculate a ray to a light source
    uint32_t og_color = pixel_color;
    float shortest_distance = INT_MAX;
    int relevant_shape_index = -1;
    int spotlight_counter = 0; //only used for spotlight since I have to sync light_sources_directions and spotlight_positions. That's the cost of winging it and not doing classes.
    for (int i = 0; i < distances_input.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
    {
        if (distances_input[i] >= 0 && distances_input[i] < shortest_distance) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
        {
            shortest_distance = distances_input[i];
            relevant_shape_index = i; // we save this so we won't look for intersections with ourselves
            //pixel_value = shapes_input[shapes_input.size() - 1 - i]->getColor(calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance)); //the index reverse is because we work in stacks, but calculate chronologically. I might flip it over to sync them properly, but it works so eh, may change later
        }
    }

    //170224 - inserted post-reflection add, only deals with a situation where we've hit a mirror. do note that in perPixel we cared about the color, here we care about the distance, the rest should be taken care of
    if (relevant_shape_index != -1 && shapes_input[relevant_shape_index]->get_reflecting_status() == 1)
    {
        std::vector<float> reflections_distances;

        if (shapes_input[relevant_shape_index]->get_reflecting_status() == 1) //meaning it reflects light 
        {
            //find the reflected shape
            //checking for reflections now - we only do 1 hop!
            //we do it again now to grab more info, and fire one more ray, lets do it
            rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distances_input[relevant_shape_index]);
            normal_on_mirror = shapes_input[relevant_shape_index]->calculate_normal_direction(rayOrigin_on_mirror);
            glm::vec3 rayDirection_from_mirror = glm::reflect(rayDirection, normal_on_mirror); //may need to negate the normal on mirror

            //we check intersections with all the shapes from our reflected ray, again
            for (int j = 0; j < shapes_input.size(); j++)
            {
                if (relevant_shape_index != j) //dont check against ourselves
                {
                    float reflection_intersection_result = shapes_input[j]->intersection(rayOrigin_on_mirror, rayDirection_from_mirror, pixel_color_result);
                    reflections_distances.push_back(reflection_intersection_result);
                }
                else
                {
                    reflections_distances.push_back(-1);
                }

            }

            //again, we check for distances, we want to return the closest one
            shortest_distance = INT_MAX; //arbitrary value, hopefully we don't reach those distances in reality
            for (int i = 0; i < reflections_distances.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
            {
                if (reflections_distances[i] >= 0 && reflections_distances[i] < shortest_distance) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
                {
                    shortest_distance = reflections_distances[i];
                    relevant_reflection_shape_index = i;
                    original_object_color = shapes_input[relevant_reflection_shape_index]->getColor(calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance));
                    rayOrigin = rayOrigin_on_mirror;
                    rayDirection = rayDirection_from_mirror;
                }
            }
            if (shortest_distance == INT_MAX) //meaning the reflection doesnt hit anything
            {
                shortest_distance = -1;
            }
            else //meaning we've hit something, don't care what it is - since we have no recursion currently
            {

                //pixel_value = shapes_input[relevant_reflection_shape_index]->getColor(calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance));

                ////OH WE CARE NOW!
                if (shapes_input[relevant_reflection_shape_index]->get_reflecting_status() != 1) //meaning it's a solid object
                {
                    pixel_value = shapes_input[relevant_reflection_shape_index]->getColor(calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance));
                }
                else if (shapes_input[relevant_reflection_shape_index]->get_reflecting_status() == 1) //mirror again
                {

                    //glm::vec3 rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distances[relevant_shape_index]);
                    //glm::vec3 normal_on_mirror = shapes_input[relevant_shape_index]->calculate_normal_direction(rayOrigin_on_mirror);
                    //glm::vec3 rayDirection_from_mirror = glm::reflect(rayDirection, normal_on_mirror); //may need to negate the normal on mirror

                    //INSERT REFLECTION CODE HERE AGAIN, AND BREAK IT INTO A FUNCTION WHICH WILL CONTINUE BY ITSELF
                    //rayOrigin, rayDirection, & distances, & reflections_distances, relevant_shape_index, & shapes_input, pixel_value, shortest_distance
                    int iterations = 1; //i did one reflection previously, finishing up the rest
                    pixel_value = 1;
                    int prev_relevant_shape_index = relevant_shape_index;
                    int relevant_re_reflection_shape_index = relevant_reflection_shape_index;

                    while (pixel_value != 0 && shapes_input[relevant_re_reflection_shape_index]->get_reflecting_status() == 1 && iterations < 5)
                    {
                        rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance);
                        normal_on_mirror = shapes_input[relevant_reflection_shape_index]->calculate_normal_direction(rayOrigin_on_mirror);
                        rayOrigin = rayOrigin_on_mirror;

                        //if(shortest_distance < 0)
                        //{
                        //    normal_on_mirror = -normal_on_mirror;
                        //}

                        rayDirection_from_mirror = glm::reflect(rayDirection_from_mirror, normal_on_mirror); //from original mirror to new one
                        rayDirection = rayDirection_from_mirror;
                        //int previous_relevant_reflection_shape_index = relevant_reflection_shape_index;
                        uint32_t next_object_color = do_the_intersection_thing_again_re_check(rayOrigin_on_mirror, rayDirection_from_mirror, distances_input, reflections_distances, relevant_re_reflection_shape_index, relevant_reflection_shape_index, shapes_input, pixel_value, shortest_distance, prev_relevant_shape_index);
                        original_object_color = next_object_color;
                        iterations++;

                    }
                    if (iterations >= 5)
                    {
                        original_object_color = 0;
                        shortest_distance = -1;
                        relevant_shape_index = -1;
                        relevant_reflection_shape_index = -1;

                    }
                    else
                    {
                        relevant_shape_index = prev_relevant_shape_index;
                        relevant_reflection_shape_index = relevant_re_reflection_shape_index;
                        //relevant_reflection_shape_index = relevant_re_reflection_shape_index;
                        //relevant_shape_index = relevant_reflection_shape_index;

                    }

                }

                else if (shapes_input[relevant_shape_index]->get_transparency_status() == 1)
                {
                    relevant_shape_index = -1;
                }
                //else if (shapes_input[relevant_shape_index]->get_transparency_status() == 1) //DO NOTE: THIS SHOULD WORK FOR A GLASS BALL, IT MAY FAIL WITH A POINT, AS WELL AS WITH A PLANE, I'M NOT TESTING FOR THAT FOR NOW
                //{
                //    //meaning we are transparent, and thus continue the calculation from that point
                //    float distance_to_glass = shortest_distance;
                //    glm::vec3 entry_glass_hitpoint = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distance_to_glass);


                //    //now we calculate a new vector inside the glass until we reach the end of said shape
                //    glm::vec3 glass_normal = -shapes_input[relevant_shape_index]->calculate_normal_direction(entry_glass_hitpoint);

                //    //NOTE: I add a tiny bit of distance to my hitpoint, so as to move a bit from the ball's surface, doing so will let me calculate the next intersection organically, without rewriting the intersection code 
                //    // (it won't return me a 0 is what I'm saying)
                //    entry_glass_hitpoint = glm::vec3(entry_glass_hitpoint.x - glass_normal.x * 0.001, entry_glass_hitpoint.y - glass_normal.y * 0.001, entry_glass_hitpoint.z - glass_normal.z * 0.001);

                //    glm::vec3 glass_normal_normalized = glm::normalize(glass_normal);
                //    glm::vec3 rayDirection_normalized = glm::normalize(rayDirection);

                //    //float angle_hit = acos(glm::dot(glass_normal_normalized, rayDirection_normalized)); //shift-cosinus on the dot product of two normalized vectors to get the angle value (in radians)

                //    //now we calculate the vector direction coordinates inside of the sphere

                //    glm::vec3 rayDirection_in_glass = glm::refract(rayDirection_normalized, glass_normal_normalized, 1.0f / 1.5f);

                //    //glm::vec3 rayDirection_in_glass = glm::refract(rayDirection_normalized, glass_normal_normalized, 0.75f / 1.5f);
                //    //glm::vec3 rayDirection_in_glass = manual_refraction(rayDirection, -glass_normal, 1.5f);


                //    //if (glm::length(rayDirection) > 1)
                //    //{
                //    //    rayDirection_in_glass = rayDirection_in_glass * glm::length(rayDirection);
                //    //}

                //    //rayDirection_in_glass = rayDirection_in_glass * 10.0f;
                //    std::vector<float> distances_after_refraction;

                //    glm::vec3 exit_glass_hitpoint;
                //    glm::vec3 exit_glass_normal;
                //    glm::vec3 exit_refraction_direction;
                //    if (rayDirection_in_glass != glm::vec3(0, 0, 0))
                //    {
                //        float distance_inside_glass = shapes_input[relevant_shape_index]->intersection(entry_glass_hitpoint, rayDirection_in_glass, pixel_value); //finding the intersection distance inside the glass object
                //        exit_glass_hitpoint = calculate_hitpoint_from_distance(entry_glass_hitpoint, rayDirection_in_glass, distance_inside_glass);


                //        exit_glass_normal = shapes_input[relevant_shape_index]->calculate_normal_direction(exit_glass_hitpoint);
                //        exit_glass_hitpoint = glm::vec3(exit_glass_hitpoint.x + glass_normal.x * 0.001, exit_glass_hitpoint.y + glass_normal.y * 0.001, exit_glass_hitpoint.z + glass_normal.z * 0.001);

                //        //exit_refraction_direction = glm::refract((glm::normalize(rayDirection_in_glass)), glm::normalize(exit_glass_normal), 1.0f);

                //        //exit_refraction_direction = manual_refraction(rayDirection_in_glass, -exit_glass_normal, 1.0f);

                //        exit_refraction_direction = glm::refract(glm::normalize(rayDirection_in_glass), glm::normalize(exit_glass_normal), 1.0f / 1.0f);



                //        //if we have an actual vector, we can fire a new ray from the exit hitpoint
                //        for (Shape_custom* shape : shapes_input)
                //        {

                //            float refraction_cast_result = shape->intersection(exit_glass_hitpoint, exit_refraction_direction, pixel_value); //rayDirection WILL CHANGE NOW
                //            if (refraction_cast_result != -1)
                //            {
                //                //distances_after_refraction.push_back(shortest_distance + distance_inside_glass + refraction_cast_result);
                //                distances_after_refraction.push_back(refraction_cast_result);
                //            }
                //            else
                //            {
                //                distances_after_refraction.push_back(-1);
                //            }

                //        }
                //    }
                //    else
                //    {
                //        for (int j = 0; j < shapes_input.size(); j++)
                //        {
                //            //otherwise we pretend the hit was never attempted, and we continue from the original hitpoint, ignoring the ball
                //            if (j != relevant_shape_index)
                //            {
                //                distances_after_refraction.push_back(shapes_input[j]->intersection(rayOrigin, rayDirection, pixel_value));
                //            }
                //            else //since we ignore the object we hit, it becomes invisible to us
                //            {
                //                distances_after_refraction.push_back(-1);
                //            }

                //        }
                //        exit_glass_hitpoint = rayDirection_in_glass;
                //    }


                //    //fire a new ray from that point



                //    float refracted_shortest_distance = INT_MAX;
                //    int refracted_relevant_shape = -1;
                //    for (int j = 0; j < distances_after_refraction.size(); j++)
                //    {
                //        if (distances_after_refraction[j] < refracted_shortest_distance && distances_after_refraction[j] > 0 && j != relevant_shape_index)
                //        {
                //            refracted_shortest_distance = distances_after_refraction[j];
                //            refracted_relevant_shape = j;
                //        }
                //    }
                //    if (refracted_shortest_distance == INT_MAX)
                //    {
                //        original_object_color = 0;
                //    }
                //    else
                //    {
                //        glm::vec3 final_hitpoint = calculate_hitpoint_from_distance(exit_glass_hitpoint, exit_refraction_direction, refracted_shortest_distance);
                //        glm::vec3 final_vector_direction = calculate_vector_direction(exit_glass_hitpoint, final_hitpoint);
                //        original_object_color = shapes_input[refracted_relevant_shape]->getColor(calculate_hitpoint_from_distance(final_hitpoint, final_vector_direction, refracted_shortest_distance));
                //        rayOrigin = exit_glass_hitpoint;
                //        rayDirection = final_vector_direction;
                //        shortest_distance = refracted_shortest_distance;

                //        relevant_shape_index = refracted_relevant_shape;
                //        
                //    }



                //}
            }


        }
    }
    else if (relevant_shape_index != -1 && shapes_input[relevant_shape_index]->get_transparency_status() == 1)
    {

        //meaning we are transparent, and thus continue the calculation from that point
        float distance_to_glass = shortest_distance;
        glm::vec3 entry_glass_hitpoint = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distance_to_glass);


        //now we calculate a new vector inside the glass until we reach the end of said shape
        glm::vec3 glass_normal = -shapes_input[relevant_shape_index]->calculate_normal_direction(entry_glass_hitpoint);

        //NOTE: I add a tiny bit of distance to my hitpoint, so as to move a bit from the ball's surface, doing so will let me calculate the next intersection organically, without rewriting the intersection code 
        // (it won't return me a 0 is what I'm saying)
        entry_glass_hitpoint = glm::vec3(entry_glass_hitpoint.x - glass_normal.x * 0.001, entry_glass_hitpoint.y - glass_normal.y * 0.001, entry_glass_hitpoint.z - glass_normal.z * 0.001);

        glm::vec3 glass_normal_normalized = glm::normalize(glass_normal);
        glm::vec3 rayDirection_normalized = glm::normalize(rayDirection);

        //float angle_hit = acos(glm::dot(glass_normal_normalized, rayDirection_normalized)); //shift-cosinus on the dot product of two normalized vectors to get the angle value (in radians)

        //now we calculate the vector direction coordinates inside of the sphere

        glm::vec3 rayDirection_in_glass = glm::refract(rayDirection_normalized, glass_normal_normalized, 1.0f / 1.5f);

        //glm::vec3 rayDirection_in_glass = glm::refract(rayDirection_normalized, glass_normal_normalized, 0.75f / 1.5f);
        //glm::vec3 rayDirection_in_glass = manual_refraction(rayDirection, -glass_normal, 1.5f);


        //if (glm::length(rayDirection) > 1)
        //{
        //    rayDirection_in_glass = rayDirection_in_glass * glm::length(rayDirection);
        //}

        //rayDirection_in_glass = rayDirection_in_glass * 10.0f;
        std::vector<float> distances_after_refraction;

        glm::vec3 exit_glass_hitpoint;
        glm::vec3 exit_glass_normal;
        glm::vec3 exit_refraction_direction;
        if (rayDirection_in_glass != glm::vec3(0, 0, 0))
        {
            float distance_inside_glass = shapes_input[relevant_shape_index]->intersection(entry_glass_hitpoint, rayDirection_in_glass, pixel_value); //finding the intersection distance inside the glass object
            exit_glass_hitpoint = calculate_hitpoint_from_distance(entry_glass_hitpoint, rayDirection_in_glass, distance_inside_glass);


            exit_glass_normal = shapes_input[relevant_shape_index]->calculate_normal_direction(exit_glass_hitpoint);
            exit_glass_hitpoint = glm::vec3(exit_glass_hitpoint.x + glass_normal.x * 0.001, exit_glass_hitpoint.y + glass_normal.y * 0.001, exit_glass_hitpoint.z + glass_normal.z * 0.001);

            //exit_refraction_direction = glm::refract((glm::normalize(rayDirection_in_glass)), glm::normalize(exit_glass_normal), 1.0f);

            //exit_refraction_direction = manual_refraction(rayDirection_in_glass, -exit_glass_normal, 1.0f);

            exit_refraction_direction = glm::refract(glm::normalize(rayDirection_in_glass), glm::normalize(exit_glass_normal), 1.0f / 1.0f);



            //if we have an actual vector, we can fire a new ray from the exit hitpoint
            for (Shape_custom* shape : shapes_input)
            {

                float refraction_cast_result = shape->intersection(exit_glass_hitpoint, exit_refraction_direction, pixel_value); //rayDirection WILL CHANGE NOW
                if (refraction_cast_result != -1)
                {
                    //distances_after_refraction.push_back(shortest_distance + distance_inside_glass + refraction_cast_result);
                    distances_after_refraction.push_back(refraction_cast_result);
                }
                else
                {
                    distances_after_refraction.push_back(-1);
                }

            }
        }
        else
        {
            for (int j = 0; j < shapes_input.size(); j++)
            {
                //otherwise we pretend the hit was never attempted, and we continue from the original hitpoint, ignoring the ball
                if (j != relevant_shape_index)
                {
                    distances_after_refraction.push_back(shapes_input[j]->intersection(rayOrigin, rayDirection, pixel_value));
                }
                else //since we ignore the object we hit, it becomes invisible to us
                {
                    distances_after_refraction.push_back(-1);
                }

            }
            exit_glass_hitpoint = rayDirection_in_glass;
        }


        //fire a new ray from that point



        float refracted_shortest_distance = INT_MAX;
        int refracted_relevant_shape = -1;
        for (int j = 0; j < distances_after_refraction.size(); j++)
        {
            if (distances_after_refraction[j] < refracted_shortest_distance && distances_after_refraction[j] > 0 && j != relevant_shape_index)
            {
                refracted_shortest_distance = distances_after_refraction[j];
                refracted_relevant_shape = j;
            }
        }
        if (refracted_shortest_distance == INT_MAX)
        {
            original_object_color = 0;
        }
        else
        {
            glm::vec3 final_hitpoint = calculate_hitpoint_from_distance(exit_glass_hitpoint, exit_refraction_direction, refracted_shortest_distance);
            glm::vec3 final_vector_direction = calculate_vector_direction(exit_glass_hitpoint, final_hitpoint);
            original_object_color = shapes_input[refracted_relevant_shape]->getColor(calculate_hitpoint_from_distance(final_hitpoint, final_vector_direction, refracted_shortest_distance));
            rayOrigin = exit_glass_hitpoint;
            rayDirection = final_vector_direction;
            shortest_distance = refracted_shortest_distance;

            relevant_shape_index = refracted_relevant_shape;


        }
    }



    //we now check collision with other shapes on our way to the lighting
    int light_source_index = 0;
    for (glm::vec4 light : light_sources_directions) //I may only filter specific light source types, MAY CHANGE.
    {
        char in_shade = 1; //I'm in shade until proven not to be. This is important because we potentially may be under shade with one source of light, but another light may still shine at us.
        glm::vec3 light_ray = glm::vec3(light.x, light.y, light.z); //presumably the  direction is lightsource -> object
        glm::vec3 ORIGINAL_light_ray = light_ray; //this is a backup for reflection, for spotlight specifically, only for the cone dot calculation
        glm::vec3 normalized_light_ray;

        //170224 - added reflection status, I'll have to work with reflected light here
        if (relevant_shape_index != -1 && shapes_input[relevant_shape_index]->get_reflecting_status() == 1)
        {


            //COMMENT: TURN ON THESE 2 CODE LINES TO GET PROPER REFLECTED LIGHTING. FOR SOME REASON THE REFERENCE IMAGE DOES NOT INCLUDE REFLECTED LIGHTING FROM SOURCE AS A FEATURE, WTF
            if (light_source_reflection == 1)
            {
                glm::vec3 reflected_light_ray = glm::reflect(light_ray, normal_on_mirror);
                light_ray = reflected_light_ray; //meaning the light ray we work with is the reflected one
            }

            normalized_light_ray = glm::normalize(light_ray); //presumably the  direction is lightsource -> object, normalized



        }
        //else if (relevant_shape_index != -1 && shapes_input[relevant_shape_index]->get_transparency_status() == 1)
        //{
        //
        //}
        else //we work as per usual, no reflections
        {
            normalized_light_ray = glm::normalize(light_ray); //presumably the  direction is lightsource -> object, normalized
        }


        //check light_type
        if (light.w == 0.0 && relevant_shape_index != -1 && original_object_color != 0) //directional light
        {
            glm::vec3 hitpoint = calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance);


            int potential_intersections;
            if (relevant_reflection_shape_index == -1)
            {
                potential_intersections = shapes_input.size() - 1;
            }
            else
            {
                potential_intersections = shapes_input.size() - 2;
                if (potential_intersections < 0) //make sure we don't go negative
                {
                    potential_intersections = 0;
                }
            }

            for (int i = 0; i < shapes_input.size(); i++) //originally a for each loop, moved from it to accommodate for filtering out an indexed shape 
            {

                if (((i != relevant_shape_index && i != relevant_reflection_shape_index) || (shapes_input.size() == 1)) && relevant_shape_index != -1) //don't check intersection with myself. ADDITIONALLY: the i of the distances may be in inverse to the shapes vector, may have to switch them around. 
                {
                    float distance_to_intersection;
                    if (shapes_input.size() == 1)
                    {
                        distance_to_intersection = shortest_distance;
                    }
                    else
                    {
                        distance_to_intersection = shapes_input[i]->intersection(hitpoint, -normalized_light_ray, pixel_color); //notice that light ray is reversed, otherwise i get a negative direction in intersection and dont return it
                    }
                    if (distance_to_intersection < 0 || distance_to_intersection >= shortest_distance)
                    {
                        potential_intersections--;
                        if (potential_intersections <= 0)
                        {

                            in_shade = 0;
                            glm::vec4 directional_light_color = glm::vec4(0.7, 0.5, 0.0, 1.0); //TEMPORARILY HARDCODED, WILL CHANGE SOON

                            if (shapes_input[relevant_shape_index]->get_reflecting_status() == 1)
                            {
                                pixel_color_result = pixel_color_result + 1;
                                pixel_color_result = pixel_color_result - 1;
                            }




                            //POTENTIAL OVERFLOW PROBLEM$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                            uint32_t direct_directional_lighting_result;
                            direct_directional_lighting_result = PerPixelLight(0, glm::vec3(0, 0, 0), light_sources_directions, image_data_input, original_object_color, rayOrigin, rayDirection, distances_input, shapes_input, pixel_color, spotlight_positions, ambient_light, relevant_shape_index, relevant_reflection_shape_index, hitpoint, light_ray, shortest_distance, spotlight_counter, light_sources_colors[light_source_index]); //ambient_lighting argument means color
                            //direct_directional_lighting_result = 0;
                            //testing
                            if (direct_directional_lighting_result != 0xff000000 && shapes_input[relevant_shape_index]->get_reflecting_status() == 1)
                            {
                                //std::cout << "I added light to reflected surface";
                            }
                            //
                            //uint32_t direct_directional_lighting_result = PerPixelLight(0, glm::vec3(0, 0, 0), light_sources_directions, image_data_input, original_object_color, rayOrigin, rayDirection, distances_input, shapes_input, pixel_color, spotlight_positions, ambient_light, relevant_shape_index, hitpoint, light_ray, shortest_distance, spotlight_counter, light_sources_colors[light_source_index]); //ambient_lighting argument means color

                            glm::vec4 pixel_color_result_vector_values = convert_uint32_t_to_vec4_rgba(pixel_color_result, 0);
                            glm::vec4 direct_directional_lighting_result_values = convert_uint32_t_to_vec4_rgba(direct_directional_lighting_result, 0);

                            glm::vec4 final_pixel_color_values = glm::vec4(clamp(pixel_color_result_vector_values.x + direct_directional_lighting_result_values.x), clamp(pixel_color_result_vector_values.y + direct_directional_lighting_result_values.y), clamp(pixel_color_result_vector_values.z + direct_directional_lighting_result_values.z), 255);;


                            pixel_color_result = convert_vec4_rgba_to_uint32_t(final_pixel_color_values, 0);

                            if (shapes_input[relevant_shape_index]->getColor(glm::vec3(0)) == convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 0.0, 0.8, 10.0), 1))
                            {
                                pixel_color_result = pixel_color_result + 1;
                                pixel_color_result = pixel_color_result - 1;
                                glm::vec4 color_difference = glm::vec4(convert_uint32_t_to_vec4_rgba(pixel_color, 0) - convert_uint32_t_to_vec4_rgba(pixel_color_result, 0));
                                if (color_difference != glm::vec4(0))
                                {
                                    //std::cout << color_difference.x << ", " << color_difference.y << ", " << color_difference.z << ", " << color_difference.w << std::endl; 
                                }

                            }
                            //pixel_color_result += PerPixelLight(0,glm::vec3 (0,0,0),light_sources_directions,image_data_input, original_object_color,eye,from_eye_direction,distances_input,shapes_input,pixel_color,spotlight_positions,ambient_light,relevant_shape_index,hitpoint,light_ray,shortest_distance,spotlight_counter,directional_light_color); //ambient_lighting argument means color
                        }
                    }

                }
            }

            if (in_shade == 1)
            {
                //pixel_color = rgba_uint32_t_scalar_multiplication(pixel_color, 0.5);
                //pixel_color_result += rgba_uint32_t_scalar_multiplication(pixel_color, 0.5);
                //std::cout << "directional coliision\n";

            }
        }
        else if (light.w == 1.0 && relevant_shape_index != -1 && original_object_color != 0) //spotlight - added vector normalization
        {
            glm::vec3 hitpoint = calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance);
            glm::vec3 hitpoint_on_reflected_object;
            //glm::vec3 spotlight_pos;

            //if we're not dealing with a mirror, just work normally, since the hitpoint will be on the end of the ray
            if (relevant_reflection_shape_index == -1)
            {
                //glm::vec3 hitpoint = calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance);
                //spotlight_pos = glm::vec3(spotlight_positions[spotlight_counter].x, spotlight_positions[spotlight_counter].y, spotlight_positions[spotlight_counter].z);
            }
            //if we're hitting a mirror, the rayOrigin and direction will result in a reflected hitpoint, but our goal is to see that the light actually hits the mirror properly first, so we put the hitpoint back on the mirror
            else
            {
                hitpoint_on_reflected_object = hitpoint;
                hitpoint = rayOrigin_on_mirror; // we want to work with the original mirror hitpoint, only if we made sure we hit it properly we can adjust it
                //spotlight_pos = rayOrigin; //this should be the spot on the mirror
            }
            glm::vec3 spotlight_pos = glm::vec3(spotlight_positions[spotlight_counter].x, spotlight_positions[spotlight_counter].y, spotlight_positions[spotlight_counter].z);


            glm::vec3 light_to_surface_vec;

            if (light_source_reflection == 1)
            {
                light_to_surface_vec = (calculate_vector_direction(spotlight_pos, hitpoint));
            }
            else
            {
                if (relevant_reflection_shape_index == -1)
                {
                    light_to_surface_vec = (calculate_vector_direction(spotlight_pos, hitpoint));
                }
                else
                {
                    light_to_surface_vec = (calculate_vector_direction(spotlight_pos, hitpoint_on_reflected_object));
                }
            }





            //unused
            float angle_radians = std::acos(glm::dot(light_to_surface_vec, light_ray) / (glm::length(light_to_surface_vec) * glm::length(light_ray)));
            float angle_degrees = angle_radians * (180.0 / 3.14159);
            //I dunno, I think it makes sense
            //if (angle_degrees < 0)
            //{
            //    angle_degrees = 180 + angle_degrees;
            //}
            //

            //DO NOTE: THIS DOES NOT MAKE SENSE IN ITS CURRENT CURRENT FORM, THE COMPARISON SHOULD GO THE OTHER WAY, OUR COS(a) SHOULD BE SMALLER THAN THE CUTOFF ANGLE, FOR SOME REASON IT PRODUCES THE INVERSE OF THE RESULTS, THIS DOES NOT MAKE ANY SENSE.
            if (glm::dot(glm::normalize(light_to_surface_vec), glm::normalize(light_ray)) > spotlight_positions[spotlight_counter].w) //if our angle is smaller than the cutoff angle, light could theoretically reach us
            {

                // now that we've passed the first "obstacle" (if it's concrete then we're done and we just keep on going, but not for mirrors), we may fire the actual reflected ray.
                if (relevant_reflection_shape_index == -1) //this means we're NOT working with a mirror
                {

                }
                else //we do work with a mirror, so we adjust the points. the hitpoint will be on the reflected object, and the origin will be on the mirror (origin being the lamp position)
                {
                    hitpoint = hitpoint_on_reflected_object;


                    //COMMENT 2: TURN ON THIS CODE LINE TO GET PROPER REFLECTED LIGHTING. FOR SOME REASON THE REFERENCE IMAGE DOES NOT INCLUDE REFLECTED LIGHTING FROM SOURCE AS A FEATURE, WTF
                    if (light_source_reflection == 1)
                    {
                        spotlight_pos = rayOrigin_on_mirror;
                    }
                }

                int potential_intersections;
                if (relevant_reflection_shape_index == -1) // if we're not dealing with a reflection, then we don't compare with only the shape we're on
                {
                    potential_intersections = shapes_input.size() - 1;
                }
                else //otherwise we also don't compare with the shape we're reflecting on, make sure we don't fall below 0 though
                {
                    potential_intersections = shapes_input.size() - 2;
                    if (potential_intersections < 0) //make sure we don't go negative
                    {
                        potential_intersections = 0;
                    }
                }

                for (int i = 0; i < shapes_input.size(); i++) //originally a for each loop, moved from it to accommodate for filtering out an indexed shape 
                {
                    //if this is not a reflection, relevant_reflection_shape will be -1 and won't be relevant either way
                    if (((i != relevant_shape_index && i != relevant_reflection_shape_index) || (shapes_input.size() == 1)) && relevant_shape_index != -1) //don't check intersection with myself. ADDITIONALLY: the i of the distances may be in inverse to the shapes vector, may have to switch them around. 
                    {
                        float distance_to_intersection;
                        if (shapes_input.size() == 1)
                        {
                            distance_to_intersection = shortest_distance;
                        }
                        else
                        {
                            distance_to_intersection = shapes_input[i]->intersection(hitpoint, calculate_vector_direction(hitpoint, spotlight_pos), pixel_color);
                        }

                        if (distance_to_intersection < 0 || distance_to_intersection >= shortest_distance)
                        {
                            potential_intersections--;
                            if (potential_intersections <= 0)
                            {
                                in_shade = 0;

                                //HARD-CODED VARIABLE TO BE REPLACED WITH PROPER INPUT
                                //glm::vec4 spotlight_light_color = glm::vec4(0.2, 0.5, 0.7, 1.0);
                                //glm::vec4 ambient_light_color = glm::vec4(0.1, 0.2, 0.3, 1.0);


                                //POTENTIAL OVERFLOW PROBLEM$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                                //pixel_color_result += PerPixelLight(0, glm::vec3(0, 0, 0), light_sources_directions, image_data_input, original_object_color, eye, from_eye_direction, distances_input, shapes_input, pixel_color, spotlight_positions, ambient_light, relevant_shape_index, hitpoint, light_ray, shortest_distance, spotlight_counter, spotlight_light_color); //ambient_lighting argument means color

                                //POTENTIAL OVERFLOW PROBLEM$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
                                uint32_t spotlight_light_result;

                                spotlight_light_result = PerPixelLight(1, spotlight_pos, light_sources_directions, image_data_input, original_object_color, rayOrigin, rayDirection, distances_input, shapes_input, pixel_color, spotlight_positions, ambient_light, relevant_shape_index, relevant_reflection_shape_index, hitpoint, light_to_surface_vec, shortest_distance, spotlight_counter, light_sources_colors[light_source_index]); //ambient_lighting argument means color
                                //spotlight_light_result = 0;

                                //testing
                                if (spotlight_light_result != 0xff000000 && shapes_input[relevant_shape_index]->get_reflecting_status() == 1)
                                {
                                    //std::cout << "I added light to reflected surface"; 
                                }
                                if (spotlight_light_result == 0xff000000 && shapes_input[relevant_shape_index]->getColor(glm::vec3(0)) != convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 1.0, 1.0, 10.0), 1))
                                {
                                    //std::cout << "I added light to reflected surface"; 
                                }
                                //

                                //uint32_t spotlight_light_result = PerPixelLight(1, spotlight_pos, light_sources_directions, image_data_input, original_object_color, rayOrigin, rayDirection, distances_input, shapes_input, pixel_color, spotlight_positions, ambient_light, relevant_shape_index, hitpoint, light_ray, shortest_distance, spotlight_counter, light_sources_colors[light_source_index]); //ambient_lighting argument means color


                                glm::vec4 pixel_color_result_vector_values = convert_uint32_t_to_vec4_rgba(pixel_color_result, 0);
                                glm::vec4 direct_spotlight_lighting_result_values = convert_uint32_t_to_vec4_rgba(spotlight_light_result, 0);

                                glm::vec4 final_pixel_color_values = glm::vec4(clamp(pixel_color_result_vector_values.x + direct_spotlight_lighting_result_values.x), clamp(pixel_color_result_vector_values.y + direct_spotlight_lighting_result_values.y), clamp(pixel_color_result_vector_values.z + direct_spotlight_lighting_result_values.z), 255);;

                                pixel_color_result = convert_vec4_rgba_to_uint32_t(final_pixel_color_values, 0);




                            }
                        }

                    }
                }
                if (in_shade == 1)
                {
                    //pixel_color = rgba_uint32_t_scalar_multiplication(pixel_color, 0.5);
                    //pixel_color_result += rgba_uint32_t_scalar_multiplication(pixel_color, 0.5);
                    //std::cout << "spotlight collision\n";

                }
            }
            else
            {
                in_shade = 1;
                //pixel_color = rgba_uint32_t_scalar_multiplication(pixel_color,0.5);
                //pixel_color_result += rgba_uint32_t_scalar_multiplication(pixel_color, 0.5);
                //std::cout << "spotlight out of cutoff\n";

            }

            spotlight_counter++; //if we encounter another spotlight, we'll start from that one
        }
        light_source_index++;

    }


    return pixel_color_result;
}


uint32_t PerPixel(glm::vec2 coord, glm::vec3 rayDirection, std::vector<Shape_custom*> shapes_input, std::vector<float>& distances, glm::vec3 rayOrigin) //essentially cherno's raytracing vid #3 as linked in the assignment
{
    //TO_ADD INPUTS:
    //glm::vec3 eye_position

    //uint8_t r = (uint8_t)(coord.x * 255.0f);
    //uint8_t g = (uint8_t)(coord.y * 255.0f);

    // a+bt - ray formula. a = ray origin, b = ray direction , t = hit distance

    //------------TEMPORARY VALUES, WILL LIKELY CHANGE------------------
    std::vector<float> original_distances;
    //glm::vec3 rayDirection(coord.x, coord.y, -rayOrigin.z);
    float radius = 0.5f;
    uint32_t pixel_value = 0; // this will change as it goes through more processes

    for (Shape_custom* shape : shapes_input)
    {

        distances.push_back(shape->intersection(rayOrigin, rayDirection, pixel_value));
        original_distances.push_back(shape->intersection(rayOrigin, rayDirection, pixel_value));

        //i--;

    }







    float shortest_distance = INT_MAX; //arbitrary value, hopefully we don't reach those distances in reality
    int relevant_shape_index = 0;
    for (int i = 0; i < distances.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
    {
        if (distances[i] >= 0 && distances[i] < shortest_distance) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
        {
            shortest_distance = distances[i];
            relevant_shape_index = i;
        }
    }


    if (shortest_distance == INT_MAX) //meaning no intersections whatsoever
    {
        pixel_value = 0;
    }
    else  //we check for if the shortest distance hits a reflective/transparent surface now. do note: in this function we care about the pixel color, not the points/distance
    {
        std::vector<float> reflections_distances;
        int relevant_reflection_shape_index;
        if (shapes_input[relevant_shape_index]->get_reflecting_status() == 1) //meaning it reflects light 
        {
            //find the reflected shape's color
            //checking for reflections now - we only do 1 hop!
            //we do it again now to grab more info, and fire one more ray, lets do it
            glm::vec3 rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distances[relevant_shape_index]);
            glm::vec3 normal_on_mirror = shapes_input[relevant_shape_index]->calculate_normal_direction(rayOrigin_on_mirror);
            glm::vec3 rayDirection_from_mirror = glm::reflect(rayDirection, normal_on_mirror); //may need to negate the normal on mirror


            //we check intersections with all the shapes from our reflected ray, again
            for (int j = 0; j < shapes_input.size(); j++)
            {
                if (relevant_shape_index != j) //dont check against ourselves
                {
                    float reflection_intersection_result = shapes_input[j]->intersection(rayOrigin_on_mirror, rayDirection_from_mirror, pixel_value);

                    reflections_distances.push_back(reflection_intersection_result);
                }
                else
                {
                    reflections_distances.push_back(-1);
                }

            }

            //again, we check for distances, we want to return the color of the closest one
            shortest_distance = INT_MAX; //arbitrary value, hopefully we don't reach those distances in reality
            relevant_reflection_shape_index = 0;
            for (int i = 0; i < reflections_distances.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
            {
                if (reflections_distances[i] != -1 && reflections_distances[i] < shortest_distance) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
                {
                    shortest_distance = reflections_distances[i];
                    relevant_reflection_shape_index = i;
                }
            }
            if (shortest_distance == INT_MAX) //meaning the reflection doesnt hit anything
            {
                pixel_value = 0;
            }
            else //meaning we've hit something, don't care what it is - since we have no recursion currently
            {

                //pixel_value = shapes_input[relevant_reflection_shape_index]->getColor(calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance));

                ////OH WE CARE NOW!
                if (shapes_input[relevant_reflection_shape_index]->get_reflecting_status() != 1) //meaning it's a solid object
                {
                    pixel_value = shapes_input[relevant_reflection_shape_index]->getColor(calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance));
                }
                else if (shapes_input[relevant_reflection_shape_index]->get_reflecting_status() == 1) //mirror again
                {

                    //glm::vec3 rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distances[relevant_shape_index]);
                    //glm::vec3 normal_on_mirror = shapes_input[relevant_shape_index]->calculate_normal_direction(rayOrigin_on_mirror);
                    //glm::vec3 rayDirection_from_mirror = glm::reflect(rayDirection, normal_on_mirror); //may need to negate the normal on mirror

                    //INSERT REFLECTION CODE HERE AGAIN, AND BREAK IT INTO A FUNCTION WHICH WILL CONTINUE BY ITSELF
                    //rayOrigin, rayDirection, & distances, & reflections_distances, relevant_shape_index, & shapes_input, pixel_value, shortest_distance
                    int iterations = 1; //i did one reflection previously, finishing up the rest
                    pixel_value = 1;
                    int prev_relevant_shape_index = relevant_shape_index;
                    int relevant_re_reflection_shape_index = relevant_reflection_shape_index;
                    //int prev_prev_relevant_shape_index = relevant_shape_index; //for 3 collision in a tight spot

                    while (pixel_value != 0 && shapes_input[relevant_re_reflection_shape_index]->get_reflecting_status() == 1 && iterations < 5)
                    {
                        rayOrigin_on_mirror = calculate_hitpoint_from_distance(rayOrigin_on_mirror, rayDirection_from_mirror, shortest_distance);
                        normal_on_mirror = shapes_input[relevant_reflection_shape_index]->calculate_normal_direction(rayOrigin_on_mirror);
                        //if(shortest_distance < 0)
                        //{
                        //    normal_on_mirror = -normal_on_mirror;
                        //}

                        rayDirection_from_mirror = glm::reflect(rayDirection_from_mirror, normal_on_mirror); //from original mirror to new one
                        //int previous_relevant_reflection_shape_index = relevant_reflection_shape_index;
                        uint32_t next_object_color = do_the_intersection_thing_again_re_check(rayOrigin_on_mirror, rayDirection_from_mirror, distances, reflections_distances, relevant_re_reflection_shape_index, relevant_reflection_shape_index, shapes_input, pixel_value, shortest_distance, prev_relevant_shape_index);
                        pixel_value = next_object_color;
                        iterations++;

                    }
                    if (iterations >= 5)
                    {
                        pixel_value = 0;
                    }

                }
                else if ((shapes_input[relevant_reflection_shape_index]->get_transparency_status() == 1)) //meaning it's a transparent object - NOT DEALING WITH IT FOR NOW
                {
                    //NOT YET WORRYING ABOUT THIS ONE
                }
            }

        }
        else if (relevant_shape_index != -1 && shapes_input[relevant_shape_index]->get_transparency_status() == 1) //DO NOTE: THIS SHOULD WORK FOR A GLASS BALL, IT MAY FAIL WITH A POINT, AS WELL AS WITH A PLANE, I'M NOT TESTING FOR THAT FOR NOW
        {
            //meaning we are transparent, and thus continue the calculation from that point
            float distance_to_glass = shortest_distance;
            glm::vec3 entry_glass_hitpoint = calculate_hitpoint_from_distance(rayOrigin, rayDirection, distance_to_glass);


            //now we calculate a new vector inside the glass until we reach the end of said shape
            glm::vec3 glass_normal = -shapes_input[relevant_shape_index]->calculate_normal_direction(entry_glass_hitpoint);

            //NOTE: I add a tiny bit of distance to my hitpoint, so as to move a bit from the ball's surface, doing so will let me calculate the next intersection organically, without rewriting the intersection code 
            // (it won't return me a 0 is what I'm saying)
            entry_glass_hitpoint = glm::vec3(entry_glass_hitpoint.x - glass_normal.x * 0.001, entry_glass_hitpoint.y - glass_normal.y * 0.001, entry_glass_hitpoint.z - glass_normal.z * 0.001);

            glm::vec3 glass_normal_normalized = glm::normalize(glass_normal);
            glm::vec3 rayDirection_normalized = glm::normalize(rayDirection);

            //float angle_hit = acos(glm::dot(glass_normal_normalized, rayDirection_normalized)); //shift-cosinus on the dot product of two normalized vectors to get the angle value (in radians)

            //now we calculate the vector direction coordinates inside of the sphere

            glm::vec3 rayDirection_in_glass = glm::refract(rayDirection_normalized, glass_normal_normalized, 1.0f / 1.5f);

            //glm::vec3 rayDirection_in_glass = glm::refract(rayDirection_normalized, glass_normal_normalized, 0.75f / 1.5f);
            //glm::vec3 rayDirection_in_glass = manual_refraction(rayDirection, -glass_normal, 1.5f);


            //if (glm::length(rayDirection) > 1)
            //{
            //    rayDirection_in_glass = rayDirection_in_glass * glm::length(rayDirection);
            //}

            //rayDirection_in_glass = rayDirection_in_glass * 10.0f;
            std::vector<float> distances_after_refraction;

            glm::vec3 exit_glass_hitpoint;
            glm::vec3 exit_glass_normal;
            glm::vec3 exit_refraction_direction;
            if (rayDirection_in_glass != glm::vec3(0, 0, 0))
            {
                float distance_inside_glass = shapes_input[relevant_shape_index]->intersection(entry_glass_hitpoint, rayDirection_in_glass, pixel_value); //finding the intersection distance inside the glass object
                exit_glass_hitpoint = calculate_hitpoint_from_distance(entry_glass_hitpoint, rayDirection_in_glass, distance_inside_glass);


                exit_glass_normal = shapes_input[relevant_shape_index]->calculate_normal_direction(exit_glass_hitpoint);
                exit_glass_hitpoint = glm::vec3(exit_glass_hitpoint.x + glass_normal.x * 0.001, exit_glass_hitpoint.y + glass_normal.y * 0.001, exit_glass_hitpoint.z + glass_normal.z * 0.001);

                //exit_refraction_direction = glm::refract((glm::normalize(rayDirection_in_glass)), glm::normalize(exit_glass_normal), 1.0f);

                //exit_refraction_direction = manual_refraction(rayDirection_in_glass, -exit_glass_normal, 1.0f);

                exit_refraction_direction = glm::refract(glm::normalize(rayDirection_in_glass), glm::normalize(exit_glass_normal), 1.0f / 1.0f);



                //if we have an actual vector, we can fire a new ray from the exit hitpoint
                for (Shape_custom* shape : shapes_input)
                {

                    float refraction_cast_result = shape->intersection(exit_glass_hitpoint, exit_refraction_direction, pixel_value); //rayDirection WILL CHANGE NOW
                    if (refraction_cast_result != -1)
                    {
                        //distances_after_refraction.push_back(shortest_distance + distance_inside_glass + refraction_cast_result);
                        distances_after_refraction.push_back(refraction_cast_result);
                    }
                    else
                    {
                        distances_after_refraction.push_back(-1);
                    }

                }
            }
            else
            {
                for (int j = 0; j < shapes_input.size(); j++)
                {
                    //otherwise we pretend the hit was never attempted, and we continue from the original hitpoint, ignoring the ball
                    if (j != relevant_shape_index)
                    {
                        distances_after_refraction.push_back(shapes_input[j]->intersection(rayOrigin, rayDirection, pixel_value));
                    }
                    else //since we ignore the object we hit, it becomes invisible to us
                    {
                        distances_after_refraction.push_back(-1);
                    }

                }
                exit_glass_hitpoint = rayDirection_in_glass;
            }


            //fire a new ray from that point



            float refracted_shortest_distance = INT_MAX;
            int refracted_relevant_shape = -1;
            for (int j = 0; j < distances_after_refraction.size(); j++)
            {
                if (distances_after_refraction[j] < refracted_shortest_distance && distances_after_refraction[j] > 0 && j != relevant_shape_index)
                {
                    refracted_shortest_distance = distances_after_refraction[j];
                    refracted_relevant_shape = j;
                }
            }
            if (refracted_shortest_distance == INT_MAX)
            {
                pixel_value = 0;
            }
            else
            {
                glm::vec3 final_hitpoint = calculate_hitpoint_from_distance(exit_glass_hitpoint, exit_refraction_direction, refracted_shortest_distance);
                glm::vec3 final_vector_direction = calculate_vector_direction(exit_glass_hitpoint, final_hitpoint);
                pixel_value = shapes_input[refracted_relevant_shape]->getColor(calculate_hitpoint_from_distance(final_hitpoint, final_vector_direction, refracted_shortest_distance));
            }



        }
        else //just return our color
        {
            pixel_value = shapes_input[relevant_shape_index]->getColor(calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance)); //the index reverse is because we work in stacks, but calculate chronologically. I might flip it over to sync them properly, but it works so eh, may change later
        }
    }


    //now we decide what color we bring back, let's take the smallest distance that is higher than 0, if one exists
    //float shortest_distance = INT_MAX; //arbitrary value, hopefully we don't reach those distances in reality
    //for (int i = 0; i < distances.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
    //{
    //    if (distances[i] >= 0 && distances[i] < shortest_distance) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
    //    {
    //        shortest_distance = distances[i];
    //        pixel_value = shapes_input[i]->getColor(calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance)); //the index reverse is because we work in stacks, but calculate chronologically. I might flip it over to sync them properly, but it works so eh, may change later
    //    }
    //}
    //if (shortest_distance == INT_MAX) //meaning no intersections whatsoever
    //{
    //    pixel_value = 0; //this doesn't do anything for now but eh
    //}




    distances = original_distances;
    return pixel_value;
}

Texture::Texture(int width, int height) //added by me
{

    //SCENE 5
    //std::vector<Shape_custom*> shapes;
    //std::vector<int> shapes_reflecting_statuses;
    //std::vector<float> distances; //this will include distance for each hit, -1 if non-existent
    //uint32_t sphere_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.6, 0.0, 0.8, 10.0), 1);
    //uint32_t sphere_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(1, 0, 0, 10.0), 1);
    //uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 1.0, 1.0, 10.0), 1);

    //Sphere* sphere_1 = new Sphere(0.5, glm::vec3(-0.7, 0.7, -2.0), sphere_1_color,0);
    //Sphere* sphere_2 = new Sphere(0.5, glm::vec3(0.6, 0.5, -1.0), sphere_2_color,0);
    //Plane* plane_1 = new Plane(glm::vec4(0, -0.5, -1.0, -3.5), plane_1_color,1);
    //glm::vec3 rayOrigin_original(0.0f, 0.0f, 4.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    //std::vector<glm::vec4> light_sources_original;
    //std::vector<glm::vec4> light_sources_colors;
    //std::vector<glm::vec4> spotlight_positions;
    //glm::vec3 rayDirection_original; //from eye to screen
    //glm::vec4 ambientLight_original = glm::vec4(0.1, 0.2, 0.3, 1.0);

    //light_sources_original.push_back(glm::vec4(0.5, 0.0, -1.0, 1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(0.2, 0.5, 0.7, 1.0)); //spot light color


    //light_sources_original.push_back(glm::vec4(0.0, 0.5, -1.0, 0.0)); //directional light
    //light_sources_colors.push_back(glm::vec4(0.7, 0.5, 0.0, 1.0)); //directional light color

    //spotlight_positions.push_back(glm::vec4(2.0, 1.0, 3.0, 0.6)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg

    //shapes.push_back(sphere_1);
    //shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
    //shapes.push_back(sphere_2);
    //shapes_reflecting_statuses.push_back(sphere_2->get_reflecting_status());
    //shapes.push_back(plane_1);
    //shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());

    //SCENE 1

    //std::vector<Shape_custom*> shapes;
    //std::vector<int> shapes_reflecting_statuses;
    //std::vector<float> distances; //this will include distance for each hit, -1 if non-existent
    //uint32_t sphere_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.6, 0.0, 0.8, 10.0), 1);
    //uint32_t sphere_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(1, 0, 0, 10.0), 1);
    //uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 1.0, 1.0, 10.0), 1);

    //Sphere* sphere_1 = new Sphere(0.5, glm::vec3(-0.7, -0.7, -2.0), sphere_1_color, 0,10.0);
    //Sphere* sphere_2 = new Sphere(0.5, glm::vec3(0.6, -0.5, -1.0), sphere_2_color, 0,10.0);
    //Plane* plane_1 = new Plane(glm::vec4(0, -0.5, -1.0, -3.5), plane_1_color, 0,10.0);
    //glm::vec3 rayOrigin_original(0.0f, 0.0f, 4.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    //std::vector<glm::vec4> light_sources_original;
    //std::vector<glm::vec4> light_sources_colors;
    //std::vector<glm::vec4> spotlight_positions;
    //glm::vec3 rayDirection_original; //from eye to screen
    //glm::vec4 ambientLight_original = glm::vec4(0.1, 0.2, 0.3, 1.0);

    //light_sources_original.push_back(glm::vec4(0.5, 0.0, -1.0, 1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(0.2, 0.5, 0.7, 1.0)); //spot light color
    //spotlight_positions.push_back(glm::vec4(2.0, 1.0, 3.0, 0.6)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg

    //light_sources_original.push_back(glm::vec4(0.0, 0.5, -1.0, 0.0)); //directional light
    //light_sources_colors.push_back(glm::vec4(0.7, 0.5, 0.0, 1.0)); //directional light color

    //shapes.push_back(sphere_1);
    //shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
    //shapes.push_back(sphere_2);
    //shapes_reflecting_statuses.push_back(sphere_2->get_reflecting_status());
    //shapes.push_back(plane_1);
    //shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());


    //SCENE 2 - HALL OF MIRRORS

    //std::vector<Shape_custom*> shapes;
    //std::vector<int> shapes_reflecting_statuses;
    //std::vector<float> distances; //this will include distance for each hit, -1 if non-existent
    //uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.6,0.0,0.8,20.0), 1);
    //uint32_t plane_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.7,0.7,0.0,10.0), 1);
    //uint32_t plane_3_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0,0.9,0.0,15.0), 1);
    //uint32_t plane_4_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 0.8, 0.8, 10.0), 1);
    //uint32_t sphere_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(1.0,0.0,0.0,15.0), 1);
    //uint32_t sphere_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0,1.0,0.8,10.0), 1);
    //uint32_t plane_5_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.9,0.0,0.1,15.0), 1);
    //
    //


    //Plane* plane_1 = new Plane(glm::vec4(1.0,0.0,-0.1,-3.0), plane_1_color, 1, 20.0);
    //Plane* plane_2 = new Plane(glm::vec4(0.0,0.0,-1.0,-3.5), plane_2_color, 1, 10.0);
    //Plane* plane_3 = new Plane(glm::vec4(-1.0,0.0,-0.1,-3.0), plane_3_color, 1, 15.0);
    //Plane* plane_4 = new Plane(glm::vec4(0.0,1.0, -0.1, -3.0), plane_4_color, 1, 10.0);
    //Sphere* sphere_1 = new Sphere(1.0,glm::vec3(-0.7,0.7,-2.0), sphere_1_color, 0, 15.0);
    //Sphere* sphere_2 = new Sphere(0.7,glm::vec3(0.8,-0.5,-1.0), sphere_2_color, 0, 10.0);
    //Plane* plane_5 = new Plane(glm::vec4(0.0,-1.0,-0.1,-3.0), plane_5_color, 1, 15.0);
    //
    //
    //glm::vec3 rayOrigin_original(0.0f, 0.0f, 1.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    //std::vector<glm::vec4> light_sources_original;
    //std::vector<glm::vec4> light_sources_colors;
    //std::vector<glm::vec4> spotlight_positions;
    //glm::vec3 rayDirection_original; //from eye to screen
    //glm::vec4 ambientLight_original = glm::vec4(0.1,0.2,0.3,1.0);

    //light_sources_original.push_back(glm::vec4(0.0,0.5,-1.0,1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(0.3,0.9,0.2,1.0)); //spot light color
    //spotlight_positions.push_back(glm::vec4(0.0,0.0,0.0,0.8)); //spotlight position

    //light_sources_original.push_back(glm::vec4(0.5,0.0,-1.0,1.0)); //spotlight light
    //light_sources_colors.push_back(glm::vec4(0.8,0.5,0.7,1.0)); //spotlight light color
    //spotlight_positions.push_back(glm::vec4(0.0,0.0,0.0,0.9)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg


    //light_sources_original.push_back(glm::vec4(-0.4,-0.3,-1.0,1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(0.8,0.5,0.7,1.0)); //spot light color
    //spotlight_positions.push_back(glm::vec4(-0.2,0.0,0.0,0.7)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg

    ////light_sources_original.push_back(glm::vec4(0.3,0.5,-1.0,0.0)); //directional light
    ////light_sources_colors.push_back(glm::vec4(0.7,0.8,0.3,1.0)); //directional light color

    //shapes.push_back(plane_1);
    //shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());
    //shapes.push_back(plane_2);
    //shapes_reflecting_statuses.push_back(plane_2->get_reflecting_status());
    //shapes.push_back(plane_3);
    //shapes_reflecting_statuses.push_back(plane_3->get_reflecting_status());
    //shapes.push_back(plane_4);
    //shapes_reflecting_statuses.push_back(plane_4->get_reflecting_status());
    //shapes.push_back(sphere_1);
    //shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
    //shapes.push_back(sphere_2);
    //shapes_reflecting_statuses.push_back(sphere_2->get_reflecting_status());
    //shapes.push_back(plane_5);
    //shapes_reflecting_statuses.push_back(plane_5->get_reflecting_status());



    //SCENE 3

    //std::vector<Shape_custom*> shapes;
    //std::vector<int> shapes_reflecting_statuses;
    //std::vector<float> distances; //this will include distance for each hit, -1 if non-existent
    //uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.6,0.0,0.8,20.0), 1);
    //uint32_t plane_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.7,0.7,0.0,10.0), 1);
    //uint32_t plane_3_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0,0.9,0.5,15.0), 1);
    //uint32_t plane_4_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0,0.8,0.8,10.0), 1);
    //uint32_t plane_5_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.9,0.0,0.1,15.0), 1);


    //Plane* plane_1 = new Plane(glm::vec4(1.0,0.0,-0.1,-3.0), plane_1_color, 0, 20.0);
    //Plane* plane_2 = new Plane(glm::vec4(0.0,0.0,-1.0,-3.5), plane_2_color, 0, 10.0);
    //Plane* plane_3 = new Plane(glm::vec4(-1.0,0.0,-0.1,-3.0), plane_3_color, 0, 15.0);
    //Plane* plane_4 = new Plane(glm::vec4(0.0,1.0,-0.1,-3.0), plane_4_color, 0, 10.0);
    //Plane* plane_5 = new Plane(glm::vec4(0.0,-1.0,-0.1,-3.0), plane_5_color, 0, 15.0);
    //glm::vec3 rayOrigin_original(0.0f, 0.0f, 1.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    //std::vector<glm::vec4> light_sources_original;
    //std::vector<glm::vec4> light_sources_colors;
    //std::vector<glm::vec4> spotlight_positions;
    //glm::vec3 rayDirection_original; //from eye to screen
    //glm::vec4 ambientLight_original = glm::vec4(0.2,0.1,0.0,1.0);

    //light_sources_original.push_back(glm::vec4(0.0,0.5,-1.0,1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(0.3,0.9,0.2,1.0)); //spot light color
    //spotlight_positions.push_back(glm::vec4(0.0,0.0,0.0,0.8)); //spotlight position

    //light_sources_original.push_back(glm::vec4(0.5,0.0,-1.0,1.0)); //directional light
    //light_sources_colors.push_back(glm::vec4(0.9,0.5,0.5,1.0)); //directional light color
    //spotlight_positions.push_back(glm::vec4(0.0,0.0,0.0,0.9)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg


    //light_sources_original.push_back(glm::vec4(-0.4,-0.3,-1.0,1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(0.2,0.5,0.7,1.0)); //spot light color
    //spotlight_positions.push_back(glm::vec4(-0.2,0.0,0.0,0.7)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg

    //shapes.push_back(plane_1);
    //shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());
    //shapes.push_back(plane_2);
    //shapes_reflecting_statuses.push_back(plane_2->get_reflecting_status());
    //shapes.push_back(plane_3);
    //shapes_reflecting_statuses.push_back(plane_3->get_reflecting_status());
    //shapes.push_back(plane_4);
    //shapes_reflecting_statuses.push_back(plane_4->get_reflecting_status());
    //shapes.push_back(plane_5);
    //shapes_reflecting_statuses.push_back(plane_5->get_reflecting_status());
    // 
    // 


    //SCENE 4

    //std::vector<Shape_custom*> shapes;
    //std::vector<int> shapes_reflecting_statuses;
    //std::vector<float> distances; //this will include distance for each hit, -1 if non-existent
    //
    //uint32_t Sphere_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0,1.0,1.0,10.0), 1);
    //uint32_t Sphere_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(1.0,0.0,0.0,20.0), 1);
    //uint32_t Sphere_3_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.6,0.0,0.8,15.0), 1);
    //uint32_t Sphere_4_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.9,0.0,0.0,10.0), 1);
    //uint32_t Sphere_5_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0,0.0,0.8,10.0), 1);
    //uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.7,0.7,0.0,10.0), 1);


    //Sphere* Sphere_1 = new Sphere(0.3, glm::vec3(0.0, -0.5, -1.0), Sphere_1_color, 0, 10.0);
    //Sphere* Sphere_2 = new Sphere(0.7,glm::vec3(-1.7,-0.7,-3.0), Sphere_2_color, 0, 20.0);
    //Sphere* Sphere_3 = new Sphere(2.5, glm::vec3(2.6,-1.5,-10.0 ), Sphere_3_color, 0, 15.0);
    //Sphere* Sphere_4 = new Sphere(1.5, glm::vec3(1.3,1.5,-7.0), Sphere_4_color, 0, 10.0);
    //Sphere* Sphere_5 = new Sphere(1.0, glm::vec3(-0.6,-0.5,-5.0), Sphere_5_color, 0, 10.0);
    //Plane* plane_1 = new Plane(glm::vec4(0.0,-1.0,-1.0,-8.5), plane_1_color, 0, 10.0);

    //glm::vec3 rayOrigin_original(0.0,0.0,3.0); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    //std::vector<glm::vec4> light_sources_original;
    //std::vector<glm::vec4> light_sources_colors;
    //std::vector<glm::vec4> spotlight_positions;
    //glm::vec3 rayDirection_original; //from eye to screen
    //glm::vec4 ambientLight_original = glm::vec4(0.2,0.2,0.3,1.0);


    //light_sources_original.push_back(glm::vec4(0.0, -0.7, -1.0, 0.0)); //directional light
    //light_sources_colors.push_back(glm::vec4(0.9, 0.5, 0.0, 1.0)); //directional light color


    //light_sources_original.push_back(glm::vec4(1.5, 0.9, -1.0, 1.0)); //spot light
    //light_sources_colors.push_back(glm::vec4(1.0, 1.0, 1.0, 1.0)); //spot light color
    //spotlight_positions.push_back(glm::vec4(-2.0, -1.0, 3.0, 0.6)); //spotlight position


    //shapes.push_back(Sphere_1);
    //shapes_reflecting_statuses.push_back(Sphere_1->get_reflecting_status());

    //shapes.push_back(Sphere_2);
    //shapes_reflecting_statuses.push_back(Sphere_2->get_reflecting_status());
    //shapes.push_back(Sphere_3);
    //shapes_reflecting_statuses.push_back(Sphere_3->get_reflecting_status());
    //shapes.push_back(Sphere_4);
    //shapes_reflecting_statuses.push_back(Sphere_4->get_reflecting_status());

    //shapes.push_back(Sphere_5);
    //shapes_reflecting_statuses.push_back(Sphere_5->get_reflecting_status());
    //shapes.push_back(plane_1);
    //shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());


    //CUSTOM SCENE

    /*std::vector<Shape_custom*> shapes;
    std::vector<int> shapes_reflecting_statuses;
    std::vector<float> distances; //this will include distance for each hit, -1 if non-existent

    uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 1.0, 1.0, 10.0), 1);
    uint32_t sphere_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(1.0,0.0,0.0,10.0), 1);
    uint32_t sphere_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.6, 0.0, 0.8, 10.0), 1);


    Plane* plane_1 = new Plane(glm::vec4(0, -0.5, -1.0, -3.5), plane_1_color, 0, 10.0);
    Sphere* sphere_1 = new Sphere(0.3, glm::vec3(-0.4,0.0,0.0), sphere_1_color, 0,10.0,1);
    Sphere* sphere_2 = new Sphere(0.5, glm::vec3(0.5,0.0,-2.0), sphere_2_color, 0,10.0);

    glm::vec3 rayOrigin_original(0.0f, 0.0f, 4.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    std::vector<glm::vec4> light_sources_original;
    std::vector<glm::vec4> light_sources_colors;
    std::vector<glm::vec4> spotlight_positions;
    glm::vec3 rayDirection_original; //from eye to screen
    glm::vec4 ambientLight_original = glm::vec4(0.1, 0.2, 0.3, 1.0);

    light_sources_original.push_back(glm::vec4(0.5, 0.0, -1.0, 1.0)); //spot light
    light_sources_colors.push_back(glm::vec4(0.2, 0.5, 0.7, 1.0)); //spot light color
    spotlight_positions.push_back(glm::vec4(2.0, 1.0, 3.0, 0.6)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg

    light_sources_original.push_back(glm::vec4(0.0, 0.5, -1.0, 0.0)); //directional light
    light_sources_colors.push_back(glm::vec4(0.7, 0.5, 0.0, 1.0)); //directional light color

    shapes.push_back(sphere_1);
    shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
    shapes.push_back(sphere_2);
    shapes_reflecting_statuses.push_back(sphere_2->get_reflecting_status());
    shapes.push_back(plane_1);
    shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());*/

    std::vector<Shape_custom*> shapes;
    std::vector<int> shapes_reflecting_statuses;
    std::vector<float> distances;
    glm::vec3 rayOrigin_original; //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    std::vector<glm::vec4> light_sources_original;
    std::vector<glm::vec4> light_sources_colors;
    std::vector<glm::vec4> spotlight_positions;
    glm::vec3 rayDirection_original; //from eye to screen
    glm::vec4 ambientLight_original;

    std::ifstream in("../res/scenes/scene1.txt", std::ios::in);

    std::string line;
    std::vector<std::string> temp_coords;

    int colorCounter = 0;


    while (std::getline(in, line)) { //while didn't reach EOF
        // EYE VEC
        if (line.at(0) == 'e') {
            std::string e(line.substr(2)); //take eye coordinates
            temp_coords = split(e, " ");
            glm::vec4 eye;
            eye = set_coords(temp_coords);
            rayOrigin_original = glm::vec3(eye.x, eye.y, eye.z);
        }
        // AMBIENT VEC
        else if (line.at(0) == 'a') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 ambient;
            ambientLight_original = set_coords(temp_coords);
        }
        // OBJECT VEC
        else if (line.at(0) == 'o') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 object;
            object = set_coords(temp_coords);
            if (object[3] > 0) {
                glm::vec3 center = glm::vec3(object.x, object.y, object.z);
                Sphere* sphere_1 = new Sphere(object.w, center);
                shapes.push_back(sphere_1);
                shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
            }
            else {
                Plane* plane_1 = new Plane(object);
                shapes.push_back(plane_1);
                shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());
            }
        }
        // COLOR OF OBJECT VEC
        else if (line.at(0) == 'c') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 color;
            color = set_coords(temp_coords);
            Shape_custom* curr = shapes[colorCounter];
            uint32_t color1 = convert_vec4_rgba_to_uint32_t(color, 1);
            curr->setColor(color1);
            curr->setShininess(color.w);


            colorCounter++;
        }
        // DIRECT LIGHT VEC
        else if (line.at(0) == 'd') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 direct;
            direct = set_coords(temp_coords);
            light_sources_original.push_back(direct);

        }
        // SPOTLIGHT VEC
        else if (line.at(0) == 'p') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 position;
            position = set_coords(temp_coords);
            spotlight_positions.push_back(position);

        }
        // LIGHT INTENSITY VEC
        else if (line.at(0) == 'i') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 intensity;
            intensity = set_coords(temp_coords);
            light_sources_colors.push_back(intensity);
        }
        // REFLECTIVE OBJECT VEC
        else if (line.at(0) == 'r') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 reflective;
            reflective = set_coords(temp_coords);

            if (reflective[3] > 0) {
                glm::vec3 center = glm::vec3(reflective.x, reflective.y, reflective.z);
                Sphere* sphere_1 = new Sphere(center, reflective.w, 1, 0);
                shapes.push_back(sphere_1);
                shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
            }
            else {
                Plane* plane_1 = new Plane(1, 0, reflective);
                shapes.push_back(plane_1);
                shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());
            }
        }
        // TRANSPARENT OBJECT VEC
        else if (line.at(0) == 't') {
            std::string a(line.substr(2)); //take ambient coordinates
            temp_coords = split(a, " ");
            glm::vec4 transparent;
            transparent = set_coords(temp_coords);
            if (transparent[3] > 0) {
                glm::vec3 center = glm::vec3(transparent.x, transparent.y, transparent.z);
                Sphere* sphere_1 = new Sphere(center, transparent.w, 0, 1);
                shapes.push_back(sphere_1);
                shapes_reflecting_statuses.push_back(sphere_1->get_reflecting_status());
            }
            else {
                Plane* plane_1 = new Plane(0, 1, transparent);
                shapes.push_back(plane_1);
                shapes_reflecting_statuses.push_back(plane_1->get_reflecting_status());
            }
        }

    }
    in.close();




    uint32_t* image_data = new uint32_t[width * height]; //right-to-left -> 2bytes(R) -> 2bytes(G) -> 2bytes(B) -> 2bytes(alpha) -> 0x meaning we write in hexadecimal
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {

            glm::vec2 coord =
            {
                (float)x / (float)width , 1.0f - (float)y / (float)height  //do note that I have inverted the y axis
            };
            coord = coord * 2.0f - 1.0f; //normalizing the coordinates from -1 to 1
            rayDirection_original = glm::vec3(coord.x, coord.y, -rayOrigin_original.z);


            //this gets us the original pixel color, with no lighting/shadows, anything like that
            image_data[x + y * width] = PerPixel(coord, rayDirection_original, shapes, distances, rayOrigin_original);


            //we save a backup of the original pixel color
            uint32_t original_object_color = image_data[x + y * width];



            //if(y > 0 && image_data[(x-2) + y * width] == 0 && image_data[x + (y-1) * width] == 0 && image_data[(x - 1) + y * width] == 4278190335 && image_data[x + y * width] == 0)
            //{
            //    for (float dist : distances)
            //    {
            //        distances.pop_back();
            //    }
            //    glm::vec2 repeat_coord =
            //    {
            //        (float)(x-1) / (float)width , 1.0f - (float)y / (float)height  //do note that I have inverted the y axis
            //    };
            //    repeat_coord = repeat_coord * 2.0f - 1.0f; //normalizing the coordinates from -1 to 1
            //    glm::vec3 rayDirection_repeat_original = glm::vec3(repeat_coord.x, repeat_coord.y, -rayOrigin_original.z);


            //    uint32_t test_result = PerPixel(repeat_coord, rayDirection_repeat_original, shapes, distances, rayOrigin_original);
            //    std::cout << "aaa";
            //
            //}









            //we apply ambient lighting to the original pixel color (meaning we make it dimmer, as the only lighting parameter for now is ambient)
            image_data[x + y * width] = convert_vec4_rgba_to_uint32_t(multiply_vector_by_vec_scalars(convert_uint32_t_to_vec4_rgba(image_data[x + y * width], 0), ambientLight_original), 0); //ambient lighting calculation
            uint32_t pixel_color_after_ambient = image_data[x + y * width];
            glm::vec4 pixel_color_after_ambient_vector = convert_uint32_t_to_vec4_rgba(image_data[x + y * width], 0);
            //we now look to apply all the other light parameters

            image_data[x + y * width] = PerPixelShadow(light_sources_original, light_sources_colors, image_data, original_object_color, rayOrigin_original, rayDirection_original, distances, shapes, image_data[x + y * width], spotlight_positions, ambientLight_original);
            uint32_t pixel_color_final = image_data[x + y * width];
            glm::vec4 pixel_color_final_vector = convert_uint32_t_to_vec4_rgba(image_data[x + y * width], 0);

            if (pixel_color_final == 4278190335 && original_object_color == 0)
            {
                for (float dist : distances)
                {
                    distances.pop_back();
                }
                image_data[x + y * width] = PerPixel(coord, rayDirection_original, shapes, distances, rayOrigin_original);
                image_data[x + y * width] = PerPixelShadow(light_sources_original, light_sources_colors, image_data, original_object_color, rayOrigin_original, rayDirection_original, distances, shapes, image_data[x + y * width], spotlight_positions, ambientLight_original);

                std::cout << "penis" << std::endl;
            }


            if (pixel_color_after_ambient_vector.x > pixel_color_final_vector.x)
            {
                std::cout << "BUG!" << std::endl;
                std::cout << "after_ambient: " << pixel_color_after_ambient_vector.x << std::endl;
                std::cout << "final: " << pixel_color_final_vector.x << std::endl;
            }
            //before we end this loop, we ought to pop the distances back out
            for (float dist : distances)
            {
                distances.pop_back();
            }


        }
    }

    //antialiasing?
    //for (int y = 1; y < height-1; y++)
    //{
    //    for (int x = 1; x < width-1; x++)
    //    {
    //        glm::vec4 one = convert_uint32_t_to_vec4_rgba(image_data[(x - 1) + y * width], 0);
    //        glm::vec4 two = convert_uint32_t_to_vec4_rgba(image_data[x + (y + 1) * width], 0);
    //        glm::vec4 three = convert_uint32_t_to_vec4_rgba(image_data[(x - 1) + (y - 1) * width], 0);
    //        glm::vec4 four = convert_uint32_t_to_vec4_rgba(image_data[(x + 1) + (y + 1) * width], 0);
    //        glm::vec4 init = convert_uint32_t_to_vec4_rgba(image_data[x + y * width], 0);

    //        init = (one + two + three + four) / 4.0f;
    //        init = (glm::vec4(init.x, init.y, init.z, 255));
    //        uint32_t result = convert_vec4_rgba_to_uint32_t(init, 0);
    //        image_data[x + y * width] = result;

    //    }
    //}


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

























//IRRELEVANT CODE FROM THIS POINT












Texture::Texture(const std::string& fileName)
{
    int width, height, numComponents;
    unsigned char* data = stbi_load((fileName).c_str(), &width, &height, &numComponents, 4);

    if (data == NULL)
        std::cerr << "Unable to load texture: " << fileName << std::endl;

    glGenTextures(1, &m_texture);
    Bind(m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -0.4f);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
}

Texture::Texture(int width, int height, unsigned char* data)
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




////------------------------------------------------------------------------------------------DEPRECATED CODE------------------------------------------------------------------------------------------
//
////sphere intersection func
////this return the color of the sphere, assuming we hit it, if not, we return 0.
////NOTE: the return process is somewhat convoluted, because when we start checking for both intersections with transparent objects, I might have to restructure the return type, for now it stays like this.
////uint32_t sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 sphere_center, float sphere_radius, uint32_t sphere_color, uint32_t original_color) //if -1, no hits
////{
////    //line vector: x = ray_origin.x + (ray_direction.x * t) , y = ray_origin.y + (ray_direction.y * t) , z = ray_origin.z + (ray_direction.z * t)
////    //( x - a )^2 + (y - b)^2 + (z - c)^2 - r^2 = 0
////    // (x^2 - 2xa + a^2) + (y^2 - 2yb + b^2) + (z^2 - 2zc + c^2) - r^2 = 0
////    //  ( (x_0 + d_x * t)^2 - 2(x_0 + d_x * t)*a + (a^2) ) + ( (y_0 + d_y * t)^2 - 2(y_0 + d_y * t)*b + b^2 ) + ( (z_0 + d_z * t)^2 - 2(z_0 + d_z * t) * c + c^2 ) - r^2 = 0
////    //  ... grouping by t (which will appear as sol_1,sol_2 upon solving in func)
////    // A + B*t + C*t^2 = 0, 
////    float a = glm::dot(ray_direction, ray_direction); //(dirx^2+diry^2+dirz^2)
////    float b = 2 * ray_origin.x * ray_direction.x - 2 * ray_direction.x * sphere_center.x + 2 * ray_origin.y * ray_direction.y - 2 * ray_direction.y * sphere_center.y + 2 * ray_origin.z * ray_direction.z - 2 * sphere_center.z; //(2*p0x*dirx-2*dirx*cx+2*p0y*diry-2*diry*cy+2*p0z*dirz-2*dirz*cz)
////    float c = glm::dot(ray_origin, ray_origin) + glm::dot(sphere_center, sphere_center) - pow(sphere_radius,2); //p0x^2+p0y^2+p0z^2+cx^2+cy^2+cz^2-R^2
////
////    //Quatratic formula discriminant:
////    // b^2 - 4ac
////
////    float discriminant = b * b - 4.0f * a * c; //calculating ray intersection with sphere
////    if (discriminant > 0.0f)
////    {
////        //so if this happens, I have 2 intersections to return, but temporarily I will ignore the 2nd one, as we're not talking transparent objects yet. It will likely come back though, for now I return the smaller t
////        //float sol_1 = (- b + sqrt(discriminant))/2*a;
////        //float sol_2 = (- b - sqrt(discriminant))/2*a;
////        float sol_1 = (-b + sqrt(discriminant)) / (2 * a);
////        float sol_2 = (-b - sqrt(discriminant)) / (2 * a);
////       
////        //return the smaller one, as long as it is positive. otherwise, return -1, which means no intersection
////        if (sol_1 >= 0 || sol_2 >= 0) 
////        {
////            return sphere_color;
////        }
////        else 
////        {
////            return original_color;
////        }
////        //return 0xffff00ff; //returning the spheres colour, temporarily hardcoded
////    }
////    else if (discriminant == 0.0f) 
////    {
////        float sol_1 = -b / (2 * a);
////        return sol_1 >= 0 ? sphere_color : original_color;
////
////    }
////    else //no intersection
////    {
////        return original_color;
////    }
////    
////    // 
////    
////}
//
//uint32_t sphere_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 sphere_center, float sphere_radius, uint32_t sphere_color, uint32_t original_color)
//{
//    // Calculate coefficients for the quadratic equation
//    float a = glm::dot(ray_direction, ray_direction);
//    glm::vec3 oc = ray_origin - sphere_center;
//    float b = 2.0f * glm::dot(oc, ray_direction);
//    float c = glm::dot(oc, oc) - sphere_radius * sphere_radius;
//
//    // Calculate discriminant
//    float discriminant = b * b - 4.0f * a * c;
//
//    // Check if there is an intersection
//    if (discriminant > 0.0f)
//    {
//        // Calculate solutions for t (intersection points)
//        float t1 = (-b + sqrt(discriminant)) / (2.0f * a);
//        float t2 = (-b - sqrt(discriminant)) / (2.0f * a);
//
//        // Check if the intersection points are in front of the viewer
//        if (t1 >= 0 || t2 >= 0)
//        {
//            // Return the sphere's color
//            return sphere_color;
//        }
//    }
//    else if (discriminant == 0.0f)
//    {
//        // There is a single intersection point
//        float t = -b / (2.0f * a);
//
//        // Check if the intersection point is in front of the viewer
//        if (t >= 0)
//        {
//            // Return the sphere's color
//            return sphere_color;
//        }
//    }
//
//    // No intersection, return original color
//    return original_color;
//}
//
////------------FOR PLANE CHECKERBOARD COLOR PATTERN, COPIED FROM PS5 ------------------------
//uint32_t getColor(glm::vec3 hitPoint, uint32_t original_color)
//{
//    // Checkerboard pattern
//    float scale_parameter = 0.5f;
//    float chessboard = 0;
//
//    if (hitPoint.x < 0)
//    {
//        chessboard += floor((0.5 - hitPoint.x) / scale_parameter);
//    }
//    else
//    {
//        chessboard += floor(hitPoint.x / scale_parameter);
//    }
//
//    if (hitPoint.y < 0) {
//        chessboard += floor((0.5 - hitPoint.y) / scale_parameter);
//    }
//    else
//    {
//        chessboard += floor(hitPoint.y / scale_parameter);
//    }
//
//    chessboard = (chessboard * 0.5) - int(chessboard * 0.5);
//    chessboard *= 2;
//    if (chessboard > 0.5) {
//        return 0.5f * original_color;
//    }
//    return original_color;
//
//}
//
////------------------------------------------------------------------------------------------
//
//
//
//uint32_t plane_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec4 plane_coord, uint32_t plane_color, uint32_t original_color)//plane coords (a,b,c,d), TO ADD: the checkers pattern on the plane
//{
//    //first we deal with the checkerboard pattern:
//
//
//    //-----------MATH CALCULATIONS
//    //ax+by+cz+d = 0
//    //(ray_origin.x + t * ray_direction.x) * plane.coord.x + (ray_origin.y + t * ray_direction.y) * plane.coord.y + (ray_origin.z + t * ray_direction.z) * plane.coord.z + plane_coord.d = 0
//    //(ray_direction.x * plane_coord.x + ray_direction.y * plane_coord.y + ray_direction.z * plane_coord.z ) * t = - (ray_origin.x * plane_coord.x + ray_origin.y * plane_coord.y + ray_origin.z * plane_coord.z + plane_coord.d)
//    //Now we isolate t : t = - (ray_origin.x * plane_coord.x + ray_origin.y * plane_coord.y + ray_origin.z * plane_coord.z + plane_coord.d) / (ray_direction.x * plane_coord.x + ray_direction.y * plane_coord.y + ray_direction.z * plane_coord.z )
//    //--------------------------------
//    float denominator = (ray_direction.x * plane_coord.x + ray_direction.y * plane_coord.y + ray_direction.z * plane_coord.z);
//    float nominator = -(ray_origin.x * plane_coord.x + ray_origin.y * plane_coord.y + ray_origin.z * plane_coord.z + plane_coord.w); //w is the 4th coord
//    if (denominator == 0) //either we have infinite results, or none at all 
//    {
//        if (nominator == 0)
//        {
//            //infinite results - we currently don't do anything with it, but we might later down the line, so for now we return same result as a single intersection
//            return plane_color;
//        }
//        else
//        {
//            //none at all
//            return original_color;
//        }
//    }
//    else // a single intersection
//    {
//        float result = nominator / denominator; //we currently do nothing with it, but perhaps we might later down the line
//        glm::vec3 hitpoint = glm::vec3(ray_origin.x + ray_direction.x * result, ray_origin.y + ray_direction.y * result, ray_origin.z + ray_direction.z * result);
//        plane_color = getColor(hitpoint, plane_color);
//        if (result >= 0)
//        {
//            return plane_color;
//        }
//        else
//        {
//            return original_color;
//        }
//
//    }
//
//    return original_color;
//}
//
////------------------------------------------------------------------------------------------DEPRECATED CODE END------------------------------------------------------------------------------------------