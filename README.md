# OpenGL Scene Renderer

This project is an OpenGL-based scene renderer that demonstrates advanced rendering techniques including toon shading, normal mapping, HDR rendering, and bloom effects. It leverages GLSL shaders, framebuffer objects (FBOs), and interactive camera controls to create a dynamic 3D scene featuring a rotating car, a textured platform, and a skybox.

Development of this project was perfomed using Visual Studio 2022 on a pc using windows 10

---

## Table of Contents

- [Overview](#overview)
- [Main Code Components](#main-code-components)
  - [Scene Initialisation & Setup](#scene-initialization--setup)
  - [Rendering Pipeline](#rendering-pipeline)
  - [Shader Programs](#shader-programs)
  - [Camera Controls Class](#camera-controls-class)
- [Animations](#animations)
- [Controls](#controls)
- [Video Link](#video-link)
---

## Overview

This project renders a scene using a combination of advanced OpenGL techniques. The scene features:

- **Dynamic Lighting:** A spotlight that orbits the scene.
- **Toon Shading & Normal Mapping:** Toggleable effects for artistic rendering.
- **HDR & Bloom:** Post-processing effects for enhanced visual quality.
- **Interactive Camera:** Mouse and scroll-based camera controls.

The project is structured around a central scene class, shader programs for various rendering passes, and a camera control module to manage user input.

---

## Main Code Components

### Scene Initialization & Setup

The **`SceneBasic_Uniform`** class is the core of the scene management. It is responsible for:

- **Loading Models & Textures:** Loading a car model and multiple textures.
- **Compiling Shaders:** Compiling and linking vertex and fragment shaders.
- **Framebuffer Setup:** Configuring an HDR framebuffer and a secondary FBO for bloom effects.

---

### Rendering Pipeline

The rendering process is divided into several passes:

1. **Pass 1:** Render the scene to an HDR framebuffer.
2. **Pass 2:** Extract bright areas using a bright-pass filter.
3. **Pass 3 & 4:** Apply a two-pass blur on the bright areas.
4. **Pass 5:** Combine the blurred texture with the original scene and apply tone mapping.

Each pass is implemented in a dedicated function (e.g., `pass1()`, `pass2()`, ..., `pass5()`) to streamline the post-processing workflow.

---

### Shader Programs

The GLSL shader files implement various rendering effects within basic_uniform.vert and basic_uniform.frag. Key features include:

- **Textured and Non-Textured Options:** Through uniforms there is rendering options for both textured and non textured objects. Textured objects can have up to two combined textures.
- **Toon Shading Toggle:** Enabled via the `IsToonLighting` uniform.
- **Normal Mapping:** Utilizes a normal map texture with an optional toggle.
- **HDR Tone Mapping & Bloom:** Combines high dynamic range imaging with bloom post-processing.

---

### Camera Controls Class

The **`CamControls`** class manages camera manipulation and input handling. Key functionalities include:

- **Mouse Dragging:** Rotate the camera around the scene.
- **Scrolling:** Zoom in and out.

The camera uses spherical coordinates (radius, theta, phi) for smooth and intuitive navigation.

## Animations

### SpotLight Animation
- There is a spotlight animation present. It rotates around the middle point of the scene

### Car Rotation Animationl
- The car automatically spins in an opposite rotation to the light. This can be controlled. More details in controls section.

---

## Controls

### Rendering & Effect Toggles
- **`T` Key:** Toggle **Toon Shading**.
- **`N` Key:** Toggle **Normal Mapping**.

### Car Rotation Controls
- **`Space` Key:** Toggle automatic car rotation.
- **`Right Arrow` / `Left Arrow` Keys:** Manually rotate/speed up the car.

### Texture Selection (Colour Mapping)
The car’s texture can be changed by pressing the following number keys:
- **Key 1:** Use **orange** texture.
- **Key 2:** Use **black** texture.
- **Key 3:** Use **blue** texture.
- **Key 4:** Use **dark blue** texture.
- **Key 5:** Use **dark grey** texture.
- **Key 6:** Use **grey** texture.
- **Key 7:** Use **red** texture.

*Note: Holding **Left Shift** while selecting a texture will selectively bind only one texture layer. This allows user to make custom colours based on the combinations they use*

### Camera Controls
- **Left Mouse Button:** Engage camera rotation mode (cursor is hidden during dragging).
- **Mouse Drag:** Rotate the camera. Only works if holding left mouse button
- **Scroll Wheel:** Zoom in and out.

---

## Build & Run Instructions

- Clone and run the repository anywhere. Make sure the neccesary include and lib files are present at C:\Users\Public\OpenGL

---

## Video link
- 
