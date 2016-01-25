#include "PostprocessingStage.h"

#include <glbinding/gl/gl.h>

#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>

#include <globjects/Texture.h>
#include <globjects/Program.h>
#include <globjects/Framebuffer.h>

#include <gloperate/primitives/ScreenAlignedQuad.h>
#include <gloperate/painter/AbstractViewportCapability.h>
#include <gloperate/painter/AbstractPerspectiveProjectionCapability.h>
#include <gloperate/painter/AbstractCameraCapability.h>

using namespace gl;


namespace 
{
    std::vector<glm::vec3> ssaoKernel(unsigned int size)
    {
        static const auto minDistance = 0.1f;
        const auto inverseSize = 1.f / static_cast<float>(size);

        auto kernel = std::vector<glm::vec3>();

        if (size == 1)
        {
            kernel.push_back(glm::vec3(0.f));

            return kernel;
        }

        while (kernel.size() < size)
        {
            auto v = glm::sphericalRand(1.f);
            v.z = glm::abs(v.z);
            if (v.z < 0.1)
                continue;

            auto scale = static_cast<float>(kernel.size()) * inverseSize;
            scale = scale * scale * (1.f - minDistance) + minDistance;

            v *= scale;

            kernel.push_back(v);
        }

        return kernel;
    }

    std::vector<glm::vec3> ssaoNoise(const unsigned int size)
    {
        auto kernel = std::vector<glm::vec3>();

        for (auto y = 0u; y < size; ++y)
        {
            for (auto x = 0u; x < size; ++x)
            {
                auto c = glm::circularRand(1.f);
                auto v = glm::vec3(c.x, c.y, 0.0f);

                kernel.push_back(v);
            }
        }

        return kernel;
    }

    globjects::Texture* ssaoKernelTexture(unsigned int size)
    {
        auto texture = new globjects::Texture(gl::GL_TEXTURE_1D);
        texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);

        texture->image1D(0, gl::GL_RGBA32F, size, 0, gl::GL_RGB, gl::GL_FLOAT, ssaoKernel(size).data());

        return texture;
    }

    globjects::Texture* ssaoNoiseTexture(unsigned int size)
    {
        auto texture = new globjects::Texture(gl::GL_TEXTURE_2D);
        texture->setParameter(gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
        texture->setParameter(gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
        texture->setParameter(gl::GL_TEXTURE_WRAP_S, gl::GL_MIRRORED_REPEAT);
        texture->setParameter(gl::GL_TEXTURE_WRAP_T, gl::GL_MIRRORED_REPEAT);

        texture->image2D(0, gl::GL_RGBA32F, glm::ivec2(size), 0, gl::GL_RGB, gl::GL_FLOAT, ssaoNoise(size).data());

        return texture;
    }

    const unsigned int kernelSize = 16;
    const unsigned int noiseSize = 128;
}


PostprocessingStage::PostprocessingStage()
{
    addInput("viewport", viewport);
    addInput("color", color);
    addInput("normal", normal);
    addInput("depth", depth);

    addOutput("postprocessedFrame", postprocessedFrame);
}

void PostprocessingStage::initialize()
{
    postprocessedFrame.data() = globjects::Texture::createDefault(GL_TEXTURE_2D);
    
    m_fbo = new globjects::Framebuffer();
    m_fbo->attachTexture(GL_COLOR_ATTACHMENT0, postprocessedFrame.data());

    m_ssaoNoiseTexture = ssaoNoiseTexture(noiseSize);

    m_screenAlignedQuad = new gloperate::ScreenAlignedQuad(
        globjects::Shader::fromFile(GL_FRAGMENT_SHADER, "data/shaders/postprocessing.frag")
    );
}

void PostprocessingStage::process()
{
    const auto screenSize = glm::vec2(viewport.data()->width(), viewport.data()->height());

    if (viewport.hasChanged())
    {
        resizeTexture(viewport.data()->width(), viewport.data()->height());
    }

    m_ssaoKernelTexture = ssaoKernelTexture(kernelSize);

    m_fbo->bind();
    m_fbo->setDrawBuffer(GL_COLOR_ATTACHMENT0);

    color.data()->bindActive(0);
    normal.data()->bindActive(1);
    depth.data()->bindActive(2);
    m_ssaoKernelTexture->bindActive(3);
    m_ssaoNoiseTexture->bindActive(4);

    m_screenAlignedQuad->program()->setUniform("colorSampler", 0);
    m_screenAlignedQuad->program()->setUniform("normalSampler", 1);
    m_screenAlignedQuad->program()->setUniform("depthSampler", 2);
    m_screenAlignedQuad->program()->setUniform("ssaoKernelSampler", 3);
    m_screenAlignedQuad->program()->setUniform("ssaoNoiseSampler", 4);

    m_screenAlignedQuad->program()->setUniform("projectionMatrix", projection.data()->projection());
    m_screenAlignedQuad->program()->setUniform("projectionInverseMatrix", projection.data()->projectionInverted());
    m_screenAlignedQuad->program()->setUniform("normalMatrix", camera.data()->normal());
    m_screenAlignedQuad->program()->setUniform("farZ", projection.data()->zFar());
    m_screenAlignedQuad->program()->setUniform("screenSize", screenSize);
    m_screenAlignedQuad->program()->setUniform("samplerSizes", glm::vec4(kernelSize, 1.f / kernelSize, noiseSize, 1.f / noiseSize));

    m_screenAlignedQuad->draw();

    m_fbo->unbind();
}

void PostprocessingStage::resizeTexture(int width, int height)
{
    postprocessedFrame.data()->image2D(0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    m_fbo->printStatus(true);
}