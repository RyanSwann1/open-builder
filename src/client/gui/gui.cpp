#include "gui.h"
#include "../gl/primitive.h"
#include "../gl/shader.h"
#include "../maths.h"

#include <iostream>

//
//  GUI Image
//

void GuiImage::setSource(const std::string &imageSource)
{
    if (m_image.textureExists()) {
        m_image.destroy();
    }
    m_image.create(imageSource);
}

void GuiImage::setSize(float width, float height)
{
    size = {width, height};
}

void GuiImage::setPosition(float x, float y)
{
    position = {x, y};
}

void GuiImage::setColour(float r, float g, float b)
{
    colour = {r, g, b};
}

Gui::Gui()
    : m_quad(makeQuadVertexArray(1.f, 1.f))
{
    // GUI Shader
    m_guiShader.program.create("gui", "gui");
    m_guiShader.program.bind();
    m_guiShader.modelLocation =
        m_guiShader.program.getUniformLocation("modelMatrix");
    m_guiShader.colorLocation =
        m_guiShader.program.getUniformLocation("color3");
}

void Gui::addUsertypes(sol::table &gui_api)
{
    auto udim2_type = gui_api.new_usertype<GDim>(
        "GDim", sol::constructors<GDim(), GDim(float, float, float, float)>());

    auto guiImage = gui_api.new_usertype<GuiImage>("Image");
    guiImage["setSource"] = &GuiImage::setSource;
    guiImage["setSize"] = &GuiImage::setSize;
    guiImage["setPosition"] = &GuiImage::setPosition;
    guiImage["setColor"] = &GuiImage::setColour;
}

void Gui::addImage(sol::userdata image)
{
    if (!image.is<GuiImage>()) {
        std::cout << "Invalid type\n";
        return;
    }
    m_images.push_back(image);
}

void Gui::render(int width, int height)
{
    m_guiShader.program.bind();
    auto d = m_quad.getDrawable();
    d.bind();

    for (auto &img : m_images) {
        auto &image = img.as<GuiImage>();
    }
}

/*
void Gui::render(int width, int height)
{
    float pixel_width = 2.f / width;
    float pixel_height = 2.f / height;

    m_guiShader.program.bind();
    auto d = m_quad.getDrawable();
    d.bind();

    for (auto &g_img : m_images) {
        auto &img = g_img.as<GuiImage>();
        glm::mat4 modelMatrix{1.0f};
        modelMatrix = glm::translate(
            modelMatrix, glm::vec3(img.m_position.scale.x * 2 - 1 +
                                       img.m_position.offset.x * pixel_width,
                                   1 - img.m_position.scale.y * -2 - 2 +
                                       img.m_position.offset.y * pixel_height,
                                   0));

        modelMatrix = glm::scale(
            modelMatrix,
            glm::vec3(
                img.m_size.scale.x * 2 + img.m_size.offset.x * pixel_width,
                img.m_size.scale.y * 2 + img.m_size.offset.y * pixel_height,
                1));

        gl::loadUniform(m_guiShader.modelLocation, modelMatrix);
        gl::loadUniform(m_guiShader.colorLocation, img.colour);
        img.m_image.bind();
        d.draw();
    }
}
*/
