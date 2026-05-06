# Illumination, Raytracing, Texture, and Cell Shading
Raycasts mathematical primitives based on a scene input file into a pixel buffer. In addition, it illuminates the scene through the addition of the new object type, light, in the scene input file. Then, it combines these color values and writes the pixel buffer to a PPM formatted file to render a basic scene with lighting. Finally, raytracing and reflections were implemented through recursion. Textures, if applicable and in ppm format, are applied to spheres through UV mapping. 

Allows for cell shaded items that are outlined in black.

## Authors
Carson Shah (email: cas2343@nau.edu)

Maya Harvey (email: mah992@nau.edu)

## Usage
Render a ppm file from a scene file using the syntax:
 
 "raycast width height input.scene output.ppm"

Or use our make file using the syntax: 

 "make" or "make run" 

## Cel-Shade Reference
We needed to understand what exactly cel-shading should include. Therefore, we looked at the Wikipedia article for cel-shading. Although we did not follow the recommended process, we were able to figure out key points, and further references in media, that we could follow.  


## Known Issues
Ray casting is currently working with given file format and multiple shapes. Illumination and reflection are working as shown in the assignment's example photo. Texture can be applied to spheres. Cell shaded items are rendered cleanly along with a black outline

Cel-shading outlines do not appear in the reflection. 
