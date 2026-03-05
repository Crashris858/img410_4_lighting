# Illumination
Raycasts mathematical primitives based on a scene input file into a pixel buffer. In addition, it illuminates the scene through the addition of the new object type, light, in the scene input file. Then, it combines these color values and writes the pixel buffer to a PPM formatted file.

## Authors
Carson Shah (email: cas2343@nau.edu)

Maya Harvey (email: mah992@nau.edu)

## Usage
Render a ppm file from a scene file Using syntax:
 
 "raycast width height input.scene output.ppm"

Or use or make file using syntax: 

 "make" and then "make run" 

## Known Issues
Ray casting is currently working with given file format and multiple shapes. Illumination is working as shown in assignment's example photo.