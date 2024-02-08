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


//for input from files

uint32_t convert_vec4_rgba_to_uint32_t(glm::vec4 color) 
{
    unsigned char r = color.x * 255;
    unsigned char g = color.y * 255;
    unsigned char b = color.z * 255;
    unsigned char alpha = color.w * 255;
    return r | g << 8 | b << 16 | alpha << 24;
}



//algebraic functions for vectors

float distance(glm::vec3 origin, glm::vec3 destination) 
{
    return sqrt(pow((destination.x - origin.x),2) + pow((destination.y - origin.y),2) + pow((destination.z - origin.z),2));
}

glm::vec3 calculate_hitpoint_from_distance(glm::vec3 origin,glm::vec3 direction, float dist) 
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
class Shape_custom
{
    public:
        virtual float intersection(glm::vec3 ray_origin, glm::vec3 ray_direction, uint32_t original_color) const = 0;
        virtual uint32_t getColor(glm::vec3 hitPoint) const = 0;
};

class Sphere : public Shape_custom
{
    private:
        float radius;
        glm::vec3 center;
        uint32_t color;
    public:
        //setters
        void set_color(uint32_t color)
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
        
        //constructors
        Sphere(float radius, glm::vec3 center, uint32_t color) 
        {
            this->radius = radius;
            this->center = center;
            this->color = color;
            
        }
        Sphere(float radius, glm::vec3 center) //we use this if we don't know the current color
        {
            this->radius = radius;
            this->center = center;
            this->color = 0xff000000; //fully opaque, and black.
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
                // Calculate solutions for t (intersection points)
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

class Plane : public Shape_custom
{
    private:
        glm::vec4 plane_coord;
        uint32_t color;


    public:
        //constructors
        Plane(glm::vec4 plane_coord, uint32_t color) 
        {
            this->plane_coord = plane_coord;
            this->color = color;
        }
        Plane(glm::vec4 plane_coord) 
        {
            this->plane_coord = plane_coord;
            this->color = 0xff000000; //fully opaque, and black.
        }
        //setters
        void setColor(uint32_t color) 
        {
            this->color = color;
        }
        //getters
        glm::vec4 getCoord()
        {
            return this->plane_coord;
        }
        //------------FOR PLANE CHECKERBOARD COLOR PATTERN, COPIED FROM PS5 ------------------------
        uint32_t getColor(glm::vec3 hitPoint) const
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
                return 0.5f * this->color;
            }
            return this->color;

        }

