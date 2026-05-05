//main raytracer file

//libraries: 
#include "v3math.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <list>

#define PLANEDISTANCEZ -1
#define PI 3.141592
#define MAXLEVEL 7 
#define AMBIENTPERCENT 0.20
#define OUTLINEMARGIN 0.2
#define BORDEROUTLINE 5 
#define DISTANCEMARGIN 0.5
using namespace std;

//ppm functions: 


//function: ppm_write 
void ppm_write(char *filename, uint8_t **pixmap, int height, int width, int max_colors)
{
    FILE *out = fopen(filename, "wb");
    if (!out) {
        fprintf(stderr, "Error: cannot open output file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fprintf(out, "P3\n");
    fprintf(out, "%d %d\n", width, height);
    fprintf(out, "%d\n", max_colors);

    int total = width * height * 3;
    for (int i = 0; i < total; i++) {
        fprintf(out, "%d ", (*pixmap)[i]);
        if ((i + 1) % 9 == 0) fprintf(out, "\n"); // nice formatting
    }

    fclose(out);
}

//reads comments in ppm file 
//read comment: reads comments and lingering characters 
int readComment(FILE* filename){
    int c = fgetc(filename);
    //if comment
    if(c == '#'){
        //skip
        while((c = fgetc(filename)) != '\n' && c != EOF);
        return 1;
    }
    //if newline
    if(c == '\n'){
        return 1;
    }
    //else not comment 
    ungetc(c, filename);
    return 0; 
}

class texture{ 
    public: 
        int width=0;
        int height=0;
        int max_colors=0;
        uint8_t *pixmap = NULL;

    
    
    //destructor 
    ~texture(){
        delete []pixmap;
    }

};

//reads ppm file and stores data in a pixmap 
void ppm_read(char *filename, uint8_t **pixmap, texture *CurrentTexture)
{
    char filetype[3]; 
    FILE *ppmData = fopen(filename, "r");
    //check if filename is correct 
    assert(ppmData!=NULL);
    //read metadata
    fscanf(ppmData ,"%s ", filetype);
    assert(strcmp(filetype,"P3")==0);

    // rid of comments
    while (readComment(ppmData)); 
    // get widths
    if (fscanf(ppmData, "%d %d", &CurrentTexture->width, &CurrentTexture->height) != 2) 
    {
        fprintf(stderr, "Error: invalid image size\n");
        fclose(ppmData);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "Width: %d, Height: %d\n", CurrentTexture->width, CurrentTexture->height);
    while(readComment(ppmData));
    // get max color opacity
    if (fscanf(ppmData, "%d", &CurrentTexture->max_colors) != 1 || CurrentTexture->max_colors != 255) 
    {
        fprintf(stderr, "Error: invalid max color value\n");
        fclose(ppmData);
        exit(EXIT_FAILURE);
    }
    while(readComment(ppmData));
    // allocate memory for the pixmap to copy data to
    size_t size = CurrentTexture->width * CurrentTexture->height * 3;
    *pixmap = new uint8_t[size];
    if (!*pixmap) 
    {
        fprintf(stderr, "Error: memory allocation failed\n");
        fclose(ppmData);
        exit(EXIT_FAILURE);
    }

    // variables for reading, each pixel has r, g, b values
    int r, g, b;

    //loop through rows and columns to image data
    for (int i = 0; i < CurrentTexture->height; i++) 
    {
        for (int j = 0; j < CurrentTexture->width; j++) 
        {
            if (fscanf(ppmData, "%d %d %d", &r, &g, &b) != 3) 
            {
                fprintf(stderr, "Error: invalid pixel data\n");
                delete[] *pixmap;
                fclose(ppmData);
                exit(EXIT_FAILURE);
            }

            int index = (i * CurrentTexture->width + j) * 3;
            (*pixmap)[index]     = (uint8_t) r;
            (*pixmap)[index + 1] = (uint8_t) g;
            (*pixmap)[index + 2] = (uint8_t) b;
        }
    }

    // close the file
    fclose(ppmData);
}


//classes:

struct camera{
    float width=0;
    float height=0;
    float position[3]={0,0,0};
    
};

struct image{
    float width=0;
    float height=0;
};


struct ray{
    float direction[3]={0,0,0};
    float origin[3]={0,0,0};

    void find_intersection_point(float t, float* dst)
    {
        dst[0]=origin[0]+direction[0]*t;
        dst[1]=origin[1]+direction[1]*t;
        dst[2]=origin[2]+direction[2]*t;
    }
};

//ver2: added light class 
class light{
    public: 
        //atrributes
        float position[3]={0,0,0};
        float color[3]={0,0,0};
        float radial_a0=0;
        float radial_a1=0;
        float radial_a2=0;
        //determies spotlight
        float theta=0; 
        float angular_a0=0; 
        //determines pointlight 
        float direction[3]={0,0,0};
        float ns = 20; 

        //methods 
        void find_data( FILE* scene){
            //find data for the spehre 
            char buffer[50];
            do{
                fscanf(scene, "%s ", buffer);
                //color 
                if(strcmp(buffer,"position:")==0){
                    fscanf(scene ," %f %f %f ", &position[0], &position[1], &position[2]);
                }
                else if(strcmp(buffer,"color:")==0){
                    fscanf(scene ," %f %f %f ", &color[0], &color[1], &color[2]);
                }
                else if(strcmp(buffer, "radial_a0:")==0){
                    fscanf(scene, " %f ", &radial_a0);
                }
                else if(strcmp(buffer, "radial_a1:")==0){
                    fscanf(scene, " %f ", &radial_a1);
                }
                else if(strcmp(buffer, "radial_a2:")==0){
                    fscanf(scene, " %f ", &radial_a2);
                }
                else if(strcmp(buffer, "theta:")==0){
                    fscanf(scene, " %f ", &theta);
                }
                else if(strcmp(buffer, "angular_a0:")==0){
                    fscanf(scene, " %f ", &angular_a0);
                }
                else if(strcmp(buffer,"direction:")==0){
                    fscanf(scene ," %f %f %f ", &direction[0], &direction[1], &direction[2]);
                }
                else if(strcmp(buffer,"ns:")==0){
                    fscanf(scene ," %f ", &ns);
                }
            }while(strcmp(buffer,";")!=0);
        }

        //destructor 
        ~light(){
        }
};

class object{
    public: 
        //attributes
        float c_diff[3]={0,0,0};
        float c_spec[3]={0,0,0};
        float position[3]={0,0,0}; 
        float reflection =0.0; 
        int isCelShaded=false; 
        char textureName[64]={0};
        texture* objectTexture = NULL;
        


        //methods
        virtual float find_intersection(float *ro, float *rd){
            return 1; 
        }

        virtual void find_data( FILE* filename){
            return; 
        }
        virtual void get_normal(float* new_normal, float* intersection_point){
            return; 
        }

        virtual void get_uv(float* point, float* center, float* uv){
            uv[0] = 0;
            uv[1] = 0;
        }

        //destructor: used for proper deallocation of virtual functions
        virtual ~object() {
            //deallocate texture if possible 
            if(objectTexture != NULL){
                delete objectTexture;
            }
        }
};

class sphere: public object{
    //attributes
    public: 
    float radius=0; 

        //methods
        float find_intersection(float *ro, float *rd)override{
                // Note: rd is normalized so A is 1

                // calculate B and C variables
                float B = 2.0f * ( 
                    rd[0] * ( ro[0] - position[0] ) + 
                    rd[1] * ( ro[1] - position[1] ) + 
                    rd[2] * ( ro[2] - position[2] )
                );

                float C =
                    (ro[0] - position[0]) * (ro[0] - position[0]) +
                    (ro[1] - position[1]) * (ro[1] - position[1]) +
                    (ro[2] - position[2]) * (ro[2] - position[2]) -
                    radius * radius;

                float discriminate = B * B - 4.0f * C;
                // if discriminate is less than 0, it doesn't intersect
                if(discriminate < 0.0f)
                {
                    return -1.0f;
                }


                float t0 = (-B - sqrt(discriminate)) / 2.0f;
                float t1 = (-B + sqrt(discriminate)) / 2.0f;

                // if both t0 and t1 are posisitve, return the smallest
                if(t0 > 0.0f && t1 > 0.0f)
                {
                    if(t0 < t1){
                        return t0; 
                    }
                    else{
                        return t1; 
                    }
                }

                // else determine if at least one is positive 
                if (t0 > 0.0f)
                {
                    return t0;
                }

                if (t1 > 0.0f)
                {
                    return t1;
                }

                // if both are negative, no intersection (both behind camera)
                return -1.0f;
        }


        void find_data( FILE* scene) override{
            //find data for the spehre 
            char buffer[50];
            do{
                fscanf(scene, "%s ", buffer);
                //color 
                if(strcmp(buffer,"c_diff:")==0){
                    fscanf(scene ," %f %f %f ", &c_diff[0], &c_diff[1], &c_diff[2]);
                }
                else if(strcmp(buffer,"position:")==0){
                    fscanf(scene ," %f %f %f ", &position[0], &position[1], &position[2]);
                }
                else if(strcmp(buffer,"c_spec:")==0){
                    fscanf(scene ," %f %f %f ", &c_spec[0], &c_spec[1], &c_spec[2]);
                }
                else if(strcmp(buffer, "radius:")==0){
                    fscanf(scene, " %f ", &radius);
                }
                else if(strcmp(buffer, "reflection:")==0){
                    fscanf(scene, " %f ", &reflection);
                }
                else if(strcmp(buffer, "texture:")==0){
                    fscanf(scene, " \"%[^\"]\"  ", textureName);
                    //allocate memory for texture data 
                    objectTexture = new texture;
                    //read 
                    ppm_read(textureName, &objectTexture->pixmap, objectTexture);
                } 
                else if(strcmp(buffer, "cel-shaded:")==0){
                    fscanf(scene, " %d ", &isCelShaded);
                }
            }while(strcmp(buffer,";")!=0);

        }
    
        void get_normal(float* new_normal, float* intersection_point)override{
             v3_subtract(new_normal, intersection_point, position);
             v3_normalize(new_normal, new_normal);
        }

        void get_uv(float* point, float* center, float* uv) override{
            float D[3];
            v3_subtract(D, point, center);
            v3_normalize(D, D);

            float phi = atan2(D[2], D[0]);
            float theta = acos(D[1]);

            uv[0] = phi / (2 * PI) + 0.5f;
            uv[1] = theta / PI;
        }

};


class plane: public object{
    public: 
    float normal[3]={0,0,0};

        //methods
        float find_intersection(float *ro, float *rd)override{

            float denominator =
                normal[0]*rd[0] +
                normal[1]*rd[1] +
                normal[2]*rd[2];

            // if denominator is 0, no intersection (parallel to plane)
            if (fabs(denominator) < 0.0001f)
                return -1.0f;
            
            float diff[3] = {
                position[0] - ro[0],
                position[1] - ro[1],
                position[2] - ro[2]
            };

            float t =
               (diff[0]*normal[0] +
                diff[1]*normal[1] +
                diff[2]*normal[2]) / denominator;

            if (t > 0.0f)
                return t;

            return -1.0f;
        }

        void find_data(FILE* scene) override{
            //find data for the plane 
            char buffer[50];
            do{
                fscanf(scene, "%s ", buffer);
                //color 
                if(strcmp(buffer,"c_diff:")==0){
                    fscanf(scene ," %f %f %f ", &c_diff[0], &c_diff[1], &c_diff[2]);
                }
                else if(strcmp(buffer,"c_spec:")==0){
                    fscanf(scene ," %f %f %f ", &c_spec[0], &c_spec[1], &c_spec[2]);
                }
                else if(strcmp(buffer,"position:")==0){
                    fscanf(scene ," %f %f %f ", &position[0], &position[1], &position[2]);
                }
                else if(strcmp(buffer, "normal:")==0){
                    fscanf(scene, " %f %f %f ", &normal[0], &normal[1], &normal[2]);
                }
                else if(strcmp(buffer, "reflection:")==0){
                    fscanf(scene, " %f ", &reflection);
                }
                else if(strcmp(buffer, "cel-shaded:")==0){
                    fscanf(scene, " %d ", &isCelShaded);
                }
            }while(strcmp(buffer,";")!=0);

            //DEBUG:
            //printf("Plane: c_diff %f %f %f position: %f %f %f normal %f %f %f\n", 
            //  c_diff[0],c_diff[1],c_diff[2], position[0], position[1], position[2], normal[0], normal[1], normal[2]);
        }

        void get_normal(float* new_normal, float* intersection_point)override{
             new_normal[0]=normal[0];
             new_normal[1]=normal[1];
             new_normal[2]=normal[2];
             v3_normalize(new_normal, new_normal);
        }
};



//func: read scene
//input: pointer to object array
//output: fills the aray with scene data 
void read_scene(list<object*>* scene_information, list<light*>* light_information, 
    const char* filename, camera* current_camera){
    //open file 
    FILE* scene = fopen(filename, "r");
    assert(scene!=NULL);

    char buffer[50];
    //check for file header 
    fscanf(scene,"%s ", buffer);
    assert(strcmp(buffer,"img410scene")==0);

    while(strcmp(buffer,"end")!=0){
        fscanf(scene,"%s ", buffer);
        //object camera 
        if(strcmp(buffer,"camera")==0){
            do{
                //find properties
                fscanf(scene, "%s ", buffer);

                if(strcmp(buffer,"width:")==0)
                {
                    fscanf(scene ," %f ", &current_camera->width);
                }
                else if(strcmp(buffer,"height:")==0)
                {
                    fscanf(scene ," %f ", &current_camera->height);
                }
            }while(strcmp(buffer,";")!=0);

            //DEBUG:
            //printf("camera: width %f height %f\n", current_camera->width, current_camera->height);
        }
        //object plane
        else if(strcmp(buffer, "plane")==0){
            object* current_plane = new plane;
            current_plane->find_data(scene);
            scene_information->push_front(current_plane);

        }
        //object sphere 
        else if(strcmp(buffer,"sphere")==0){
            sphere* current_sphere = new sphere; 
            current_sphere->find_data(scene);
            scene_information->push_front(current_sphere);
        }
        //light 
        else if(strcmp(buffer,"light")==0){
            light* current_light = new light; 
            current_light->find_data(scene);
            light_information->push_front(current_light);
        }
        //DEBUG:
        //printf("One object");
    }
    fclose(scene);
}

//function: findCurrentRay 
//input: pixel_w, pixel_h camera object, image struct ray
//output: direction of ray 
void find_current_Ray(int current_w, int current_h, float pix_width, float pix_height, 
    camera camera_info, image image_info, ray *current_ray){
    // set ray origin 
    current_ray->origin[0]=camera_info.position[0];
    current_ray->origin[1]=camera_info.position[1];
    current_ray->origin[2]=camera_info.position[2];

    //get X,y, and z componenta
    current_ray->direction[1]= camera_info.height / 2  - pix_height * (current_h+0.5); 
    current_ray->direction[0]= -camera_info.width /2 + pix_width*(current_w+0.5);
    current_ray->direction[2]= PLANEDISTANCEZ; 

    //normalize 
    v3_normalize(current_ray->direction, current_ray->direction);
}

//function: CheckCelObjects 

//function: ShadedGradient
//Input:
//output: degree of color 
void cel_shade_gradient(float* object_diff,  int sections, float normal_dot_ray, float view_dot_normal
                    ,float view_dot_reflection , light* current_light, 
                    float* I_diff, float *I_ambient,float * I_spec, bool* is_outline){
        //get gradient boundaries 
        float boundary_seperation=1.0f/sections; 
        float dot_step = ceil(normal_dot_ray/boundary_seperation)*boundary_seperation;
        for(int c=0; c<3; c++){
            I_diff[c]= object_diff[c]*dot_step;
        }
        //calculate specular
        if(view_dot_reflection > 0.90f){
            // calculate specular 
            float spec = pow(view_dot_reflection, current_light->ns);
            if(spec>0.95){
                for(int c = 0; c < 3; c++){
                //strong white light, else defaults black
                I_spec[c] = 1;
                }
            }
        }
}

//CalcualteLightCelShade 

//logic: wanted to avoid further conditional mess in shade function
void calculate_light_cel(object* target_object, light* current_light, float normal_dot_ray, float view_dot_normal,
       float view_dot_reflection, float ns, 
       float f_rad, float f_ang, float* I, float* I_diff, float I_ambient, float* I_spec, bool* is_outline)
{
    //handles three types of light
    cel_shade_gradient(target_object->c_diff, 2, normal_dot_ray, view_dot_normal, view_dot_reflection,
                        current_light, I_diff, &I_ambient, I_spec, &*is_outline);
    //full lighting calculation (excludes ambient since in cel-shade-gradient)
    I[0] += f_rad * f_ang * (I_spec[0] + I_diff[0]);
    I[1] += f_rad * f_ang * (I_spec[1] + I_diff[1]);
    I[2] += f_rad * f_ang * (I_spec[2] + I_diff[2]);
    for(int c = 0; c < 3; c++)
    {
        if(I[c] > 1.0f) I[c] = 1.0f;
        if(I[c] < 0.0f) I[c] = 0.0f;
    }
}

//function: CalculateLight 
//input: target_object
//output: float* I(Illumination)
void Calculate_Light(object* target_object, ray* current_ray, float lowest_t,
    list<object*> *scene_information, list<light*> *light_information, float* I){

    //set points and variables for lighting 
    float distance =0; 
    float intersection_normal[3]={0,0,0};
    float intersection_point[3]={0,0,0};
    current_ray->find_intersection_point(lowest_t, intersection_point);
    target_object->get_normal(intersection_normal,intersection_point);
    float shadow_origin[3] = {
    intersection_point[0] + intersection_normal[0] * 0.0001f,
    intersection_point[1] + intersection_normal[1] * 0.0001f,
    intersection_point[2] + intersection_normal[2] * 0.0001f
    };
    // apply effects for each light in scene
    for(light* current_light : *light_information){

        // construct ray from intersection to light
        float L_vector[3];
        v3_from_points(L_vector, intersection_point, current_light->position);
        distance = v3_length(L_vector);
        v3_normalize(L_vector, L_vector);


        bool is_shadowed = false;
        bool cel_intercept =false; 
        bool same_object_shadow = false;


        //find objects blocking the light 
        for(object* current_object: *scene_information){
            if(current_object==target_object){
                same_object_shadow=(current_object==target_object);
                continue; 
            }
            float t = current_object->find_intersection(shadow_origin, L_vector);
            // if the object is closer, current object is shadowed
            if(t > 0.0001f && t < distance - 0.0001f){
                //set as shadowed
                is_shadowed = true;
                //check for extra conditions 
                cel_intercept=(current_object->isCelShaded==1);
                //DEBUG
                //printf("DEBUG: is shadowed\n");
                break; 
            }
        }
        
        if(is_shadowed){
            if(cel_intercept)
            {
                //apply black for cartoon look 
                for(int c=0; c<3; c++)
                {
                    I[c]=0; 
                }
                continue;
            }
            else if(same_object_shadow)
            {
                for(int c=0; c<3; c++)
                {
                    I[c]=target_object->c_diff[c]; 
                }
                continue;
            }
            else
            {
                //apply ambient lighting
                for(int c=0; c<3; c++)
                {
                    I[c]=AMBIENTPERCENT;
                }
                continue; 
            }

        }
        else{
            //Light the scene
            //printf("DEBUG: LIGHTING Started");
            // radial attenuation
            float f_rad = 1.0f;

            float denom = current_light->radial_a0 +
                        current_light->radial_a1 * distance +
                        current_light->radial_a2 * distance * distance;

            if(denom > 0.0f)
                f_rad = 1.0f / denom;

            // angular attenuation
            float f_ang = 1.0f;

            if(current_light->theta > 0.0f)
            {
                float VL[3];
                float VO[3];

                // VL = direction from light to surface
                v3_from_points(VL, current_light->position, intersection_point);
                v3_normalize(VL, VL);

                // VO = spotlight direction (should already point outward)
                VO[0] = current_light->direction[0];
                VO[1] = current_light->direction[1];
                VO[2] = current_light->direction[2];
                v3_normalize(VO, VO);

                float cos_alpha = v3_dot_product(VO, VL);
                float cos_theta = cos(current_light->theta * PI / 180.0f);

                if(cos_alpha < cos_theta)
                    f_ang = 0.0f;
                else
                    f_ang = pow(cos_alpha, current_light->angular_a0);
            }

            //calculate light values 
            v3_normalize(L_vector, L_vector);
            float normal[3]={0,0,0};
            target_object->get_normal(normal,intersection_point);
            
            //reverse view vector
            float view_vector[3]=
                {-current_ray->direction[0],
                -current_ray->direction[1],
                -current_ray->direction[2]};
                    
            //inverse light vector 
            float L_in[3] = {
                -L_vector[0],
                -L_vector[1],
                -L_vector[2]
            };

            //reflecct L across normal 
            float reflection[3] = {0,0,0};
            v3_reflect(reflection, L_in, normal);
            
            //get dot and reflection vector 
            float normal_dot_ray = v3_dot_product(normal, L_vector);
            float view_dot_reflection = v3_dot_product(view_vector, reflection);
            float view_dot_normal = v3_dot_product(view_vector, normal);
            
            
            float I_diff[3]={0,0,0};
            float I_spec[3]={0,0,0};
            float I_ambient=AMBIENTPERCENT; 

            //if cel-shaded logic
            if(target_object->isCelShaded==1 && normal_dot_ray>0){
                //set depth to lowest t 
                bool is_outline = false; 
                //func: cel-shade
                calculate_light_cel(target_object, current_light, normal_dot_ray, view_dot_normal,
                view_dot_reflection, current_light->ns , f_rad, f_ang, I, I_diff, I_ambient, I_spec, &is_outline);
                //if outline set to black 

            }

            else if(normal_dot_ray > 0){
                //if texture, find texture coord 
                if(target_object->objectTexture !=NULL)
                {
                    //get uv
                    float uv[2];
                    target_object->get_uv(intersection_point,target_object->position,uv);
                    float u = (uv[0]*(target_object->objectTexture->width)-1);
                    float v = (uv[1])*(target_object->objectTexture->height)-1;

                    //assign i_diff
                    for(int c = 0; c < 3; c++){
                        //get index
                        int index=((int)v*target_object->objectTexture->width+(int)u)*3+c;
                        I_diff[c] += (target_object->objectTexture->pixmap[index] / 255.0f) *
                        current_light->color[c] *
                        normal_dot_ray;
                    }   
                    

                    //book keep for outlining

                }
                //else use objects color 
                else
                {
                    for(int c = 0; c < 3; c++){
                        I_diff[c] += target_object->c_diff[c] *
                        current_light->color[c] *
                        normal_dot_ray;
                    }   

                    //book keep for outlining 

                }
                if(view_dot_reflection > 0 && normal_dot_ray > 0)
                {
                    // calculate specular for RGB
                    float spec = pow(view_dot_reflection, current_light->ns);

                    for(int c = 0; c < 3; c++)
                    {
                        I_spec[c] += target_object->c_spec[c] *
                                current_light->color[c] *
                                spec;
                    }
                }

                I[0] += f_rad * f_ang * (I_spec[0] + I_diff[0])+I_ambient;
                I[1] += f_rad * f_ang * (I_spec[1] + I_diff[1])+I_ambient;
                I[2] += f_rad * f_ang * (I_spec[2] + I_diff[2])+I_ambient;
                for(int c = 0; c < 3; c++){
                    if(I[c] > 1.0f) I[c] = 1.0f;
                    if(I[c] < 0.0f) I[c] = 0.0f;
                }
            }
        }
    }
}


//function: Shade(recursive)
//input: current ray, scene information, light information, color, and level 
//output: final color 
void Shade( ray *current_ray,list<object*> *scene_information, 
    list<light*> *light_information, int h, int w, image* image_info, float* depth_map, float* Color, int level){
    //base case: hits max cases 
    if(level>=MAXLEVEL){

        return; 
    }
    else{
        object *target_object = NULL;
        float lowest_t = INFINITY;
        int pixel_index=(h*image_info->width+w);
        //find closest object 
        for(object* current_object : *scene_information){
            //DEBUG: check position
            //printf("current object position: %f %f %f \n", current_object->position[0], current_object->position[1], current_object->position[2]);
            float current_t = current_object->find_intersection(current_ray->origin, current_ray->direction);
            if(current_t > 0 && current_t < lowest_t)
            {
                target_object = current_object;
                lowest_t=current_t;
            }
        }

        //depth map logic 
        if(level==1)
        {
            if (target_object==NULL){
                depth_map[pixel_index]=INFINITY; 
            }
            else if(target_object->isCelShaded==1){
                //index depth using h and w
                depth_map[pixel_index]=lowest_t;
            }
            else if(target_object->isCelShaded==0){
                depth_map[pixel_index]=INFINITY;
            }
        }

        if(target_object!=NULL){
            //check for cel-object base case
            if(target_object->isCelShaded==1)
            {
                // //set current cel to object color 
                Color[0]=target_object->c_diff[0];
                Color[1]=target_object->c_diff[1];
                Color[2]=target_object->c_diff[2];
                Calculate_Light(target_object, current_ray, lowest_t, scene_information, light_information, Color);
                return;
            }
            //find reflection ray
            ray reflection;
            float normal[3]={0,0,0};
            current_ray->find_intersection_point(lowest_t ,reflection.origin);
            target_object->get_normal(normal, reflection.origin);
            
            //offset reflection origin
            reflection.origin[0]+= normal[0] *0.0001f;
            reflection.origin[1]+= normal[1] *0.0001f;
            reflection.origin[2]+= normal[2] *0.0001f;

            //get reflection direction 
            v3_reflect(reflection.direction, current_ray->direction, normal);

            //find current ray's color
            float reflectContribution[3]={0,0,0};
            //get reflection contribution 
            Shade(&reflection,scene_information,light_information,h,w,image_info,depth_map,reflectContribution,level+1);
            //func: calculateLight 
            Calculate_Light(target_object, current_ray, lowest_t, scene_information, light_information, Color);
            

            //assing color (add reflection contribution and currnt color)
            Color[0]=Color[0]*(1-target_object->reflection)+reflectContribution[0]*target_object->reflection;
            Color[1]=Color[1]*(1-target_object->reflection)+reflectContribution[1]*target_object->reflection;
            Color[2]=Color[2]*(1-target_object->reflection)+reflectContribution[2]*target_object->reflection;
        }
        return;
        //chat, recurse this guy
    }
}

//func: depthMapDrawOUtlines
//input: depth_Map, pixMap
//output: altered pixmap with outlines
void depth_map_draw_outlines(float* depth_map, uint8_t* pixmap, int h, int w, image image_info){
    //index
    int depth_index=(h*image_info.width+w);
    int pixel_index=depth_index*3; 
    //check if outline viable object
    if(depth_map[depth_index]==INFINITY) return;
    int displacement=BORDEROUTLINE/2; 
    //check direction using border outline
    for(int k=0; k<BORDEROUTLINE; k++){
        for(int l=0; l<BORDEROUTLINE; l++){
            int ni = h+k-displacement;
            int nj = w+l-displacement;
            //added this due to odd error in 3 lines
            int temp = ni * image_info.width + nj;
            //edge case
            if(ni>image_info.height||ni<0||nj<0||nj>image_info.width) continue; 
            float current_neighbor=depth_map[temp];
            // null
            if(current_neighbor==INFINITY){
                //make black 
                for(int c = 0; c < 3; c++)
                {
                    pixmap[pixel_index + c] = 0;
                }
            }
            //else
            else{
                //check if distance between objects is too far 
                if(fabs(depth_map[depth_index]-current_neighbor)>DISTANCEMARGIN){ 
                    for(int c = 0; c < 3; c++)
                    {
                        pixmap[pixel_index + c] = 0;
                    }
                }
            }
        }
    }

    
}


//main:
int main (int argc, char* argv[]){
    if (argc!=5){
        //print error message
        printf("Incorrect Number of Arguements");
        return 1; 
    }

    //cast image info 
    image image_info; 
    sscanf (argv[1],"%f",&image_info.width);
    sscanf (argv[2],"%f",&image_info.height);
    //allocate memory 
    int size = image_info.width * image_info.height*3; 
    uint8_t *pixmap = new uint8_t[size];
    float *depth_map = new float[size/3];
    camera *current_camera = new camera;
    list<object*> *scene_information = new list<object*>;
    list<light*> *light_information = new list<light*>;


    //func: read scene
    read_scene(scene_information,light_information, argv[3],current_camera);
    printf("file read! \n");
    assert(current_camera!=NULL);
    float pix_height= current_camera->height/image_info.height; 
    float pix_width= current_camera->width/image_info.width; 
    ray *current_ray = new ray; 
    //loop through pixels
    for(int h=0; h<image_info.height; h++){
        for(int w=0; w<image_info.width; w++){
            find_current_Ray(w,h,pix_width,pix_height,*current_camera,image_info,current_ray);
            float I[3]= {0,0,0};
            //func: Shade (raytrace)
            Shade(current_ray,scene_information,light_information,h,w,&image_info,depth_map,I,1);

            //find index
            int pixel_index=(h*image_info.width+w)*3; 
            for(int c = 0; c < 3; c++)
            {
                pixmap[pixel_index + c] = (uint8_t)(I[c] * 255);
            }
            
        }
    }

    //second pass: draw outlines 
    for(int h=0; h<image_info.height; h++){
        for(int w=0; w<image_info.width; w++){
            depth_map_draw_outlines(depth_map,pixmap,h,w,image_info);
        }
    }
    
    ppm_write(argv[4], &pixmap, image_info.height, image_info.width, 255);

    //deallocate 
    for (object* obj : *scene_information) {
        delete obj;
    }
    scene_information->clear(); 
    for (light* obj : *light_information) {
        delete obj;
    }

    light_information->clear(); 
    delete scene_information;
    delete current_camera;
    delete current_ray; 
    delete []pixmap;
    delete []depth_map;

    return 0; 
}