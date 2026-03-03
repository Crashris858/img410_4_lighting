//main raytracer file

//libraries: 
#include "v3math.h"
#include <list>
#include <string.h>
#include <stdint.h>
#define PLANEDISTANCEZ -1
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
                    fscanf(scene ," %f %f %f ", &ns, &ns, &ns);
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
        


        //methods
        virtual float find_intersection(float *ro, float *rd){
            return 1; 
        }

        virtual void find_data( FILE* filename){
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
            }while(strcmp(buffer,";")!=0);

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
            }while(strcmp(buffer,";")!=0);

            //DEBUG:
            //printf("Plane: c_diff %f %f %f position: %f %f %f normal %f %f %f\n", 
            //  c_diff[0],c_diff[1],c_diff[2], position[0], position[1], position[2], normal[0], normal[1], normal[2]);
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

//main:
int main (int argc, char* argv[]){
    if (argc!=5){
        //print error message
        printf("Incorrect Number of Arguements");
        return 0; 
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
            
            object *target_object = NULL;
            float lowest_t = INFINITY;
            //func: findRay 
            find_current_Ray(w, h,pix_width, pix_height, *current_camera, image_info, current_ray);
            
            //find closest object 
            for(object* current_object : *scene_information){
        
                //DEBUG: check position
                //printf("current object position: %f %f %f \n", current_object->position[0], current_object->position[1], current_object->position[2]);
                float current_t = current_object->find_intersection(current_ray->origin, current_ray->direction);
                if(current_t > 0 && current_t < lowest_t)
                {
                    target_object = current_object;
                    lowest_t=current_t;
                    //DEBUG: chheck intersection
                    //printf("intersection found at (%d %d)\n", h, w);
                }
            }
            //ver2: loop through light sources to get object 
            for(light* current_light : *light_information){
                //pull light 
                
            }
            //set color in pixmap 
            int pixel_index=(h*image_info.width+w)*3; 
            
            //determine color 
            if(target_object==NULL){
                pixmap[pixel_index]=0;
                pixmap[pixel_index+1]=0;
                pixmap[pixel_index+2]=0;
            }
            else{
                pixmap[pixel_index]=target_object->c_diff[0]*255;
                pixmap[pixel_index+1]=target_object->c_diff[1]*255;
                pixmap[pixel_index+2]=target_object->c_diff[2]*255;
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
}