        //------------------------------------------------------------------------------------------

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
                if (result >= 0)
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

//will likely need to add: RayOrigin
uint32_t lightTest(glm::vec2 coord,std::vector<Shape_custom*>, std::vector<glm::vec4> light_sources, uint32_t* image_data_input) // temporary, may change. Have to implement directional,ambient,and spot light
{
    return 0;
}
uint32_t PerPixelShadow(std::vector<glm::vec4> light_sources_directions, uint32_t* image_data_input, glm::vec3 eye, glm::vec3 direction, std::vector<float> &distances_input, std::vector<Shape_custom*> &shapes_input, uint32_t pixel_color,std::vector<glm::vec4> &spotlight_positions)
{
    //calculate the shortest distance from the eye, this is how we find a hitpoint, from which we calculate a ray to a light source

    float shortest_distance = INT_MAX;
    int relevant_shape_index;
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
    //we now check collision with other shapes on our way to the lighting
    char in_shade = 1; //I'm in shade until proven not to be. This is important because we potentially may be under shade with one source of light, but another light may still shine at us.
    for (glm::vec4 light : light_sources_directions) //I may only filter specific light source types, MAY CHANGE.
    {
        //check light_type
        if (light.w == 0.0) //directional light
        {
            glm::vec3 light_ray = glm::vec3(light.x, light.y, light.z);
            glm::vec3 hitpoint = calculate_hitpoint_from_distance(eye, direction, shortest_distance);
            for (int i = 0, potential_intersections = shapes_input.size() - 1; i < shapes_input.size(); i++) //originally a for each loop, moved from it to accommodate for filtering out an indexed shape 
            {
                if (i != relevant_shape_index) //don't check intersection with myself. ADDITIONALLY: the i of the distances may be in inverse to the shapes vector, may have to switch them around. 
                {
                    float distance_to_intersection = shapes_input[i]->intersection(hitpoint, -light_ray, pixel_color); //notice that light ray is reversed, otherwise i get a negative direction in intersection and dont return it
                    if (distance_to_intersection < 0)
                    {
                        potential_intersections--;
                        if (potential_intersections == 0)
                        {
                            in_shade = 0;
                        }
                    }

                }
            }
        }
        else if (light.w == 1.0) //spotlight - added vector normalization
        {
            glm::vec3 light_ray = glm::normalize(glm::vec3(light.x, light.y, light.z));
            glm::vec3 hitpoint = calculate_hitpoint_from_distance(eye, direction, shortest_distance);
            glm::vec3 spotlight_pos = glm::vec3(spotlight_positions[spotlight_counter].x, spotlight_positions[spotlight_counter].y, spotlight_positions[spotlight_counter].z);
            glm::vec3 light_to_surface_vec = glm::normalize(calculate_vector_direction(spotlight_pos, hitpoint));

            //unused
            float angle_radians = std::acos(glm::dot(light_to_surface_vec, light_ray)/(glm::length(light_to_surface_vec)*glm::length(light_ray)));
            float angle_degrees = angle_radians * (180.0 / 3.14159);
            //I dunno, I think it makes sense
            //if (angle_degrees < 0)
            //{
            //    angle_degrees = 180 + angle_degrees;
            //}
            //

            //DO NOTE: THIS DOES NOT MAKE SENSE IN ITS CURRENT CURRENT FORM, THE COMPARISON SHOULD GO THE OTHER WAY, OUR COS(a) SHOULD BE SMALLER THAN THE CUTOFF ANGLE, FOR SOME REASON IT PRODUCES THE INVERSE OF THE RESULTS, THIS DOES NOT MAKE ANY SENSE.
            if (glm::dot(light_to_surface_vec, light_ray) / (glm::length(light_to_surface_vec) * glm::length(light_ray)) > spotlight_positions[spotlight_counter].w) //if our angle is smaller than the cutoff angle, light could theoretically reach us
            {

                for (int i = 0, potential_intersections = shapes_input.size() - 1; i < shapes_input.size(); i++) //originally a for each loop, moved from it to accommodate for filtering out an indexed shape 
                {
                    if (i != relevant_shape_index) //don't check intersection with myself. ADDITIONALLY: the i of the distances may be in inverse to the shapes vector, may have to switch them around. 
                    {
                        float distance_to_intersection = shapes_input[i]->intersection(hitpoint, calculate_vector_direction(hitpoint, spotlight_pos), pixel_color);
                        if (distance_to_intersection < 0)
                        {
                            potential_intersections--;
                            if (potential_intersections == 0)
                            {
                                in_shade = 0;
                            }
                        }

                    }
                }
            }
            else 
            {
                in_shade = 1;
            }

            spotlight_counter++; //if we encounter another spotlight, we'll start from that one
        }

        else //point light, I dunno why, I made it already so might as well lol. do note that it has no attenuation
        {
            glm::vec3 light_point = glm::vec3(light.x, light.y, light.z);
            glm::vec3 hitpoint = calculate_hitpoint_from_distance(eye, direction, shortest_distance);
            for (int i = 0, potential_intersections = shapes_input.size() - 1; i < shapes_input.size(); i++) //originally a for each loop, moved from it to accommodate for filtering out an indexed shape 
            {
                if (i != relevant_shape_index) //don't check intersection with myself. ADDITIONALLY: the i of the distances may be in inverse to the shapes vector, may have to switch them around. 
                {
                    float distance_to_intersection = shapes_input[i]->intersection(hitpoint, calculate_vector_direction(hitpoint, light_point), pixel_color);
                    if (distance_to_intersection < 0)
                    {
                        potential_intersections--;
                        if (potential_intersections == 0)
                        {
                            in_shade = 0;
                        }
                    }

                }
            }
        }

        if (in_shade == 1)
        {
            pixel_color = pixel_color * 0.25;
        }

    }

    //if (in_shade == 1) 
    //{
    //    pixel_color = pixel_color * 0.25;
    //}
    return pixel_color;
}


uint32_t PerPixel(glm::vec2 coord, glm::vec3 rayDirection, std::vector<Shape_custom*> shapes_input, std::vector<float> &distances,glm::vec3 rayOrigin) //essentially cherno's raytracing vid #3 as linked in the assignment
{ 
    //TO_ADD INPUTS:
    //glm::vec3 eye_position

    //uint8_t r = (uint8_t)(coord.x * 255.0f);
    //uint8_t g = (uint8_t)(coord.y * 255.0f);

    // a+bt - ray formula. a = ray origin, b = ray direction , t = hit distance

    //------------TEMPORARY VALUES, WILL LIKELY CHANGE------------------
    
    //glm::vec3 rayDirection(coord.x, coord.y, -rayOrigin.z);
    float radius = 0.5f;
    uint32_t pixel_value = 0; // this will change as it goes through more processes
    //FOR DEBUGGING
    //std::ofstream myfile;
    //myfile.open("getting coordinate edges.txt");
    //

    //if (coord.x == 0 && coord.y == 0) {
    //    std::cout << "coord: " << coord.x << "," << coord.y << std::endl;
    //    std::cout << "vec3: " << rayOrigin.x << "+ t * " << rayDirection.x << "," << rayOrigin.y << "+ t * " << rayDirection.y << "," << rayOrigin.z << "+ t * " << rayDirection.z << std::endl;
    //}

    //if (coord.x >= 0.998 && coord.y == 0) {
    //    std::cout << "coord: " << coord.x << "," << coord.y << std::endl;
    //    std::cout << "vec3: " << rayOrigin.x << "+ t * " << rayDirection.x << "," << rayOrigin.y << "+ t * " << rayDirection.y << "," << rayOrigin.z << "+ t * " << rayDirection.z << std::endl;
    //}
    //if (coord.x >= 0.998 && coord.y == 0.998) {
    //    std::cout << "coord: " << coord.x << "," << coord.y << std::endl;
    //    std::cout << "vec3: " << rayOrigin.x << "+ t * " << rayDirection.x << "," << rayOrigin.y << "+ t * " << rayDirection.y << "," << rayOrigin.z << "+ t * " << rayDirection.z << std::endl;
    //}
    //Sphere* sphere_1 = new Sphere(radius, glm::vec3(-0.7, -0.7, -2), 0xffaa00ff);
    //Sphere* sphere_2 = new Sphere(radius, glm::vec3(0.6, -0.5, -1), 0xff00a0ff);
    //Plane* plane_1 = new Plane(glm::vec4(0, -0.5, -1.0, -3.5), 0xffaaaaff);

    //int i; //counter for filling out distances
    //shapes_input.size() > 0 ? i = shapes_input.size() - 1 : 

    //filling out distances in the respective indeces of the shapes, in the "distances" vector
    //int i = shapes_input.size() - 1; //counter for filling out distances
    for (Shape_custom* shape : shapes_input)
    {
        //alright, let's talk logic.
        //for each intersection, I return the distance to the object ()
        distances.push_back(shape->intersection(rayOrigin, rayDirection, pixel_value));
        //i--;

    }

    //now we decide what color we bring back, let's take the smallest distance that is higher than 0, if one exists
    float shortest_distance = INT_MAX; //arbitrary value, hopefully we don't reach those distances in reality
    for (int i = 0; i < distances.size() && i < shapes_input.size(); i++) //distances and shapes ought to be synced, but I do this explicitly for readability
    {
        if (distances[i] >= 0 && distances[i] < shortest_distance) //meaning if we're farther than 0 and shorter than our current shortest distance. NOTE: if distances are the same, we will return the color of the object that was added to the stack first
        {
            shortest_distance = distances[i];
            pixel_value = shapes_input[i]->getColor(calculate_hitpoint_from_distance(rayOrigin, rayDirection, shortest_distance)); //the index reverse is because we work in stacks, but calculate chronologically. I might flip it over to sync them properly, but it works so eh, may change later
        }
    }
    if (shortest_distance == INT_MAX) //meaning no intersections whatsoever
    {
        pixel_value = 0; //this doesn't do anything for now but eh
    } 

    //pixel_value = plane_1->intersection(rayOrigin, rayDirection, pixel_value);
    //pixel_value = sphere_1->intersection(rayOrigin, rayDirection, pixel_value);
    //pixel_value = sphere_2->intersection(rayOrigin, rayDirection, pixel_value);

    //pixel_value = plane_intersection(rayOrigin, rayDirection, glm::vec4(0, -0.5, -1.0, -3.5), 0xffaaaaff, pixel_value);
    //pixel_value = sphere_intersection(rayOrigin, rayDirection, glm::vec3(-0.7, -0.7, -2), radius, 0xffaa00ff, pixel_value);
    //pixel_value = sphere_intersection(rayOrigin, rayDirection, glm::vec3(0.6, -0.5, -1), radius, 0xff00a0ff, pixel_value);




    //for (float dist : distances_input)
    //{
    //    distances_input.pop_back();
    //}

    
    //TODO: calculate intersection over all of the shapes, return the color of the intersection that is closest to your eye

    return pixel_value;
}

Texture::Texture(int width, int height) //added by me
{
    std::vector<Shape_custom*> shapes;
    std::vector<float> distances; //this will include distance for each hit, -1 if non-existent
    uint32_t sphere_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4( 0.6,0.0,0.8,10.0));
    uint32_t sphere_2_color = convert_vec4_rgba_to_uint32_t(glm::vec4(1, 0, 0, 10.0));
    uint32_t plane_1_color = convert_vec4_rgba_to_uint32_t(glm::vec4(0.0, 1.0, 1.0, 10.0));

