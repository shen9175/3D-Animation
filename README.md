# 3D-Animation
Bone Animation with Fix function lighting with shader approach

See the video demo at: https://youtu.be/QvLUrdLL5ko

Implement the common bone Animation algorithm with tank control by keyboard Arrow key with character. 
Camera is controlled by mouse and "w","a","s","d" The simple basic old school fix function lighting with PS shader approach. 
(Ambient, Diffuse, Specular) There are 3 Directional lights, 10 point lights(pillars) with yellow color, 
2 spotlights. One is fixed position in the center of the sky with green light, but its direction dynamically always points to the moving character. 
The other spotlight is located on the character's head. 
So the position is dynamic while moving, but its direction is always points the character's local forward direction. 
The color of this light is white. All the 15 lights can turn on/off independently. 
The fog effect can be turned off. The demo sometimes stutter because I use the hand toggle the lights change camera etc, 
and the animation will stop in the middle when I don't press arrow key seems like stutter, but it is not. 
The model is from Frank Luna's 3D programming book CD, I changed a bit of his lighting method about Point Light and Spot light.
His approach defines a range for the these two lights, which the light will has no effect when exceed the range, 
which seem unrealistic especially in Point Light. I let all the light including ambient decrease by the distance. 
Tuned att factor to be about just right.

See the video demo at: https://youtu.be/QvLUrdLL5ko
