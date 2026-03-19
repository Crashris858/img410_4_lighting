//main raytracer file

//libraries: 
#include "v3math.h"
#include <list>
#include <string.h>
#include <stdint.h>
#define PLANEDISTANCEZ -1
#define PI 3.141592
#define MAXLEVEL 7 
using namespace std;

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
};

class object{
    public: 
        //attributes
        float c_diff[3]={0,0,0};
        float c_spec[3]={0,0,0};
        float position[3]={0,0,0}; 
        float reflection =0.0; 
        


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
        //destructor: used for proper deallocation of virtual functions
        virtual ~object() {}
};

class sphere: public object{
    //attributes
    public: 
    float radius=0; 

        //methods
        float find_intersection(float *ro, float *rd)override{
                // make sure rd is normalized so A is 1

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
            }while(strcmp(buffer,";")!=0);

        }
    
        void get_normal(float* new_normal, float* intersection_point)override{
             v3_subtract(new_normal, intersection_point, position);
             v3_normalize(new_normal, new_normal);
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


        //find objects blocking the light 
        for(object* current_object: *scene_information){
            if(current_object==target_object){
                continue; 
            }
            float t = current_object->find_intersection(shadow_origin, L_vector);
            // if the object is closer, current object is shadowed
            if(t < distance&& t>0.00001f){
                //set as shadowed
                is_shadowed = true;
                //DEBUG
                //printf("DEBUG: is shadowed\n");
                break; 
            }
        }
        
        if(is_shadowed){
            // this pixel is not effect by this light
            continue;
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

            float view_vector[3]=
                {-current_ray->direction[0],
                -current_ray->direction[1],
                -current_ray->direction[2]};

            float L_in[3] = {
                -L_vector[0],
                -L_vector[1],
                -L_vector[2]
            };

            float reflection[3] = {0,0,0};
            v3_reflect(reflection, L_in, normal);
            
            float normal_dot_ray = v3_dot_product(normal, L_vector);
            float view_dot_reflection = v3_dot_product(view_vector, reflection);
            
            float I_diff[3]={0,0,0};
            float I_spec[3]={0,0,0};

            if(normal_dot_ray > 0){
                // calculate diffusion for RGB
                for(int c = 0; c < 3; c++){
                    I_diff[c] += target_object->c_diff[c] *
                            current_light->color[c] *
                            normal_dot_ray;
                }
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

            I[0] += f_rad * f_ang * (I_spec[0] + I_diff[0]);
            I[1] += f_rad * f_ang * (I_spec[1] + I_diff[1]);
            I[2] += f_rad * f_ang * (I_spec[2] + I_diff[2]);
            for(int c = 0; c < 3; c++)
            {
                if(I[c] > 1.0f) I[c] = 1.0f;
                if(I[c] < 0.0f) I[c] = 0.0f;
            }
        }
    }

}

//function: Shade(recursive)
//input: current ray, scene information, light information, color, and level 
//output: final color 
void Shade( ray *current_ray,list<object*> *scene_information, 
    list<light*> *light_information, float* Color, int level){
    //base case: hits max cases 
    if(level>=MAXLEVEL){

        return; 
    }
    else{
        object *target_object = NULL;
        float lowest_t = INFINITY;
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
        if(target_object!=NULL){
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
            Shade(&reflection,scene_information,light_information,reflectContribution,level+1);
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

//function: find col 

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
            Shade(current_ray,scene_information,light_information,I,1);

            //find index
            int pixel_index=(h*image_info.width+w)*3; 
            for(int c = 0; c < 3; c++)
            {
                pixmap[pixel_index + c] = (uint8_t)(I[c] * 255);
            }
            
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

    return 0; 
}