    Sphere* sphere_1 = new Sphere(0.5, glm::vec3(-0.7, -0.7, -2), 0xffaa00ff);
    Sphere* sphere_2 = new Sphere(0.5, glm::vec3(0.6, -0.5, -1), 0xff00a0ff);
    Plane* plane_1 = new Plane(glm::vec4(0, -0.5, -1.0, -3.5), 0xffaaaaff);
    glm::vec3 rayOrigin_original(0.0f, 0.0f, 4.0f); //note that moving forward in the z direction is actually backwards in relation to the ray we shoot, since it shoots in the negative direction
    std::vector<glm::vec4> light_sources_original;
    std::vector<glm::vec4> spotlight_positions;
    glm::vec3 rayDirection_original; //from eye to screen

    //light_sources_original.push_back(glm::vec4(0.5, 0.0, -1.0, 1.0)); //spot light
    //light_sources_original.push_back(glm::vec4(-0.7, -0.7, -2.0, 1.0)); //spot light
    //light_sources_original.push_back(glm::vec4(0.5,0.0,-1.0,1.0)); //spot light

    light_sources_original.push_back(glm::vec4(0.0, 0.5, -1.0, 0.0)); //directional light

    spotlight_positions.push_back(glm::vec4(2.0,1.0,3.0,0.6)); //do note that the 4th parameter is probably the cutoff angle: 0.6 * 255deg

