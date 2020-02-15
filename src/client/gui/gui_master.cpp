#include "gui_master.h"

#include "../gl/primitive.h"
#include "gui_container.h"
#include <glm/gtc/matrix_transform.hpp>

GuiMaster::GuiMaster(float viewportWidth, float viewportHeight)
    : m_quadVao(makeQuadVertexArray(1.f, 1.f))
    , m_viewport(viewportWidth, viewportHeight)
{
    m_projection = glm::ortho(0.0f, viewportWidth, 0.0f, viewportHeight, -1.0f, 1.0f);

    m_shader.bind();
    m_shader.updateProjection(m_projection);

    // Todo, maybe change font in config?
    m_font.init("res/VeraMono-Bold.ttf", 512);
}

GuiContainer* GuiMaster::addGui()
{
    return m_containers.emplace_back(std::make_unique<GuiContainer>(m_font)).get();
}

void GuiMaster::render()
{
    m_shader.bind();

    glm::vec2 viewport = m_viewport / 100.0f;
    auto quad = m_quadVao.getDrawable();
    quad.bind();
    for (auto& container : m_containers) {
        if (!container->isHidden()) {
            container->renderRects(m_shader, viewport, quad, m_textures);
        }
    }

    m_font.bindTexture();
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);
    for (auto& container : m_containers) {
        if (!container->isHidden()) {
            container->renderText(m_shader, viewport);
        }
    }

    glDisable(GL_BLEND);
}

int GuiMaster::getTexture(const std::string& textureName)
{
    assert(m_textureIds.size() == m_textures.size());
    auto itr = m_textureIds.find(textureName);
    if (itr != m_textureIds.end()) {
        return itr->second;
    }
    else {
        int index = m_textures.size();
        gl::Texture2d& texture = m_textures.emplace_back();
        texture.create(textureName);
        m_textureIds.emplace(textureName, index);
        return index;
    }
}

int GuiMaster::guiCount() const
{
    return m_containers.size();
}

int GuiMaster::textureCount() const
{
    return m_containers.size();
}