    shapes.push_back(sphere_1);
    shapes.push_back(sphere_2);
    shapes.push_back(plane_1);


    //for (int i = 0; i < shapes.size(); i++) 
    //{
    //    distances.push_back(-1); //temporary distance
    //}

    

    uint32_t* image_data = new uint32_t[width * height]; //right-to-left -> 2bytes(R) -> 2bytes(G) -> 2bytes(B) -> 2bytes(alpha) -> 0x meaning we write in hexadecimal
    for (int y = 0; y < height; y++) 
    {
        for (int x = 0; x < width; x++) 
        {
            //if (x == 799) 
            //{
            //    std::cout << "lol";
            //}
            glm::vec2 coord = 
            { 
                (float)x / (float)width , 1.0f - (float)y / (float)height  //do note that I have inverted the y axis
            };
            coord = coord * 2.0f - 1.0f; //normalizing the coordinates from -1 to 1
            rayDirection_original = glm::vec3(coord.x, coord.y, -rayOrigin_original.z);
            image_data[x + y * width] = PerPixel(coord,rayDirection_original,shapes,distances, rayOrigin_original);

            image_data[x + y * width] = PerPixelShadow(light_sources_original, image_data, rayOrigin_original, rayDirection_original, distances,shapes ,image_data[x + y * width],spotlight_positions);
            //before we end this loop, we ought to pop the distances back out
            for (float dist : distances)
            {
                distances.pop_back();
            }


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

















//IRRELEVANT CODE FROM THIS POINT












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